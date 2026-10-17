#!/usr/bin/env python3
"""Read VCell solver output (.sim files inside the *NN.zip archives) and compare two runs.

A .sim file is VCell's big-endian SimData format (VCell/src/DataSet.cpp): a 44-byte header
(magic[16], version[8], numBlocks, firstBlockOffset, sizeX, sizeY, sizeZ as int32), then
numBlocks block descriptors (varName[124], varType, size, dataOffset as int32), then the
doubles each descriptor points at.

    simdata.py summary <output-dir> <base-name>             print a JSON summary
    simdata.py compare <output-dir> <base-name> <reference>  compare against a reference
        reference = another output directory (full, element-wise comparison) or a summary
        JSON written by `summary`: with "mode": "deterministic", every statistic (sum, sum of
        squares, min, max per variable and time point) is compared; otherwise (stochastic
        solvers) the per-variable totals are compared by their time means

Only the standard library and NumPy are needed.
"""
from __future__ import annotations

import json
import re
import sys
import zipfile
from pathlib import Path

import numpy as np

HEADER = 44
NAME = 124
BLOCK = NAME + 12


def parse_sim(buf: bytes) -> dict[str, np.ndarray]:
    magic = buf[:16].split(b"\0")[0].decode()
    if not magic.startswith("VCell"):
        raise ValueError(f"not a VCell .sim file (magic {magic!r})")
    nblocks, _first, *_size = np.frombuffer(buf[24:HEADER], dtype=">i4")
    out = {}
    for i in range(int(nblocks)):
        off = HEADER + i * BLOCK
        name = buf[off:off + NAME].split(b"\0")[0].decode()
        _vtype, size, data_off = np.frombuffer(buf[off + NAME:off + BLOCK], dtype=">i4")
        out[name] = np.frombuffer(buf, dtype=">f8", count=int(size), offset=int(data_off)).astype(float)
    return out


def read_run(outdir: Path, base: str) -> list[dict[str, np.ndarray]]:
    """All time points of a run, in order, from <base>NN.zip (or loose <base>NNNN.sim files)."""
    sims: dict[str, bytes] = {}
    for z in sorted(outdir.glob(f"{base}[0-9][0-9].zip")):
        with zipfile.ZipFile(z) as zf:
            for n in zf.namelist():
                if n.endswith(".sim"):
                    sims[n] = zf.read(n)
    if not sims:
        for p in outdir.glob(f"{base}[0-9][0-9][0-9][0-9].sim"):
            sims[p.name] = p.read_bytes()
    if not sims:
        raise SystemExit(f"no .sim output for {base} in {outdir}")
    key = lambda n: int(re.search(r"(\d+)\.sim$", n).group(1))  # noqa: E731
    return [parse_sim(sims[n]) for n in sorted(sims, key=key)]


def summary(run: list[dict[str, np.ndarray]]) -> dict:
    """Per variable and time point: sum, sum of squares, min and max."""
    names = sorted(run[0])
    stats = {"sum": np.sum, "sumsq": lambda x: np.sum(x * x), "min": np.min, "max": np.max}
    return {
        "timepoints": len(run),
        "variables": {
            v: {"size": int(run[0][v].size), **{k: [float(f(t[v])) for t in run] for k, f in stats.items()}}
            for v in names
        },
    }


def compare_stats(s: dict, ref: dict, rtol: float) -> list[str]:
    """Deterministic comparison against a summary: every statistic at every time point within
    rtol of the reference, relative to that statistic's largest magnitude over time."""
    errs = []
    if s["timepoints"] != ref["timepoints"]:
        return [f"timepoints differ: {s['timepoints']} vs {ref['timepoints']}"]
    worst = 0.0
    for v, r in ref["variables"].items():
        if v not in s["variables"]:
            errs.append(f"missing variable {v}")
            continue
        for k in ("sum", "sumsq", "min", "max"):
            x, y = np.array(s["variables"][v][k]), np.array(r[k])
            scale = max(float(np.max(np.abs(y))), 1e-300)
            rel = float(np.max(np.abs(x - y))) / scale
            worst = max(worst, rel)
            if rel > rtol:
                errs.append(f"{v} {k}: relative diff {rel:.3e}")
    print(f"statistics comparison: {len(ref['variables'])} variables x {ref['timepoints']} time points, "
          f"max relative diff {worst:.3e} (tolerance {rtol})")
    return errs


def compare_full(a: list, b: list, rtol: float, atol: float) -> list[str]:
    errs = []
    if len(a) != len(b):
        return [f"timepoints differ: {len(a)} vs {len(b)}"]
    worst = 0.0
    for i, (ta, tb) in enumerate(zip(a, b)):
        if set(ta) != set(tb):
            errs.append(f"t[{i}] variables differ: {sorted(set(ta) ^ set(tb))}")
            continue
        for v in ta:
            x, y = ta[v], tb[v]
            if x.shape != y.shape:
                errs.append(f"t[{i}] {v}: shape {x.shape} vs {y.shape}")
            else:
                scale = max(float(np.max(np.abs(y))), 1e-300)
                worst = max(worst, float(np.max(np.abs(x - y))) / scale)
                if not np.allclose(x, y, rtol=rtol, atol=atol):
                    errs.append(f"t[{i}] {v}: max |diff| {np.max(np.abs(x - y)):.3e}")
    print(f"full comparison: {len(a)} time points, max relative diff {worst:.3e}")
    return errs


def compare_summary(s: dict, ref: dict, rtol: float) -> list[str]:
    """Stochastic comparison: same time points, and each variable's time-averaged total within
    rtol (relative) of the reference's. Also reports whether the trajectories are identical."""
    errs = []
    if s["timepoints"] != ref["timepoints"]:
        errs.append(f"timepoints differ: {s['timepoints']} vs {ref['timepoints']}")
    for v, r in ref["variables"].items():
        if v not in s["variables"]:
            errs.append(f"missing variable {v}")
            continue
        x, y = np.array(s["variables"][v]["sum"]), np.array(r["sum"])
        if x.shape != y.shape:
            errs.append(f"{v}: {x.size} vs {y.size} time points")
            continue
        mx, my = float(x.mean()), float(y.mean())
        rel = abs(mx - my) / max(abs(my), 1e-300)
        same = "identical trajectory" if np.array_equal(x, y) else f"max per-time diff {np.max(np.abs(x - y)):g}"
        print(f"{v}: time-mean total {mx:.6g} vs reference {my:.6g} (relative diff {rel:.3e}, tolerance {rtol}); {same}")
        if rel > rtol:
            errs.append(f"{v}: time-mean total differs by {rel:.3e}")
    # When the reference conserves the sum over all variables (e.g. molecules moving between
    # states), the run must conserve the same total at every time point.
    names = [v for v in ref["variables"] if v in s["variables"]]
    if names and not errs:
        ref_total = np.sum([ref["variables"][v]["sum"] for v in names], axis=0)
        if np.allclose(ref_total, ref_total[0]):
            total = np.sum([s["variables"][v]["sum"] for v in names], axis=0)
            ok = np.allclose(total, ref_total[0])
            print(f"conserved total {ref_total[0]:g}: {'held at every time point' if ok else 'VIOLATED'}")
            if not ok:
                errs.append(f"total over {names} not conserved: {total.min():g}..{total.max():g}")
    return errs


def main(argv: list[str]) -> int:
    if len(argv) >= 3 and argv[0] == "summary":
        print(json.dumps(summary(read_run(Path(argv[1]), argv[2])), indent=1))
        return 0
    if len(argv) >= 4 and argv[0] == "compare":
        rtol = float(argv[4]) if len(argv) > 4 else 1e-6
        run = read_run(Path(argv[1]), argv[2])
        ref = Path(argv[3])
        if ref.is_dir():
            errs = compare_full(run, read_run(ref, argv[2]), rtol=rtol, atol=rtol * 1e-3)
        else:
            r = json.loads(ref.read_text())
            if r.get("mode") == "deterministic":
                errs = compare_stats(summary(run), r, rtol=rtol)
            else:
                errs = compare_summary(summary(run), r, rtol=rtol)
        for e in errs:
            print("MISMATCH:", e)
        print("OK" if not errs else f"FAILED ({len(errs)} mismatches)")
        return 1 if errs else 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
