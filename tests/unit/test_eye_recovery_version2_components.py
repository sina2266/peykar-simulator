"""Retina 1.1 dose rule and series, IOP 1.2 series, and the version-2 harness."""

from __future__ import annotations

import math
from dataclasses import replace

import numpy as np
import pytest

from peykar.model_learning import eye_recovery_population2 as population
from peykar.model_learning import iop_population_1_2 as iop12
from peykar.model_learning.retina_thickness_population_1_1 import (
    DEVELOPMENT_CANDIDATES,
    DEXAMETHASONE_IMPLANT_MONTHS,
    effect_duration_months,
)
from peykar.validation import eye_recovery_evaluation2 as evaluation
from peykar.validation.eye_iop_data import PressureSeries
from peykar.validation.eye_iop_data_1_2 import IopSeries
from peykar.validation.eye_retina_data import RetinaSeries
from peykar.validation.eye_retina_data_1_1 import to_retina_series_1_1
from peykar_core.errors import InvalidRequest

CONFIG = DEVELOPMENT_CANDIDATES["s2"]
MONTHS = (0.25, 1.0, 2.0, 4.0, 6.0)


def test_dose_rule() -> None:
    ozurdex = "dexamethasone 0.7 mg intravitreal implant (Ozurdex), single injection"
    assert effect_duration_months("steroid_implant", "single", ozurdex) == 3.5
    assert DEXAMETHASONE_IMPLANT_MONTHS == 3.5
    assert effect_duration_months("steroid_implant", "single", "fluocinolone (ILUVIEN)") is None
    assert effect_duration_months("steroid_implant", "prn", ozurdex) is None
    assert effect_duration_months("steroid_implant", "single", None) is None
    assert effect_duration_months("combination", "single", "vitrectomy plus Ozurdex") == 3.5
    assert effect_duration_months("combination", "single", "phaco + dexamethasone drops") is None
    assert effect_duration_months("anti_vegf", "single", "ranibizumab") is None


def _record() -> dict:
    return {
        "schema": "eye.retina-thickness-record-1.0.0",
        "study_id": "ret-synthetic",
        "doi": "10.0/x",
        "species": "human",
        "indication": "dme",
        "treatment": "steroid_implant",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [
            {"group": "dex", "role": "control", "agent": "dexamethasone implant (Ozurdex)"},
            {"group": "fac", "role": "control", "agent": "fluocinolone implant (ILUVIEN)"},
        ],
        "series": [
            {
                "series_id": f"{group}/cst",
                "group": group,
                "kind": "group_mean",
                "n": 20,
                "value_kind": "thickness_absolute",
                "measure": "central_subfield",
                "device": "spectralis",
                "regimen": "single",
                "prior_treatment": "naive",
                "months": [0.0, 2.0, 4.0],
                "values": [500.0, 350.0, 450.0],
                "source_cells": ["a.xml::T1!R1C1", "a.xml::T1!R1C2", "a.xml::T1!R1C3"],
            }
            for group in ("dex", "fac")
        ],
    }


def test_retina_series_carry_the_declared_duration() -> None:
    dex, fac = to_retina_series_1_1(_record())
    assert population.effect_duration(dex) == 3.5
    assert population.effect_duration(fac) is None
    assert population.effect_duration(dex.prefix(2.0)) == 3.5


def test_iop_series_expose_the_generic_reference() -> None:
    item = IopSeries(
        series_id="s/1",
        study_id="s",
        context="human/tube",
        reference_mmhg=24.0,
        months=(0.25, 1.0, 3.0),
        fractions=(0.6, 0.7, 0.65),
        aggregate_n=30,
    )
    assert item.reference_value == 24.0 and isinstance(item, PressureSeries)
    prefix = item.prefix(1.0)
    assert isinstance(prefix, IopSeries) and prefix.months == (0.25, 1.0)
    c0 = iop12.DEVELOPMENT_CANDIDATES["c0"]
    assert c0.deviation_fraction == 0.0 and c0.early_noise is None
    assert c0.covariate_reference == 20.0 and c0.grouping == ("context",)


def _series() -> list[RetinaSeries]:
    rng = np.random.default_rng(13)
    contexts = {
        "human/dme/anti_vegf": (-0.3, 0.05, -0.1, 2.0, None),
        "human/rvo/anti_vegf": (-0.4, 0.05, -0.1, 2.0, None),
        "human/dme/steroid_implant": (-0.4, 0.1, -0.05, 0.5, 3.5),
    }
    out = []
    for c, (context, (level, excess, transient, tau, duration)) in enumerate(
        sorted(contexts.items())
    ):
        for s in range(2):
            study = f"ret-st{c}{s}"
            for k, n in enumerate((25, None)):
                reference = float(rng.uniform(320, 600))
                x = math.log(reference / 300.0)
                beta = np.array([level - 0.5 * x + rng.normal(0, 0.03), excess, transient])
                curve = np.exp(
                    population.design(CONFIG, MONTHS, np.array([math.log(tau)]), duration)[0] @ beta
                )
                values = curve * np.exp(rng.normal(0, 0.03 if n else 0.06, len(MONTHS)))
                out.append(
                    RetinaSeries(
                        series_id=f"{study}/{k}",
                        study_id=study,
                        context=context,
                        reference_value=reference,
                        months=MONTHS,
                        fractions=tuple(float(v) for v in values),
                        aggregate_n=n,
                        metadata={"effect_duration_months": duration},
                    )
                )
    return out


def test_version_2_harness_uses_the_duration_in_every_task() -> None:
    series = _series()
    report = evaluation.leave_one_study_out(CONFIG, series, studies=["ret-st10"])
    study = report["studies"][0]
    assert study["evaluation_version"] == "2.0.0" and "ret-st10" not in study["training_studies"]
    implant = next(e for e in study["initial_state"] if e["kind"] == "group_mean")
    predicted = implant["predictions"]["population_initial_state"]
    assert predicted[-1] > predicted[2]  # the forecast rises after the wear-off
    without = [replace(s, metadata={}) for s in series]
    plain = evaluation.leave_one_study_out(CONFIG, without, studies=["ret-st10"])
    plain_entry = next(e for e in plain["studies"][0]["initial_state"] if e["kind"] == "group_mean")
    assert plain_entry["predictions"]["population_initial_state"] != predicted
    rows = study["next_observation"][0]["rows"]
    assert {"persistence", "population_assimilation", "analog_knn"} <= set(rows[0]["predictions"])
    with pytest.raises(InvalidRequest):
        evaluation.evaluate_study(CONFIG, study_series=series[:2], training=series)
