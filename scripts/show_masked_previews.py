"""Print masked supplementary previews for outcome-blind triage.

Previews written by ``acquire_corneal_wound_supplements.py`` hide numeric cells and
pure numeric text. This display additionally hides numbers embedded in text cells
that could carry outcomes: decimals, percentages, ``±``/SD expressions and any text
containing ``=`` (equations). Integers in labels (``Day 3``, ``Mouse 2``, ``24 h``)
stay readable. Screen metadata (species/method keywords, wound sizes, figure and
supplement captions) is printed next to each study. Nothing is written.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

_EMBEDDED = re.compile(r"[-+±]?\d+[.,]\d+|\d+\s*%|±\s*\d+|\d+\s*/\s*\d+|\d+\s+of\s+\d+")


def strict(text: str) -> str:
    """Hide outcome-like numbers inside a label string."""
    if "=" in text or "±" in text or "+/-" in text:
        return re.sub(r"\d", "#", text)
    return _EMBEDDED.sub("#~", text)


def show(pmcid: str, preview: dict[str, Any], screen: dict[str, Any] | None, rows: int) -> str:
    lines = [f"=== {pmcid} {preview.get('title', '')[:110]}"]
    if screen:
        lines.append(
            f"  doi={screen.get('doi')} year={screen.get('pub_year')}"
            f" species={screen.get('species')} methods={screen.get('methods')}"
            f" diam={screen.get('candidate_wound_diameters_mm')}"
        )
        for figure in screen.get("wound_time_figures", [])[:4]:
            lines.append(f"  FIG: {strict(str(figure))[:300]}")
        for supplement in screen.get("supplements", [])[:6]:
            caption = strict(f"{supplement.get('label', '')} {supplement.get('caption', '')}")
            lines.append(f"  SUP: {caption[:200]} {supplement.get('hrefs')}")
    for name, member in (preview.get("previews") or {}).items():
        if "sheets" not in member:
            lines.append(f"  [{name}] {json.dumps(member)[:160]}")
            continue
        for sheet, content in member["sheets"].items():
            lines.append(
                f"  [{name}::{sheet}] cells={content.get('cells')} "
                f"numeric={content.get('numeric_cells')} rows={content.get('max_row')} "
                f"cols={content.get('max_column')} hits={member.get('keyword_hits')}"
            )
            for row in content.get("rows", [])[:rows]:
                cells = []
                for segment in row.split(" | "):
                    reference, _, text = segment.partition(":")
                    cells.append(f"{reference}:{strict(text)}")
                lines.append(f"     {' | '.join(cells)[:220]}")
    for key in ("outcome", "error", "note"):
        if key in preview:
            lines.append(f"  {key}: {preview[key]}")
    return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--previews", type=Path, required=True)
    parser.add_argument("--screen", type=Path)
    parser.add_argument("--pmcids", nargs="*")
    parser.add_argument("--rows", type=int, default=14)
    parser.add_argument("--only-data", action="store_true", help="skip studies without sheets")
    args = parser.parse_args(argv)
    screen = {}
    if args.screen:
        screen = {item["pmcid"]: item for item in json.loads(args.screen.read_text())}
    paths = sorted(args.previews.glob("*.json"))
    for path in paths:
        pmcid = path.stem
        if args.pmcids and pmcid not in args.pmcids:
            continue
        preview = json.loads(path.read_text())
        has_sheets = any("sheets" in m for m in (preview.get("previews") or {}).values())
        if args.only_data and not has_sheets:
            continue
        print(show(pmcid, preview, screen.get(pmcid), args.rows))
    return 0


if __name__ == "__main__":
    sys.exit(main())
