"""Childhood-myopia axial-length records and additive pseudo-fractions (synthetic data)."""

from __future__ import annotations

import hashlib
import math

import pytest

from peykar.validation.myopia_axial_data import (
    COVARIATE_UNIT_MM,
    MODEL_UNIT_MM,
    REPORT_UNIT_MM,
    axial_number,
    build_record,
    change_from_fraction,
    reference_from_value,
    structural_summary,
    to_axial_series,
    validate_record,
)
from peykar_core.errors import InvalidRequest


def _record(**overrides: object) -> dict:
    record = {
        "schema": "eye.myopia-axial-record-1.0.0",
        "study_id": "myo-pmc1",
        "doi": "10.1/x",
        "species": "human",
        "treatment": "atropine",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [
            {
                "group": "atropine-0.01",
                "role": "control",
                "region": "east_asia",
                "mean_age_years": 8.4,
                "atropine_percent": 0.01,
            },
            {
                "group": "placebo",
                "role": "control",
                "treatment": "single_vision",
                "region": "east_asia",
                "mean_age_years": 8.3,
            },
            {
                "group": "ortho-k",
                "role": "control",
                "treatment": "orthokeratology",
                "region": "europe",
            },
        ],
        "series": [
            {
                "series_id": "atropine-0.01/al",
                "group": "atropine-0.01",
                "kind": "group_mean",
                "n": 110,
                "device": "iolmaster",
                "value_kind": "axial_absolute",
                "months": [0, 6, 12, 24],
                "values": [24.70, 24.85, 24.98, 25.21],
                "source_cells": ["a", "b", "c", "d"],
            },
            {
                "series_id": "placebo/change",
                "group": "placebo",
                "kind": "group_mean",
                "n": 111,
                "device": "iolmaster",
                "value_kind": "axial_change",
                "reference_value": 24.82,
                "reference_cell": "a.xml::t1!R2C3",
                "months": [0, 12],
                "values": [0.0, 0.41],
                "source_cells": ["definition:x", "e"],
            },
            {
                "series_id": "ortho-k/al",
                "group": "ortho-k",
                "kind": "individual",
                "device": "lenstar",
                "value_kind": "axial_absolute",
                "months": [0, 12],
                "values": [25.10, 25.18],
                "source_cells": ["f", "g"],
            },
        ],
    }
    record.update(overrides)
    return record


def test_axial_changes_and_exact_pseudo_fractions() -> None:
    checked = validate_record(_record())
    assert checked["contexts"] == [
        "human/atropine",
        "human/orthokeratology",
        "human/single_vision",
    ]
    atropine, placebo, orthok = checked["series"]
    assert atropine["changes_mm"] == pytest.approx([0.0, 0.15, 0.28, 0.51])
    assert atropine["atropine_percent"] == 0.01 and atropine["mean_age_years"] == 8.4
    assert placebo["reference_mm"] == 24.82
    assert placebo["changes_mm"] == pytest.approx([0.0, 0.41])
    assert orthok["changes_mm"] == pytest.approx([0.0, 0.08]) and orthok["region"] == "europe"
    series = to_axial_series(_record())
    first = series[0]
    assert first.months == (6.0, 12.0, 24.0) and first.reference_mm == 24.70
    assert [change_from_fraction(f) for f in first.fractions] == pytest.approx([0.15, 0.28, 0.51])
    assert reference_from_value(first.reference_value) == pytest.approx(24.70)
    assert math.log(first.reference_value) == pytest.approx(24.70 / COVARIATE_UNIT_MM)
    assert MODEL_UNIT_MM == 1.0 and REPORT_UNIT_MM == 0.01
    assert first.prefix(12.0).changes_mm == pytest.approx((0.15, 0.28))
    assert first.kind == "aggregate" and series[2].kind == "individual"
    assert series[2].device == "lenstar" and series[1].context == "human/single_vision"
    summary = structural_summary([_record()])["studies"][0]
    assert summary["atropine_percents"] == [0.01] and summary["mean_age_reported"] == 2
    assert summary["regions"] == ["east_asia", "europe"] and "values" not in summary


def test_invalid_axial_records_are_rejected() -> None:
    with pytest.raises(InvalidRequest):
        validate_record(_record(species="rabbit"))
    with pytest.raises(InvalidRequest):
        validate_record(_record(treatment="lasik"))
    no_dose = _record()
    del no_dose["groups"][0]["atropine_percent"]
    with pytest.raises(InvalidRequest):
        validate_record(no_dose)
    no_region = _record()
    del no_region["groups"][1]["region"]
    with pytest.raises(InvalidRequest):
        validate_record(no_region)
    adult = _record()
    adult["groups"][0]["mean_age_years"] = 24.0
    with pytest.raises(InvalidRequest):
        validate_record(adult)
    out_of_range = _record()
    out_of_range["series"][0]["values"] = [34.0, 24.85, 24.98, 25.21]
    with pytest.raises(InvalidRequest):
        validate_record(out_of_range)
    unreferenced = _record()
    del unreferenced["series"][1]["reference_cell"]
    with pytest.raises(InvalidRequest):
        validate_record(unreferenced)
    late = _record()
    late["series"][0]["months"] = [1, 6, 12, 24]
    with pytest.raises(InvalidRequest):
        validate_record(late)


XML = (
    b'<article><body><table-wrap id="t1"><table><thead><tr><th>Visit</th><th>AL (mm)</th>'
    b"</tr></thead><tbody>"
    b"<tr><td>Baseline</td><td>24,61 \xc2\xb1 0,92</td></tr>"
    b"<tr><td>6 months</td><td>24.75 \xc2\xb1 0.93</td></tr>"
    b"<tr><td>12 months</td><td>24.90 \xc2\xb1 0.95</td></tr>"
    b"<tr><td>Change at 12 months</td><td>.29 \xc2\xb1 .12</td></tr>"
    b"</tbody></table></table-wrap></body></article>"
)


def test_builder_reads_cells_and_checks_hashes() -> None:
    spec = {
        "study_id": "myo-pmc2",
        "doi": "10.1/y",
        "species": "human",
        "treatment": "defocus_spectacles",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": hashlib.sha256(XML).hexdigest()}],
        "groups": [{"group": "dims", "role": "control", "region": "east_asia"}],
        "series": [
            {
                "series_id": "dims/al",
                "group": "dims",
                "kind": "group_mean",
                "n": 90,
                "device": "iolmaster",
                "value_kind": "axial_absolute",
                "months": [0, 6, 12],
                "cells": ["a.xml::t1!R2C2", "a.xml::t1!R3C2", "a.xml::t1!R4C2"],
            },
            {
                "series_id": "dims/change",
                "group": "dims",
                "kind": "group_mean",
                "n": 90,
                "device": "iolmaster",
                "value_kind": "axial_change",
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
    assert record["series"][0]["values"] == pytest.approx([24.61, 24.75, 24.90])
    assert record["series"][1]["values"] == pytest.approx([0.0, 0.29])
    assert record["series"][1]["reference_value"] == pytest.approx(24.61)
    with pytest.raises(InvalidRequest):
        build_record(spec, {"a.xml": XML + b" "})
    assert axial_number("\u2212.05 \u00b1 .2") == pytest.approx(-0.05)


def test_myopia_candidates_centre_the_covariate_and_read_family_contexts() -> None:
    from peykar.model_learning.eye_recovery_population import level_covariate
    from peykar.model_learning.myopia_population import (
        DEVELOPMENT_CANDIDATES,
        adapter,
        family_context,
    )

    assert set(DEVELOPMENT_CANDIDATES) == {"m0", "m1", "m2"}
    base = DEVELOPMENT_CANDIDATES["m0"]
    assert base.transient_months is None and base.dimensions == 3
    assert base.forecast_noise_pooling == "pooled" and base.noise_pooling == "median"
    assert base.study_effects == "per_group" and base.grouping == ("context",)
    assert DEVELOPMENT_CANDIDATES["m1"].grouping == ("treatment", "context")
    assert DEVELOPMENT_CANDIDATES["m2"].forecast_noise_pooling is None
    series = adapter("m0")(_record())
    # The level covariate is (AL0 - 24.5 mm) / 1 mm.
    assert level_covariate(base, series[0]) == pytest.approx(24.70 - 24.5)
    assert [s.context for s in series] == [
        "human/atropine",
        "human/single_vision",
        "human/orthokeratology",
    ]
    assert [s.context for s in adapter("m1")(_record())] == [
        "human/atropine/atropine",
        "human/single_vision/control",
        "human/orthokeratology/optical",
    ]
    assert family_context("human/multifocal_contacts") == "human/multifocal_contacts/optical"
    assert family_context("human/combination") == "human/combination/atropine"
    assert family_context("human/red_light") == "human/red_light/light"


def test_myopia_publisher_keeps_iop_comparators_and_redacts_initial_states() -> None:
    import importlib.util
    from pathlib import Path

    path = Path(__file__).resolve().parents[2] / "scripts/publish_myopia_population.py"
    spec = importlib.util.spec_from_file_location("publish_myopia_population", path)
    assert spec is not None and spec.loader is not None
    publish = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(publish)
    assert publish.base.H3_COMPARATORS == ("persistence", "pooled_curve_ratio", "analog_knn")
    assert publish.base.PRIMARY == publish.PRIMARY
    assert "baseline_level" in publish.base.REPORTED
    entry = {
        "series_id": "s",
        "context": "human/atropine",
        "kind": "group_mean",
        "reference_value": 24.7,
        "observed": [1.28],
        "months": [12.0],
        "predictions": {"population_initial_state": [1.3]},
        "initial_state_intervals": {"population_initial_state": {"0.1": [1.2]}},
        "scores": {},
    }
    redacted = publish.redact({"initial_state": [entry], "next_observation": [], "landmark": []})
    assert set(redacted["initial_state"][0]) <= publish.INITIAL_STATE_PUBLIC_FIELDS
    assert "reference_value" not in redacted["initial_state"][0]
    assert "initial_state_intervals" not in redacted["initial_state"][0]
