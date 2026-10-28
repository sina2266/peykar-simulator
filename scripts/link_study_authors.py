"""Explicit offline operator: author links between the reserved studies of a dataset.

Generic form of ``scripts/link_endothelium_authors.py`` (kept unchanged because a
frozen protocol pins it). Reads the Europe PMC ``lite`` search results kept by a
literature screen (author strings, journal and year only; no abstracts or
outcome values), checks them against the dataset's screening receipts, and
applies the declared overlap rule to the frozen split reservation:

* pairs of reserved studies sharing at least two normalized author names;
* development clusters (connected components of development-development links),
  held out together in leave-one-study-out;
* for each sealed study, the development studies it is linked to, which are
  excluded from that sealed study's training set.

Shared authors suggest a possibly shared patient cohort; the rule removes that
dependence without reading any sealed article. Output must not already exist.
"""

from __future__ import annotations

import argparse
import hashlib
import itertools
import json
import re
import sys
from collections.abc import Mapping
from datetime import UTC, datetime
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
MIN_SHARED = 2
_PREFIX = re.compile(r"^[a-z][a-z0-9]{1,15}-$")
_SCHEMA = re.compile(r"^[a-z0-9][a-z0-9.-]{2,120}$")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def authors(text: str) -> set[str]:
    names = (part.strip().rstrip(".").lower() for part in text.split(","))
    return {name for name in names if name and name != "et al"}


def _display(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(ROOT))
    except ValueError:
        return str(path)


def link(
    *,
    split: Mapping[str, str],
    meta: Mapping[str, set[str]],
    prefix: str,
    min_shared: int = MIN_SHARED,
) -> dict[str, Any]:
    """Overlap pairs, development clusters and sealed training exclusions."""
    missing = [key for key in split if key.removeprefix(prefix) not in meta]
    if missing:
        raise ValueError(f"No author metadata for {missing}")
    links: list[tuple[str, str, int]] = []
    for a, b in itertools.combinations(sorted(split), 2):
        shared = meta[a.removeprefix(prefix)] & meta[b.removeprefix(prefix)]
        if len(shared) >= min_shared:
            links.append((a, b, len(shared)))
    pairs = [{"studies": [a, b], "splits": [split[a], split[b]], "shared": n} for a, b, n in links]
    development = sorted(k for k, s in split.items() if s == "development")
    parent = {k: k for k in development}

    def find(key: str) -> str:
        while parent[key] != key:
            key = parent[key]
        return key

    for a, b, _ in links:
        if split[a] == split[b] == "development":
            parent[find(a)] = find(b)
    clusters: dict[str, list[str]] = {}
    for key in development:
        clusters.setdefault(find(key), []).append(key)
    exclusions: dict[str, list[str]] = {}
    for a, b, _ in links:
        if {split[a], split[b]} == {"development", "sealed_test"}:
            sealed, dev = (a, b) if split[a] == "sealed_test" else (b, a)
            exclusions.setdefault(sealed, []).append(dev)
    return {
        "pairs": pairs,
        "development_clusters": sorted(sorted(c) for c in clusters.values() if len(c) > 1),
        "sealed_training_exclusions": {k: sorted(v) for k, v in sorted(exclusions.items())},
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--dataset",
        type=Path,
        required=True,
        help="dataset folder with split-reservation-1.0.0.json and screening receipts",
    )
    parser.add_argument(
        "--raw", type=Path, required=True, help="literature-screen folder with search-*.json"
    )
    parser.add_argument("--prefix", required=True, help="study key prefix, e.g. 'cct-'")
    parser.add_argument("--schema", required=True, help="output schema identifier")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    if not _PREFIX.match(args.prefix):
        parser.error("--prefix must look like 'cct-'")
    if not _SCHEMA.match(args.schema):
        parser.error("--schema must be a lower-case dotted identifier")
    reservation_path = args.dataset / "split-reservation-1.0.0.json"
    reservation = json.loads(reservation_path.read_text())
    receipts = {
        item["file"]: item["sha256"]
        for item in json.loads(
            (args.dataset / "screening/literature-screen-receipts.json").read_text()
        )
        if item.get("file", "").startswith("search-")
    }
    meta: dict[str, set[str]] = {}
    sources = []
    for path in sorted(args.raw.glob("search-*.json")):
        if receipts.get(path.name) != sha256(path):
            raise SystemExit(f"{path.name} does not match its screening receipt")
        sources.append({"file": path.name, "sha256": sha256(path)})
        for result in json.loads(path.read_text()).get("resultList", {}).get("result", []):
            if result.get("pmcid"):
                meta[result["pmcid"].lower()] = authors(result.get("authorString", ""))
    if not sources or len(sources) != len(receipts):
        raise SystemExit("Every receipted search file must be present")
    split = {
        item["study_key"]: item["split"]
        for item in reservation["studies"]
        if item["split"] in {"development", "sealed_test"}
    }
    if any(not key.startswith(args.prefix) for key in split):
        raise SystemExit(f"Reserved study keys must start with {args.prefix!r}")
    try:
        result = link(split=split, meta=meta, prefix=args.prefix)
    except ValueError as error:
        raise SystemExit(str(error)) from error
    output = {
        "schema": args.schema,
        "utc": datetime.now(UTC).isoformat(),
        "rule": (
            f"reserved studies sharing at least {MIN_SHARED} normalized Europe PMC author names"
        ),
        "inputs": {
            "split_reservation": {
                "path": _display(reservation_path),
                "sha256": sha256(reservation_path),
            },
            "europe_pmc_lite_search_files": sources,
        },
        "outcome_values_read": False,
        **result,
    }
    with args.output.open("x", encoding="utf-8") as handle:
        handle.write(json.dumps(output, indent=1) + "\n")
    print(json.dumps(result, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
