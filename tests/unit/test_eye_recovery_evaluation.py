"""Generic eye recovery harness on synthetic retinal-thickness series."""

from __future__ import annotations

import math

import numpy as np
import pytest

from peykar.model_learning import eye_recovery_population as population
from peykar.model_learning.retina_thickness_population import DEVELOPMENT_CANDIDATES
from peykar.validation import eye_recovery_evaluation as evaluation
from peykar.validation.eye_retina_data import RetinaSeries
from peykar_core.errors import InvalidRequest

CONFIG = DEVELOPMENT_CANDIDATES["r1"]
MONTHS = (0.25, 1.0, 3.0, 6.0, 12.0)


def _series() -> list[RetinaSeries]:
    rng = np.random.default_rng(11)
    contexts = {
        "human/dme/anti_vegf": (-0.3, 0.05, -0.1, 2.0),
        "human/rvo/anti_vegf": (-0.4, 0.05, -0.1, 2.0),
        "human/dme/steroid_implant": (-0.1, -0.3, 0.2, 1.5),
    }
    out = []
    for c, (context, (level, excess, transient, tau)) in enumerate(sorted(contexts.items())):
        for s in range(2):
            study = f"ret-st{c}{s}"
            for k, n in enumerate((25, None)):
                reference = float(rng.uniform(320, 600))
                x = math.log(reference / 300.0)
                curve = population.law_fraction(
                    CONFIG,
                    MONTHS,
                    level=level - 0.5 * x + rng.normal(0, 0.03),
                    excess=excess,
                    transient=transient,
                    tau_months=tau,
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
                        role="control",
                    )
                )
    return out


def test_leave_one_study_out_scores_every_task_without_leakage() -> None:
    series = _series()
    report = evaluation.leave_one_study_out(
        CONFIG, series, studies=["ret-st00", "ret-st20"], clusters=[["ret-st20", "ret-st21"]]
    )
    first, second = report["studies"]
    assert report["model_version"] == CONFIG.model_version
    assert "ret-st00" not in first["training_studies"]
    assert second["training_exclusions"] == ["ret-st21"]
    assert "ret-st21" not in second["training_studies"]
    entry = first["initial_state"][0]
    assert {"population_initial_state", "pooled_training_curve"} <= set(entry["predictions"])
    assert entry["population_level_used"] == entry["context"]
    rows = first["next_observation"][0]["rows"]
    assert len(rows) == len(MONTHS) - 1
    assert {"persistence", "baseline_level", "population_assimilation", "weak_prior"} <= set(
        rows[0]["predictions"]
    )
    landmark = first["landmark"][0]["rows"]
    assert [r["target_month"] for r in landmark] == list(MONTHS[1:])
    summary = report["summary"]
    assert summary["next_observation_group_means"]["persistence"]["series"] == 2
    assert "central retinal thickness" in report["criteria"]["unit"]


def test_unseen_indication_uses_its_treatment_and_leakage_is_refused() -> None:
    series = _series()
    held = [
        RetinaSeries(
            series_id="ret-new/0",
            study_id="ret-new",
            context="human/uveitic/steroid_implant",
            reference_value=480.0,
            months=(1.0, 3.0),
            fractions=(0.7, 0.8),
            aggregate_n=20,
        )
    ]
    result = evaluation.evaluate_study(CONFIG, study_series=held, training=series)
    entry = result["initial_state"][0]
    assert entry["population_level_used"] == "treatment:steroid_implant"
    assert entry["pooled_level_used"] == "treatment:steroid_implant"
    with pytest.raises(InvalidRequest):
        evaluation.evaluate_study(CONFIG, study_series=held, training=[*series, *held])
    curve = evaluation.pooled_log_curve(CONFIG, series, "human/csc/laser", [1.0])
    assert curve is not None and curve[1] == "family:human/all"
