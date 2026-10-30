"""Articles reserved for other components' untested sealed or unread tranches.

The myopia axial-length dataset never triages, extracts or reads an article that another
eye component keeps for a test it has not run yet: IOP tranche 5 (sealed for IOP model
1.4 when this screen was made). Every other tranche of the earlier datasets has been
consumed by a sealed test or is a development set. Identity is the lower-case DOI, or
the PMCID when an article has no DOI. Only identifiers are read, never a value.
"""

from __future__ import annotations

import json
from functools import cache
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
RESERVED_TRANCHES = {
    "experiments/data/iop-glaucoma-surgery-2026-10-28/tranche-order-1.0.0.json": 5,
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
