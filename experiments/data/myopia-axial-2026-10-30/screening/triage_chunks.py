"""Masked triage chunks for one axial-length tranche (structure only, no values).

Run from the repository root with ``uv run --frozen python
experiments/data/myopia-axial-2026-10-30/screening/triage_chunks.py --tranche 1
--output <new directory>``.

Articles of the tranche are taken in rank order and dealt round-robin into
``--chunks`` files. Each article shows its masked qualifying tables (data cells and
numbers in labels hidden by ``show_masked_previews.strict``).
"""

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
DATASET = ROOT / "experiments/data/myopia-axial-2026-10-30"
sys.path.insert(0, str(DATASET / "screening"))
sys.path.insert(0, str(ROOT / "scripts"))
from show_masked_previews import strict  # noqa: E402
from triage_rule import PREVIEWS, qualifying_tables  # noqa: E402

TITLE, LICENCE, CAPTION, FOOTNOTE, ROW, ROWS, TABLES = 200, 80, 300, 400, 220, 22, 4


def block(preview: dict) -> str:
    lines = [
        f"=== {preview['pmcid']} doi={preview.get('doi')} | "
        f"{strict(preview.get('title', ''))[:TITLE]}",
        f"    species={preview.get('species')} licence={str(preview.get('license'))[:LICENCE]}",
    ]
    for table in qualifying_tables(preview)[:TABLES]:
        caption = strict(table.get("caption", ""))[:CAPTION]
        lines.append(f"  [{table['id']}] {strict(table.get('label', ''))} | caption: {caption}")
        footnote = strict(table.get("footnote", ""))[:FOOTNOTE]
        if footnote:
            lines.append(f"     footnote: {footnote}")
        for row in table["rows"][:ROWS]:
            lines.append(f"     {strict(row)[:ROW]}")
    return "\n".join(lines)


def chunks(tranche: int, count: int) -> list[tuple[list[str], str]]:
    order = json.loads((DATASET / "tranche-order-1.0.0.json").read_text())
    ids = [
        a["pmcid"]
        for a in sorted(order["articles"], key=lambda a: a["rank"])
        if a["tranche"] == tranche
    ]
    out = []
    for k in range(count):
        members = ids[k::count]
        text = "\n".join(
            block(json.loads((PREVIEWS / f"{pmcid}.json").read_text())) for pmcid in members
        )
        out.append((members, text + "\n"))
    return out


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tranche", type=int, required=True)
    parser.add_argument("--chunks", type=int, default=3)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise SystemExit("--output must be a new directory")
    args.output.mkdir(parents=True)
    for k, (members, text) in enumerate(chunks(args.tranche, args.chunks), start=1):
        (args.output / f"chunk-{k}-ids.json").write_text(json.dumps(members) + "\n")
        (args.output / f"chunk-{k}.txt").write_text(text)
        print(k, len(members), len(text))


if __name__ == "__main__":
    main()
