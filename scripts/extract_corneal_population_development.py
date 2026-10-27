"""Build development study records for the corneal population dataset (offline).

Reviewed, study-specific cell mappings for the two newly admitted development
studies. Every value is read from its original workbook cell and cited; no value
is typed by hand. Raw files are copied byte-for-byte with SHA-256. Records follow
extraction protocol 1.0.0 and are validated before writing. Consumed earlier
studies keep their existing adapters and are not rewritten here.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import sys
from pathlib import Path
from typing import Any

from peykar.data_fabric.spreadsheet import Cell, read_xlsx
from peykar.validation.cornea_population_data import validate_study

ROOT = Path(".peykar")
DIO = ROOT / "corneal-supplements-2026-10-27/files/PMC7473521/pone.0238750.s001.xlsx"
KETAMINE = (
    ROOT / "corneal-supplements-extra-2026-10-27/files/PMC9744879/41598_2022_24639_MOESM1_ESM.xlsx"
)


def _cells(path: Path, sheet: str) -> dict[str, Cell]:
    return {cell.ref: cell for cell in read_xlsx(path.read_bytes())[sheet]}


def _measured(cells: dict[str, Cell], ref: str) -> float | None:
    cell = cells.get(ref)
    if cell is None:
        return None
    if cell.is_formula or not cell.is_number:
        raise ValueError(f"{ref} is not a measured numeric cell")
    return float(cell.value)


def _source(path: Path, origin: str) -> dict[str, Any]:
    raw = path.read_bytes()
    return {
        "name": path.name,
        "sha256": hashlib.sha256(raw).hexdigest(),
        "bytes": len(raw),
        "origin": origin,
    }


def dio_record() -> dict[str, Any]:
    sheet = "Fig 5C"
    cells = _cells(DIO, sheet)
    hours = {"D": 0.0, "E": 18.0, "F": 24.0, "G": 30.0}
    for column, label in zip("DEFG", ("0h", "18h", "24h", "30h"), strict=True):
        header = str(cells[f"{column}5"].value)
        if not header.startswith(label) or not str(cells[f"{column}18"].value).startswith(label):
            raise ValueError("DIO Fig 5C time header layout changed")
    series = []
    duplicates = []
    for group, rows in (("normal diet", range(6, 14)), ("high fat diet", range(19, 27))):
        for row in rows:
            animal = str(cells[f"C{row}"].value)
            values, cited, used_hours = [], [], []
            for column, hour in hours.items():
                value = _measured(cells, f"{column}{row}")
                if value is None:
                    continue
                values.append(value)
                cited.append(f"{sheet}!{column}{row}")
                used_hours.append(hour)
            series.append(
                {
                    "series_id": f"{group.replace(' ', '-')}/{animal}",
                    "group": group,
                    "kind": "individual",
                    "value_kind": "area_absolute",
                    "hours": used_hours,
                    "values": values,
                    "source_cells": cited,
                    "baseline_observed": True,
                    "animal_label": animal,
                }
            )
    normal = [s for s in series if s["group"] == "normal diet"]
    for first, second in zip(normal[:4], normal[4:], strict=True):
        if first["values"][1] == second["values"][1]:
            duplicates.append(f"{first['animal_label']}={second['animal_label']} at 18h")
    return {
        "schema": "cornea.wound-study-record-1.0.0",
        "study_id": "dio-mouse-plos-one-2020",
        "doi": "10.1371/journal.pone.0238750",
        "pmcid": "PMC7473521",
        "title": "Corneal dysfunction precedes the onset of hyperglycemia in a mouse model of "
        "diet-induced obesity",
        "context": "mouse",
        "split": "development",
        "license": {
            "statement": "CC0 1.0 public domain dedication (article and S1 Dataset)",
            "url": "https://creativecommons.org/publicdomain/zero/1.0/",
        },
        "source_files": [_source(DIO, "Europe PMC supplementary archive, S1 Dataset")],
        "species": "Mus musculus C57BL/6, male, 6 weeks old at diet start",
        "wound": {
            "diameter_mm": 2.0,
            "basement_membrane": "unknown",
            "method": "central 2 mm trephine mark, abraded with golf-club spud",
            "injury_family": "epithelial_debridement",
            "location": "central",
        },
        "measurement": {
            "stain": "fluorescein",
            "quantity": "open wound area in pixels (Fiji), normalized to each cornea's 0 h area",
            "schedule": "at injury, then every 6 h starting 18 h until healed",
        },
        "groups": [
            {
                "group": "normal diet",
                "role": "control",
                "description": "10 days normal diet before wounding",
                "n_reported": 8,
            },
            {
                "group": "high fat diet",
                "role": "treated",
                "description": "10 days high-fat diet before wounding",
                "n_reported": 8,
            },
        ],
        "series": series,
        "exclusions": [
            {"what": "Fig 5C percentage columns H-K", "reason": "formulas derived from pixels"},
            {"what": "Average/STDV rows 15-16 and 28-29", "reason": "summary formulas"},
            {
                "what": "Fig 5A, 5B, 5D, Fig 6",
                "reason": "venule diameter, mitosis, thickness, immune cells",
            },
        ],
        "notes": [
            "Data-quality flag (not used to exclude): normal-diet 18 h pixel values repeat "
            "exactly between corneas " + "; ".join(duplicates) + ", and No3/No4 share the "
            "same 0 h value. Possible spreadsheet copy error in the deposited data.",
            "Wounds were closed (0 pixels) at 24 h in every normal-diet cornea; zeros kept.",
        ],
    }


def ketamine_record() -> dict[str, Any]:
    sheet = "Sheet1"
    cells = _cells(KETAMINE, sheet)
    columns = "BCDEFGHI"
    for header_row in (3, 18):
        labels = [str(cells[f"{c}{header_row}"].value) for c in columns]
        if labels != [f"day {d}" for d in range(8)]:
            raise ValueError("Ketamine day header layout changed")
    series = []
    missing = []
    for group, rows in (("ketamine", range(4, 17)), ("saline", range(19, 33))):
        for row in rows:
            if _measured(cells, f"B{row}") is None:
                continue  # rat label row without measurements (not in analysed groups)
            animal = int(_measured(cells, f"A{row}") or 0)
            values, cited, used_hours = [], [], []
            for day, column in enumerate(columns):
                value = _measured(cells, f"{column}{row}")
                if value is None:
                    missing.append(f"{group} rat {animal} day {day}")
                    continue
                values.append(value)
                cited.append(f"{sheet}!{column}{row}")
                used_hours.append(24.0 * day)
            series.append(
                {
                    "series_id": f"{group}/rat-{animal}",
                    "group": group,
                    "kind": "individual",
                    "value_kind": "area_absolute",
                    "hours": used_hours,
                    "values": values,
                    "source_cells": cited,
                    "baseline_observed": True,
                    "animal_label": animal,
                }
            )
    # Control rat 15 appears only in Sheet2's otherwise identical defect block.
    second = _cells(KETAMINE, "Sheet2")
    if [str(second[f"{c}13"].value) for c in columns] != [f"day {d}" for d in range(8)]:
        raise ValueError("Ketamine Sheet2 control header layout changed")
    for row in range(14, 21):
        animal = int(_measured(second, f"A{row}") or 0)
        existing = [s for s in series if s["series_id"] == f"saline/rat-{animal}"]
        values, cited, used_hours = [], [], []
        for day, column in enumerate(columns):
            value = _measured(second, f"{column}{row}")
            if value is not None:
                values.append(value)
                cited.append(f"Sheet2!{column}{row}")
                used_hours.append(24.0 * day)
        if existing:
            if existing[0]["values"] != values or existing[0]["hours"] != used_hours:
                raise ValueError(f"Sheet1/Sheet2 disagree for control rat {animal}")
            continue
        series.append(
            {
                "series_id": f"saline/rat-{animal}",
                "group": "saline",
                "kind": "individual",
                "value_kind": "area_absolute",
                "hours": used_hours,
                "values": values,
                "source_cells": cited,
                "baseline_observed": True,
                "animal_label": animal,
            }
        )
    return {
        "schema": "cornea.wound-study-record-1.0.0",
        "study_id": "ketamine-rat-sci-rep-2022",
        "doi": "10.1038/s41598-022-24639-y",
        "pmcid": "PMC9744879",
        "title": "The effect of topical ketamine administration on the corneal epithelium repair",
        "context": "rat",
        "split": "development",
        "license": {
            "statement": "CC BY 4.0 (article and supplementary information)",
            "url": "https://creativecommons.org/licenses/by/4.0/",
        },
        "source_files": [_source(KETAMINE, "Europe PMC supplementary archive, MOESM1")],
        "species": "Rattus norvegicus, male, 200 g; right eyes only",
        "wound": {
            "diameter_mm": 5.0,
            "diameter_basis": "ASSUMED: whole corneal epithelium removed; diameter not reported",
            "basement_membrane": "unknown",
            "method": "20% ethanol 15 s then No.15 blade, whole corneal epithelium removed",
            "injury_family": "epithelial_debridement",
            "location": "whole cornea",
        },
        "measurement": {
            "stain": "fluorescein strips",
            "quantity": "epithelial defect size (caliper and ImageJ; unit not stated), "
            "normalized to each rat's day-0 value",
            "schedule": "daily photographs, day 0-7",
        },
        "groups": [
            {
                "group": "saline",
                "role": "control",
                "description": "0.9% NaCl drop every 6 h; chloramphenicol 0.5% every 12 h",
                "n_reported": 7,
            },
            {
                "group": "ketamine",
                "role": "treated",
                "description": "ketamine 50 mg/ml drop every 6 h for 7 days; chloramphenicol",
                "n_reported": 8,
            },
        ],
        "series": series,
        "exclusions": [
            {
                "what": "Sheet2 rows other than control rat 15",
                "reason": "identical to Sheet1 (checked cell by cell) plus histopathology grades",
            },
            {"what": "missing day cells", "reason": "not reported; dropped, never filled"},
        ],
        "notes": [
            "Missing daily cells (dropped, not interpolated): " + "; ".join(missing),
            "Integer-resolution defect sizes; some later values exceed earlier ones.",
            "Wound diameter 5.0 mm is an explicit assumption for whole-cornea removal.",
        ],
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    records_dir = args.output / "records"
    raw_dir = args.output / "raw"
    records_dir.mkdir(parents=True, exist_ok=True)
    raw_dir.mkdir(parents=True, exist_ok=True)
    for record, source in ((dio_record(), DIO), (ketamine_record(), KETAMINE)):
        validate_study(record)
        target = records_dir / f"{record['study_id']}.json"
        target.write_text(json.dumps(record, indent=1, ensure_ascii=False) + "\n")
        copy = raw_dir / record["study_id"] / source.name
        copy.parent.mkdir(exist_ok=True)
        shutil.copyfile(source, copy)
        print(target, len(record["series"]), "series")
    return 0


if __name__ == "__main__":
    sys.exit(main())
