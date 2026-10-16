"""Acquire selected pinned vCornea reference files without downloading the entire ZIP."""

import argparse
import json
from pathlib import Path

from peykar.integrations.vcornea_sources import (
    acquire_code_archive,
    acquire_reference_members,
    list_reference_members,
)
from peykar_core.errors import PeykarError


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    listing = commands.add_parser("list", help="Read the ZIP64 inventory using HTTP Range")
    listing.add_argument("--contains")
    code = commands.add_parser("code", help="Download or verify the exact code archive")
    code.add_argument("--destination", type=Path, required=True)
    selected = commands.add_parser("members", help="Acquire exact scalar reference member paths")
    selected.add_argument("--destination", type=Path, required=True)
    selected.add_argument("--member", action="append", required=True)
    args = parser.parse_args()
    try:
        if args.command == "list":
            result = list_reference_members(args.contains)
        elif args.command == "code":
            result = acquire_code_archive(args.destination)
        else:
            result = acquire_reference_members(args.destination, tuple(args.member))
    except PeykarError as exc:
        parser.exit(1, f"{exc}\n")
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
