"""Convert legacy BIFF .xls files into an inert cell inventory (JSON), offline.

Runs only inside the separate ``runtime_profiles/xls-reader`` environment. Each
non-empty cell is written with its sheet, A1 reference, raw value and xlrd type.
Formulas are not evaluated (xlrd returns cached results). No network access is
needed. The JSON output, not the xls file, is what Peykar readers consume.
"""

from __future__ import annotations

import hashlib
import json
import sys
from pathlib import Path

import xlrd  # type: ignore[import-not-found]


def letters(index: int) -> str:
    result = ""
    index += 1
    while index:
        index, remainder = divmod(index - 1, 26)
        result = chr(65 + remainder) + result
    return result


def convert(path: Path) -> dict[str, object]:
    raw = path.read_bytes()
    book = xlrd.open_workbook(file_contents=raw, on_demand=True)
    sheets = {}
    for sheet in book.sheets():
        cells = []
        for row in range(sheet.nrows):
            for column in range(sheet.ncols):
                cell = sheet.cell(row, column)
                if cell.ctype in (xlrd.XL_CELL_EMPTY, xlrd.XL_CELL_BLANK):
                    continue
                cells.append(
                    {
                        "ref": f"{letters(column)}{row + 1}",
                        "value": cell.value,
                        "type": xlrd.sheet.ctype_text.get(cell.ctype, str(cell.ctype)),
                    }
                )
        sheets[sheet.name] = cells
    return {
        "source": path.name,
        "source_sha256": hashlib.sha256(raw).hexdigest(),
        "reader": f"xlrd {xlrd.__version__}",
        "sheets": sheets,
    }


def main() -> int:
    failures = 0
    for argument in sys.argv[1:]:
        source = Path(argument)
        target = source.with_suffix(source.suffix + ".cells.json")
        try:
            payload = convert(source)
        except xlrd.XLRDError as error:
            # Many ".xls" supplements are tab-separated text; record, never guess.
            failures += 1
            payload = {
                "source": source.name,
                "source_sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
                "reader": f"xlrd {xlrd.__version__}",
                "error": str(error)[:300],
                "sheets": {},
            }
        target.write_text(json.dumps(payload, indent=0) + "\n")
        print(target, "error" if "error" in payload else "ok")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
