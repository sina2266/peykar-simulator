"""Explicit offline operator: build one IOP-after-surgery record from a spec.

``--spec`` is a JSON extraction spec (IOP extraction protocol 1.0.0) that cites
``file::table!RrCc`` cells or explicit ``definition:`` references.
``--source NAME=PATH`` provides each source file; SHA-256 values must match. The
record is validated and written exclusively to ``--output``; only its structure
is printed.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from peykar.validation.eye_iop_data import build_record, structural_summary


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--spec", type=Path, required=True)
    parser.add_argument("--source", action="append", default=[], metavar="NAME=PATH")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    sources = {}
    for item in args.source:
        name, _, path = item.partition("=")
        sources[name] = Path(path).read_bytes()
    record = build_record(json.loads(args.spec.read_text()), sources)
    with args.output.open("x", encoding="utf-8") as handle:
        handle.write(json.dumps(record, indent=1, ensure_ascii=False) + "\n")
    print(json.dumps(structural_summary([record]), indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
