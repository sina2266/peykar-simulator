"""Write the extraction specs of the 1.1.0 development studies (protocol 1.1.0).

Specs cite table cells of each article's JATS XML (``file::table!RrCc``); they hold
no outcome values. ``scripts/build_corneal_table_record.py`` resolves the cells.
Cell positions were chosen from masked table structure (data cells hidden).
"""

# ruff: noqa: E501  (descriptive provenance strings are kept on one line)

from __future__ import annotations

import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[4]
FULLTEXT = ROOT / ".peykar/corneal-literature-screen-2026-10-27-concurrent/raw/fulltext"
BY = "https://creativecommons.org/licenses/by/4.0/"


def source(pmcid: str) -> dict:
    raw = (FULLTEXT / f"{pmcid}.xml").read_bytes()
    return {
        "name": f"{pmcid}.xml",
        "sha256": hashlib.sha256(raw).hexdigest(),
        "bytes": len(raw),
        "origin": f"Europe PMC fullTextXML for {pmcid}",
    }


def cells(pmcid: str, table: str, refs: list[tuple[int, int]]) -> list[str]:
    return [f"{pmcid}.xml::{table}!R{r}C{c}" for r, c in refs]


def healed(pmcid: str, table: str, refs: list[tuple[int, int]]) -> list[dict]:
    """Protocol 1.1.1: remaining % = 100 - measured healed % (non-zero hour 0)."""
    return [{"transform": "100-minus", "cell": cell} for cell in cells(pmcid, table, refs)]


def series(sid, group, n, kind, hours, cell_refs, **extra):
    item = {
        "series_id": sid,
        "group": group,
        "kind": "group_mean",
        "n": n,
        "value_kind": kind,
        "hours": hours,
        "cells": cell_refs,
        "baseline_observed": True,
    }
    item.update(extra)
    return item


def base(study_id, doi, pmcid, title, context, licence, licence_url, species, wound, groups):
    return {
        "study_id": study_id,
        "doi": doi,
        "pmcid": pmcid,
        "title": title,
        "context": context,
        "split": "development",
        "license": {"statement": licence, "url": licence_url},
        "source_files": [source(pmcid)],
        "species": species,
        "wound": wound,
        "groups": groups,
        "extraction_protocol": "extraction-protocol-1.1.1",
        "series": [],
    }


def propolis():
    pm, t = "PMC4224050", "T1"
    spec = base(
        "propolis-rat-alkali-bmc-cam-2013",
        "10.1186/1472-6882-13-337",
        pm,
        "Topical Brazilian propolis improves corneal wound healing and inflammation in rats",
        "rat",
        "CC BY 2.0",
        "https://creativecommons.org/licenses/by/2.0/",
        "Wistar rats, male, 250-300 g; right eyes",
        {
            "diameter_mm": 1.0,
            "diameter_basis": "stated: discrete lesion of 1 mm diameter",
            "basement_membrane": "unknown",
            "method": "silver nitrate/potassium nitrate applicator 2 s on central cornea (authors: alkali burn)",
            "injury_family": "other",
        },
        [
            {
                "group": "vehicle",
                "role": "control",
                "n_reported": 40,
                "description": "microemulsion without propolis",
            },
            {
                "group": "propolis",
                "role": "treated",
                "n_reported": 40,
                "description": "1% Brazilian propolis microemulsion",
            },
        ],
    )
    hours = [0.0, 12.0, 24.0, 48.0, 120.0]
    rows = [2, 3, 4, 5, 6]
    for group, col in (("vehicle", 2), ("propolis", 3)):
        spec["series"].append(
            series(
                f"{group}/mean",
                group,
                40,
                "area_absolute",
                hours,
                cells(pm, t, [(r, col) for r in rows]),
                notes="mean ± SEM, mm2; n = 40 per group stated for the examined subset",
            )
        )
    return spec


def aloe():
    pm, t = "PMC7001018", "T2"
    spec = base(
        "aloe-vera-rabbit-alkali-jovr-2020",
        "10.18502/jovr.v15i1.5932",
        pm,
        "Aloe Vera Gel-derived Eye Drops for Alkaline Corneal Injury in a Rabbit Model",
        "rabbit",
        "Creative Commons Attribution License (article statement)",
        "https://creativecommons.org/licenses/by/4.0/",
        "New Zealand albino rabbits, ~2.5 kg; right eyes",
        {
            "diameter_mm": 6.0,
            "diameter_basis": "stated: 6 mm round filter paper",
            "basement_membrane": "unknown",
            "method": "6 mm filter paper with 2.5 N NaOH for 30 s, 3 min saline irrigation",
            "injury_family": "alkali_burn",
        },
        [
            {
                "group": "control",
                "role": "control",
                "n_reported": 10,
                "description": "sterile normal saline drops four times daily",
            },
            {
                "group": "medical-therapy",
                "role": "treated",
                "n_reported": 10,
                "description": "chloramphenicol, betamethasone, sodium citrate, ascorbic acid, atropine",
            },
            {
                "group": "aloe-vera",
                "role": "treated",
                "n_reported": 10,
                "description": "Aloe vera gel-derived drops four times daily",
            },
        ],
    )
    hours = [0.0, 24.0, 48.0, 96.0, 168.0]
    rows = [4, 5, 9, 13, 17]
    for group, col in (("control", 2), ("medical-therapy", 3), ("aloe-vera", 4)):
        spec["series"].append(
            series(
                f"{group}/mean",
                group,
                10,
                "area_absolute",
                hours,
                cells(pm, t, [(r, col) for r in rows]),
                notes="mean ± SD, mm2",
            )
        )
    return spec


def fl2():
    pm, t = "PMC7804583", "tbl1"
    spec = base(
        "fl2-rat-alkali-tvst-2021",
        "10.1167/tvst.10.1.17",
        pm,
        "A Novel Therapeutic Approach to Corneal Alkaline Burn Model by Targeting Fidgetin-Like 2",
        "rat",
        "CC BY-NC-ND 4.0 (derived record kept private)",
        "https://creativecommons.org/licenses/by-nc-nd/4.0/",
        "Sprague-Dawley rats, male, 12 weeks; right eyes",
        {
            "diameter_mm": 5.0,
            "diameter_basis": (
                "ASSUMED: whole corneal epithelium removed before a 4 mm NaOH disc; values "
                "are % of the corneal surface; 5 mm rat cornea as in ketamine-rat record"
            ),
            "basement_membrane": "unknown",
            "method": "epithelium removed with foam tip, then 4 mm 1 M NaOH filter disc 10 s",
            "injury_family": "alkali_burn",
        },
        [
            {"group": "prednisone", "role": "treated", "n_reported": 6},
            {
                "group": "empty-np",
                "role": "control",
                "n_reported": 6,
                "description": "empty nanoparticles (vehicle)",
            },
            {
                "group": "control-np-si-1um",
                "role": "control",
                "n_reported": 6,
                "description": "non-targeting siRNA nanoparticles (negative-control reagent)",
            },
            {
                "group": "control-np-si-10um",
                "role": "control",
                "n_reported": 6,
                "description": "non-targeting siRNA nanoparticles",
            },
            {
                "group": "control-np-si-20um",
                "role": "control",
                "n_reported": 6,
                "description": "non-targeting siRNA nanoparticles",
            },
            {"group": "fl2-np-si-1um", "role": "treated", "n_reported": 6},
            {"group": "fl2-np-si-10um", "role": "treated", "n_reported": 6},
            {"group": "fl2-np-si-20um", "role": "treated", "n_reported": 6},
        ],
    )
    spec["reservation_key"] = "fl2-mouse-alkali-tvst-2021"
    spec["reservation_key_note"] = "triage/reservation label says mouse; Methods state rats"
    hours = [0.0, 48.0, 96.0, 144.0, 192.0, 240.0, 288.0, 336.0]
    groups = [
        "prednisone",
        "empty-np",
        "control-np-si-1um",
        "control-np-si-10um",
        "control-np-si-20um",
        "fl2-np-si-1um",
        "fl2-np-si-10um",
        "fl2-np-si-20um",
    ]
    for row, group in zip(range(3, 11), groups, strict=True):
        spec["series"].append(
            series(
                f"{group}/mean",
                group,
                6,
                "area_absolute",
                hours,
                cells(pm, t, [(row, c) for c in range(2, 10)]),
                notes="% of corneal surface with defect, normalized by the day-0 value; N = 6 for clinical analysis",
            )
        )
    return spec


def epo():
    pm, t = "PMC11082528", "T1"
    spec = base(
        "epo-rabbit-alkali-jovr-2023",
        "10.18502/jovr.v18i3.13772",
        pm,
        "Risk of Induction of Corneal Neovascularization with Topical Erythropoietin",
        "rabbit",
        "Creative Commons Attribution License (article statement)",
        "https://creativecommons.org/licenses/by/4.0/",
        "New Zealand albino rabbits, male, 12-18 months",
        {
            "diameter_mm": 8.0,
            "diameter_basis": "stated: 8 mm filter paper disc",
            "basement_membrane": "unknown",
            "method": "8 mm filter paper soaked 10 s in 1 M NaOH, applied 60 s, PBS irrigation",
            "injury_family": "alkali_burn",
        },
        [
            {
                "group": "saline",
                "role": "control",
                "n_reported": 5,
                "description": "normal saline every 8 h; ciprofloxacin until re-epithelialization",
            },
            {
                "group": "erythropoietin",
                "role": "treated",
                "n_reported": 5,
                "description": "topical erythropoietin 3000 IU/mL every 8 h; ciprofloxacin",
            },
        ],
    )
    hours = [0.0, 72.0, 168.0, 336.0, 720.0, 1440.0]
    rows = [4, 5, 6, 7, 8, 9]
    for group, col in (("erythropoietin", 2), ("saline", 3)):
        spec["series"].append(
            series(
                f"{group}/mean",
                group,
                5,
                "area_absolute",
                hours,
                cells(pm, t, [(r, col) for r in rows]),
                notes="mean ± SD, mm2",
            )
        )
    return spec


def pje_blue():
    pm, t = "PMC10295055", "antioxidants-12-01171-t003"
    spec = base(
        "pje-bluelight-rat-antioxidants-2023",
        "10.3390/antiox12061171",
        pm,
        "Therapeutic Potential of Peucedanum japonicum Thunb. in a Delayed Corneal Wound Healing Model",
        "rat",
        "CC BY 4.0",
        BY,
        "rats; wounded right eye (OD)",
        {
            "diameter_mm": 4.0,
            "diameter_basis": (
                "ASSUMED: not stated (methods cite an earlier report); the same laboratory's "
                "particulate-matter study (PMC8614870) used a 4 mm Alger brush wound"
            ),
            "basement_membrane": "unknown",
            "method": "corneal epithelial wound (method by reference), fluorescein-stained healed area",
            "injury_family": "epithelial_debridement",
        },
        [
            {
                "group": "normal",
                "role": "control",
                "n_reported": 10,
                "description": "wounded right eye, no blue light, no extract (NR)",
            },
            {
                "group": "blue-light",
                "role": "treated",
                "n_reported": 10,
                "description": "blue light 5000 lux for 48 h",
            },
            *[
                {
                    "group": f"pje-{d}",
                    "role": "treated",
                    "n_reported": 10,
                    "description": f"PJE {d} mg/kg plus blue light",
                }
                for d in (25, 50, 100, 200)
            ],
        ],
    )
    hours = [0.0, 12.0, 24.0, 36.0, 48.0]
    spec["series"].append(
        series(
            "normal/mean",
            "normal",
            10,
            "area_absolute",
            hours,
            healed(pm, t, [(3, c) for c in (3, 5, 7, 9, 11)]),
            notes="100 - healed area (%) (protocol 1.1.1); hour-0 healed is measured, non-zero; mean ± SD",
        )
    )
    for row, group in zip(
        range(4, 9), ["blue-light", "pje-25", "pje-50", "pje-100", "pje-200"], strict=True
    ):
        spec["series"].append(
            series(
                f"{group}/mean",
                group,
                10,
                "area_absolute",
                hours,
                healed(pm, t, [(row, c) for c in (2, 4, 6, 8, 10)]),
                notes="100 - healed area (%) (protocol 1.1.1); hour-0 healed is measured, non-zero; mean ± SD",
            )
        )
    return spec


def pje_upm():
    pm, t = "PMC8614870", "antioxidants-10-01717-t001"
    spec = base(
        "pje-upm-rat-antioxidants-2021",
        "10.3390/antiox10111717",
        pm,
        "Peucedanum japonicum Thunberg Mitigates Oxidative Stress in Particulate-Matter Corneal Injury",
        "rat",
        "CC BY 4.0",
        BY,
        "rats; wounded right eye",
        {
            "diameter_mm": 4.0,
            "diameter_basis": "stated: 4 mm diameter wound with an Alger brush rotating burr",
            "basement_membrane": "unknown",
            "method": "Alger brush rotating burr, 4 mm",
            "injury_family": "epithelial_debridement",
        },
        [
            {
                "group": "normal",
                "role": "control",
                "n_reported": 6,
                "description": "wounded right eye without particulate matter (NR)",
            },
            {
                "group": "particulate-matter",
                "role": "treated",
                "n_reported": 6,
                "description": "urban particulate matter drops 4x/day from 5 days before wounding",
            },
            *[
                {
                    "group": f"pje-{d}",
                    "role": "treated",
                    "n_reported": 6,
                    "description": f"PJE {d} mg/kg plus particulate matter",
                }
                for d in (25, 50, 100, 200, 400)
            ],
        ],
    )
    hours = [0.0, 8.0, 16.0, 24.0, 36.0, 48.0, 72.0]
    even = (2, 4, 6, 8, 10, 12, 14)
    spec["series"].append(
        series(
            "normal/mean",
            "normal",
            6,
            "area_absolute",
            hours,
            healed(pm, t, [(3, c) for c in even]),
            notes="100 - % healed (protocol 1.1.1); hour 0 measured; means",
        )
    )
    spec["series"].append(
        series(
            "particulate-matter/mean",
            "particulate-matter",
            6,
            "area_absolute",
            hours,
            healed(pm, t, [(4, c) for c in even]),
            notes="100 - % healed (protocol 1.1.1); means",
        )
    )
    spec["series"].append(
        series(
            "pje-25/mean",
            "pje-25",
            6,
            "area_absolute",
            hours,
            healed(pm, t, [(5, c) for c in (3, 5, 7, 9, 11, 13, 15)]),
            notes="100 - % healed (protocol 1.1.1); means; row starts with the dose cell",
        )
    )
    for row, dose in zip(range(6, 10), (50, 100, 200, 400), strict=True):
        spec["series"].append(
            series(
                f"pje-{dose}/mean",
                f"pje-{dose}",
                6,
                "area_absolute",
                hours,
                healed(pm, t, [(row, c) for c in even]),
                notes="100 - % healed (protocol 1.1.1); means",
            )
        )
    return spec


def nicergoline():
    pm, t = "PMC7871528", "Tab2"
    spec = base(
        "nicergoline-diabetic-rat-bmc-ophthalmol-2021",
        "10.1186/s12886-021-01835-4",
        pm,
        "Role of nicergoline in corneal wound healing in diabetic rats",
        "rat",
        "CC BY 4.0",
        BY,
        "Wistar rats, male, 8-10 weeks, streptozotocin diabetes (6 weeks)",
        {
            "diameter_mm": 3.0,
            "diameter_basis": "stated: 3.0 mm corneal trephine mark",
            "basement_membrane": "unknown",
            "method": "3.0 mm trephine mark, AlgerBrush II diamond burr de-epithelialization",
            "injury_family": "epithelial_debridement",
        },
        [
            {
                "group": "placebo",
                "role": "treated",
                "n_reported": 24,
                "description": "diabetic rats, placebo (diabetes counts as treated)",
            },
            {
                "group": "nicergoline",
                "role": "treated",
                "n_reported": 24,
                "description": "diabetic rats, nicergoline 10 mg/kg/day",
            },
        ],
    )
    spec["triage_note"] = (
        "triage expected means only at 0/12/24 h; the cells hold 'mean (SD), median (IQR)' "
        "at every time, so the leading mean is used at all five times"
    )
    hours = [0.0, 12.0, 24.0, 48.0, 72.0]
    for group, col in (("placebo", 2), ("nicergoline", 3)):
        spec["series"].append(
            series(
                f"{group}/mean",
                group,
                24,
                "area_absolute",
                hours,
                cells(pm, t, [(r, col) for r in range(3, 8)]),
                notes="leading mean of 'mean (SD), median (IQR)', mm2",
            )
        )
    return spec


def tprk():
    pm, t = "PMC8832736", "Tab3"
    spec = base(
        "tprk-human-bmc-ophthalmol-2022",
        "10.1186/s12886-022-02293-2",
        pm,
        "Comparison of transepithelial and conventional photorefractive keratectomy",
        "human",
        "CC BY 4.0",
        BY,
        "myopic adults, contralateral randomized design (arm A 39 patients, arm B 33)",
        {
            "diameter_mm": 8.5,
            "diameter_basis": "stated 8.5 mm marker/well for mPRK/aaPRK; groups use measured operation-day area",
            "basement_membrane": "removed",
            "method": "PRK (OZ 6.5 mm) after transepithelial laser, spatula or 20% alcohol epithelial removal; MMC",
            "injury_family": "keratectomy",
        },
        [
            {
                "group": "arm-a-tprk",
                "role": "control",
                "n_reported": 39,
                "diameter_from_area_cell": f"{pm}.xml::{t}!R2C3",
                "description": "standard care: betamethasone, chloramphenicol, artificial tears, bandage lens",
            },
            {
                "group": "arm-a-mprk",
                "role": "control",
                "n_reported": 39,
                "diameter_from_area_cell": f"{pm}.xml::{t}!R3C2",
                "description": "fellow eye, mechanical PRK, same standard care",
            },
            {
                "group": "arm-b-tprk",
                "role": "control",
                "n_reported": 33,
                "diameter_from_area_cell": f"{pm}.xml::{t}!R4C3",
                "description": "standard care",
            },
            {
                "group": "arm-b-aaprk",
                "role": "control",
                "n_reported": 33,
                "diameter_from_area_cell": f"{pm}.xml::{t}!R5C2",
                "description": "fellow eye, alcohol-assisted PRK, same standard care",
            },
        ],
    )
    hours = [0.0, 24.0, 48.0, 72.0]
    for group, row, first, n in (
        ("arm-a-tprk", 2, 3, 39),
        ("arm-a-mprk", 3, 2, 39),
        ("arm-b-tprk", 4, 3, 33),
        ("arm-b-aaprk", 5, 2, 33),
    ):
        spec["series"].append(
            series(
                f"{group}/mean",
                group,
                n,
                "area_absolute",
                hours,
                cells(pm, t, [(row, first + k) for k in range(4)]),
                notes="mean ± SD, mm2; fellow-eye groups are paired",
            )
        )
    return spec


def insulin():
    pm, t = "PMC12734778", "medicina-61-02101-t002"
    spec = base(
        "insulin-cxl-human-medicina-2025",
        "10.3390/medicina61122101",
        pm,
        "Can Insulin Drops Accelerate Corneal Healing After Corneal Cross-Linking?",
        "human",
        "CC BY 4.0",
        BY,
        "progressive keratoconus patients, 4 per group",
        {
            "diameter_mm": 8.5,
            "diameter_basis": "stated: 8.5 mm epithelial removal (theoretical area 56.7 mm2)",
            "basement_membrane": "unknown",
            "method": "epithelium-off CXL, Dresden protocol (riboflavin, 370 nm UVA 3 mW/cm2, 30 min)",
            "injury_family": "other",
        },
        [
            {
                "group": "control",
                "role": "control",
                "n_reported": 4,
                "description": "standard care: tobramycin-dexamethasone, ofloxacin, bandage lens",
            },
            {
                "group": "insulin",
                "role": "treated",
                "n_reported": 4,
                "description": "topical insulin 1 IU/mL 5x/day plus standard care",
            },
        ],
    )
    baseline = {
        "definition": "definition:Table 2 footnote, % of theoretical maximum area 56.7 mm2",
        "value": 100.0,
    }
    for group, rows in (("insulin", (2, 4, 6)), ("control", (3, 5, 7))):
        spec["series"].append(
            series(
                f"{group}/mean",
                group,
                4,
                "percent_remaining",
                [0.0, 24.0, 48.0, 72.0],
                [baseline, *cells(pm, t, [(r, 4) for r in rows])],
                notes="% of theoretical defect, mean ± SD; hour 0 definitional",
            )
        )
    return spec


SPECS = [propolis, aloe, fl2, epo, pje_blue, pje_upm, nicergoline, tprk, insulin]
PRIVATE = {"fl2-rat-alkali-tvst-2021"}

if __name__ == "__main__":
    for make in SPECS:
        spec = make()
        target = HERE / f"{spec['study_id']}.json"
        target.write_text(json.dumps(spec, indent=1, ensure_ascii=False) + "\n")
        print(target.name, spec["source_files"][0]["sha256"][:12])
