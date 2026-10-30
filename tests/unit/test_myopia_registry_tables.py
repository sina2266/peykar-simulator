"""Registry tranche-1 conversion of ClinicalTrials.gov results into citable tables
(synthetic record)."""

from __future__ import annotations

import importlib.util
from pathlib import Path

from peykar.validation.cornea_table_records import cell_text, table_rows
from peykar.validation.myopia_axial_data import axial_number

ROOT = Path(__file__).resolve().parents[2]
FOLDER = ROOT / "experiments/data/myopia-axial-2026-10-30/registry-1"


def _load(name: str):  # type: ignore[no-untyped-def]
    spec = importlib.util.spec_from_file_location(name, FOLDER / f"{name}.py")
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


STUDY = {
    "protocolSection": {
        "identificationModule": {"nctId": "NCT00000001", "briefTitle": "Synthetic"},
        "eligibilityModule": {"maximumAge": "12 Years"},
        "armsInterventionsModule": {
            "armGroups": [
                {"label": "Atropine 0.01%", "type": "EXPERIMENTAL", "interventionNames": ["Drug"]},
                {"label": "Placebo", "type": "PLACEBO_COMPARATOR"},
            ]
        },
    },
    "resultsSection": {
        "baselineCharacteristicsModule": {
            "groups": [{"id": "BG000", "title": "Atropine"}, {"id": "BG001", "title": "Placebo"}],
            "denoms": [
                {
                    "units": "Participants",
                    "counts": [
                        {"groupId": "BG000", "value": "50"},
                        {"groupId": "BG001", "value": "48"},
                    ],
                }
            ],
            "measures": [
                {
                    "title": "Axial length",
                    "paramType": "MEAN",
                    "dispersionType": "STANDARD_DEVIATION",
                    "unitOfMeasure": "mm",
                    "classes": [
                        {
                            "categories": [
                                {
                                    "measurements": [
                                        {"groupId": "BG000", "value": "24.50", "spread": "0.8"},
                                        {"groupId": "BG001", "value": "24.40", "spread": "0.9"},
                                    ]
                                }
                            ]
                        }
                    ],
                }
            ],
        },
        "outcomeMeasuresModule": {
            "outcomeMeasures": [
                {
                    "title": "Change in Axial Length",
                    "timeFrame": "Baseline, 12 months",
                    "unitOfMeasure": "mm",
                    "paramType": "MEAN",
                    "dispersionType": "95% Confidence Interval",
                    "groups": [
                        {"id": "OG000", "title": "Atropine"},
                        {"id": "OG001", "title": "Placebo"},
                    ],
                    "denoms": [
                        {
                            "units": "Eyes",
                            "counts": [
                                {"groupId": "OG000", "value": "49"},
                                {"groupId": "OG001", "value": "46"},
                            ],
                        }
                    ],
                    "classes": [
                        {
                            "title": "12 months",
                            "categories": [
                                {
                                    "measurements": [
                                        {
                                            "groupId": "OG000",
                                            "value": "-0.05",
                                            "lowerLimit": "-0.1",
                                            "upperLimit": "0.0",
                                        },
                                        {"groupId": "OG001"},
                                    ]
                                }
                            ],
                        }
                    ],
                }
            ]
        },
    },
}


def test_tables_are_citable_and_carry_the_posted_values() -> None:
    tables = _load("registry_tables")
    xml = tables.convert(STUDY, "0" * 64)
    assert xml == tables.convert(STUDY, "0" * 64)  # deterministic
    baseline = table_rows(xml, "b1")
    assert baseline[0][1].startswith("BG000")
    assert baseline[1] == ["number analysed (Participants)", "50", "48"]
    assert axial_number(cell_text(xml, "b1!R3C2")) == 24.50
    assert cell_text(xml, "b1!R3C3") == "24.40 ± 0.9"
    outcome = table_rows(xml, "o1")
    assert outcome[1] == ["number analysed (Eyes)", "49", "46"]
    assert outcome[2][0] == "12 months"
    assert axial_number(cell_text(xml, "o1!R3C2")) == -0.05
    assert outcome[2][2] == "NA"
    assert table_rows(xml, "a1")[1][0] == "Atropine 0.01%"


def test_screen_reads_ages_and_axial_outcomes_only() -> None:
    screen = _load("registry_screen")
    assert screen.age_years("12 Years") == 12.0
    assert screen.age_years("18 Months") == 1.5
    assert screen.age_years(None) is None
    assert [o["title"] for o in screen.axial_outcomes(STUDY)] == ["Change in Axial Length"]
