"""Extract only checksum-pinned reviewed files from the licensed vCornea archive."""

import argparse
from pathlib import Path

from peykar.integrations.vcornea_runtime import extract_source


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", required=True, type=Path)
    parser.add_argument("--pins", required=True, type=Path)
    parser.add_argument("--destination", required=True, type=Path)
    args = parser.parse_args()
    print(extract_source(args.archive, args.pins, args.destination))


if __name__ == "__main__":
    main()
