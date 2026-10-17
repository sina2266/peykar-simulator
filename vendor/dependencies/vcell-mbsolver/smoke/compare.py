"""Compare a MovingBoundary run's .h5 against the committed reference.

    python smoke/compare.py <reference.h5> <result.h5> [--rtol R] [--atol A]

Compares /Times and every species array under /Solution/timeNNNNNN (the
per-output-time concentrations on the 31x31 node grid). A species passes when
max|result - reference| <= atol + rtol * max|reference| over its finite values
and both files have NaNs in the same places. Needs h5py and numpy.

    python smoke/compare.py --make-reference <legacy-result.h5> <reference.h5>

rewrites the reference from a legacy run (only /Times and /Solution are kept).
"""

from __future__ import annotations

import argparse
import sys

import h5py
import numpy as np


def load(path: str) -> tuple[np.ndarray, dict[str, dict[str, np.ndarray]]]:
    with h5py.File(path, "r") as f:
        times = f["Times"][()]
        sol = {t: {s: f["Solution"][t][s][()] for s in f["Solution"][t]} for t in sorted(f["Solution"])}
    return times, sol


def make_reference(src: str, dst: str) -> int:
    times, sol = load(src)
    with h5py.File(src, "r") as f, h5py.File(dst, "w") as out:
        out.attrs["source"] = (
            "MovingBoundary_x64 v0.0.44-dev4 (vcell-solvers, macOS x86_64; bit-identical to "
            "ghcr.io/virtualcell/vcell-solvers:v0.8.2) on smoke/SimID_254696951_0_mb.xml"
        )
        out.create_dataset("Times", data=times)
        for t, species in sol.items():
            g = out.create_group(f"Solution/{t}")
            for k, v in f["Solution"][t].attrs.items():
                g.attrs[k] = v
            for s, arr in species.items():
                g.create_dataset(s, data=arr, compression="gzip", compression_opts=9, shuffle=True)
    print(f"wrote {dst}: {len(times)} times, species {sorted(next(iter(sol.values())))}")
    return 0


def compare(ref_path: str, res_path: str, rtol: float, atol: float) -> int:
    rt, rsol = load(ref_path)
    xt, xsol = load(res_path)
    ok = True
    if rt.shape != xt.shape or not np.allclose(rt, xt, rtol=0, atol=1e-12):
        print(f"FAIL Times differ: reference {rt} result {xt}")
        return 1
    if rsol.keys() != xsol.keys():
        print(f"FAIL time groups differ: {sorted(rsol)} vs {sorted(xsol)}")
        return 1
    worst: dict[str, float] = {}
    for t in rsol:
        for s, ref in rsol[t].items():
            res = xsol[t].get(s)
            if res is None or res.shape != ref.shape:
                print(f"FAIL {t}/{s}: missing or shape mismatch")
                ok = False
                continue
            if not np.array_equal(np.isnan(ref), np.isnan(res)):
                print(f"FAIL {t}/{s}: NaN pattern differs")
                ok = False
                continue
            m = np.isfinite(ref)
            diff = float(np.max(np.abs(res[m] - ref[m]), initial=0.0))
            scale = float(np.max(np.abs(ref[m]), initial=0.0))
            worst[s] = max(worst.get(s, 0.0), diff / scale if scale else diff)
            if diff > atol + rtol * scale:
                print(f"FAIL {t}/{s}: max|diff| {diff:.3e} > {atol:.1e} + {rtol:.1e}*{scale:.3e}")
                ok = False
    for s, w in sorted(worst.items()):
        print(f"{s}: worst max|diff|/max|ref| over {len(rsol)} times = {w:.3e}")
    print("PASS" if ok else "FAIL", f"(rtol={rtol}, atol={atol})")
    return 0 if ok else 1


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("a")
    p.add_argument("b")
    p.add_argument("--rtol", type=float, default=1e-6)
    p.add_argument("--atol", type=float, default=1e-12)
    p.add_argument("--make-reference", action="store_true")
    args = p.parse_args()
    if args.make_reference:
        return make_reference(args.a, args.b)
    return compare(args.a, args.b, args.rtol, args.atol)


if __name__ == "__main__":
    sys.exit(main())
