"""Retinal visual-acuity model 1.1 configurations and dose adapter (synthetic data only)."""

from __future__ import annotations

import math

import numpy as np

from peykar.model_learning import eye_recovery_population2 as population
from peykar.model_learning import retina_acuity_population as version_1_0
from peykar.model_learning.retina_acuity_population_1_1 import (
    DEVELOPMENT_CANDIDATES,
    adapter,
    to_retina_acuity_series_dose,
)
from peykar.validation.eye_retina_acuity_data import RetinaAcuitySeries, to_retina_acuity_series


def _record(agent: str, treatment: str, regimen: str) -> dict:
    return {
        "schema": "eye.retina-acuity-record-1.0.0",
        "study_id": "rac-synthetic-1",
        "doi": "10.0/rac1",
        "species": "human",
        "indication": "dme",
        "treatment": treatment,
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [{"group": "g", "role": "control", "agent": agent}],
        "series": [
            {
                "series_id": "g/bcva",
                "group": "g",
                "kind": "group_mean",
                "n": 30,
                "value_kind": "logmar_absolute",
                "regimen": regimen,
                "prior_treatment": "naive",
                "missing_data": "observed",
                "months": [0.0, 1.0, 2.0, 4.0, 6.0],
                "values": [0.8, 0.6, 0.5, 0.65, 0.75],
                "source_cells": [f"a.xml::T1!R{i}C1" for i in range(5)],
            }
        ],
    }


def test_reference_is_the_frozen_model_and_candidates_differ_only_as_declared() -> None:
    reference = DEVELOPMENT_CANDIDATES["w0"]
    assert reference is version_1_0.DEVELOPMENT_CANDIDATES["v3"]
    base = reference.describe()
    changed = {
        name: {k for k, v in config.describe().items() if base.get(k) != v}
        for name, config in DEVELOPMENT_CANDIDATES.items()
    }
    assert changed["w0"] == set()
    assert changed["w1"] == {"model_version", "transient_days", "penalty_modes", "robust_sd"}
    assert changed["w2"] == {"model_version", "treatment_fallback"}
    assert changed["w3"] == {"model_version", "wear_off_loss"}
    assert DEVELOPMENT_CANDIDATES["w1"].dimensions == 3 and reference.dimensions == 4


def test_dose_adapter_sets_durations_for_one_dexamethasone_implant_only() -> None:
    single = to_retina_acuity_series_dose(_record("Ozurdex 0.7 mg", "steroid_implant", "single"))
    assert single[0].metadata["effect_duration_months"] == 3.5
    repeated = to_retina_acuity_series_dose(_record("Ozurdex 0.7 mg", "steroid_implant", "prn"))
    assert repeated[0].metadata["effect_duration_months"] is None
    long_acting = to_retina_acuity_series_dose(
        _record("fluocinolone acetonide implant", "steroid_implant", "single")
    )
    assert long_acting[0].metadata["effect_duration_months"] is None
    plain = to_retina_acuity_series(_record("Ozurdex 0.7 mg", "steroid_implant", "single"))
    assert "effect_duration_months" not in plain[0].metadata
    assert adapter("w3") is to_retina_acuity_series_dose
    assert adapter("w1") is to_retina_acuity_series


def test_the_law_without_transient_fits_and_forecasts() -> None:
    config = DEVELOPMENT_CANDIDATES["w1"]
    rng = np.random.default_rng(5)
    months = (1.0, 3.0, 6.0, 12.0)
    series = []
    for s in range(6):
        for context in ("human/dme/anti_vegf", "human/rvo/anti_vegf"):
            logmar0 = float(rng.uniform(0.4, 0.9))
            changes = [
                -0.15 - 0.1 * math.exp(-(m - 1 / 30.4375) / 2.0) + float(rng.normal(0, 0.01))
                for m in months
            ]
            series.append(
                RetinaAcuitySeries(
                    series_id=f"st{s}/{context}",
                    study_id=f"st{s}",
                    context=context,
                    reference_logmar=logmar0,
                    months=months,
                    changes_logmar=tuple(changes),
                    aggregate_n=30,
                )
            )
    fit = population.fit_population(config, series)
    held = series[0]
    prior = fit.prior(held.context, held.kind, population.level_covariate(config, held))
    assert len(prior["mean"]) == 3
    posterior = population.posterior(
        config,
        population.gaussian(prior["mean"], prior["covariance"]),
        series=None,
        noise=fit.noise(held.kind, held.context, 30),
    )
    forecast = population.predictive(config, posterior, months=[0.0329, 6.0], quantiles=(0.5,))
    changes = [math.log(f) for f in forecast["quantiles"]["0.5"]]
    assert all(-0.6 < c < 0.1 for c in changes)


def _publisher():
    import importlib.util
    from pathlib import Path

    path = Path(__file__).resolve().parents[2] / "scripts/publish_retina_acuity_population_1_1.py"
    spec = importlib.util.spec_from_file_location("publish_retina_acuity_population_1_1", path)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _report(next_error: float, landmark_error: float, initial_error: float, method: str) -> dict:
    def rows(error: float) -> list[dict]:
        return [
            {
                "origin_month": 1.0,
                "target_month": 3.0,
                "observed": 0.9,
                "predictions": {method: 0.9 + error / 100.0},
            }
        ]

    def entry(error: float, context: str = "human/dme/anti_vegf") -> dict:
        return {
            "series_id": "g/bcva",
            "context": context,
            "kind": "group_mean",
            "rows": rows(error),
        }

    return {
        "studies": [
            {
                "study_id": "rac-pmc1",
                "next_observation": [entry(next_error), entry(50.0, "rabbit/dme/anti_vegf")],
                "landmark": [entry(landmark_error)],
                "initial_state": [
                    {
                        "series_id": "g/bcva",
                        "context": "human/dme/anti_vegf",
                        "kind": "group_mean",
                        "months": [1.0, 3.0],
                        "observed": [0.9, 0.8],
                        "predictions": {
                            "population_initial_state": [
                                0.9 + initial_error / 100.0,
                                0.8 - initial_error / 100.0,
                            ]
                        },
                    }
                ],
            }
        ]
    }


def test_retina_acuity_1_1_publisher_points_the_frozen_analysis_at_1_1() -> None:
    module = _publisher()
    assert module.MODEL_VERSION == "1.1.0-w1"
    assert module.REFERENCE_VERSION == "1.0.0-v3"
    expected = {
        "next_observation": "population_anchored_t4",
        "landmark": "population_anchored_t4",
        "initial_state": "population_initial_state",
    }
    assert expected == module.PRIMARY
    assert expected == module.previous.PRIMARY
    assert expected == module.base.PRIMARY
    assert module.previous.RECORD_DIRS is module.RECORD_DIRS
    assert len(module.RECORD_DIRS["development"]) == 3
    assert module.RECORD_DIRS["sealed"][0].name == "records"
    assert "tranche2" in str(module.RECORD_DIRS["sealed"][0])


def test_retina_acuity_1_1_h5_compares_common_human_group_means() -> None:
    module = _publisher()
    new = _report(3.0, 4.0, 5.0, "population_anchored_t4")
    old = _report(3.02, 6.0, 7.0, "population_assimilation_t4")
    h5 = module.versus_reference(new, old)
    assert h5["next_observation"]["common_targets"] == 1
    assert math.isclose(h5["next_observation"]["model_1_1_rmse_pp"], 3.0)
    assert math.isclose(h5["landmark"]["model_1_0_rmse_pp"], 6.0)
    assert h5["initial_state"]["common_targets"] == 2
    assert math.isclose(h5["initial_state"]["model_1_1_rmse_pp"], 5.0)
    assert module.h5_passes(h5)
    worse_next = module.versus_reference(_report(3.1, 4.0, 5.0, "population_anchored_t4"), old)
    assert not module.h5_passes(worse_next)
    worse_landmark = module.versus_reference(_report(3.0, 6.0, 5.0, "population_anchored_t4"), old)
    assert not module.h5_passes(worse_landmark)
