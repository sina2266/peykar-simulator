"""Convert kept registry tranche-1 records into JATS-like XML tables (deterministic).

Run from the repository root with ``uv run --frozen python
experiments/data/myopia-axial-2026-10-30/registry-1/registry_tables.py --raw <fetch dir>/raw
--screen <screen file> --output <new dir>``.

Each kept ClinicalTrials.gov record becomes one file ``<NCT number>.xml`` whose
``<table-wrap>`` tables custodians cite as ``<NCT number>.xml::<id>!R<r>C<c>``, read by the
unchanged record builder (rows are every ``<tr>`` in document order, cells every child of
a row, both from 1):

- ``a1`` arms: one row per arm group (label, type, description, interventions);
- ``b1`` baseline measures: a header row of the baseline groups, one row per denominator
  ("number analysed (<units>)"), then one row per measure, class and category;
- ``o1``, ``o2``, ... one table per posted outcome measure in posted order, its caption
  giving the title, description, time frame, population, parameter, dispersion, unit and
  units analysed; a header row of the outcome groups, one row per denominator, then one
  row per class and category.

A measurement cell reads ``value``, ``value ± spread`` or ``value (lower to upper)`` as
posted (``NA`` without a value), so its leading number is the posted value. The raw
record's SHA-256 is written into the file. Nothing is printed but file names and counts.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path
from typing import Any
from xml.sax.saxutils import escape

VERSION = "1.0.0"


def _cell(text: object, tag: str = "td") -> str:
    return f"<{tag}>{escape(' '.join(str(text or '').split()))}</{tag}>"


def _row(cells: list[str]) -> str:
    return "<tr>" + "".join(cells) + "</tr>"


def _measurement(item: dict[str, Any] | None) -> str:
    if not item or item.get("value") in (None, ""):
        return "NA"
    text = str(item["value"])
    if item.get("spread") not in (None, ""):
        text += f" ± {item['spread']}"
    elif item.get("lowerLimit") not in (None, "") or item.get("upperLimit") not in (None, ""):
        text += f" ({item.get('lowerLimit', 'NA')} to {item.get('upperLimit', 'NA')})"
    return text


def _denominators(denoms: list[dict[str, Any]], groups: list[str]) -> list[str]:
    rows = []
    for denom in denoms:
        counts = {c["groupId"]: c.get("value") for c in denom.get("counts", [])}
        label = f"number analysed ({denom.get('units', 'participants')})"
        rows.append(_row([_cell(label), *(_cell(counts.get(g, "NA")) for g in groups)]))
    return rows


def _classes(classes: list[dict[str, Any]], prefix: str, groups: list[str]) -> list[str]:
    rows = []
    for klass in classes:
        class_rows = _denominators(klass.get("denoms", []), groups)
        rows.extend(
            row.replace("<td>number analysed", "<td>class denominator: number analysed", 1)
            for row in class_rows
        )
        for category in klass.get("categories", []):
            values = {m["groupId"]: m for m in category.get("measurements", [])}
            label = " | ".join(
                part for part in (prefix, klass.get("title"), category.get("title")) if part
            )
            rows.append(_row([_cell(label), *(_cell(_measurement(values.get(g))) for g in groups)]))
    return rows


def _wrap(table_id: str, label: str, caption: str, header: list[str], rows: list[str]) -> str:
    return (
        f'<table-wrap id="{table_id}"><label>{escape(label)}</label>'
        f"<caption><p>{escape(' '.join(caption.split()))}</p></caption><table><thead>"
        + _row([_cell(h, "th") for h in header])
        + "</thead><tbody>"
        + "".join(rows)
        + "</tbody></table></table-wrap>"
    )


def convert(study: dict[str, Any], raw_sha256: str) -> bytes:
    protocol = study["protocolSection"]
    results = study.get("resultsSection", {})
    nct = protocol["identificationModule"]["nctId"]
    parts = []
    arms = protocol.get("armsInterventionsModule", {}).get("armGroups", [])
    arm_rows = [
        _row(
            [
                _cell(a.get("label")),
                _cell(a.get("type")),
                _cell(a.get("description")),
                _cell("; ".join(a.get("interventionNames", []))),
            ]
        )
        for a in arms
    ]
    parts.append(
        _wrap("a1", "Arms", "Arm groups", ["arm", "type", "description", "interventions"], arm_rows)
    )
    baseline = results.get("baselineCharacteristicsModule", {})
    groups = baseline.get("groups", [])
    ids = [g["id"] for g in groups]
    rows = _denominators(baseline.get("denoms", []), ids)
    for measure in baseline.get("measures", []):
        prefix = " | ".join(
            str(part)
            for part in (
                measure.get("title"),
                f"{measure.get('paramType', '')} {measure.get('dispersionType', '')}".strip(),
                measure.get("unitOfMeasure"),
            )
            if part
        )
        rows.extend(_classes(measure.get("classes", []), prefix, ids))
    header = ["baseline measure", *(f"{g['id']}: {g.get('title', '')}" for g in groups)]
    parts.append(_wrap("b1", "Baseline", baseline.get("populationDescription", ""), header, rows))
    measures = results.get("outcomeMeasuresModule", {}).get("outcomeMeasures", [])
    for index, measure in enumerate(measures, start=1):
        groups = measure.get("groups", [])
        ids = [g["id"] for g in groups]
        caption = "; ".join(
            f"{name}: {measure.get(key)}"
            for name, key in (
                ("title", "title"),
                ("description", "description"),
                ("time frame", "timeFrame"),
                ("population", "populationDescription"),
                ("parameter", "paramType"),
                ("dispersion", "dispersionType"),
                ("unit", "unitOfMeasure"),
                ("units analysed", "typeUnitsAnalyzed"),
                ("type", "type"),
            )
            if measure.get(key)
        )
        rows = _denominators(measure.get("denoms", []), ids)
        rows.extend(_classes(measure.get("classes", []), "", ids))
        header = ["row", *(f"{g['id']}: {g.get('title', '')}" for g in groups)]
        parts.append(_wrap(f"o{index}", f"Outcome {index}", caption, header, rows))
    body = "".join(parts)
    xml = (
        '<?xml version="1.0" encoding="UTF-8"?>'
        f'<article registry="ClinicalTrials.gov" nct="{nct}" converter="registry_tables.py '
        f'{VERSION}" raw_sha256="{raw_sha256}"><body>{body}</body></article>\n'
    )
    return xml.encode("utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--raw", type=Path, required=True)
    parser.add_argument("--screen", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    screen = json.loads(args.screen.read_text())
    args.output.mkdir(parents=True)
    written = []
    for row in screen["records"]:
        if row["decision"] != "keep":
            continue
        path = args.raw / row["file"]
        body = path.read_bytes()
        digest = hashlib.sha256(body).hexdigest()
        if digest != row["sha256"]:
            raise SystemExit(f"{path.name} differs from the screen")
        xml = convert(json.loads(body), digest)
        target = args.output / f"{row['nct']}.xml"
        target.write_bytes(xml)
        written.append({"nct": row["nct"], "sha256": hashlib.sha256(xml).hexdigest()})
    (args.output / "tables.json").write_text(
        json.dumps({"converter_version": VERSION, "files": written}, indent=1) + "\n"
    )
    print(json.dumps({"files": len(written)}))
    return 0


if __name__ == "__main__":
    sys.exit(main())
