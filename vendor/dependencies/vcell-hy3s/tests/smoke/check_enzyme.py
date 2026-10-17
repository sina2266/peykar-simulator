#!/usr/bin/env python3
"""Check a solved tests/resources/enzyme.nc: it ran, and it conserved mass.

    check_enzyme.py <solved.nc> [--baseline <baseline.nc>]

The container smoke tests run the ctest suite's all-discrete enzyme case
through the image and the SIF. What must hold is that the solver wrote a
solution, and that S + E <-> ES -> E + P conserved total enzyme (E + ES) and
total substrate (S + ES + P) at every save point. With --baseline the State
must also match the committed per-compiler baseline, as the ctest suite
requires.
"""
import argparse
import sys

import numpy as np
from scipy.io import netcdf_file


def load(path):
    with netcdf_file(path, "r", mmap=False) as f:
        names = [bytes(r).decode().strip() for r in f.variables["Species_names"].data]
        return (getattr(f, "Data_Written", 0), f.variables["State"].data.astype(float).copy(),
                f.variables["Time"].data.copy(), names)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("solved")
    ap.add_argument("--baseline")
    args = ap.parse_args()

    written, state, time, names = load(args.solved)
    x = {n: state[0, :, i] for i, n in enumerate(names)}
    problems = []
    if written != 1:
        problems.append("Data_Written is not set: the solver did not finish")
    if not np.all(np.isfinite(state)) or np.all(state[0, -1] == 0):
        problems.append("no solution at the final time")
    enzyme = x["E_Cellular"] + x["ES_Cellular"]
    substrate = x["S_Cellular"] + x["ES_Cellular"] + x["P_Cellular"]
    for label, total in (("E + ES", enzyme), ("S + ES + P", substrate)):
        drift = np.max(np.abs(total - total[0])) / max(total[0], 1.0)
        print(f"{label:12s} {total[0]:.0f} -> {total[-1]:.3f}  (max relative drift {drift:.1e})")
        if drift > 1e-9:
            problems.append(f"{label} not conserved (drift {drift:.1e})")
    print(f"t = {time[-1]}: " + ", ".join(f"{n} = {v[-1]:.1f}" for n, v in x.items()))

    if args.baseline:
        _, base, _, _ = load(args.baseline)
        if base.shape != state.shape or not np.allclose(state, base, rtol=1e-9, atol=0):
            problems.append(f"State differs from {args.baseline}")
        else:
            print(f"matches {args.baseline}")

    if problems:
        print("FAILED: " + "; ".join(problems))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
