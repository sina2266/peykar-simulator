"""Maximum-keratometry (Kmax) records and additive pseudo-fractions (synthetic data)."""

from __future__ import annotations

import hashlib
import math

import pytest

from peykar.validation.cornea_kmax_data import (
    COVARIATE_UNIT_D,
    MODEL_UNIT_D,
    build_record,
    change_from_fraction,
    kmax_number,
    reference_from_value,
    structural_summary,
    to_kmax_series,
    validate_record,
)
from peykar_core.errors import InvalidRequest


def _record(**overrides: object) -> dict:
    record = {
        "schema": "cornea.kmax-record-1.0.0",
        "study_id": "kmx-pmc1",
        "doi": "10.1/x",
        "species": "human",
        "procedure": "cxl_epi_off",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [
            {
                "group": "dresden",
                "role": "control",
                "indication": "keratoconus",
                "age_group": "adult",
            },
            {
                "group": "accelerated",
                "role": "control",
                "procedure": "cxl_accelerated",
                "indication": "keratoconus",
                "age_group": "mixed",
            },
            {
                "group": "sham",
                "role": "control",
                "procedure": "observation",
                "indication": "keratoconus",
                "age_group": "pediatric",
            },
        ],
        "series": [
            {
                "series_id": "dresden/kmax",
                "group": "dresden",
                "kind": "group_mean",
                "n": 40,
                "device": "pentacam",
                "value_kind": "kmax_absolute",
                "months": [0, 1, 6, 12],
                "values": [58.2, 59.0, 57.4, 57.0],
                "source_cells": ["a", "b", "c", "d"],
            },
            {
                "series_id": "accelerated/change",
                "group": "accelerated",
                "kind": "group_mean",
                "n": 30,
                "device": "sirius",
                "value_kind": "kmax_change",
                "reference_value": 56.0,
                "reference_cell": "a.xml::t1!R2C2",
                "months": [0, 12],
                "values": [0.0, -0.8],
                "source_cells": ["definition:x", "e"],
            },
            {
                "series_id": "sham/kmax",
                "group": "sham",
                "kind": "individual",
                "device": "unstated",
                "value_kind": "kmax_absolute",
                "months": [0, 12],
                "values": [52.0, 53.5],
                "source_cells": ["f", "g"],
            },
        ],
    }
    record.update(overrides)
    return record


def test_kmax_changes_and_exact_pseudo_fractions() -> None:
    checked = validate_record(_record())
    assert checked["contexts"] == [
        "human/cxl_accelerated",
        "human/cxl_epi_off",
        "human/observation",
    ]
    dresden, accelerated, sham = checked["series"]
    assert dresden["changes_d"] == pytest.approx([0.0, 0.8, -0.8, -1.2])
    assert accelerated["reference_d"] == 56.0
    assert accelerated["changes_d"] == pytest.approx([0.0, -0.8])
    assert sham["changes_d"] == pytest.approx([0.0, 1.5])
    series = to_kmax_series(_record())
    first = series[0]
    assert first.months == (1.0, 6.0, 12.0) and first.reference_d == 58.2
    assert [change_from_fraction(f) for f in first.fractions] == pytest.approx([0.8, -0.8, -1.2])
    assert reference_from_value(first.reference_value) == pytest.approx(58.2)
    assert math.log(first.reference_value) == pytest.approx(58.2 / COVARIATE_UNIT_D)
    assert MODEL_UNIT_D == 5.0
    assert first.prefix(6.0).changes_d == pytest.approx((0.8, -0.8))
    assert first.kind == "aggregate" and series[2].kind == "individual"
    assert series[1].device == "sirius" and series[2].age_group == "pediatric"
    summary = structural_summary([_record()])["studies"][0]
    assert summary["devices"] == ["pentacam", "sirius", "unstated"]
    assert summary["age_groups"] == ["adult", "mixed", "pediatric"] and "values" not in summary


def test_invalid_kmax_records_are_rejected() -> None:
    with pytest.raises(InvalidRequest):
        validate_record(_record(species="rabbit"))
    with pytest.raises(InvalidRequest):
        validate_record(_record(procedure="lasik"))
    no_age = _record()
    del no_age["groups"][0]["age_group"]
    with pytest.raises(InvalidRequest):
        validate_record(no_age)
    no_device = _record()
    del no_device["series"][0]["device"]
    with pytest.raises(InvalidRequest):
        validate_record(no_device)
    out_of_range = _record()
    out_of_range["series"][0]["values"] = [120.0, 59.0, 57.4, 57.0]
    with pytest.raises(InvalidRequest):
        validate_record(out_of_range)
    unreferenced = _record()
    del unreferenced["series"][1]["reference_cell"]
    with pytest.raises(InvalidRequest):
        validate_record(unreferenced)
    nonzero = _record()
    nonzero["series"][1]["values"] = [0.5, -0.8]
    with pytest.raises(InvalidRequest):
        validate_record(nonzero)
    late = _record()
    late["series"][0]["months"] = [1, 2, 6, 12]
    with pytest.raises(InvalidRequest):
        validate_record(late)


XML = (
    b'<article><body><table-wrap id="t1"><table><thead><tr><th>Time</th><th>Kmax (D)</th>'
    b"</tr></thead><tbody>"
    b"<tr><td>Preop</td><td>57,8 \xc2\xb1 4,1</td></tr>"
    b"<tr><td>1 month</td><td>58.6 \xc2\xb1 4.3</td></tr>"
    b"<tr><td>12 months</td><td>56.9 \xc2\xb1 4.0</td></tr>"
    b"<tr><td>Change at 12 months</td><td>\xe2\x88\x920.9 \xc2\xb1 1.2</td></tr>"
    b"</tbody></table></table-wrap></body></article>"
)


def test_builder_reads_cells_and_checks_hashes() -> None:
    spec = {
        "study_id": "kmx-pmc2",
        "doi": "10.1/y",
        "species": "human",
        "procedure": "cxl_epi_off",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": hashlib.sha256(XML).hexdigest()}],
        "groups": [
            {"group": "cxl", "role": "control", "indication": "keratoconus", "age_group": "adult"}
        ],
        "series": [
            {
                "series_id": "cxl/kmax",
                "group": "cxl",
                "kind": "group_mean",
                "n": 20,
                "device": "pentacam",
                "value_kind": "kmax_absolute",
                "months": [0, 1, 12],
                "cells": ["a.xml::t1!R2C2", "a.xml::t1!R3C2", "a.xml::t1!R4C2"],
            },
            {
                "series_id": "cxl/change",
                "group": "cxl",
                "kind": "group_mean",
                "n": 20,
                "device": "pentacam",
                "value_kind": "kmax_change",
                "months": [0, 12],
                "reference_cell": "a.xml::t1!R2C2",
                "cells": [
                    {"definition": "definition:change at baseline", "value": 0},
                    "a.xml::t1!R5C2",
                ],
            },
        ],
    }
    record = build_record(spec, {"a.xml": XML})
    assert record["series"][0]["values"] == pytest.approx([57.8, 58.6, 56.9])
    assert record["series"][1]["values"] == pytest.approx([0.0, -0.9])
    assert record["series"][1]["reference_value"] == pytest.approx(57.8)
    with pytest.raises(InvalidRequest):
        build_record(spec, {"a.xml": XML + b" "})
    assert kmax_number("\u2212.5 \u00b1 .2") == pytest.approx(-0.5)


def test_kmax_candidates_centre_the_covariate_and_read_family_contexts() -> None:
    from peykar.model_learning.eye_recovery_population import level_covariate
    from peykar.model_learning.eye_support import SupportConfig
    from peykar.model_learning.kmax_population import (
        DEVELOPMENT_CANDIDATES,
        adapter,
        family_context,
    )

    assert set(DEVELOPMENT_CANDIDATES) == {"k0", "k1", "k2", "k3", "k4", "k5", "k6", "k7"}
    adopted = DEVELOPMENT_CANDIDATES["k6"]
    assert adopted.transient_months is None and adopted.dimensions == 3
    assert adopted.forecast_noise_pooling == "pooled" and adopted.noise_pooling == "median"
    assert DEVELOPMENT_CANDIDATES["k7"].noise_pooling == "pooled"
    assert DEVELOPMENT_CANDIDATES["k2"].grouping == ("treatment", "context")
    assert isinstance(DEVELOPMENT_CANDIDATES["k3"], SupportConfig)
    assert DEVELOPMENT_CANDIDATES["k5"].transient_months == 1.0
    series = adapter("k6")(_record())
    # The level covariate is (Kmax0 - 55 D) / 5 D.
    assert level_covariate(adopted, series[0]) == pytest.approx((58.2 - 55.0) / 5.0)
    assert [s.context for s in series] == [
        "human/cxl_epi_off",
        "human/cxl_accelerated",
        "human/observation",
    ]
    assert [s.context for s in adapter("k2")(_record())] == [
        "human/cxl_epi_off/cxl",
        "human/cxl_accelerated/cxl",
        "human/observation/observation",
    ]
    assert family_context("human/other") == "human/other/cxl"
    assert family_context("human/icrs") == "human/icrs/icrs"


def test_kmax_publisher_keeps_iop_comparators_and_redacts_initial_states() -> None:
    import importlib.util
    from pathlib import Path

    path = Path(__file__).resolve().parents[2] / "scripts/publish_kmax_population.py"
    spec = importlib.util.spec_from_file_location("publish_kmax_population", path)
    assert spec is not None and spec.loader is not None
    publish = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(publish)
    assert publish.base.H3_COMPARATORS == ("persistence", "pooled_curve_ratio", "analog_knn")
    assert publish.base.PRIMARY["next_observation"] == "population_assimilation_t4"
    assert "baseline_level" in publish.base.REPORTED
    entry = {
        "series_id": "s",
        "context": "human/cxl_epi_off",
        "kind": "group_mean",
        "reference_value": 56.0,
        "observed": [0.9],
        "months": [12.0],
        "predictions": {"population_initial_state": [0.92]},
        "initial_state_intervals": {"population_initial_state": {"0.1": [0.8]}},
        "scores": {},
    }
    redacted = publish.redact({"initial_state": [entry], "next_observation": [], "landmark": []})
    assert set(redacted["initial_state"][0]) <= publish.INITIAL_STATE_PUBLIC_FIELDS
    assert "reference_value" not in redacted["initial_state"][0]
    assert "initial_state_intervals" not in redacted["initial_state"][0]
