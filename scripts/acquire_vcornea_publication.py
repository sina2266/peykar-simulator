"""Explicitly acquire pinned scalar cohorts and final-paper supplementary documents.

Data acquisition never imports or executes downloaded model/analysis code.
"""

import argparse
import hashlib
import json
from pathlib import Path
from urllib.request import Request, urlopen

from peykar.integrations.vcornea_sources import acquire_reference_members


def checked_file(path, sha):
    if path.exists() and hashlib.sha256(path.read_bytes()).hexdigest() != sha:
        raise ValueError("Preserve conflicting source data; choose another destination")
    return path.exists()


def acquire(catalog, destination):
    catalog = json.loads(catalog.read_bytes())
    members = catalog.get("archive_members", catalog.get("scalar_and_analysis_members", []))
    missing = [
        m for m in members if not checked_file(destination / "archive" / m["path"], m["sha256"])
    ]
    for offset in range(0, len(missing), 32):
        group = missing[offset : offset + 32]
        receipt = acquire_reference_members(
            destination / "archive", tuple(m["path"] for m in group)
        )
        observed = {m["path"]: m["sha256"] for m in receipt["members"]}
        if any(observed.get(m["path"]) != m["sha256"] for m in group):
            raise ValueError("Published archive member differs from the pinned acquisition")
        print("Verified archive members", offset + len(group), "/", len(missing), flush=True)
    for doc in catalog.get("paper_supplements", []):
        path = destination / "paper-supplements" / f"{doc['id']}.docx"
        if checked_file(path, doc["sha256"]):
            continue
        # URLs come from the reviewed, checked-in publisher catalog, not a model manifest.
        with urlopen(
            Request(doc["url"], headers={"User-Agent": "Peykar-research/1.0"}), timeout=45
        ) as response:
            raw = response.read(doc["bytes"] + 1)
        if len(raw) != doc["bytes"] or hashlib.sha256(raw).hexdigest() != doc["sha256"]:
            raise ValueError("Publisher supplement differs from the pinned bytes")
        path.parent.mkdir(parents=True, exist_ok=True)
        # Never overwrite an earlier concurrent acquisition.
        with path.open("xb") as output:
            output.write(raw)
    print("All catalog-pinned original members and supplements verified", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--catalog",
        type=Path,
        default=Path("experiments/vcornea/publication/source-catalog-1.0.0.json"),
    )
    parser.add_argument("--destination", required=True, type=Path)
    args = parser.parse_args()
    acquire(args.catalog, args.destination)
