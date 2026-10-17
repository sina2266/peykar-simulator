#!/usr/bin/env python3
"""Release checks: run the packaged solvers the way VCell runs them and check the answers.

ctest (tests/CMakeLists.txt) checks the binaries a build tree produced. This checks what
ships -- the unpacked release archive, the Docker image or the Apptainer SIF -- with
nothing from the build tree. It needs only Python 3, numpy and h5py on the host; the
solver may be a local executable or the tail of a container command line:

    # an unpacked archive
    check_release.py --bin-dir dist/linux64 --work /tmp/w

    # the SIF, the way SlurmProxy runs it: a bare executable name, the working
    # directory bound at /simdata, a trailing -tid, status to a (fake) broker
    check_release.py --work /tmp/w --mount /simdata --messaging \\
        --runner "apptainer run --containall --bind /tmp/w:/simdata vcell-chombo.sif"

Cases, each in 2D and 3D:

  usage        no input file: the solver prints its usage and exits non-zero
  smoke        uniform field in a disc/sphere; the .log, .mesh.hdf5 and .hdf5.zip
               VCell reads are written. Run from a directory whose absolute path is
               longer than 256 characters, which overran the old 128-byte buffers.
  regression   Gaussian bump; the final timepoint matches the committed baseline
               (tests/resources/regress<N>d.baseline.txt) within rtol/atol
  analytic     sin-product eigenmode; the solver's own error against the closed form
               is the one the scheme predicts (tests/README.md)
  reference    two species exchanging across the embedded-boundary membrane; the
               summed total is conserved while mass actually crosses

With --messaging every input gets a JMS_PARAM block pointing at a small HTTP server
started here, the solver gets `-tid 0`, and each run must report starting, progress
and completed -- and no failure -- to it.

Exit status 0 when everything passes; the summary table goes to stdout.
"""

from __future__ import annotations

import argparse
import http.server
import os
import re
import shlex
import shutil
import subprocess
import sys
import threading
import time
import urllib.parse
import zipfile
from dataclasses import dataclass, field
from pathlib import Path

import h5py
import numpy as np

RESOURCES = Path(__file__).resolve().parent.parent / "resources"

# VCell marks cells outside the solved region with this value; it must match exactly.
OUTSIDE_DOMAIN = 1.23456789e300

# Predicted relative L2 error against the closed form, and the accepted band -- the same
# figures tests/CMakeLists.txt uses; tests/README.md derives them.
ANALYTIC_EXPECTED = {2: 3.528e-04, 3: 1.386e-03}
ANALYTIC_BAND = 0.15

# JobEvent status codes (vcell-messaging/include/VCELL/JobEventStatus.h).
JOB_STARTING, JOB_DATA, JOB_PROGRESS, JOB_FAILURE, JOB_COMPLETED = 999, 1000, 1001, 1002, 1003

# Conservation in the reference model: total(U) + total(V) may drift by at most this
# relative amount over the run (the linear solves run to 1e-9), and at least this
# fraction of the mass must have crossed the membrane, or the check proves nothing.
CONSERVATION_RTOL = 1e-6
MIN_EXCHANGED = 0.05


# --------------------------------------------------------------------------------------
# A stand-in for the broker's REST bridge
# --------------------------------------------------------------------------------------
class FakeBroker:
    """Records the worker events vcell-messaging POSTs to /api/message/workerEvent."""

    def __init__(self) -> None:
        self.events: list[dict[str, str]] = []
        broker = self

        class Handler(http.server.BaseHTTPRequestHandler):
            def do_POST(self) -> None:  # noqa: N802 (http.server API)
                length = int(self.headers.get("Content-Length") or 0)
                if length:
                    self.rfile.read(length)
                query = urllib.parse.urlparse(self.path).query
                broker.events.append({k: v[-1] for k, v in urllib.parse.parse_qs(query).items()})
                self.send_response(200)
                self.send_header("Content-Length", "0")
                self.end_headers()

            def log_message(self, *args: object) -> None:
                pass

        self.server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler)
        self.port = self.server.server_address[1]
        threading.Thread(target=self.server.serve_forever, daemon=True).start()

    def for_sim(self, sim_key: int) -> list[dict[str, str]]:
        return [e for e in self.events if e.get("SimKey") == str(sim_key)]


# --------------------------------------------------------------------------------------
# Running one case
# --------------------------------------------------------------------------------------
@dataclass
class Case:
    name: str
    dim: int
    kind: str  # usage | smoke | regression | analytic | reference
    input: str | None
    timepoints: int = 0
    subdir: str = ""


@dataclass
class Result:
    case: Case
    ok: bool = False
    detail: str = ""
    seconds: float = 0.0
    notes: list[str] = field(default_factory=list)


def cases() -> list[Case]:
    out = []
    # A directory name that pushes the absolute base path past 256 characters.
    deep = "a-directory-name-long-enough-to-overrun-a-128-byte-path-buffer-" * 2
    for d in (2, 3):
        out += [
            Case(f"usage{d}d", d, "usage", None),
            Case(f"smoke{d}d", d, "smoke", f"smoke{d}d.fvinput", 3, subdir=f"{deep}/{deep}"),
            Case(f"regress{d}d", d, "regression", f"regress{d}d.fvinput", 5),
            Case(f"analytic{d}d", d, "analytic", f"analytic{d}d.fvinput", 3),
            Case(f"reference{d}d", d, "reference", f"reference{d}d.fvinput", 5),
        ]
    return out


def jms_block(port: int, sim_key: int) -> str:
    return (
        "JMS_PARAM_BEGIN\n"
        f"JMS_BROKER 127.0.0.1:{port}\n"
        "JMS_USER serverUser unused\n"
        "JMS_QUEUE workerEvent\n"
        "JMS_TOPIC serviceControl\n"
        "VCELL_USER release-check\n"
        f"SIMULATION_KEY {sim_key}\n"
        "JOB_INDEX 0\n"
        "JMS_PARAM_END\n\n"
    )


def base_name(text: str) -> str:
    m = re.search(r"^BASE_FILE_NAME\s+(\S+)", text, re.M)
    if not m:
        raise ValueError("input has no BASE_FILE_NAME")
    return m.group(1)


def run_case(case: Case, args: argparse.Namespace, broker: FakeBroker | None, sim_key: int) -> Result:
    res = Result(case)
    exe = f"VCellChombo{case.dim}D_x64"
    exe_cmd = str(Path(args.bin_dir) / exe) if args.bin_dir else exe
    runner = shlex.split(args.runner) if args.runner else []

    if case.kind == "usage":
        proc = subprocess.run(runner + [exe_cmd], capture_output=True, text=True, timeout=120)
        out = proc.stdout + proc.stderr
        res.ok = proc.returncode != 0 and "fvInputFile" in out
        res.detail = f"exit {proc.returncode}, usage {'printed' if 'fvInputFile' in out else 'MISSING'}"
        m = re.search(r"version (\S+)", out)
        if m:
            res.notes.append(f"reports version {m.group(1)}")
        return res

    host_dir = Path(args.work) / case.name / case.subdir
    shutil.rmtree(Path(args.work) / case.name, ignore_errors=True)
    host_dir.mkdir(parents=True)
    solver_dir = Path(args.mount or args.work) / case.name / case.subdir

    text = (RESOURCES / case.input).read_text()
    base = base_name(text)
    text = re.sub(r"^BASE_FILE_NAME\s+\S+", f"BASE_FILE_NAME {solver_dir / base}", text, count=1, flags=re.M)
    argv = runner + [exe_cmd, str(solver_dir / case.input)]
    if broker:
        text = jms_block(broker.port, sim_key) + text
        argv += ["-tid", "0"]
    (host_dir / case.input).write_text(text)

    t0 = time.monotonic()
    proc = subprocess.run(argv, capture_output=True, text=True, timeout=args.timeout)
    res.seconds = time.monotonic() - t0
    output = proc.stdout + proc.stderr
    (host_dir / "solver.out").write_text(output)
    if proc.returncode != 0 or "Exception :" in output:
        res.detail = f"solver exit {proc.returncode}: " + output.strip().splitlines()[-1][:300] if output.strip() else f"solver exit {proc.returncode}"
        return res

    try:
        final = check_outputs(host_dir, base, case.timepoints)
        if case.kind == "smoke":
            res.detail = f"{case.timepoints} timepoints; base path {len(str(solver_dir / base))} chars"
        elif case.kind == "regression":
            res.detail = compare_baseline(final, RESOURCES / f"regress{case.dim}d.baseline.txt", args.rtol, args.atol)
        elif case.kind == "analytic":
            res.detail = check_analytic(final, case.dim)
        elif case.kind == "reference":
            res.detail = check_conservation(host_dir / f"{base}.hdf5")
        if broker:
            res.notes.append(check_messages(broker.for_sim(sim_key)))
        res.ok = True
    except CheckFailed as e:
        res.detail = str(e)
    return res


class CheckFailed(Exception):
    pass


def check_outputs(work: Path, base: str, expected: int) -> Path:
    """The files VCell reads: the mesh, a .log row per saved time, and the zipped .sim.hdf5."""
    if not (work / f"{base}.mesh.hdf5").is_file():
        raise CheckFailed(f"no {base}.mesh.hdf5")
    rows = [ln.split() for ln in (work / f"{base}.log").read_text().splitlines() if ln.strip()]
    if len(rows) != expected:
        raise CheckFailed(f"{len(rows)} timepoints in {base}.log, expected {expected}")
    extracted = work / "extracted"
    for _, sim_file, zip_file, _ in rows:
        with zipfile.ZipFile(work / zip_file) as z:
            if sim_file not in z.namelist():
                raise CheckFailed(f"{sim_file} missing from {zip_file}")
            z.extract(sim_file, extracted)
    return extracted / rows[-1][1]


def float_datasets(path: Path) -> dict[str, np.ndarray]:
    out: dict[str, np.ndarray] = {}

    def visit(name: str, obj: object) -> None:
        if isinstance(obj, h5py.Dataset) and obj.dtype.kind == "f":
            out[name] = np.asarray(obj[()], dtype=float).ravel()

    with h5py.File(path, "r") as f:
        f.visititems(visit)
    return out


def load_baseline(path: Path) -> dict[str, np.ndarray]:
    fields: dict[str, list[float]] = {}
    current = None
    for line in path.read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        if line.startswith("@"):
            current = line[1:].split()[0]
            fields[current] = []
        else:
            fields[current].append(float(line))  # type: ignore[index]
    return {k: np.array(v) for k, v in fields.items()}


def compare_baseline(solution: Path, baseline: Path, rtol: float, atol: float) -> str:
    """The same mixed test as tests/compare_solution.cpp: |got - want| <= atol + rtol |want|."""
    got, want = float_datasets(solution), load_baseline(baseline)
    problems, worst = [], 0.0
    for name, w in want.items():
        g = got.get(name)
        if g is None or g.shape != w.shape:
            problems.append(f"{name}: {'missing' if g is None else f'size {g.size} != {w.size}'}")
            continue
        sentinel = (w == OUTSIDE_DOMAIN) | (g == OUTSIDE_DOMAIN)
        if np.any(sentinel & (g != w)):
            problems.append(f"{name}: covered cells differ")
        inside = ~sentinel
        diff = np.abs(g[inside] - w[inside])
        if np.any(diff > atol + rtol * np.abs(w[inside])):
            problems.append(f"{name}: {int(np.sum(diff > atol + rtol * np.abs(w[inside])))} values out of tolerance")
        if inside.any():
            rel = diff / np.where(np.abs(w[inside]) > 0, np.abs(w[inside]), 1.0)
            worst = max(worst, float(rel.max()))
    extra = sorted(set(got) - set(want))
    if extra:
        problems.append(f"unexpected datasets {extra}")
    if problems:
        raise CheckFailed("; ".join(problems) + f" (worst relative difference {worst:.3g})")
    return f"{len(want)} datasets match (rtol {rtol:g}); worst relative difference {worst:.2e}"


def check_analytic(solution: Path, dim: int) -> str:
    with h5py.File(solution, "r") as f:
        attrs = f["solution/U"].attrs
        if "relative L2 error" not in attrs:
            raise CheckFailed("no 'relative L2 error' attribute on solution/U")
        measured = float(np.asarray(attrs["relative L2 error"]).ravel()[0])
    expected = ANALYTIC_EXPECTED[dim]
    lo, hi = expected * (1 - ANALYTIC_BAND), expected * (1 + ANALYTIC_BAND)
    if not lo <= measured <= hi:
        raise CheckFailed(f"relative L2 error {measured:.4e} outside [{lo:.4e}, {hi:.4e}]")
    return f"relative L2 error {measured:.4e} (predicted {expected:.3e}, ratio {measured / expected:.3f})"


def check_conservation(pp_file: Path) -> str:
    """total(U) + total(V) over time, from the solver's variable statistics."""
    if not pp_file.is_file():
        raise CheckFailed(f"no post-processing file {pp_file.name}")
    with h5py.File(pp_file, "r") as f:
        group = f["PostProcessing/VariableStatistics"]
        names = {}
        for key, value in group.attrs.items():
            m = re.fullmatch(r"comp_(\d+)_name", key)
            if m:
                names[value.decode() if isinstance(value, bytes) else str(value)] = int(m.group(1))
        times = np.asarray(f["PostProcessing/Times"][()])
        stats = np.array([group[k][()] for k in sorted(k for k in group if k.startswith("time"))])
    if "U_total" not in names or "V_total" not in names:
        raise CheckFailed(f"no U_total/V_total among {sorted(names)}")
    u, v = stats[:, names["U_total"]], stats[:, names["V_total"]]
    total = u + v
    drift = float(np.max(np.abs(total - total[0])) / abs(total[0]))
    moved = float(v[-1] / total[0])
    detail = (f"{len(times)} times to t={times[-1]:g}: total {total[0]:.10g} -> {total[-1]:.10g}, "
              f"max drift {drift:.2e}; {100 * moved:.1f}% crossed the membrane")
    if drift > CONSERVATION_RTOL:
        raise CheckFailed("mass not conserved -- " + detail)
    if moved < MIN_EXCHANGED:
        raise CheckFailed("too little exchange to be a test -- " + detail)
    return detail


def check_messages(events: list[dict[str, str]]) -> str:
    statuses = [int(e.get("WorkerEvent_Status", -1)) for e in events]
    for needed, label in ((JOB_STARTING, "starting"), (JOB_PROGRESS, "progress"), (JOB_COMPLETED, "completed")):
        if needed not in statuses:
            raise CheckFailed(f"broker never saw {label}; statuses {statuses}")
    if JOB_FAILURE in statuses:
        raise CheckFailed(f"solver reported failure: {events[statuses.index(JOB_FAILURE)]}")
    if statuses[-1] != JOB_COMPLETED:
        raise CheckFailed(f"last status {statuses[-1]}, expected completed; statuses {statuses}")
    return f"broker: {len(events)} events, last=completed"


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--bin-dir", help="directory holding VCellChombo{2,3}D_x64 (an unpacked archive)")
    p.add_argument("--runner", help="command prefix that runs a bare executable name (a container)")
    p.add_argument("--work", required=True, help="scratch directory on this host")
    p.add_argument("--mount", help="where the solver sees --work (default: the same path)")
    p.add_argument("--messaging", action="store_true", help="run with -tid 0 against a fake broker")
    p.add_argument("--only", help="regex selecting case names")
    p.add_argument("--rtol", type=float, default=1e-9)
    p.add_argument("--atol", type=float, default=1e-12)
    p.add_argument("--timeout", type=float, default=900)
    args = p.parse_args()
    if bool(args.bin_dir) == bool(args.runner):
        p.error("give exactly one of --bin-dir and --runner")

    Path(args.work).mkdir(parents=True, exist_ok=True)
    broker = FakeBroker() if args.messaging else None
    results = []
    for i, case in enumerate(cases()):
        if args.only and not re.search(args.only, case.name):
            continue
        r = run_case(case, args, broker, sim_key=1000 + i)
        results.append(r)
        mark = "PASS" if r.ok else "FAIL"
        extra = f"  [{'; '.join(r.notes)}]" if r.notes else ""
        print(f"{mark}  {case.name:<12} {r.seconds:6.1f}s  {r.detail}{extra}", flush=True)
    failed = [r for r in results if not r.ok]
    print(f"\n{len(results) - len(failed)}/{len(results)} passed")
    for r in failed:
        out = Path(args.work) / r.case.name / r.case.subdir / "solver.out"
        if out.is_file():
            print(f"\n--- tail of {r.case.name} solver output ---")
            print("\n".join(out.read_text().splitlines()[-40:]))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
