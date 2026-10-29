"""Retinal visual-acuity records: validation, contexts, letter conversion and the builder."""

from __future__ import annotations

import hashlib
import math
from typing import Any

import pytest

from peykar.validation.cornea_acuity_data import AcuitySeries
from peykar.validation.eye_retina_acuity_data import (
    RetinaAcuitySeries,
    build_record,
    structural_summary,
    to_retina_acuity_series,
    validate_record,
)
from peykar_core.errors import InvalidRequest


def _record(**overrides: object) -> dict[str, Any]:
    record: dict[str, Any] = {
        "schema": "eye.retina-acuity-record-1.0.0",
        "study_id": "rac-pmc1",
        "doi": "10.1/x",
        "species": "human",
        "indication": "namd",
        "treatment": "anti_vegf",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [
            {"group": "afl", "role": "control", "agent": "aflibercept"},
            {
                "group": "dex",
                "role": "treated",
                "indication": "dme",
                "treatment": "steroid_implant",
            },
        ],
        "series": [
            {
                "series_id": "afl/bcva",
                "group": "afl",
                "kind": "group_mean",
                "n": 60,
                "value_kind": "letters_absolute",
                "regimen": "treat_and_extend",
                "prior_treatment": "naive",
                "missing_data": "observed",
                "months": [0, 1, 3, 12],
                "values": [55.0, 60.0, 62.0, 63.5],
                "source_cells": ["a", "b", "c", "d"],
            },
            {
                "series_id": "dex/change",
                "group": "dex",
                "kind": "group_mean",
                "n": 30,
                "value_kind": "logmar_change",
                "regimen": "single",
                "prior_treatment": "previously_treated",
                "missing_data": "locf",
                "reference_value": 0.6,
                "reference_cell": "a.xml::t1!R2C2",
                "months": [0, 2, 4],
                "values": [0.0, -0.2, -0.05],
                "source_cells": ["definition:x", "e", "f"],
            },
        ],
    }
    record.update(overrides)
    return record


def test_contexts_changes_and_pseudo_fractions() -> None:
    checked = validate_record(_record())
    assert checked["contexts"] == ["human/dme/steroid_implant", "human/namd/anti_vegf"]
    afl, dex = to_retina_acuity_series(_record())
    assert isinstance(afl, RetinaAcuitySeries) and isinstance(afl, AcuitySeries)
    assert afl.context == "human/namd/anti_vegf" and afl.treatment == "anti_vegf"
    assert afl.regimen == "treat_and_extend" and afl.prior_treatment == "naive"
    assert afl.reference_logmar == pytest.approx(0.6)  # 55 letters
    assert afl.changes_logmar == pytest.approx((-0.1, -0.14, -0.17))
    assert afl.fractions[0] == pytest.approx(math.exp(-0.1))
    assert afl.reference_value == pytest.approx(math.exp(0.6))
    assert afl.aggregate_n == 60 and afl.months == (1.0, 3.0, 12.0)
    assert dex.context == "human/dme/steroid_implant" and dex.role == "treated"
    assert dex.changes_logmar == pytest.approx((-0.2, -0.05))
    summary = structural_summary([_record()])["studies"][0]
    assert summary["missing_data"] == ["locf", "observed"]
    assert summary["regimens"] == ["single", "treat_and_extend"]


def test_invalid_records_are_rejected() -> None:
    bad_series: list[dict[str, Any]] = []
    for key, value in (
        ("missing_data", "imputed"),
        ("regimen", "monthly"),
        ("prior_treatment", None),
        ("value_kind", "snellen"),
    ):
        series = dict(_record()["series"][0])
        series[key] = value
        bad_series.append(series)
    out_of_range = dict(_record()["series"][0])
    out_of_range["values"] = [55.0, 60.0, 62.0, 101.0]
    bad_series.append(out_of_range)
    no_reference = dict(_record()["series"][1])
    no_reference.pop("reference_cell")
    bad_series.append(no_reference)
    for series in bad_series:
        with pytest.raises(InvalidRequest):
            validate_record(_record(series=[series]))
    for overrides in (
        {"species": "rabbit"},
        {"indication": "glaucoma"},
        {"treatment": "injection"},
        {"schema": "cornea.acuity-record-1.0.0"},
    ):
        with pytest.raises(InvalidRequest):
            validate_record(_record(**overrides))


XML = (
    b"<article><body><table-wrap id='t1'><table><thead><tr><th>Visit</th><th>BCVA</th>"
    b"</tr></thead><tbody><tr><td>Baseline</td><td>58.2 \xc2\xb1 10.1</td></tr>"
    b"<tr><td>Month 3</td><td>64.0 \xc2\xb1 9.8</td></tr>"
    b"<tr><td>Month 12</td><td>66.5 \xc2\xb1 11.0</td></tr></tbody></table></table-wrap>"
    b"</body></article>"
)


def test_builder_reads_cited_cells_and_checks_hashes() -> None:
    spec = {
        **{k: v for k, v in _record().items() if k not in {"series", "schema"}},
        "source_files": [{"name": "a.xml", "sha256": hashlib.sha256(XML).hexdigest()}],
        "series": [
            {
                **{
                    k: v
                    for k, v in _record()["series"][0].items()
                    if k not in {"values", "source_cells"}
                },
                "months": [0, 3, 12],
                "cells": ["a.xml::t1!R2C2", "a.xml::t1!R3C2", "a.xml::t1!R4C2"],
            }
        ],
    }
    record = build_record(spec, {"a.xml": XML})
    assert record["series"][0]["values"] == [58.2, 64.0, 66.5]
    assert record["schema"] == "eye.retina-acuity-record-1.0.0"
    with pytest.raises(InvalidRequest):
        build_record(spec, {"a.xml": XML + b" "})
