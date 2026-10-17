#!/usr/bin/env python3
"""Run the reference inputs through a VCellStoch build and compare with the legacy results.

The references in ``legacy-v0.0.44-dev4-linux64/`` were produced by the ``VCellStoch_x64`` of
vcell-solvers v0.0.44-dev4 (the binary VCell's desktop client shipped until the split repos),
run on linux/amd64:

    VCellStoch_x64 gibson <case>.stochInput <case>.ida

Every case uses a fixed SEED and the same generator (``std::mt19937_64`` +
``std::uniform_real_distribution``), so a build on the same C++ standard library (libstdc++ on
Linux) reproduces the legacy output number for number. Other platforms draw different random
numbers from the same seed (libc++ and MSVC implement ``uniform_real_distribution``
differently), so there the comparison is statistical: the means (and standard deviations) must
agree with the legacy run, and with the closed-form answer where there is one, within a few
standard errors.

    compare.py <VCellStoch executable> [--work DIR] [--exact] [--case NAME ...] [--extra-args ...]

``--exact`` fails unless every output matches the legacy file to printed precision (use it for
Linux builds). Without it an exact match is reported, but only the statistics decide.
Needs numpy and h5py.
"""

from __future__ import annotations

import argparse
import math
import os
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

import h5py
import numpy as np

HERE = Path(__file__).resolve().parent
LEGACY = HERE / "legacy-v0.0.44-dev4-linux64"
Z = 5.0  # standard errors allowed before a statistical difference counts as a failure

# closed forms: birth-death  0 -> X at 10, X -> 0 at 0.5 X, X(0) = 0:  X(t) ~ Poisson(20 (1 - e^{-t/2}))
#               statstest    s0 + s2 -> s1 at k s0 s2, one of each:   P(s0 = 1 at t) = e^{-k t}
K_STATSTEST = 0.9996443474639611


def birthdeath_mean(t: np.ndarray) -> np.ndarray:
    return 20.0 * (1.0 - np.exp(-0.5 * t))


CASES = ["birthdeath_single", "birthdeath_keepevery", "birthdeath_histogram", "birthdeath_stats", "statstest"]


def read_table(path: Path) -> tuple[str, np.ndarray]:
    """A VCellStoch text output: a header line, then rows of numbers."""
    lines = path.read_text().splitlines()
    rows = [[float(x) for x in line.split()] for line in lines[1:] if line.strip()]
    return lines[0].strip(), np.array(rows, dtype=float)


def read_stats(path: Path) -> dict[str, np.ndarray]:
    with h5py.File(path, "r") as f:
        out = {k: np.array(f[k]) for k in ("SimTimes", "StatMean", "StatMin", "StatMax", "StatStdDev")}
        out["VarNames"] = np.array([v.decode() if isinstance(v, bytes) else str(v) for v in f["VarNames"][()]])
    return out


def num_trials(case: str) -> int:
    for line in (HERE / f"{case}.stochInput").read_text().splitlines():
        parts = line.split()
        if parts and parts[0] == "NUM_TRIAL":
            return int(float(parts[1]))
    raise ValueError(f"{case}: no NUM_TRIAL")


class Report:
    def __init__(self) -> None:
        self.rows: list[tuple[str, str, str]] = []
        self.failed = False

    def check(self, case: str, what: str, ok: bool, detail: str) -> None:
        self.rows.append((case, what, ("PASS " if ok else "FAIL ") + detail))
        self.failed |= not ok

    def note(self, case: str, what: str, detail: str) -> None:
        self.rows.append((case, what, detail))

    def markdown(self) -> str:
        out = ["| case | check | result |", "|---|---|---|"]
        out += [f"| {c} | {w} | {r} |" for c, w, r in self.rows]
        return "\n".join(out)


def mean_z(m1: np.ndarray, s1: np.ndarray, m2: np.ndarray, s2: np.ndarray, n1: int, n2: int) -> float:
    """Largest two-sample z-score between two arrays of means."""
    se = np.sqrt(s1**2 / n1 + s2**2 / n2)
    diff = np.abs(m1 - m2)
    z = np.where(se > 0, diff / np.where(se > 0, se, 1.0), np.where(diff > 1e-12, np.inf, 0.0))
    return float(np.max(z))


def std_ok(s1: np.ndarray, s2: np.ndarray, n: int) -> tuple[bool, float]:
    """Standard deviations agree: ~Z standard errors plus 2 %, where enough events make it meaningful."""
    meaningful = (s1**2 * n > 25) & (s2**2 * n > 25)
    tol = Z * np.sqrt((s1**2 + s2**2) / (2 * n)) + 0.02 * np.maximum(s1, s2) + 1e-12
    diff = np.abs(s1 - s2)
    worst = float(np.max(np.where(meaningful, diff / tol, 0.0)))
    return worst <= 1.0, worst


def exact_table(new: np.ndarray, old: np.ndarray) -> bool:
    return new.shape == old.shape and bool(np.allclose(new, old, rtol=1e-9, atol=1e-12))


def compare_case(case: str, exe: list[str], work: Path, exe_dir: str, extra: list[str], exact: bool,
                 rep: Report) -> None:
    out = work / f"{case}.ida"
    for stale in (out, Path(f"{out}_hdf5")):
        stale.unlink(missing_ok=True)
    shutil.copy(HERE / f"{case}.stochInput", work / f"{case}.stochInput")
    cmd = [*exe, "gibson", f"{exe_dir}/{case}.stochInput", f"{exe_dir}/{case}.ida", *extra]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    (work / f"{case}.stdout").write_text(proc.stdout + proc.stderr)
    rep.check(case, "exit code", proc.returncode == 0, f"{proc.returncode}")
    if proc.returncode != 0:
        print(proc.stdout, proc.stderr, file=sys.stderr)
        return

    head_new, new = read_table(out)
    head_old, old = read_table(LEGACY / f"{case}.ida")
    rep.check(case, "header", head_new == head_old, repr(head_new))
    same = exact_table(new, old)
    if exact:
        rep.check(case, "text output = legacy", same, f"{new.shape} rows x cols")
    else:
        rep.note(case, "text output = legacy", "identical" if same else "differs (different RNG stream on this platform)")

    if case == "birthdeath_single":
        rep.check(case, "output times = legacy", np.array_equal(new[:, 0], old[:, 0]), f"{len(new)} samples")
    elif case == "birthdeath_keepevery":
        t = new[:, 0]
        rep.check(case, "event times increasing", bool(np.all(np.diff(t) > 0)) and t[-1] <= 5.0, f"{len(t)} events")
    elif case == "birthdeath_histogram":
        n = num_trials(case)
        x_new, x_old = new[:, 1], old[:, 1]
        rep.check(case, "trials", len(x_new) == n, f"{len(x_new)}")
        z = mean_z(np.mean(x_new), np.std(x_new, ddof=1), np.mean(x_old), np.std(x_old, ddof=1), n, n)
        rep.check(case, "final mean vs legacy", z <= Z, f"{np.mean(x_new):.3f} vs {np.mean(x_old):.3f} (z={z:.2f})")
        theory = float(birthdeath_mean(np.array(20.0)))
        zt = abs(np.mean(x_new) - theory) / math.sqrt(theory / n)
        rep.check(case, "final mean vs Poisson", zt <= Z, f"{np.mean(x_new):.3f} vs {theory:.3f} (z={zt:.2f})")
        vr = np.var(x_new, ddof=1) / theory
        rep.check(case, "variance/mean (Poisson = 1)", abs(vr - 1) < 0.25, f"{vr:.3f}")
    elif case in ("birthdeath_stats", "statstest"):
        n = num_trials(case)
        s_new, s_old = read_stats(Path(f"{out}_hdf5")), read_stats(LEGACY / f"{case}.ida_hdf5")
        rep.check(case, "_hdf5 layout", list(s_new["VarNames"]) == list(s_old["VarNames"])
                  and s_new["StatMean"].shape == s_old["StatMean"].shape
                  and np.allclose(s_new["SimTimes"], s_old["SimTimes"]),
                  f"vars {list(s_new['VarNames'])}, means {s_new['StatMean'].shape}")
        rep.check(case, "text means = _hdf5 StatMean", np.allclose(new[:, 1:], s_new["StatMean"], rtol=1e-9, atol=1e-12), "")
        same_h5 = all(np.allclose(s_new[k], s_old[k], rtol=1e-12, atol=1e-12)
                      for k in ("StatMean", "StatStdDev", "StatMin", "StatMax"))
        if exact:
            rep.check(case, "_hdf5 statistics = legacy", same_h5, "")
        else:
            rep.note(case, "_hdf5 statistics = legacy", "identical" if same_h5 else "differs (different RNG stream)")
        m1, sd1, m2, sd2 = s_new["StatMean"], s_new["StatStdDev"], s_old["StatMean"], s_old["StatStdDev"]
        z = mean_z(m1, sd1, m2, sd2, n, n)
        rep.check(case, "StatMean vs legacy", z <= Z, f"max z={z:.2f} over {m1.size} points, {n} trials")
        ok, worst = std_ok(sd1, sd2, n)
        rep.check(case, "StatStdDev vs legacy", ok, f"worst {worst:.2f} of tolerance")
        t = s_new["SimTimes"]
        if case == "birthdeath_stats":
            mu = birthdeath_mean(t)
            zt = np.max(np.abs(m1[:, 0] - mu) / np.sqrt(np.maximum(mu, 1e-300) / n))
            rep.check(case, "StatMean vs Poisson mean", zt <= Z, f"max z={zt:.2f}")
        else:
            names = list(s_new["VarNames"])
            p = np.exp(-K_STATSTEST * t)
            se = np.sqrt(p * (1 - p) / n)
            ms0 = m1[:, names.index("s0_Count")]
            zt = np.max(np.where(se > 0, np.abs(ms0 - p) / np.where(se > 0, se, 1), 0))
            rep.check(case, "s0 mean vs exp(-kt)", zt <= Z, f"max z={zt:.2f}")
        rep.check(case, "StatMin <= StatMean <= StatMax",
                  bool(np.all(s_new["StatMin"] <= m1 + 1e-12) and np.all(m1 <= s_new["StatMax"] + 1e-12)), "")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("exe", help="VCellStoch executable, or a command prefix such as 'apptainer run x.sif VCellStoch_x64'")
    ap.add_argument("--work", type=Path, default=Path("reference-run"), help="where inputs and outputs go")
    ap.add_argument("--exe-dir", help="the work directory as the executable sees it (e.g. /simdata in a container)")
    ap.add_argument("--exact", action="store_true", help="require number-for-number agreement (same C++ library)")
    ap.add_argument("--case", action="append", choices=CASES, help="run only these cases")
    ap.add_argument("--extra-args", nargs=argparse.REMAINDER, default=[], help="appended to argv, e.g. -tid 0")
    args = ap.parse_args()

    args.work.mkdir(parents=True, exist_ok=True)
    rep = Report()
    exe = shlex.split(args.exe)
    exe_dir = args.exe_dir or str(args.work.resolve())
    for case in args.case or CASES:
        compare_case(case, exe, args.work, exe_dir, args.extra_args, args.exact, rep)
    md = rep.markdown()
    print(md)
    summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary:
        with open(summary, "a") as f:
            f.write(f"### Gibson reference comparison ({'exact' if args.exact else 'statistical'})\n\n{md}\n\n")
    print("\nFAILED" if rep.failed else "\nall checks passed")
    return 1 if rep.failed else 0


if __name__ == "__main__":
    sys.exit(main())
