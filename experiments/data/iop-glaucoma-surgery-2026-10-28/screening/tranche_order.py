"""Outcome-blind tranche order of the mechanically flagged IOP articles.

Run from the repository root with ``uv run --frozen python
experiments/data/iop-glaucoma-surgery-2026-10-28/screening/tranche_order.py --output <file>``.

Every article that passes the mechanical rule (``triage_rule.qualifies``) gets the
rank of ``sha256(TRANCHE_SALT + lower(doi or pmcid))``. Tranche 1 (the first
``TRANCHE_SIZE`` ranks) is triaged and extracted now; later tranches stay
untriaged and unread, reserved for testing future model versions on studies that
no model has seen. The order uses identifiers only, never a value.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
from datetime import UTC, datetime
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from triage_rule import PREVIEWS, qualifies  # noqa: E402

TRANCHE_SALT = "peykar-iop-tranche-v1:"
TRANCHE_SIZE = 120


def rank_key(doi: str | None, pmcid: str) -> str:
    return hashlib.sha256((TRANCHE_SALT + (doi or pmcid).strip().lower()).encode()).hexdigest()


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    flagged = []
    for path in sorted(PREVIEWS.glob("*.json")):
        preview = json.loads(path.read_text())
        if any(qualifies(t) for t in preview["tables"]):
            flagged.append(
                {
                    "pmcid": preview["pmcid"],
                    "doi": preview.get("doi"),
                    "key": rank_key(preview.get("doi"), preview["pmcid"]),
                    "fulltext_sha256": preview.get("fulltext_sha256"),
                }
            )
    flagged.sort(key=lambda item: item["key"])
    for index, item in enumerate(flagged):
        item["rank"] = index
        item["tranche"] = 1 + index // TRANCHE_SIZE
    output = {
        "schema": "eye.iop-tranche-order-1.0.0",
        "utc": datetime.now(UTC).isoformat(),
        "rule": "rank by sha256(TRANCHE_SALT + lower(doi or pmcid)); tranche = 1 + rank // size",
        "salt": TRANCHE_SALT,
        "tranche_size": TRANCHE_SIZE,
        "outcome_values_read": False,
        "articles": flagged,
    }
    with args.output.open("x", encoding="utf-8") as handle:
        handle.write(json.dumps(output, indent=1) + "\n")
    print(len(flagged), "flagged;", sum(
        1 for a in flagged if a["tranche"] == 1), "in tranche 1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
