"""Freeze the development / sealed-test split of admitted Kmax studies.

Run once, after structural triage (masked previews only) and before any value of a
newly admitted study is extracted. The rule is deterministic and outcome-blind:
``sha256(SALT + lower(doi))`` first 8 hex digits modulo 3 equal to 0 reserves a study
for the sealed test. The salt differs from every other dataset's, so the components'
splits are independent. The output is written exclusively.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
from datetime import UTC, datetime
from pathlib import Path

SALT = "peykar-kmax-holdout-v1:"


def split_for(doi: str) -> str:
    digest = hashlib.sha256((SALT + doi.strip().lower()).encode()).hexdigest()
    return "sealed_test" if int(digest[:8], 16) % 3 == 0 else "development"


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--triage", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    raw = args.triage.read_bytes()
    triage = json.loads(raw)
    admitted = [item for item in triage["studies"] if item["decision"] == "admit"]
    if len({item["doi"].lower() for item in admitted}) != len(admitted):
        parser.error("Admitted studies must have unique DOIs")
    studies = [
        {"study_key": item["study_key"], "doi": item["doi"], "split": split_for(item["doi"])}
        for item in admitted
    ]
    reservation = {
        "schema": "cornea.kmax-split-reservation-1.0.0",
        "utc": datetime.now(UTC).isoformat(),
        "rule": "sha256(SALT + lower(doi)) first 8 hex digits mod 3 == 0 -> sealed_test",
        "salt": SALT,
        "triage_file": str(args.triage),
        "triage_sha256": hashlib.sha256(raw).hexdigest(),
        "outcome_values_read_before_reservation": False,
        "studies": studies,
    }
    with args.output.open("x", encoding="utf-8") as handle:
        handle.write(json.dumps(reservation, indent=1) + "\n")
    counts = {"development": 0, "sealed_test": 0}
    for study in studies:
        counts[study["split"]] += 1
    print(json.dumps(counts))
    return 0


if __name__ == "__main__":
    sys.exit(main())
