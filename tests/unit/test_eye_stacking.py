"""Forecast stacking, harness 6 and additive evaluation 4 (synthetic data only)."""

from __future__ import annotations

import itertools
import math

import numpy as np
import pytest

from peykar.model_learning import retina_acuity_population_1_1 as version_1_1
from peykar.model_learning.eye_stacking import (
    StackingWeights,
    combine,
    shift_quantiles,
    simplex_least_squares,
)
from peykar.validation import eye_additive_evaluation3 as additive3
from peykar.validation import eye_additive_evaluation4 as additive4
from peykar.validation import eye_recovery_evaluation2 as harness2
from peykar.validation import eye_recovery_evaluation6 as harness6
from peykar.validation import eye_stacking_evaluation as stacking
from peykar.validation.eye_retina_acuity_data import RetinaAcuitySeries
from peykar_core.errors import InvalidRequest

W1 = version_1_1.DEVELOPMENT_CANDIDATES["w1"]
SCALE = additive4.AdditiveScale(
    quantity="best-corrected visual acuity",
    unit="logMAR",
    model_unit=1.0,
    covariate_unit=1.0,
    report_unit=0.02,
)


def _series(
    study: str, context: str, months: tuple[float, ...], rng: np.random.Generator, gain: float
) -> RetinaAcuitySeries:
    changes = [
        -gain - 0.08 * math.exp(-(m - 1 / 30.4375) / 0.4) + float(rng.normal(0, 0.02))
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


def _studies(rng: np.random.Generator) -> list[RetinaAcuitySeries]:
    out = []
    for s in range(6):
        gain = float(rng.uniform(0.05, 0.2))
        out.append(_series(f"st{s}", "human/dme/anti_vegf", (1.0, 3.0, 6.0, 12.0), rng, gain))
        out.append(_series(f"st{s}", "human/rvo/anti_vegf", (2.0, 6.0, 12.0), rng, gain))
    return out


def _without(result: dict, field: str) -> dict:
    out = dict(result)
    out["initial_state"] = [
        {k: v for k, v in e.items() if k != field} for e in result["initial_state"]
    ]
    return out


def test_simplex_least_squares_finds_the_best_convex_weights() -> None:
    assert simplex_least_squares([[1.0, -1.0], [-1.0, 1.0]]) == pytest.approx((0.5, 0.5))
    # A worse member with errors of the same sign gets no weight; ties keep the model alone.
    assert simplex_least_squares([[0.1, 0.5], [-0.1, -0.5]]) == (1.0, 0.0)
    assert simplex_least_squares([[0.1, 5.0], [-0.1, 5.0]]) == pytest.approx(
        (100 / 100.04, 0.04 / 100.04)
    )
    assert simplex_least_squares([[0.3, 0.3], [-0.2, -0.2]]) == (1.0, 0.0)
    rng = np.random.default_rng(3)
    errors = rng.normal(size=(60, 3)) * np.array([1.0, 1.4, 1.2]) + np.array([0.3, -0.5, 0.1])
    weights = np.array(simplex_least_squares(errors.tolist()))
    assert weights.min() >= 0 and math.isclose(weights.sum(), 1.0)
    best = min(
        float(np.sum((errors @ np.array([a, b, 1 - a - b])) ** 2))
        for a, b in itertools.product(np.linspace(0, 1, 201), repeat=2)
        if a + b <= 1 + 1e-12
    )
    assert float(np.sum((errors @ weights) ** 2)) <= best + 1e-9
    with pytest.raises(InvalidRequest):
        simplex_least_squares([])


def test_weights_combine_and_shift() -> None:
    weights = StackingWeights(history=(0.5, 0.2, 0.3), initial=(0.7, 0.3))
    assert StackingWeights.from_description(weights.describe()) == weights
    assert combine([1.0, 2.0, 3.0], weights.history) == pytest.approx(1.8)
    # A missing pooled curve drops out; the other weights are renormalized.
    assert combine([1.0, 2.0, None], weights.history) == pytest.approx((0.5 + 0.4) / 0.7)
    assert combine([None, 2.0, 3.0], weights.history) is None
    assert combine([1.0, None, None], (0.0, 0.5, 0.5)) == 1.0
    assert shift_quantiles({"0.1": 0.9, "0.9": 1.1}, 0.05) == pytest.approx(
        {"0.1": 0.95, "0.9": 1.15}
    )
    for bad in ((0.5, 0.6, -0.1), (0.5, 0.2, 0.2), (1.0, 0.0)):
        with pytest.raises(InvalidRequest):
            StackingWeights(history=bad, initial=(1.0, 0.0))


def test_harness6_equals_harness2_and_adds_initial_state_intervals() -> None:
    series = _studies(np.random.default_rng(11))
    held = [s for s in series if s.study_id == "st0"]
    training = [s for s in series if s.study_id != "st0"]
    plain = harness2.evaluate_study(W1, study_series=held, training=training)
    kept = harness6.evaluate_study(W1, study_series=held, training=training)
    assert kept["evaluation_version"] == "6.0.0"
    assert {**_without(kept, harness6.INTERVAL_FIELD), "evaluation_version": "2.0.0"} == plain
    for entry in kept["initial_state"]:
        intervals = entry[harness6.INTERVAL_FIELD]["population_initial_state"]
        low, high = intervals["0.05"], intervals["0.95"]
        assert all(a < b for a, b in zip(low, high, strict=True))
        counts = entry["population_interval_coverage"]["population_initial_state"]
        inside = sum(
            int(a <= o <= b)
            for a, o, b in zip(intervals["0.1"], entry["observed"], intervals["0.9"], strict=True)
        )
        assert counts["80"]["inside"] == inside


def test_additive4_rescales_the_initial_state_intervals() -> None:
    series = _studies(np.random.default_rng(5))
    held = [s for s in series if s.study_id == "st1"]
    training = [s for s in series if s.study_id != "st1"]
    old = additive3.evaluate_study(W1, study_series=held, training=training, scale=SCALE)
    raw = harness6.evaluate_study(W1, study_series=held, training=training)
    new = additive4.evaluate_study(W1, study_series=held, training=training, scale=SCALE)
    expected = {**_without(new, harness6.INTERVAL_FIELD), "evaluation_version": "2.0.0"}
    assert {**expected, "additive_evaluation_version": "3.0.0"} == old
    entry, source = new["initial_state"][0], raw["initial_state"][0]
    for q, values in source[harness6.INTERVAL_FIELD]["population_initial_state"].items():
        rescaled = entry[harness6.INTERVAL_FIELD]["population_initial_state"][q]
        assert rescaled == pytest.approx([SCALE.report(v) for v in values])


def test_stacked_reports_recompute_scores_and_nest_their_weights() -> None:
    series = _studies(np.random.default_rng(23))
    report = additive4.leave_one_study_out(W1, series, scale=SCALE, clusters=[["st0", "st1"]])
    spec = stacking.StackingSpec(history_model="population_anchored")
    alone = StackingWeights(history=(1.0, 0.0, 0.0), initial=(1.0, 0.0))
    same = stacking.stack_report(report, alone, spec)
    for study in same["studies"]:
        for task in ("next_observation", "landmark"):
            for entry in study[task]:
                for row in entry["rows"]:
                    assert row["predictions"]["stacked"] == row["predictions"][spec.history_model]
                    assert row["intervals"]["stacked"] == pytest.approx(
                        row["intervals"][spec.history_model]
                    )
        for entry in study["initial_state"]:
            p = entry["predictions"]
            assert p["stacked_initial_state"] == pytest.approx(p["population_initial_state"])
    summary = same["summary"]
    assert summary["next_observation"]["stacked"]["pooled_rmse_pp"] == pytest.approx(
        summary["next_observation"][spec.history_model]["pooled_rmse_pp"]
    )
    cover = summary["coverage"]["80"]
    assert cover["next_observation"]["stacked"] == cover["next_observation"][spec.history_model]
    assert (
        cover["initial_state_group_mean"]["stacked_initial_state"]
        == cover["initial_state_group_mean"]["population_initial_state"]
    )
    nested = stacking.nested_stack_report(report, spec)
    by_study = nested["stacking"]["weights_by_held_out_study"]
    assert by_study["st0"] == by_study["st1"]
    assert (
        by_study["st0"]
        == stacking.fit_weights(report["studies"], spec, exclude={"st0", "st1"}).describe()
    )
    assert (
        by_study["st2"] == stacking.fit_weights(report["studies"], spec, exclude={"st2"}).describe()
    )
    comparison = stacking.development_comparison(nested, spec)
    assert set(comparison["tasks"]) == {"next_observation", "landmark", "initial_state"}
    assert all(v["targets"] > 0 for v in comparison["tasks"].values())
    assert isinstance(comparison["adopted"], bool)
