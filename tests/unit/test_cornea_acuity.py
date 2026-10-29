"""Corrected visual-acuity records, letter conversion and additive pseudo-fractions."""

from __future__ import annotations

import hashlib
import math

import pytest

from peykar.validation.cornea_acuity_data import (
    COVARIATE_UNIT_LOGMAR,
    MODEL_UNIT_LOGMAR,
    acuity_number,
    build_record,
    change_from_fraction,
    letters_to_logmar,
    reference_from_value,
    structural_summary,
    to_acuity_series,
    to_acuity_series_by_indication,
    validate_record,
)
from peykar_core.errors import InvalidRequest


def _record(**overrides: object) -> dict:
    record = {
        "schema": "cornea.acuity-record-1.0.0",
        "study_id": "acu-pmc1",
        "doi": "10.1/x",
        "species": "human",
        "procedure": "dmek",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [
            {"group": "dmek", "role": "control", "indication": "fuchs", "combined": False},
            {
                "group": "dsaek",
                "role": "control",
                "procedure": "dsaek",
                "indication": "bullous_keratopathy",
                "combined": None,
            },
            {
                "group": "cxl",
                "role": "treated",
                "procedure": "cxl",
                "indication": "keratoconus",
                "combined": False,
            },
        ],
        "series": [
            {
                "series_id": "dmek/bcva",
                "group": "dmek",
                "kind": "group_mean",
                "n": 40,
                "value_kind": "logmar_absolute",
                "months": [0, 1, 3, 12],
                "values": [0.62, 0.30, 0.18, 0.12],
                "source_cells": ["a", "b", "c", "d"],
            },
            {
                "series_id": "dsaek/letters",
                "group": "dsaek",
                "kind": "group_mean",
                "n": 30,
                "value_kind": "letters_absolute",
                "months": [0, 6],
                "values": [55.0, 75.0],
                "source_cells": ["e", "f"],
            },
            {
                "series_id": "cxl/change",
                "group": "cxl",
                "kind": "individual",
                "value_kind": "letters_change",
                "reference_value": 70.0,
                "reference_cell": "a.xml::t1!R2C2",
                "months": [0, 12],
                "values": [0.0, 5.0],
                "source_cells": ["definition:x", "g"],
            },
        ],
    }
    record.update(overrides)
    return record


def test_logmar_letters_and_exact_pseudo_fractions() -> None:
    assert letters_to_logmar(85) == 0.0 and letters_to_logmar(70) == pytest.approx(0.3)
    checked = validate_record(_record())
    assert checked["contexts"] == ["human/cxl", "human/dmek", "human/dsaek"]
    dmek, dsaek, cxl = checked["series"]
    assert dmek["changes_logmar"] == pytest.approx([0.0, -0.32, -0.44, -0.50])
    assert dsaek["reference_logmar"] == pytest.approx(0.6)
    assert dsaek["changes_logmar"] == pytest.approx([0.0, -0.4])
    assert cxl["reference_logmar"] == pytest.approx(0.3)
    assert cxl["changes_logmar"] == pytest.approx([0.0, -0.1])
    series = to_acuity_series(_record())
    first = series[0]
    assert first.months == (1.0, 3.0, 12.0) and first.reference_logmar == 0.62
    assert [change_from_fraction(f) for f in first.fractions] == pytest.approx([-0.32, -0.44, -0.5])
    assert reference_from_value(first.reference_value) == pytest.approx(0.62)
    assert math.log(first.reference_value) == pytest.approx(0.62 / COVARIATE_UNIT_LOGMAR)
    assert MODEL_UNIT_LOGMAR == 1.0
    assert first.prefix(3.0).changes_logmar == pytest.approx((-0.32, -0.44))
    assert first.kind == "aggregate" and series[2].kind == "individual"
    assert series[2].role == "treated" and series[1].combined_cataract is None
    by_indication = to_acuity_series_by_indication(_record())
    assert [s.context for s in by_indication] == [
        "human/fuchs/dmek",
        "human/bullous_keratopathy/dsaek",
        "human/keratoconus/cxl",
    ]
    summary = structural_summary([_record()])["studies"][0]
    assert summary["indications"] == ["bullous_keratopathy", "fuchs", "keratoconus"]
    assert summary["combined"] == ["false", "none"] and "values" not in summary


def test_invalid_acuity_records_are_rejected() -> None:
    animal = _record(species="rabbit")
    with pytest.raises(InvalidRequest):
        validate_record(animal)
    no_indication = _record()
    del no_indication["groups"][0]["indication"]
    with pytest.raises(InvalidRequest):
        validate_record(no_indication)
    no_combined = _record()
    del no_combined["groups"][0]["combined"]
    with pytest.raises(InvalidRequest):
        validate_record(no_combined)
    out_of_range = _record()
    out_of_range["series"][0]["values"] = [4.5, 1.0, 1.0, 1.0]
    with pytest.raises(InvalidRequest):
        validate_record(out_of_range)
    letters = _record()
    letters["series"][1]["values"] = [55.0, 120.0]
    with pytest.raises(InvalidRequest):
        validate_record(letters)
    unreferenced = _record()
    del unreferenced["series"][2]["reference_cell"]
    with pytest.raises(InvalidRequest):
        validate_record(unreferenced)
    nonzero = _record()
    nonzero["series"][2]["values"] = [1.0, 5.0]
    with pytest.raises(InvalidRequest):
        validate_record(nonzero)
    with pytest.raises(InvalidRequest):
        validate_record(_record(procedure="lasik"))


XML = (
    b'<article><body><table-wrap id="t1"><table><thead><tr><th>Time</th><th>BCVA</th>'
    b"<th>Snellen</th></tr></thead><tbody>"
    b"<tr><td>Preop</td><td>.72 \xc2\xb1 .30</td><td>20/100 (0.70)</td></tr>"
    b"<tr><td>1 month</td><td>0,40 \xc2\xb1 0,22</td><td>20/50 (0.40)</td></tr>"
    b"<tr><td>6 months</td><td>\xe2\x88\x920.02 \xc2\xb1 0.10</td><td>20/20 (\xe2\x88\x920.02)</td>"
    b"</tr></tbody></table></table-wrap></body></article>"
)


def test_builder_reads_cells_parenthesized_values_and_checks_hashes() -> None:
    spec = {
        "study_id": "acu-pmc2",
        "doi": "10.1/y",
        "species": "human",
        "procedure": "pk",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": hashlib.sha256(XML).hexdigest()}],
        "groups": [
            {"group": "pk", "role": "control", "indication": "keratoconus", "combined": False}
        ],
        "series": [
            {
                "series_id": "pk/bcva",
                "group": "pk",
                "kind": "group_mean",
                "n": 20,
                "value_kind": "logmar_absolute",
                "months": [0, 1, 6],
                "cells": ["a.xml::t1!R2C2", "a.xml::t1!R3C2", "a.xml::t1!R4C2"],
            },
            {
                "series_id": "pk/snellen-logmar",
                "group": "pk",
                "kind": "individual",
                "value_kind": "logmar_absolute",
                "months": [0, 1, 6],
                "cells": [
                    {"cell": f"a.xml::t1!R{r}C3", "take": "parenthesized"} for r in (2, 3, 4)
                ],
            },
        ],
    }
    record = build_record(spec, {"a.xml": XML})
    assert record["series"][0]["values"] == pytest.approx([0.72, 0.40, -0.02])
    assert record["series"][1]["values"] == pytest.approx([0.70, 0.40, -0.02])
    assert record["series"][1]["source_cells"][0] == "a.xml::t1!R2C3#parenthesized"
    with pytest.raises(InvalidRequest):
        build_record(spec, {"a.xml": XML + b" "})
    assert acuity_number("20/40 (0.30)") == 20.0
    assert acuity_number("20/40 (0.30)", "parenthesized") == pytest.approx(0.3)
    assert acuity_number("\u2212.08 \u00b1 .1") == pytest.approx(-0.08)
    with pytest.raises(InvalidRequest):
        acuity_number("0.3", "parenthesized")
    with pytest.raises(InvalidRequest):
        acuity_number("0.3", "trailing")
