"""The support hold and harness 5 (synthetic data only)."""

from __future__ import annotations

import math
from dataclasses import fields, replace

import numpy as np
import pytest

from peykar.model_learning import retina_acuity_population_1_1 as version_1_1
from peykar.model_learning.eye_support import SupportConfig, hold_month, support_months
from peykar.model_learning.retina_acuity_population_1_2 import DEVELOPMENT_CANDIDATES
from peykar.validation import eye_recovery_evaluation2 as harness2
from peykar.validation import eye_recovery_evaluation5 as harness5
from peykar.validation.eye_retina_acuity_data import RetinaAcuitySeries
from peykar_core.errors import InvalidRequest

W1 = version_1_1.DEVELOPMENT_CANDIDATES["w1"]


def _held(**changes: object) -> SupportConfig:
    values = {f.name: getattr(W1, f.name) for f in fields(W1)}
    return replace(SupportConfig(**values, support_hold=True), **changes)


def _series(study: str, context: str, months: tuple[float, ...], rng: np.random.Generator):
    changes = [
        -0.12 - 0.08 * math.exp(-(m - 1 / 30.4375) / 0.4) + float(rng.normal(0, 0.01))
        for m in months
    ]
    return RetinaAcuitySeries(
        series_id=f"{study}/{context}",
        study_id=study,
        context=context,
        reference_logmar=float(rng.uniform(0.4, 0.8)),
        months=months,
        changes_logmar=tuple(changes),
        aggregate_n=30,
    )


def _training(rng: np.random.Generator) -> list[RetinaAcuitySeries]:
    out = []
    for s in range(5):
        out.append(_series(f"st{s}", "human/dme/anti_vegf", (1.0, 3.0, 6.0, 12.0), rng))
        out.append(_series(f"st{s}", "human/rvo/anti_vegf", (2.0, 6.0, 12.0), rng))
    return out


def test_candidates_differ_from_1_1_only_by_the_declared_options() -> None:
    reference = DEVELOPMENT_CANDIDATES["x0"]
    assert reference is W1
    base = reference.describe()
    changed = {
        name: {k for k, v in config.describe().items() if base.get(k) != v}
        for name, config in DEVELOPMENT_CANDIDATES.items()
    }
    assert changed["x0"] == set()
    assert changed["x1"] == {"model_version", "support_hold", "support_rule"}
    assert changed["x2"] == {"model_version", "support_hold", "support_rule", "treatment_fallback"}
    assert changed["x3"] == {"model_version", "support_hold", "support_rule", "wear_off_loss"}
    with pytest.raises(InvalidRequest):
        _held(deviation_fraction=0.5)


def test_support_months_use_training_controls_and_fall_back_to_the_species() -> None:
    training = _training(np.random.default_rng(1))
    support = support_months(training)
    assert support["contexts"] == {"human/dme/anti_vegf": 1.0, "human/rvo/anti_vegf": 2.0}
    assert hold_month(support, "human/rvo/anti_vegf") == 2.0
    assert hold_month(support, "human/namd/anti_vegf") == 1.0
    assert hold_month(support, "rabbit/other/laser") is None


def test_hold_changes_nothing_without_early_visits_and_holds_early_ones() -> None:
    rng = np.random.default_rng(7)
    training = _training(rng)
    late = _series("held", "human/rvo/anti_vegf", (2.0, 6.0, 12.0), rng)
    plain = harness2.evaluate_study(W1, study_series=[late], training=training)
    held = harness5.evaluate_study(_held(), study_series=[late], training=training)
    for task in ("next_observation", "landmark"):
        for a, b in zip(plain[task][0]["rows"], held[task][0]["rows"], strict=True):
            for name, value in a["predictions"].items():
                assert math.isclose(value, b["predictions"][name], rel_tol=1e-12), name
    early = _series("held", "human/rvo/anti_vegf", (0.1, 0.23, 2.0, 6.0), rng)
    result = harness5.evaluate_study(_held(), study_series=[early], training=training)
    initial = result["initial_state"][0]["predictions"]["population_initial_state"]
    # Months 0.1 and 0.23 lie before the support (month 2): held at the month-2 value.
    assert math.isclose(initial[0], initial[2], rel_tol=1e-12)
    assert math.isclose(initial[1], initial[2], rel_tol=1e-12)
    # Comparators see the true months.
    plain_early = harness2.evaluate_study(W1, study_series=[early], training=training)
    for a, b in zip(plain_early["landmark"][0]["rows"], result["landmark"][0]["rows"], strict=True):
        assert a["predictions"]["persistence"] == b["predictions"]["persistence"]
        assert a["predictions"]["pooled_curve_ratio"] == b["predictions"]["pooled_curve_ratio"]
        assert b["support_hold_month"] == 2.0
