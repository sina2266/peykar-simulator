#!/usr/bin/env python3
"""Compare a SundialsSolverStandalone `.ida` output with a reference `.ida` (stdlib only).

The `.ida` format is a header line naming the columns (`t:x:y:`) followed by one whitespace-separated
row per output time. Two runs of the same input on different compilers/platforms can take slightly
different adaptive steps (KEEP_EVERY outputs), so rows are matched by time: the candidate is
interpolated linearly onto the reference's time points. Where the time grids coincide (OUTPUT_TIMES
inputs) that is an exact row-by-row comparison.

A value passes when |cand - ref| <= rtol * |ref| + atol * scale, where `scale` is the column's
largest |ref| (so atol is relative to the variable's magnitude). When the grids differ, linear
interpolation across an event or discontinuity adds its own error, so --rtol-interp (default 1e-4)
replaces --rtol. Exit 0 on pass, 1 on fail.

    compare_ida.py reference.ida candidate.ida [--rtol 1e-5] [--rtol-interp 1e-4] [--atol 1e-8]
"""

import argparse
import bisect
import sys
from typing import List, Tuple

# Python 3.6-compatible (Rocky/RHEL 8's python3), stdlib only.


def load(path: str) -> Tuple[List[str], List[List[float]]]:
    with open(path) as f:
        lines = [line for line in f.read().splitlines() if line.strip()]
    if not lines:
        raise SystemExit(f"{path}: empty")
    header = [c for c in lines[0].split(":") if c]
    rows = [[float(v) for v in line.split()] for line in lines[1:]]
    for i, r in enumerate(rows):
        if len(r) != len(header):
            raise SystemExit(f"{path}: row {i + 1} has {len(r)} values, header has {len(header)}")
    return header, rows


def interp(ts: List[float], col: List[float], t: float) -> float:
    j = bisect.bisect_left(ts, t)
    if j < len(ts) and ts[j] == t:
        # several rows can share a time (a discontinuity or event); take the last one, as the
        # reference's own row for that time is its last one too
        while j + 1 < len(ts) and ts[j + 1] == t:
            j += 1
        return col[j]
    if j == 0 or j == len(ts):
        raise ValueError(f"t={t} outside candidate range [{ts[0]}, {ts[-1]}]")
    t0, t1 = ts[j - 1], ts[j]
    w = (t - t0) / (t1 - t0)
    return col[j - 1] * (1 - w) + col[j] * w


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("reference")
    ap.add_argument("candidate")
    ap.add_argument("--rtol", type=float, default=1e-5)
    ap.add_argument("--rtol-interp", type=float, default=1e-4)
    ap.add_argument("--atol", type=float, default=1e-8)
    a = ap.parse_args()

    rh, rr = load(a.reference)
    ch, cr = load(a.candidate)
    if rh != ch:
        print(f"FAIL: headers differ\n  ref:  {rh}\n  cand: {ch}")
        return 1
    ct = [r[0] for r in cr]
    if abs(ct[-1] - rr[-1][0]) > 1e-9 * max(1.0, abs(rr[-1][0])):
        print(f"FAIL: end time {ct[-1]} != reference {rr[-1][0]}")
        return 1
    same_grid = len(rr) == len(cr) and all(abs(x[0] - y[0]) <= 1e-12 * max(1.0, abs(x[0])) for x, y in zip(rr, cr))
    rtol = a.rtol if same_grid else a.rtol_interp

    worst = (0.0, "", 0.0)
    failures = 0
    for k in range(1, len(rh)):
        ref_col = [r[k] for r in rr]
        cand_col = [r[k] for r in cr]
        scale = max(abs(v) for v in ref_col) or 1.0
        for i, row in enumerate(rr):
            ref = row[k]
            cand = cand_col[i] if same_grid else interp(ct, cand_col, row[0])
            err = abs(cand - ref)
            tol = rtol * abs(ref) + a.atol * scale
            ratio = err / tol if tol > 0 else (0.0 if err == 0 else float("inf"))
            if ratio > worst[0]:
                worst = (ratio, rh[k], row[0])
            if err > tol:
                failures += 1
                if failures <= 10:
                    print(f"  {rh[k]} at t={row[0]:.12g}: ref={ref:.17g} cand={cand:.17g} err={err:.3g} tol={tol:.3g}")

    grid = "same time grid" if same_grid else f"interpolated ({len(cr)} candidate rows onto {len(rr)} reference rows)"
    status = "FAIL" if failures else "PASS"
    print(f"{status}: {a.candidate} vs {a.reference}: {len(rh) - 1} variables, {grid}; "
          f"worst err/tol = {worst[0]:.3g} ({worst[1]} at t={worst[2]:.6g}); rtol={rtol} atol={a.atol}"
          + (f"; {failures} values out of tolerance" if failures else ""))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
