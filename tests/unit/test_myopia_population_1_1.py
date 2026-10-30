"""Childhood-myopia axial-length model 1.1 configurations (synthetic record)."""

from __future__ import annotations

import math

import numpy as np
import pytest

from peykar.model_learning import myopia_population as version1
from peykar.model_learning.myopia_population_1_1 import (
    DEVELOPMENT_CANDIDATES,
    REFERENCES,
    adapter,
)
from peykar.validation import eye_additive_evaluation5 as additive
from peykar.validation.myopia_axial_data import (
    COVARIATE_UNIT_MM,
    MODEL_UNIT_MM,
    REPORT_UNIT_MM,
    AxialSeries,
)


def _record() -> dict:
    return {
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
                "atropine_percent": 0.01,
            },
            {
                "group": "placebo",
                "role": "control",
                "treatment": "single_vision",
                "region": "east_asia",
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
                "months": [0, 6, 12],
                "values": [24.70, 24.85, 24.98],
                "source_cells": ["a", "b", "c"],
            },
            {
                "series_id": "placebo/al",
                "group": "placebo",
                "kind": "group_mean",
                "n": 111,
                "device": "iolmaster",
                "value_kind": "axial_absolute",
                "months": [0, 12],
                "values": [24.82, 25.20],
                "source_cells": ["d", "e"],
            },
        ],
    }


def test_round_zero_candidate_is_model_one_with_a_new_version_string() -> None:
    frozen = version1.DEVELOPMENT_CANDIDATES["m4"]
    assert REFERENCES["1.0.0"] is frozen
    n0 = DEVELOPMENT_CANDIDATES["n0"]
    assert n0.model_version == "1.1.0-n0" and frozen.model_version == "1.0.0-m4"
    differing = {key for key, value in n0.describe().items() if frozen.describe()[key] != value}
    assert differing == {"model_version"}


def test_round_one_candidates_change_one_setting_each() -> None:
    n0 = DEVELOPMENT_CANDIDATES["n0"].describe()

    def changed(name: str) -> dict:
        described = DEVELOPMENT_CANDIDATES[name].describe()
        return {k: v for k, v in described.items() if n0[k] != v and k != "model_version"}

    assert changed("n1") == {"max_noise_exponent": 0.0}
    assert changed("n2") == {
        "start": "zero_rate",
        "law": "ln f(t) = v (tau / R) (1 - r(t)), unchanged at t1, initial rate v per R months",
        "rate_months": 12.0,
    }
    m3 = version1.DEVELOPMENT_CANDIDATES["m3"].describe()
    two = DEVELOPMENT_CANDIDATES["n2"].describe()
    assert {k for k in m3 if m3[k] != two[k]} == {"model_version"}
    assert changed("n3") == {
        "deviation_fraction": 0.8,
        "deviation_months": 12.0,
        "deviation_in_population_fit": False,
    }


def _synthetic() -> list[AxialSeries]:
    """Three contexts, three studies each, two group means per study (zero-rate law)."""
    rng = np.random.default_rng(5)
    rates = {
        "human/single_vision/control": 0.36,
        "human/atropine/atropine": 0.26,
        "human/orthokeratology/optical": 0.18,
    }
    months = (6.0, 12.0, 18.0, 24.0)
    out = []
    for c, (context, rate) in enumerate(sorted(rates.items())):
        for s in range(3):
            study = f"st-{c}-{s}"
            for k in range(2):
                t = np.asarray(months)
                change = rate * 4.0 * (1.0 - np.exp(-t / 48.0)) + rng.normal(0.0, 0.01, t.size)
                out.append(
                    AxialSeries(
                        series_id=f"{study}/{k}",
                        study_id=study,
                        context=context,
                        reference_mm=float(rng.uniform(23.5, 25.5)),
                        months=months,
                        changes_mm=tuple(float(x) for x in change),
                        aggregate_n=int(rng.integers(20, 300)),
                    )
                )
    return out


def test_round_one_candidates_run_through_the_additive_evaluation() -> None:
    scale = additive.AdditiveScale(
        quantity="axial length",
        unit="mm",
        model_unit=MODEL_UNIT_MM,
        covariate_unit=COVARIATE_UNIT_MM,
        report_unit=REPORT_UNIT_MM,
    )
    series = _synthetic()
    for name in ("n1", "n2", "n3"):
        config = DEVELOPMENT_CANDIDATES[name]
        report = additive.leave_one_study_out(config, series, scale=scale, studies=["st-1-0"])
        assert report["harness_version"] == "7.0.0"
        study = report["studies"][0]
        assert study["population_fit"]["model_version"] == f"1.1.0-{name}"
        for task in ("next_observation", "landmark"):
            for entry in study[task]:
                for row in entry["rows"]:
                    assert math.isfinite(row["predictions"]["population_assimilation"])
                    q = row["intervals"]["population_assimilation"]
                    assert q["0.05"] <= q["0.1"] <= q["0.9"] <= q["0.95"]
    exponent = additive.leave_one_study_out(
        DEVELOPMENT_CANDIDATES["n1"], series, scale=scale, studies=["st-0-0"]
    )["studies"][0]["population_fit"]["group_mean_noise_exponent"]
    assert exponent == 0.0


def test_every_candidate_reads_family_contexts() -> None:
    for name in (*DEVELOPMENT_CANDIDATES, *REFERENCES):
        contexts = [s.context for s in adapter(name)(_record())]
        assert contexts == ["human/atropine/atropine", "human/single_vision/control"]
    with pytest.raises(KeyError):
        adapter("m0")
