"""Mechanical retinal visual-acuity triage rule over masked table previews (no values read).

Run from the repository root with ``uv run --frozen python
experiments/data/retina-acuity-2026-10-29/screening/triage_rule.py``.

A table qualifies when its caption/header/first-column labels mention corrected
visual acuity (BCVA, logMAR, ETDRS), a baseline or pretreatment reference and at
least two distinct follow-up time labels, and its caption has no exclusion term
(statistical models, reviews, agreement or normative studies, survival analyses).
It is the corneal-acuity rule with "pretreatment", "preinjection" and "before
(the first) injection" added to the reference terms. An article qualifies when one
of its tables does and it is not reserved for another component's untested sealed or
unread tranche (``reserved.py``). Prints qualifying articles with their masked
structure for manual review, or writes a JSON list with --json.
"""

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / "scripts"))
sys.path.insert(0, str(HERE))
from reserved import is_reserved  # noqa: E402
from show_masked_previews import strict  # noqa: E402

PREVIEWS = ROOT / ".peykar/retina-acuity-table-screen-2026-10-29/previews"
TOPIC = re.compile(
    r"visual\s+acuity|\bBCVA\b|\bCDVA\b|\bBSCVA\b|\bBDVA\b|\bCDVA\b|log\s*MAR|\bETDRS\b"
    r"|corrected\s+(distance\s+)?visual|\bVA\b",
    re.I,
)
REFERENCE = re.compile(
    r"baseline|pre-?op|preoperative|pre-operative|pre-?treatment|pre-?injection"
    r"|before\s+(surgery|treatment|the\s+operation|injection|the\s+first\s+injection)"
    r"|\bpre\b|\binitial\b|\b(month|week|day|visit)\s*0\b|\b[MWD]\s*0\b",
    re.I,
)
# Hyphenated labels ("3-month") count as time labels too.
TIME = re.compile(
    r"\b(\d+)\s*[-\u2010\u2011\u2013]?\s*"
    r"(hours?|hrs?|h|days?|d|weeks?|wks?|w|months?|mos?|m|years?|yrs?|y)\b"
    r"|\b(day|week|month|year|POD|visit|[MWDY])\s*[-\u2010\u2011\u2013]?\s*(\d+)\b",
    re.I,
)
EXCLUDE = re.compile(
    r"regression\s+(analysis|model|coefficient)|multivariate|multivariable|linear\s+regression"
    r"|logistic|review|meta-?analys|correlat|risk\s+factor|predict|repeatab|reproducib"
    r"|agreement|reliab|normative|healthy\s+(subjects|volunteers|controls|eyes)"
    r"|survival|kaplan",
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
    "visit": "v",
}


def unit(text: str) -> str:
    key = text.lower().rstrip("s")
    return UNITS.get(key, key)


def times(text: str) -> set[str]:
    found = set()
    for match in TIME.finditer(text):
        if match.group(1):
            value, label = int(match.group(1)), unit(match.group(2))
        else:
            value, label = int(match.group(4)), unit(match.group(3))
        if value > 0:
            found.add(f"{value}{label}")
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


def article_qualifies(preview: dict) -> bool:
    if is_reserved(preview.get("doi"), preview["pmcid"]):
        return False
    return any(qualifies(t) for t in preview["tables"])


def main() -> None:
    hits = []
    reserved = 0
    for path in sorted(PREVIEWS.glob("*.json")):
        preview = json.loads(path.read_text())
        tables = [t for t in preview["tables"] if qualifies(t)]
        if tables and is_reserved(preview.get("doi"), preview["pmcid"]):
            reserved += 1
            continue
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
    print(len(hits), "articles qualify;", reserved, "qualifying articles reserved elsewhere")


if __name__ == "__main__":
    main()
