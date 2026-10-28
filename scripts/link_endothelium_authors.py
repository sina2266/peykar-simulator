"""Explicit offline operator: author links between reserved endothelium studies.

Reads the Europe PMC ``lite`` search results kept by the literature screen (author
strings, journal and year only; no abstracts or outcome values) and the frozen
split reservation, and writes the declared overlap rule's output:

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
import sys
from datetime import UTC, datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/cornea-endothelium-2026-10-28"
RAW = ROOT / ".peykar/endothelium-literature-screen-2026-10-28/raw"
MIN_SHARED = 2


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def authors(text: str) -> set[str]:
    names = (part.strip().rstrip(".").lower() for part in text.split(","))
    return {name for name in names if name and name != "et al"}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=DATASET / "author-overlap-1.0.0.json")
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    reservation_path = DATASET / "split-reservation-1.0.0.json"
    reservation = json.loads(reservation_path.read_text())
    receipts = {
        item["file"]: item["sha256"]
        for item in json.loads((DATASET / "screening/literature-screen-receipts.json").read_text())
        if item.get("file", "").startswith("search-")
    }
    meta: dict[str, set[str]] = {}
    sources = []
    for path in sorted(RAW.glob("search-*.json")):
        if receipts.get(path.name) != sha256(path):
            raise SystemExit(f"{path.name} does not match its screening receipt")
        sources.append({"file": path.name, "sha256": sha256(path)})
        for result in json.loads(path.read_text()).get("resultList", {}).get("result", []):
            if result.get("pmcid"):
                meta[result["pmcid"].lower()] = authors(result.get("authorString", ""))
    split = {
        item["study_key"]: item["split"]
        for item in reservation["studies"]
        if item["split"] in {"development", "sealed_test"}
    }
    missing = [key for key in split if key.removeprefix("ecd-") not in meta]
    if missing:
        raise SystemExit(f"No author metadata for {missing}")
    links: list[tuple[str, str, int]] = []
    for a, b in itertools.combinations(sorted(split), 2):
        shared = meta[a.removeprefix("ecd-")] & meta[b.removeprefix("ecd-")]
        if len(shared) >= MIN_SHARED:
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
    output = {
        "schema": "cornea.endothelium-author-overlap-1.0.0",
        "utc": datetime.now(UTC).isoformat(),
        "rule": (
            f"reserved studies sharing at least {MIN_SHARED} normalized Europe PMC author names"
        ),
        "inputs": {
            "split_reservation": {
                "path": str(reservation_path.relative_to(ROOT)),
                "sha256": sha256(reservation_path),
            },
            "europe_pmc_lite_search_files": sources,
        },
        "outcome_values_read": False,
        "pairs": pairs,
        "development_clusters": sorted(sorted(c) for c in clusters.values() if len(c) > 1),
        "sealed_training_exclusions": {k: sorted(v) for k, v in sorted(exclusions.items())},
    }
    args.output.write_text(json.dumps(output, indent=1) + "\n")
    print(
        json.dumps(
            {k: output[k] for k in ("pairs", "development_clusters", "sealed_training_exclusions")},
            indent=1,
        )
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
