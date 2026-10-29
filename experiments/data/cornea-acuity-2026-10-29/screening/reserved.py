"""Articles reserved for other components' untested sealed or unread tranches.

The acuity dataset never triages, extracts or reads an article that another eye
component keeps for a test it has not run yet: retinal-thickness tranches 3 (sealed
for model 1.2) and 4 (unread), and IOP tranches 4 (sealed for model 1.3, under test
when this screen was made) and 5 (unread). Identity is the lower-case DOI, or the
PMCID when an article has no DOI. Only identifiers are read, never a value.
"""

from __future__ import annotations

import json
from functools import cache
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
RESERVED_TRANCHES = {
    "experiments/data/retina-cst-2026-10-28/tranche-order-1.0.0.json": 3,
    "experiments/data/iop-glaucoma-surgery-2026-10-28/tranche-order-1.0.0.json": 4,
}


def _key(doi: str | None, pmcid: str) -> str:
    return (doi or pmcid).strip().lower()


@cache
def reserved_keys() -> frozenset[str]:
    keys: set[str] = set()
    for relative, first in RESERVED_TRANCHES.items():
        order = json.loads((ROOT / relative).read_text())
        for article in order["articles"]:
            if article["tranche"] >= first:
                keys.add(_key(article.get("doi"), article["pmcid"]))
                keys.add(article["pmcid"].strip().lower())
    return frozenset(keys)


def is_reserved(doi: str | None, pmcid: str) -> bool:
    keys = reserved_keys()
    return _key(doi, pmcid) in keys or pmcid.strip().lower() in keys


if __name__ == "__main__":
    print(len(reserved_keys()), "reserved identifiers")
