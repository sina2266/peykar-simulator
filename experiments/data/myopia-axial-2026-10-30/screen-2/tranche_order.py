"""Outcome-blind tranche order of the mechanically flagged axial-length articles of screen 2.

Run from the repository root with ``uv run --frozen python
experiments/data/myopia-axial-2026-10-30/screen-2/tranche_order.py --output <file>``.

The first screen's ``tranche_order.py`` with this screen's masked previews and its own
salt: every article that passes the unchanged mechanical rule
(``../screening/triage_rule.py``, ``article_qualifies``) gets the rank of
``sha256(TRANCHE_SALT + lower(doi or pmcid))``. Screen-2 tranche 1 (the first
``TRANCHE_SIZE`` ranks) is triaged and reserved for the sealed test of myopia model 1.2;
later tranches stay untriaged and unread. The order uses identifiers only, never a value.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
from datetime import UTC, datetime
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
sys.path.insert(0, str(HERE.parent / "screening"))
from triage_rule import article_qualifies  # noqa: E402

PREVIEWS = ROOT / ".peykar/myopia-table-screen-2-2026-10-30/previews"
TRANCHE_SALT = "peykar-myopia-axial-screen2-tranche-v1:"
TRANCHE_SIZE = 120


def rank_key(doi: str | None, pmcid: str) -> str:
    return hashlib.sha256((TRANCHE_SALT + (doi or pmcid).strip().lower()).encode()).hexdigest()


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    flagged = []
    for path in sorted(PREVIEWS.glob("*.json")):
        preview = json.loads(path.read_text())
        if article_qualifies(preview):
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
        "schema": "eye.myopia-axial-tranche-order-1.0.0",
        "screen": 2,
        "utc": datetime.now(UTC).isoformat(),
        "rule": "rank by sha256(TRANCHE_SALT + lower(doi or pmcid)); tranche = 1 + rank // size",
        "salt": TRANCHE_SALT,
        "tranche_size": TRANCHE_SIZE,
        "previews": str(PREVIEWS.relative_to(ROOT)),
        "outcome_values_read": False,
        "articles": flagged,
    }
    args.output.write_text(json.dumps(output, indent=1) + "\n")
    counts: dict[int, int] = {}
    for item in flagged:
        counts[item["tranche"]] = counts.get(item["tranche"], 0) + 1
    print(json.dumps({"flagged": len(flagged), "tranches": counts}))
    return 0


if __name__ == "__main__":
    sys.exit(main())
