"""Mechanical axial-length triage rule over masked table previews (no values read).

Run from the repository root with ``uv run --frozen python
experiments/data/myopia-axial-2026-10-30/screening/triage_rule.py``.

A table qualifies when its caption/header/first-column labels mention ocular axial
length (axial length, AL, AXL, axial elongation or growth), a baseline reference and at
least one follow-up time label, and its caption has no exclusion term (statistical
models, reviews, agreement or normative studies, survival, prevalence or incidence).
Many myopia trials tabulate the baseline in a characteristics table and the changes in
another, so an article also qualifies when one table gives axial-length *changes* at a
follow-up time and another table gives the baseline axial length. An article qualifies
when it has such tables and it is not reserved for another component's untested sealed
or unread tranche (``reserved.py``). Prints qualifying articles with their masked
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

PREVIEWS = ROOT / ".peykar/myopia-table-screen-2026-10-30/previews"
TOPIC = re.compile(
    r"axial\s+(length|elongation|growth|change)|\bAL\b|\bAXL\b|\bA\.?L\.?\s*\(\s*mm|ocular\s+axial",
    re.I,
)
REFERENCE = re.compile(
    r"baseline|pre-?treatment|pre-?intervention|before\s+(treatment|the\s+treatment|"
    r"intervention|wearing|lens\s+wear|start)|\binitial\b|enrol?ment|\bentry\b|basic\s+value"
    r"|\bbasal\b|\b(month|week|visit)\s*0\b|\b[MW]\s*0\b|\b0\s*(months?|mos?|m|weeks?|w)\b"
    r"|\bpre\b",
    re.I,
)
CHANGE = re.compile(r"chang|elongat|increase|growth|progress|\u0394|delta", re.I)
# Hyphenated labels ("12-month") count as time labels too.
TIME = re.compile(
    r"\b(\d+)\s*[-\u2010\u2011\u2013]?\s*"
    r"(days?|d|weeks?|wks?|w|months?|mos?|m|years?|yrs?|y)\b"
    r"|\b(day|week|month|year|visit|[MWDY])\s*[-\u2010\u2011\u2013]?\s*(\d+)\b",
    re.I,
)
EXCLUDE = re.compile(
    r"regression\s+(analysis|model|coefficient)|multivariate|multivariable|linear\s+regression"
    r"|logistic|review|meta-?analys|correlat|risk\s+factor|predict|repeatab|reproducib"
    r"|agreement|reliab|normative|healthy\s+(subjects|volunteers|controls|eyes)"
    r"|survival|kaplan|prevalence|incidence",
    re.I,
)
UNITS = {
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


def _texts(table: dict) -> tuple[str, str, str] | None:
    """(caption, all labels, header and first-column labels), or None if excluded."""
    caption = f"{table.get('label', '')} {table.get('caption', '')}"
    if EXCLUDE.search(caption):
        return None
    head, firsts = labels(table)
    return caption, " ".join([caption, head, *firsts]), head + " " + " ".join(firsts)


def qualifies(table: dict) -> bool:
    """Axial length, its baseline and at least one follow-up time in one table."""
    texts = _texts(table)
    if texts is None:
        return False
    _, text, cells = texts
    return bool(TOPIC.search(text) and REFERENCE.search(text) and times(cells))


def change_table(table: dict) -> bool:
    """Axial-length changes at a follow-up time (the baseline may be elsewhere)."""
    texts = _texts(table)
    if texts is None:
        return False
    _, text, cells = texts
    return bool(TOPIC.search(text) and CHANGE.search(text) and times(cells))


def baseline_table(table: dict) -> bool:
    """Axial length with a baseline label (for example a characteristics table)."""
    texts = _texts(table)
    if texts is None:
        return False
    _, text, _ = texts
    return bool(TOPIC.search(text) and REFERENCE.search(text))


def qualifying_tables(preview: dict) -> list[dict]:
    """The tables that make an article qualify (empty if it does not)."""
    tables = [t for t in preview["tables"] if qualifies(t)]
    if tables:
        return tables
    changes = [t for t in preview["tables"] if change_table(t)]
    baselines = [t for t in preview["tables"] if baseline_table(t)]
    if changes and baselines:
        ids = {t["id"] for t in changes}
        return changes + [t for t in baselines if t["id"] not in ids]
    return []


def article_qualifies(preview: dict) -> bool:
    if is_reserved(preview.get("doi"), preview["pmcid"]):
        return False
    return bool(qualifying_tables(preview))


def main() -> None:
    hits = []
    reserved = 0
    for path in sorted(PREVIEWS.glob("*.json")):
        preview = json.loads(path.read_text())
        tables = qualifying_tables(preview)
        if tables and is_reserved(preview.get("doi"), preview["pmcid"]):
            reserved += 1
            continue
        if tables:
            hits.append((preview, tables))
    if "--json" in sys.argv:
        print(json.dumps([p["pmcid"] for p, _ in hits]))
        return
    if "--count" in sys.argv:
        direct = sum(any(qualifies(t) for t in p["tables"]) for p, _ in hits)
        print(
            len(hits),
            "articles qualify (",
            direct,
            "with one table,",
            len(hits) - direct,
            "by a change table and a baseline table );",
            reserved,
            "qualifying articles reserved elsewhere",
        )
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
