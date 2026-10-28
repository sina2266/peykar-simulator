"""Mechanical CCT triage rule over masked table previews (no values read).

Copy of the operator used for triage-1.0.0 (run from the repository root with
``uv run --frozen python experiments/data/cornea-thickness-2026-10-28/screening/triage_rule.py``).

A table qualifies when its caption/header/first-column labels mention corneal
thickness, a preoperative/baseline reference and at least two distinct follow-up
time labels, and its caption has no exclusion term. Prints qualifying articles
with their masked structure for manual review, or writes a JSON list with --json.
"""

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / "scripts"))
from show_masked_previews import strict  # noqa: E402

PREVIEWS = ROOT / ".peykar/thickness-table-screen-2026-10-28/previews"
TOPIC = re.compile(
    r"(central\s+)?corneal\s+thickness|\bCCT\b|\bCT\b|pachymetr|thinnest|corneal\s+o?edema", re.I
)
REFERENCE = re.compile(
    r"pre-?op|preoperative|pre-operative|baseline|before\s+(surgery|operation)|\bpre\b", re.I
)
TIME = re.compile(
    r"\b(\d+)\s*(hours?|hrs?|h|days?|d|weeks?|wks?|w|months?|mos?|m|years?|yrs?|y)\b"
    r"|\b(day|week|month|year|POD)\s*(\d+)\b",
    re.I,
)
EXCLUDE = re.compile(
    r"regression|review|meta-?analys|correlat|risk\s+factor|multivar|LASIK|PRK|SMILE|"
    r"cross-?link|\bCXL\b|orthokeratolog|contact\s+lens|refractive\s+surgery|predict|"
    r"repeatab|reproducib|agreement|reliab|normative|healthy\s+(subjects|volunteers)",
    re.I,
)
UNITS = {
    "h": "h",
    "hour": "h",
    "hr": "h",
    "d": "d",
    "day": "d",
    "w": "w",
    "wk": "w",
    "week": "w",
    "m": "m",
    "mo": "m",
    "month": "m",
    "y": "y",
    "yr": "y",
    "year": "y",
    "pod": "d",
}


def unit(text: str) -> str:
    key = text.lower().rstrip("s")
    return UNITS.get(key, key)


def times(text: str) -> set[str]:
    found = set()
    for match in TIME.finditer(text):
        if match.group(1):
            found.add(f"{int(match.group(1))}{unit(match.group(2))}")
        else:
            found.add(f"{int(match.group(4))}{unit(match.group(3))}")
    return found


def labels(table: dict) -> tuple[str, list[str]]:
    heads = [r.split(":", 1)[1] for r in table["rows"] if r.startswith("head")]
    firsts = [r.split(":", 1)[1].split(" | ")[0] for r in table["rows"] if r.startswith("body")]
    return " ".join(heads), firsts


def qualifies(table: dict) -> bool:
    caption = f"{table.get('label', '')} {table.get('caption', '')}"
    if EXCLUDE.search(caption):
        return False
    head, firsts = labels(table)
    text = " ".join([caption, head, *firsts])
    if not TOPIC.search(text) or not REFERENCE.search(text):
        return False
    follow = times(head + " " + " ".join(firsts))
    return len(follow) >= 2


def main() -> None:
    hits = []
    for path in sorted(PREVIEWS.glob("*.json")):
        preview = json.loads(path.read_text())
        tables = [t for t in preview["tables"] if qualifies(t)]
        if tables:
            hits.append((preview, tables))
    if "--json" in sys.argv:
        print(json.dumps([p["pmcid"] for p, _ in hits]))
        return
    for preview, tables in hits:
        title = strict(preview.get("title", ""))[:150]
        print(f"=== {preview['pmcid']} doi={preview.get('doi')} | {title}")
        print(f"    species={preview.get('species')} licence={str(preview.get('license'))[:60]}")
        for table in tables[:3]:
            caption = strict(table.get("caption", ""))[:200]
            print(f"  [{table['id']}] {strict(table.get('label', ''))} {caption}")
            for row in table["rows"][:14]:
                print(f"     {strict(row)[:200]}")
    print(len(hits), "articles qualify")


if __name__ == "__main__":
    main()
