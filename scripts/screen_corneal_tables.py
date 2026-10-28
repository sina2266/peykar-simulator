"""Outcome-blind screen of article-body tables for corneal wound-closure time courses.

Explicit offline operator over full-text JATS XML already downloaded by
``screen_corneal_wound_literature.py``. A table is flagged when its caption or
header mentions corneal wound/defect area, closure or re-epithelialization and a
time unit. For every flagged table it writes a **masked preview**: caption,
header rows and first-column labels with outcome-like numbers hidden (see
``show_masked_previews.strict``), every data cell replaced by ``#`` when it holds a
digit, and row/column counts. The XML SHA-256 is recorded. No value is read,
parsed into numbers or scored. Output goes to a new directory.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
import xml.etree.ElementTree as ET
from datetime import UTC, datetime
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))
from show_masked_previews import strict  # noqa: E402

TOPIC = re.compile(
    r"(epithelial (defect|wound)|wound (area|size|closure|healing)|defect (area|size)"
    r"|re-?epitheliali|fluorescein[- ]stain|healing (rate|time)|epithelial (healing|closure))",
    re.I,
)
TIME = re.compile(
    r"\b(\d+\s*(h|hr|hrs|hours?|d|days?)\b|day\s*\d+|POD|post-?operative day|time point|weeks?)",
    re.I,
)
CORNEA = re.compile(r"cornea", re.I)
MAX_ROWS = 80


def _text(element: ET.Element | None) -> str:
    return " ".join("".join(element.itertext()).split()) if element is not None else ""


def masked_table(table_wrap: ET.Element) -> dict[str, Any]:
    rows = []
    numeric = 0
    for section in ("thead", "tbody"):
        for part in table_wrap.iter(section):
            for r, tr in enumerate(part.iter("tr")):
                cells = []
                for c, cell in enumerate(list(tr)):
                    text = _text(cell)
                    if section == "thead" or c == 0:
                        shown = strict(text)[:60]
                    elif re.search(r"\d", text):
                        shown = "#"
                        numeric += 1
                    else:
                        shown = text[:30]
                    cells.append(shown)
                if len(rows) < MAX_ROWS:
                    rows.append(f"{section[1:]}{r + 1}: " + " | ".join(cells))
    return {
        "id": table_wrap.get("id"),
        "label": _text(table_wrap.find("label")),
        "caption": strict(_text(table_wrap.find("caption")))[:900],
        "footnote": strict(_text(table_wrap.find("table-wrap-foot")))[:600],
        "rows": rows,
        "numeric_cells": numeric,
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fulltext", type=Path, action="append", required=True)
    parser.add_argument("--screen", type=Path, action="append", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--topic", help="regex replacing the wound-area topic pattern")
    parser.add_argument("--time", help="regex replacing the time-unit pattern")
    parser.add_argument("--context", help="regex replacing the cornea context pattern")
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    topic = re.compile(args.topic, re.I) if args.topic else TOPIC
    time_pattern = re.compile(args.time, re.I) if args.time else TIME
    context = re.compile(args.context, re.I) if args.context else CORNEA
    metadata: dict[str, dict[str, Any]] = {}
    for screen in args.screen:
        for item in json.loads(screen.read_text()):
            metadata.setdefault(item["pmcid"], item)
    (args.output / "previews").mkdir(parents=True)
    receipts = []
    seen: set[str] = set()
    for directory in args.fulltext:
        for path in sorted(directory.glob("*.xml")):
            if path.stem in seen:
                continue
            seen.add(path.stem)
            raw = path.read_bytes()
            try:
                root = ET.fromstring(raw)
            except ET.ParseError:
                receipts.append({"pmcid": path.stem, "outcome": "xml_parse_error"})
                continue
            title = _text(root.find(".//article-title"))
            flagged = []
            for table_wrap in root.iter("table-wrap"):
                caption = _text(table_wrap.find("caption"))
                header = " ".join(_text(th) for th in table_wrap.iter("th"))
                body = " ".join(_text(td) for td in list(table_wrap.iter("td"))[:40])
                text = f"{caption} {header} {body}"
                if (
                    topic.search(text)
                    and time_pattern.search(text)
                    and context.search(f"{text} {title}")
                ):
                    flagged.append(masked_table(table_wrap))
            if not flagged:
                continue
            meta = metadata.get(path.stem, {})
            preview = {
                "pmcid": path.stem,
                "doi": meta.get("doi"),
                "title": title,
                "license": meta.get("license"),
                "species": meta.get("species"),
                "methods": meta.get("methods"),
                "candidate_wound_diameters_mm": meta.get("candidate_wound_diameters_mm"),
                "fulltext": str(path),
                "fulltext_sha256": hashlib.sha256(raw).hexdigest(),
                "tables": flagged,
            }
            (args.output / "previews" / f"{path.stem}.json").write_text(
                json.dumps(preview, indent=1, ensure_ascii=False)
            )
            receipts.append(
                {"pmcid": path.stem, "outcome": "tables_flagged", "tables": len(flagged)}
            )
    summary = {
        "utc": datetime.now(UTC).isoformat(),
        "fulltexts_screened": len(seen),
        "articles_flagged": sum(r["outcome"] == "tables_flagged" for r in receipts),
        "tables_flagged": sum(r.get("tables", 0) for r in receipts),
        "rule": "caption/header/first cells match topic + time unit + cornea",
        "topic_pattern": topic.pattern,
        "time_pattern": time_pattern.pattern,
        "context_pattern": context.pattern,
        "scope": "Masked table previews only; no outcome value has been read or scored.",
    }
    (args.output / "receipts.json").write_text(json.dumps(receipts, indent=1))
    (args.output / "summary.json").write_text(json.dumps(summary, indent=1))
    print(json.dumps(summary, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
