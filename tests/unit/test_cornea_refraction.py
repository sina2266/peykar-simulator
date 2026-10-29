"""Refraction records, additive pseudo-fractions and the additive-scale evaluation."""

from __future__ import annotations

import hashlib
import importlib.util
import math
from pathlib import Path
from types import ModuleType
from typing import Any

import numpy as np
import pytest

from peykar.model_learning import eye_recovery_population2 as population
from peykar.validation import eye_additive_evaluation as additive
from peykar.validation import eye_recovery_evaluation2 as harness2
from peykar.validation.cornea_refraction_data import (
    COVARIATE_UNIT_D,
    MODEL_UNIT_D,
    RefractionSeries,
    build_record,
    change_from_fraction,
    reference_from_value,
    refraction_number,
    structural_summary,
    to_refraction_series,
    validate_record,
)
from peykar_core.errors import InvalidRequest

SCALE = additive.AdditiveScale(
    quantity="spherical equivalent",
    unit="D",
    model_unit=MODEL_UNIT_D,
    covariate_unit=COVARIATE_UNIT_D,
    report_unit=0.1,
)
CONFIG = population.RecoveryConfig(
    model_id="cornea.refraction-population",
    model_version="test",
    quantity="spherical equivalent change (pseudo-fraction)",
    covariate_reference=1.0,
    covariate_unit="pseudo-reference exp(SE0 / 5 D)",
    study_effects="per_group",
)
MONTHS = (1 / 30.4375, 0.25, 1.0, 3.0, 6.0, 12.0)


def _record(**overrides: object) -> dict:
    record = {
        "schema": "cornea.refraction-record-1.0.0",
        "study_id": "ref-pmc1",
        "doi": "10.1/x",
        "species": "human",
        "procedure": "lenticule_extraction",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [
            {"group": "smile", "role": "control", "refractive_error": "myopia", "target": "plano"},
            {
                "group": "lasik",
                "role": "control",
                "procedure": "lasik",
                "refractive_error": "myopia",
                "target": "unstated",
            },
        ],
        "series": [
            {
                "series_id": "smile/se",
                "group": "smile",
                "kind": "group_mean",
                "n": 40,
                "value_kind": "se_absolute",
                "method": "manifest",
                "months": [0, 1, 3, 12],
                "values": [-6.0, -0.25, -0.1, -0.4],
                "source_cells": ["a", "b", "c", "d"],
            },
            {
                "series_id": "lasik/change",
                "group": "lasik",
                "kind": "group_mean",
                "n": 30,
                "value_kind": "se_change",
                "reference_se": -4.5,
                "reference_cell": "a.xml::t1!R2C2",
                "method": "unstated",
                "months": [0, 3],
                "values": [0.0, 4.25],
                "source_cells": ["definition:x", "e"],
            },
        ],
    }
    record.update(overrides)
    return record


def test_changes_contexts_and_exact_pseudo_fractions() -> None:
    checked = validate_record(_record())
    assert checked["contexts"] == ["human/lasik", "human/lenticule_extraction"]
    first, second = checked["series"]
    assert first["changes_d"] == pytest.approx([0.0, 5.75, 5.9, 5.6])
    assert second["changes_d"] == pytest.approx([0.0, 4.25])
    series = to_refraction_series(_record())
    smile = series[0]
    assert smile.months == (1.0, 3.0, 12.0) and smile.reference_d == -6.0
    assert [change_from_fraction(f) for f in smile.fractions] == pytest.approx([5.75, 5.9, 5.6])
    assert reference_from_value(smile.reference_value) == pytest.approx(-6.0)
    assert math.log(smile.reference_value / CONFIG.covariate_reference) == pytest.approx(-6 / 5)
    assert smile.prefix(3.0).changes_d == (5.75, 5.9) and smile.kind == "aggregate"
    summary = structural_summary([_record()])["studies"][0]
    assert summary["refractive_errors"] == ["myopia"] and "values" not in summary


def test_invalid_refraction_records_are_rejected() -> None:
    no_target = _record()
    no_target["groups"][0]["target"] = "monovision"
    with pytest.raises(InvalidRequest):
        validate_record(no_target)
    out_of_range = _record()
    out_of_range["series"][0]["values"] = [-30.0, -1.0, -1.0, -1.0]
    with pytest.raises(InvalidRequest):
        validate_record(out_of_range)
    unreferenced = _record()
    del unreferenced["series"][1]["reference_cell"]
    with pytest.raises(InvalidRequest):
        validate_record(unreferenced)
    with pytest.raises(InvalidRequest):
        validate_record(_record(procedure="phakic_iol"))


XML = (
    b'<article><body><table-wrap id="t1"><table><thead><tr><th>Time</th><th>SE (D)</th></tr>'
    b"</thead><tbody><tr><td>Preop</td><td>\xe2\x88\x925.25 \xc2\xb1 1.50</td></tr>"
    b"<tr><td>1 day</td><td>+0.12 \xc2\xb1 0.35</td></tr>"
    b"<tr><td>6 months</td><td>\xe2\x88\x920,25 \xc2\xb1 0,40</td></tr></tbody></table>"
    b"</table-wrap></body></article>"
)


def test_builder_reads_signed_cells_and_checks_hashes() -> None:
    spec = {
        "study_id": "ref-pmc2",
        "doi": "10.1/y",
        "species": "human",
        "procedure": "surface_ablation",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": hashlib.sha256(XML).hexdigest()}],
        "groups": [
            {"group": "prk", "role": "control", "refractive_error": "myopia", "target": "plano"}
        ],
        "series": [
            {
                "series_id": "prk/se",
                "group": "prk",
                "kind": "group_mean",
                "n": 20,
                "value_kind": "se_absolute",
                "method": "manifest",
                "months": [0, 0.0329, 6],
                "cells": ["a.xml::t1!R2C2", "a.xml::t1!R3C2", "a.xml::t1!R4C2"],
            }
        ],
    }
    record = build_record(spec, {"a.xml": XML})
    assert record["series"][0]["values"] == [-5.25, 0.12, -0.25]
    with pytest.raises(InvalidRequest):
        build_record(spec, {"a.xml": XML + b" "})
    assert refraction_number("\u0394\u22120.50 \u00b1 0.2") == pytest.approx(-0.5)
    assert refraction_number("\u2212\u00a02.26 \u00b1 1.1") == pytest.approx(-2.26)
    assert refraction_number("\u2212\u20090.75") == pytest.approx(-0.75)
    assert refraction_number("- 1.50 (0.4)") == pytest.approx(-1.5)
    assert refraction_number("\u20123.25 \u00b1 0.9") == pytest.approx(-3.25)
    assert refraction_number("+ 0.12") == pytest.approx(0.12)
    assert refraction_number("0.25 \u2012 0.50") == pytest.approx(0.25)
    with pytest.raises(InvalidRequest):
        refraction_number("\u2012")


def _library() -> list[RefractionSeries]:
    rng = np.random.default_rng(29)
    out = []
    for s in range(4):
        study = f"ref-syn{s}"
        offset = rng.normal(0, 0.04)
        for k, n in enumerate((40, 30)):
            se0 = float(rng.uniform(-8.0, -2.0))
            x = se0 / COVARIATE_UNIT_D
            beta = np.array([0.02 - 0.98 * x + offset, -0.03, 0.02])
            z = population.design(CONFIG, MONTHS, np.array([math.log(2.0)]))[0] @ beta
            changes = MODEL_UNIT_D * z + rng.normal(0, 0.08, len(MONTHS))
            out.append(
                RefractionSeries(
                    series_id=f"{study}/{k}",
                    study_id=study,
                    context="human/lasik",
                    reference_d=se0,
                    months=MONTHS,
                    changes_d=tuple(float(v) for v in changes),
                    aggregate_n=n,
                )
            )
    return out


def test_additive_scores_are_native_errors_in_report_units() -> None:
    series = _library()
    raw = harness2.evaluate_study(
        CONFIG,
        study_series=[s for s in series if s.study_id == "ref-syn0"],
        training=[s for s in series if s.study_id != "ref-syn0"],
    )
    report = additive.leave_one_study_out(CONFIG, series, scale=SCALE, studies=["ref-syn0"])
    study = report["studies"][0]
    assert report["criteria"]["rmse_max"] == "0.5 D" and report["scale"]["report_unit"] == 0.1
    for task in ("next_observation", "landmark"):
        for old, new in zip(raw[task], study[task], strict=True):
            native = [
                (change_from_fraction(r["predictions"]["population_assimilation"]))
                - change_from_fraction(r["observed"])
                for r in old["rows"]
            ]
            expected = math.sqrt(sum((e / 0.1) ** 2 for e in native) / len(native))
            score = new["scores"]["population_assimilation"]
            assert score["rmse_pp"] == pytest.approx(expected)
            assert new["interval_coverage"] == old["interval_coverage"]
            persistence = new["scores"]["persistence"]["rmse_pp"]
            assert score["improves_persistence_by_0_5_pp"] == (
                score["rmse_pp"] <= persistence - 0.5
            )
    initial = study["initial_state"][0]
    held = next(s for s in series if s.series_id == initial["series_id"])
    assert initial["reference_value"] == pytest.approx(held.reference_d)
    assert [100 * (g - 1) for g in initial["observed"]] == pytest.approx(
        [c / 0.1 for c in held.changes_d]
    )
    long_term = initial["predictions"]["population_initial_state"][-1]
    assert abs(100 * (long_term - 1) * 0.1 - held.changes_d[-1]) < 0.5
    assert "target_level" not in initial["predictions"]
    planned = additive.AdditiveScale(**{**SCALE.describe(), "target": 0.0})
    targeted = additive.leave_one_study_out(CONFIG, series, scale=planned, studies=["ref-syn0"])
    entry = targeted["studies"][0]["initial_state"][0]
    assert entry["predictions"]["target_level"] == pytest.approx(
        [1 - held.reference_d / 10] * len(held.months)
    )
    row = targeted["studies"][0]["next_observation"][0]["rows"][0]
    series_reference = next(s for s in series if s.series_id == entry["series_id"]).reference_d
    assert row["predictions"]["target_level"] == pytest.approx(1 - series_reference / 10)
    assert "target_level" in targeted["studies"][0]["next_observation"][0]["scores"]


def _publisher() -> ModuleType:
    path = Path(__file__).resolve().parents[2] / "scripts/publish_refraction_population.py"
    spec = importlib.util.spec_from_file_location("publish_refraction_population", path)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_refraction_publisher_tests_initial_state_against_plano_and_f0() -> None:
    publish = _publisher()
    assert "target_level" in publish.base.H2_COMPARATORS
    assert "baseline_level" in publish.base.REPORTED

    def initial(predicted: list[float], target: float) -> dict[str, Any]:
        return {
            "series_id": "s",
            "context": "human/myopia/lasik",
            "kind": "group_mean",
            "observed": [1.0, 1.0],
            "predictions": {
                "population_initial_state": predicted,
                "target_level": [target, target],
            },
            "scores": {},
        }

    report = {"studies": [{"study_id": "x", "initial_state": [initial([1.01, 0.99], 1.03)]}]}
    h4 = publish.initial_versus_target(report)
    assert h4["common_targets"] == 2 and h4["population_lower"]
    assert h4["population_initial_state_rmse_pp"] == pytest.approx(1.0)
    redacted = publish.redact(
        {
            "initial_state": [{**initial([1.0, 1.0], 1.0), "reference_value": -5.0}],
            "next_observation": [],
            "landmark": [],
        }
    )
    assert set(redacted["initial_state"][0]) <= publish.INITIAL_STATE_PUBLIC_FIELDS

    def h5(initial_f1: float, next_f1: float) -> dict[str, Any]:
        return {
            "initial_state": {"common_targets": 4, "f1_rmse_pp": initial_f1, "f0_rmse_pp": 5.5},
            "next_observation": {"common_targets": 4, "f1_rmse_pp": next_f1, "f0_rmse_pp": 1.5},
            "landmark": {"common_targets": 4, "f1_rmse_pp": 3.2, "f0_rmse_pp": 3.3},
        }

    assert publish.h5_passes(h5(5.4, 1.54))
    assert not publish.h5_passes(h5(5.6, 1.5))
    assert not publish.h5_passes(h5(5.4, 1.56))
