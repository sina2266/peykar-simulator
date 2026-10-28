"""Validation and building of central retinal thickness records."""

from __future__ import annotations

import hashlib

import pytest

from peykar.validation.eye_retina_data import (
    RetinaSeries,
    build_record,
    structural_summary,
    thickness_number,
    to_retina_series,
    validate_record,
)
from peykar_core.errors import InvalidRequest


def _record(**overrides: object) -> dict:
    record = {
        "schema": "eye.retina-thickness-record-1.0.0",
        "study_id": "ret-pmc1",
        "doi": "10.1/x",
        "species": "human",
        "indication": "dme",
        "treatment": "anti_vegf",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [
            {"group": "ranibizumab", "role": "control"},
            {"group": "dex", "role": "control", "treatment": "steroid_implant"},
        ],
        "series": [
            {
                "series_id": "ranibizumab/cst",
                "group": "ranibizumab",
                "kind": "group_mean",
                "n": 40,
                "value_kind": "thickness_absolute",
                "measure": "central_subfield",
                "device": "spectralis",
                "regimen": "loading_then_prn",
                "prior_treatment": "naive",
                "months": [0, 1, 3, 12],
                "values": [500.0, 350.0, 325.0, 300.0],
                "source_cells": ["a", "b", "c", "d"],
            },
            {
                "series_id": "dex/change",
                "group": "dex",
                "kind": "group_mean",
                "n": 30,
                "value_kind": "thickness_change",
                "measure": "central_unspecified",
                "device": "unstated",
                "regimen": "single",
                "prior_treatment": "previously_treated",
                "reference_um": 600.0,
                "reference_cell": "a.xml::t1!R2C2",
                "months": [0, 2],
                "values": [0.0, -240.0],
                "source_cells": ["definition:x", "e"],
            },
        ],
    }
    record.update(overrides)
    return record


def test_values_normalize_to_baseline_fractions_and_contexts() -> None:
    checked = validate_record(_record())
    assert checked["contexts"] == ["human/dme/anti_vegf", "human/dme/steroid_implant"]
    first, second = checked["series"]
    assert first["fractions"] == pytest.approx([1.0, 0.7, 0.65, 0.6])
    assert second["fractions"] == pytest.approx([1.0, 0.6])
    series = to_retina_series(_record())
    assert series[0].months == (1.0, 3.0, 12.0)
    assert series[0].reference_value == 500.0
    assert series[0].regimen == "loading_then_prn"
    assert series[1].context == "human/dme/steroid_implant"
    prefix = series[0].prefix(3.0)
    assert prefix.months == (1.0, 3.0) and prefix.kind == "aggregate"
    summary = structural_summary([_record()])["studies"][0]
    assert summary["measures"] == ["central_subfield", "central_unspecified"]
    assert "values" not in summary


def test_invalid_records_are_rejected() -> None:
    bad = _record()
    bad["series"][0]["values"] = [50.0, 40.0, 40.0, 40.0]
    with pytest.raises(InvalidRequest):
        validate_record(bad)
    with pytest.raises(InvalidRequest):
        validate_record(_record(treatment="eye_drops"))
    with pytest.raises(InvalidRequest):
        validate_record(_record(indication="glaucoma"))
    unreferenced = _record()
    del unreferenced["series"][1]["reference_cell"]
    with pytest.raises(InvalidRequest):
        validate_record(unreferenced)
    no_regimen = _record()
    no_regimen["series"][0]["regimen"] = "monthly"
    with pytest.raises(InvalidRequest):
        validate_record(no_regimen)
    percent = _record()
    percent["series"][1].update(value_kind="percent_change", values=[0.0, -30.0])
    assert validate_record(percent)["series"][1]["fractions"] == pytest.approx([1.0, 0.7])
    del percent["series"][1]["reference_cell"]
    with pytest.raises(InvalidRequest):
        validate_record(percent)
    del percent["series"][1]["reference_um"]
    unanchored = validate_record(percent)["series"][1]
    assert unanchored["reference_um"] == 0.0 and unanchored["fractions"][1] == pytest.approx(0.7)
    with pytest.raises(InvalidRequest):
        RetinaSeries(
            series_id="s",
            study_id="s",
            context="human/dme/anti_vegf",
            reference_value=400.0,
            months=(1.0,),
            fractions=(5.0,),
        )


XML = (
    b'<article><body><table-wrap id="t1"><table><thead><tr><th>Visit</th><th>CST</th></tr>'
    b"</thead><tbody><tr><td>Baseline</td><td>480 \xc2\xb1 90</td></tr>"
    b"<tr><td>Month 1</td><td>352,5 \xc2\xb1 70</td></tr>"
    b"<tr><td>Month 12</td><td>300 \xc2\xb1 60</td></tr></tbody></table></table-wrap>"
    b"</body></article>"
)


def test_builder_reads_cited_cells_and_checks_hashes() -> None:
    spec = {
        "study_id": "ret-pmc2",
        "doi": "10.1/y",
        "species": "human",
        "indication": "rvo",
        "treatment": "anti_vegf",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": hashlib.sha256(XML).hexdigest()}],
        "groups": [{"group": "aflibercept", "role": "control"}],
        "series": [
            {
                "series_id": "aflibercept/cst",
                "group": "aflibercept",
                "kind": "group_mean",
                "n": 20,
                "value_kind": "thickness_absolute",
                "measure": "central_subfield",
                "device": "cirrus",
                "regimen": "treat_and_extend",
                "prior_treatment": "naive",
                "months": [0, 1, 12],
                "cells": ["a.xml::t1!R2C2", "a.xml::t1!R3C2", "a.xml::t1!R4C2"],
            }
        ],
    }
    record = build_record(spec, {"a.xml": XML})
    assert record["series"][0]["values"] == [480.0, 352.5, 300.0]
    with pytest.raises(InvalidRequest):
        build_record(spec, {"a.xml": XML + b" "})


def test_change_sign_before_a_number_is_skipped() -> None:
    assert thickness_number("\u0394-132.46\u00b1196.55") == pytest.approx(-132.46)
    assert thickness_number("\u2206 -12,5 \u00b1 3,1") == pytest.approx(-12.5)
    assert thickness_number("352 \u00b1 70") == pytest.approx(352.0)
    with pytest.raises(InvalidRequest):
        thickness_number("delta -12")
