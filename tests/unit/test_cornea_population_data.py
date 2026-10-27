"""Study-record validation and conversion; synthetic records only."""

import copy

import pytest

from peykar.validation.cornea_population_data import (
    structural_summary,
    to_wound_series,
    validate_study,
)
from peykar_core.errors import InvalidRequest

RECORD = {
    "schema": "cornea.wound-study-record-1.0.0",
    "study_id": "example2024-mouse",
    "doi": "10.0000/example",
    "context": "mouse",
    "split": "development",
    "license": {"statement": "CC BY 4.0"},
    "source_files": [{"name": "s1.xlsx", "sha256": "0" * 64}],
    "wound": {
        "diameter_mm": 2.0,
        "basement_membrane": "intact",
        "method": "Algerbrush",
        "injury_family": "epithelial_debridement",
    },
    "groups": [{"group": "vehicle", "role": "control"}, {"group": "drug", "role": "treated"}],
    "series": [
        {
            "series_id": "m1",
            "group": "vehicle",
            "kind": "individual",
            "value_kind": "area_absolute",
            "hours": [0, 12, 24],
            "values": [3.0, 1.5, 0.0],
            "source_cells": ["B2", "C2", "D2"],
        },
        {
            "series_id": "drug-mean",
            "group": "drug",
            "kind": "group_mean",
            "n": 5,
            "value_kind": "percent_healed",
            "hours": [0, 12, 24],
            "values": [0, 60, 100],
            "source_cells": ["B9", "C9", "D9"],
        },
    ],
}


def test_normalizes_value_kinds_and_radius():
    checked = validate_study(RECORD)
    assert checked["radius0_um"] == pytest.approx(1000.0)
    assert checked["series"][0]["fractions"] == pytest.approx([1.0, 0.5, 0.0])
    assert checked["series"][1]["fractions"] == pytest.approx([1.0, 0.4, 0.0])
    series = to_wound_series(RECORD)
    assert series[0].hours == (12.0, 24.0) and series[0].aggregate_n is None
    assert series[1].aggregate_n == 5 and series[1].role == "treated"


def test_rejects_missing_hour_zero_licence_and_cells():
    for mutate in (
        lambda r: r["series"][0].update(hours=[6, 12, 24]),
        lambda r: r.update(license={}),
        lambda r: r["series"][0].update(source_cells=["B2"]),
        lambda r: r["groups"].append({"group": "x", "role": "maybe"}),
    ):
        record = copy.deepcopy(RECORD)
        mutate(record)
        with pytest.raises(InvalidRequest):
            validate_study(record)


def test_structural_summary_exposes_no_outcome_values():
    summary = structural_summary([RECORD])
    study = summary["studies"][0]
    assert not {"series", "values", "fractions"} & set(study)
    numbers = {
        value
        for key, value in study.items()
        if isinstance(value, (int, float)) and key != "radius0_um"
    }
    assert numbers.isdisjoint({1.5, 60, 0.5, 0.4})
    assert study["control_series"] == 1 and study["observation_hours"] == [0.0, 12.0, 24.0]
