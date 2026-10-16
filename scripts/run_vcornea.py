"""Run an explicitly admitted vCornea source copy in a bounded offline container."""

import argparse
import json
from pathlib import Path

from peykar.integrations.vcornea_runtime import execute_run, prepare_run


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--pins", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--scenario", choices=("injury", "stem-only"), default="injury")
    parser.add_argument("--steps", type=int)
    parser.add_argument("--seed", type=int)
    parser.add_argument("--image", required=True)
    parser.add_argument("--wall-seconds", type=float, default=300)
    args = parser.parse_args()
    prepare_run(
        args.source,
        args.pins,
        args.output,
        scenario=args.scenario,
        steps=args.steps,
        seed=args.seed,
    )
    print(
        json.dumps(execute_run(args.output, args.image, wall_seconds=args.wall_seconds), indent=2)
    )


if __name__ == "__main__":
    main()
