"""Validation and building of intraocular pressure after glaucoma surgery records."""

from __future__ import annotations

import hashlib

import pytest

from peykar.validation.eye_iop_data import (
    PressureSeries,
    build_record,
    structural_summary,
    to_pressure_series,
    validate_record,
)
from peykar_core.errors import InvalidRequest


def _record(**overrides: object) -> dict:
    record = {
        "schema": "eye.iop-record-1.0.0",
        "study_id": "iop-pmc1",
        "doi": "10.1/x",
        "species": "human",
        "procedure": "trabeculectomy",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [
            {"group": "mmc", "role": "control"},
            {"group": "tube", "role": "control", "procedure": "tube"},
        ],
        "series": [
            {
                "series_id": "mmc/iop",
                "group": "mmc",
                "kind": "group_mean",
                "n": 40,
                "value_kind": "iop_absolute",
                "reference_kind": "preoperative_medicated",
                "method": "goldmann",
                "months": [0, 1, 6, 12],
                "values": [25.0, 10.0, 12.5, 14.0],
                "source_cells": ["a", "b", "c", "d"],
            },
            {
                "series_id": "tube/change",
                "group": "tube",
                "kind": "group_mean",
                "n": 30,
                "value_kind": "iop_change",
                "reference_kind": "preoperative_unstated",
                "reference_iop": 28.0,
                "reference_cell": "a.xml::t1!R2C2",
                "method": "unstated",
                "months": [0, 3],
                "values": [0.0, -14.0],
                "source_cells": ["definition:x", "e"],
            },
        ],
    }
    record.update(overrides)
    return record


def test_values_normalize_to_reference_fractions_and_contexts() -> None:
    checked = validate_record(_record())
    assert checked["contexts"] == ["human/trabeculectomy", "human/tube"]
    first, second = checked["series"]
    assert first["fractions"] == pytest.approx([1.0, 0.4, 0.5, 0.56])
    assert second["fractions"] == pytest.approx([1.0, 0.5])
    series = to_pressure_series(_record())
    assert series[0].months == (1.0, 6.0, 12.0)
    assert series[0].reference_mmhg == 25.0
    assert series[0].reference_kind == "preoperative_medicated"
    assert series[1].context == "human/tube"
    prefix = series[0].prefix(6.0)
    assert prefix.months == (1.0, 6.0) and prefix.reference_kind == "preoperative_medicated"
    summary = structural_summary([_record()])["studies"][0]
    assert summary["reference_kinds"] == ["preoperative_medicated", "preoperative_unstated"]
    assert "values" not in summary


def test_invalid_records_are_rejected() -> None:
    bad_reference = _record()
    bad_reference["series"][0]["values"] = [3.0, 2.0, 2.0, 2.0]
    with pytest.raises(InvalidRequest):
        validate_record(bad_reference)
    with pytest.raises(InvalidRequest):
        validate_record(_record(procedure="lasik"))
    unreferenced = _record()
    del unreferenced["series"][1]["reference_cell"]
    with pytest.raises(InvalidRequest):
        validate_record(unreferenced)
    no_kind = _record()
    no_kind["series"][0]["reference_kind"] = "fellow_eye"
    with pytest.raises(InvalidRequest):
        validate_record(no_kind)
    with pytest.raises(InvalidRequest):
        PressureSeries(
            series_id="s",
            study_id="s",
            context="human/tube",
            reference_mmhg=20.0,
            months=(1.0,),
            fractions=(4.0,),
        )


XML = (
    b'<article><body><table-wrap id="t1"><table><thead><tr><th>Time</th><th>IOP</th></tr>'
    b"</thead><tbody><tr><td>Preop</td><td>25.0 \xc2\xb1 4</td></tr>"
    b"<tr><td>1 month</td><td>10.0 \xc2\xb1 3</td></tr>"
    b"<tr><td>12 months</td><td>14.0 \xc2\xb1 3</td></tr></tbody></table></table-wrap>"
    b"</body></article>"
)


def test_builder_reads_cited_cells_and_checks_hashes() -> None:
    spec = {
        "study_id": "iop-pmc2",
        "doi": "10.1/y",
        "species": "human",
        "procedure": "angle_mims",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": hashlib.sha256(XML).hexdigest()}],
        "groups": [{"group": "stent", "role": "control"}],
        "series": [
            {
                "series_id": "stent/iop",
                "group": "stent",
                "kind": "group_mean",
                "n": 20,
                "value_kind": "iop_absolute",
                "reference_kind": "preoperative_medicated",
                "method": "goldmann",
                "months": [0, 1, 12],
                "cells": ["a.xml::t1!R2C2", "a.xml::t1!R3C2", "a.xml::t1!R4C2"],
            }
        ],
    }
    record = build_record(spec, {"a.xml": XML})
    assert record["series"][0]["values"] == [25.0, 10.0, 14.0]
    with pytest.raises(InvalidRequest):
        build_record(spec, {"a.xml": XML + b" "})
