#!/usr/bin/env python3
"""Statistical reference check: every Hy3S integrator against the exact solution.

The ctest suite deliberately keeps every reaction a discrete jump, so the SDE
integrators -- the only thing that differs between the three binaries -- are
never entered there. This check does the opposite. On the birth-death model
(make_birth_death.py)

    0 -> X  (k),   X -> 0  (gamma X),   X(0) = x0,

VCell's default partitioning thresholds (epsilon = 100, lambda = 10) make both
reactions fast for the whole run, so each binary integrates the chemical
Langevin equation with its own scheme. The exact process has

    X(t) = Binomial(x0, p) + Poisson(k/gamma (1 - p)),   p = e^{-gamma t}
    mean(t) = x0 p + k/gamma (1 - p)
    var(t)  = x0 p (1 - p) + k/gamma (1 - p)

and because the propensities are linear, the Langevin approximation has exactly
the same mean and variance. So every integrator -- and an all-discrete SSA
control run -- must reproduce both moments at every save point, up to sampling
error and a small time-discretisation bias.

Per save point, over N trials:
    z_mean = (sample mean - mean) / sqrt(var / N)
    z_var  = (sample var  - var ) / (var * sqrt(2 / (N - 1)))
Each run passes if every |z| < --z-max (default 4). With a fixed seed the
verdict is deterministic for a given compiler. A deliberately coarse
Euler-Maruyama run is included as a negative control and must be rejected, so
the check is known to have the power to fail.

Usage:
    check_birth_death.py --bin-dir <dir with Hybrid_*_x64> [--exe-suffix .exe]
    check_birth_death.py --launcher "apptainer run --containall --bind W:/simdata img.sif" \
                         --work W --container-work /simdata
"""
import argparse
import math
import os
import shlex
import shutil
import subprocess
import sys
import tempfile

import numpy as np
from scipy.io import netcdf_file

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from make_birth_death import GAMMA, X0, effective_birth_rate  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
MODEL = os.path.join(HERE, "..", "resources", "birth_death.nc")

# name, executable, solver arguments (VCell's order: epsilon lambda MSR_tol [SDE_tol] SDE_dt),
# and whether the run is expected to pass.
RUNS = [
    # Written the way VCell's HybridSolver writes them: String.valueOf(double), lower-cased.
    ("Hybrid_EM (Euler-Maruyama)", "Hybrid_EM_x64", ["100.0", "10.0", "0.01", "0.001"], True),
    ("Hybrid_MIL (Milstein)", "Hybrid_MIL_x64", ["100.0", "10.0", "0.01", "0.001"], True),
    ("Hybrid_MIL_Adaptive", "Hybrid_MIL_Adaptive_x64", ["100.0", "10.0", "0.01", "1.0e-4", "0.01"], True),
    # Thresholds nothing can reach: every reaction stays a discrete jump, so
    # this is Gillespie's SSA -- a control that the model and the moments are right.
    # (Written "1e9", which the solver's old hand-rolled parser read as 0.)
    ("SSA control (all reactions discrete)", "Hybrid_EM_x64", ["1e9", "1e9", "0.01", "0.001"], True),
    # Negative control: the check must be able to fail. Euler-Maruyama at
    # dt = 0.25 (the save interval) biases the mean by O(gamma dt) -- about 30
    # standard errors at N = 1000 -- so this run has to be rejected.
    ("negative control: Hybrid_EM at dt = 0.25 (must FAIL)", "Hybrid_EM_x64", ["100.0", "10.0", "0.01", "0.25"], False),
]


def exact_moments(t):
    k = effective_birth_rate()
    p = np.exp(-GAMMA * t)
    mean = X0 * p + k / GAMMA * (1 - p)
    var = X0 * p * (1 - p) + k / GAMMA * (1 - p)
    return mean, var


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--bin-dir", help="directory holding the executables (run directly)")
    ap.add_argument("--exe-suffix", default="", help="e.g. .exe on Windows")
    ap.add_argument("--launcher", default="", help="command prefix, e.g. an apptainer/docker run; "
                    "the executable is then named bare, as VCell's SlurmProxy does")
    ap.add_argument("--work", help="host working directory (default: a temp dir)")
    ap.add_argument("--container-work", help="the same directory as the launcher's command sees it")
    ap.add_argument("--extra-args", default="", help="appended to every solver command, e.g. '-tid 0'")
    ap.add_argument("--model", default=MODEL, help="model file (default: tests/resources/birth_death.nc)")
    ap.add_argument("--seed", default="4711")
    ap.add_argument("--z-max", type=float, default=4.0)
    args = ap.parse_args()

    work = args.work or tempfile.mkdtemp(prefix="hy3s-stats-")
    os.makedirs(work, exist_ok=True)
    inner = args.container_work or work
    failed = []

    for i, (label, exe, params, should_pass) in enumerate(RUNS):
        name = f"run{i}.nc"
        shutil.copyfile(args.model, os.path.join(work, name))
        # Writable by whatever uid the launcher runs the solver as. Hy3S writes
        # its solution into this file, and on a NetCDF error it prints and
        # carries on, exiting 0 -- so a read-only copy looks like a silent no-op.
        os.chmod(os.path.join(work, name), 0o666)
        if args.launcher:
            cmd = shlex.split(args.launcher) + [exe]
        else:
            cmd = [os.path.join(args.bin_dir, exe + args.exe_suffix)]
        cmd += [f"{inner}/{name}", *params, "-R", args.seed, "-OV", *shlex.split(args.extra_args)]
        print(f"\n== {label}\n   $ {' '.join(cmd)}", flush=True)
        proc = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        if proc.returncode != 0:
            print(proc.stdout[-2000:], proc.stderr[-2000:])
            failed.append(f"{label}: exit {proc.returncode}")
            continue

        with netcdf_file(os.path.join(work, name), "r", mmap=False) as f:
            written = getattr(f, "Data_Written", 0)
            if "Time" in f.variables and "State" in f.variables:
                t = f.variables["Time"].data.copy()
                x = f.variables["State"].data[:, :, 0].astype(float)   # (trials, times)
            else:
                t = x = None
        if written != 1 or x is None or not np.all(np.isfinite(x)) or np.all(x == 0):
            failed.append(f"{label}: no solution written")
            print("   no solution written; solver output:\n" + proc.stdout[-3000:] + proc.stderr[-2000:])
            continue
        n = x.shape[0]
        mean, var = exact_moments(t)
        s_mean, s_var = x.mean(axis=0), x.var(axis=0, ddof=1)

        print(f"   {n} trials; {'t':>5} {'mean':>9} {'exact':>9} {'z':>6}   {'var':>8} {'exact':>8} {'z':>6}")
        worst = 0.0
        for j in range(len(t)):
            if var[j] == 0:     # t = 0: the initial condition, no spread
                ok = np.all(x[:, j] == X0)
                print(f"   {t[j]:5.2f} {s_mean[j]:9.2f} {mean[j]:9.2f} {'':>6}   {s_var[j]:8.2f} {var[j]:8.2f}"
                      f"   {'ok' if ok else 'WRONG INITIAL CONDITION'}")
                if not ok:
                    worst = math.inf
                continue
            zm = (s_mean[j] - mean[j]) / math.sqrt(var[j] / n)
            zv = (s_var[j] - var[j]) / (var[j] * math.sqrt(2.0 / (n - 1)))
            worst = max(worst, abs(zm), abs(zv))
            print(f"   {t[j]:5.2f} {s_mean[j]:9.2f} {mean[j]:9.2f} {zm:6.2f}   {s_var[j]:8.2f} {var[j]:8.2f} {zv:6.2f}")
        passed = worst < args.z_max
        print(f"   max |z| = {worst:.2f} (limit {args.z_max}) -> {'PASS' if passed else 'FAIL'}"
              f"{'' if passed == should_pass else '  <-- UNEXPECTED'}")
        if passed != should_pass:
            failed.append(f"{label}: max |z| {worst:.2f}, expected {'pass' if should_pass else 'fail'}")

    print()
    if failed:
        print("FAILED:\n  " + "\n  ".join(failed))
        return 1
    print(f"all {len(RUNS) - 1} runs match the exact birth-death moments; the negative control is rejected")
    return 0


if __name__ == "__main__":
    sys.exit(main())
