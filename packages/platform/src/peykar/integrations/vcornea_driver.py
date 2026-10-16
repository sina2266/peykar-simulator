"""Standalone entry point copied into a reviewed, isolated CompuCell3D runtime."""

import argparse
import math
import random
import runpy
import signal
import sys


def main() -> None:
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--seed", type=int)
    parser.add_argument("--wall-seconds", type=float, required=True)
    args, engine_args = parser.parse_known_args()
    if not math.isfinite(args.wall_seconds) or not 1 <= args.wall_seconds <= 86_400:
        raise ValueError("Invalid native execution deadline")
    # A kernel deadline still terminates the container if the host supervisor disappears.
    signal.signal(signal.SIGALRM, signal.SIG_DFL)
    signal.alarm(math.ceil(args.wall_seconds))
    if args.seed is not None:
        import numpy as np

        random.seed(args.seed)
        np.random.seed(args.seed)
    sys.argv = ["cc3d.run_script", *engine_args]
    runpy.run_module("cc3d.run_script", run_name="__main__")


if __name__ == "__main__":
    main()
