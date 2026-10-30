"""Forecast stacking 2.0.0: stacked forecasts from follow-up with a chosen interval
scale (synthetic data only)."""

from __future__ import annotations

import math
from fractions import Fraction

import numpy as np
import pytest

from peykar.model_learning import retina_acuity_population_1_1 as version_1_1
from peykar.model_learning.eye_stacking import StackingSpec
from peykar.model_learning.eye_stacking2 import (
    HistoryStackedCandidate,
    HistoryStacking,
    HistoryStackingSpec,
    scale_quantiles,
)
from peykar.validation import eye_additive_evaluation4 as additive4
from peykar.validation import eye_stacking_evaluation2 as stacking2
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
SPEC = HistoryStackingSpec(members=StackingSpec(history_model="population_anchored"))


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


def _report() -> dict:
    rng = np.random.default_rng(23)
    series = []
    for s in range(6):
        gain = float(rng.uniform(0.05, 0.2))
        series.append(_series(f"st{s}", "human/dme/anti_vegf", (1.0, 3.0, 6.0, 12.0), rng, gain))
        series.append(_series(f"st{s}", "human/rvo/anti_vegf", (2.0, 6.0, 12.0), rng, gain))
    return additive4.leave_one_study_out(W1, series, scale=SCALE, clusters=[["st0", "st1"]])


def test_history_stacking_describes_validates_and_scales() -> None:
    stacking = HistoryStacking(weights=(0.6, 0.3, 0.1), interval_scale=0.85)
    assert HistoryStacking.from_description(stacking.describe()) == stacking
    for scale in (0.0, -1.0, math.nan, math.inf):
        with pytest.raises(InvalidRequest):
            HistoryStacking(weights=(1.0, 0.0, 0.0), interval_scale=scale)
    with pytest.raises(InvalidRequest):
        HistoryStacking(weights=(0.6, 0.6, -0.2), interval_scale=1.0)
    with pytest.raises(InvalidRequest):
        HistoryStacking.from_description({**stacking.describe(), "stacking_version": "1.0.0"})
    moved = scale_quantiles({"0.1": 0.9, "0.5": 1.0, "0.9": 1.2}, 1.0, 1.1, 0.5)
    assert moved == pytest.approx({"0.1": 1.05, "0.5": 1.1, "0.9": 1.2})
    candidate = HistoryStackedCandidate(base="e0", spec=SPEC).describe()
    assert candidate["base"] == "e0" and candidate["stacking_version"] == "2.0.0"


def test_scale_grid_is_exact_and_checked() -> None:
    scales = SPEC.scales()
    assert len(scales) == 271
    assert scales[0] == Fraction(3, 10) and scales[-1] == 3 and Fraction(1) in scales
    for grid in (("0.30", "3.00", "0.07"), ("0", "1", "0.1"), ("1", "0.5", "0.1")):
        with pytest.raises(InvalidRequest):
            HistoryStackingSpec(members=SPEC.members, scale_grid=grid)
    with pytest.raises(InvalidRequest):
        HistoryStackingSpec(members=SPEC.members, target_coverage="1.2")


def test_scale_is_the_closest_coverage_then_closest_to_one() -> None:
    # Intervals [-c, c] around 0: a row is inside when |observed| <= c.
    rows = [(0.355 + 0.1 * i, 0.0, 0.0, -1.0, 1.0) for i in range(10)]
    # Eight of ten inside needs c in [1.055, 1.155): the grid factor closest to 1 is 1.06.
    assert stacking2.calibrate_scale(rows, SPEC) == pytest.approx(1.06)
    # Coverages 1/2 and 1 are equally far from 0.75; the factor 1 itself gives 1/2.
    tied = HistoryStackingSpec(members=SPEC.members, target_coverage="0.75")
    pair = [(0.5, 0.0, 0.0, -1.0, 1.0), (2.0, 0.0, 0.0, -1.0, 1.0)]
    assert stacking2.calibrate_scale(pair, tied) == 1.0
    # The scale acts around the stacked point s = 1 with the model's offsets q - m: the
    # upper bound is 1 + c (1 - 0.5), so 1.6025 needs c >= 1.205 and 2.6 is never inside.
    shifted = [(1.6025, 1.0, 0.5, 0.0, 1.0)] * 4 + [(2.6, 1.0, 0.5, 0.0, 1.0)]
    assert stacking2.calibrate_scale(shifted, SPEC) == pytest.approx(1.21)
    with pytest.raises(InvalidRequest):
        stacking2.calibrate_scale([], SPEC)


def test_frozen_stacking_leaves_the_model_alone_with_its_own_weight() -> None:
    report = _report()
    model = SPEC.members.history_model
    same = stacking2.stack_report2(report, HistoryStacking((1.0, 0.0, 0.0), 1.0), SPEC)
    for study, original in zip(same["studies"], report["studies"], strict=True):
        assert study["initial_state"] == original["initial_state"]
        for task in ("next_observation", "landmark"):
            for entry in study[task]:
                for row in entry["rows"]:
                    assert row["predictions"]["stacked"] == row["predictions"][model]
                    assert row["intervals"]["stacked"] == pytest.approx(row["intervals"][model])
    cover = same["summary"]["coverage"]["80"]["next_observation"]
    assert cover["stacked"] == cover[model] and cover["stacked"]["n"] > 0
    summary = same["summary"]["landmark"]
    assert summary["stacked"]["pooled_rmse_pp"] == pytest.approx(summary[model]["pooled_rmse_pp"])


def test_nested_stacking_fits_weights_and_scale_without_the_held_out_cluster() -> None:
    report = _report()
    model = SPEC.members.history_model
    nested = stacking2.nested_stack_report2(report, SPEC)
    by_study = nested["stacking"]["stacking_by_held_out_study"]
    assert by_study["st0"] == by_study["st1"]
    assert (
        by_study["st0"]
        == stacking2.fit_history(report["studies"], SPEC, exclude={"st0", "st1"}).describe()
    )
    assert by_study["st2"] == stacking2.fit_history(report["studies"], SPEC, {"st2"}).describe()
    for study in nested["studies"]:
        stacking = HistoryStacking.from_description(by_study[study["study_id"]])
        for entry in study["next_observation"]:
            for row in entry["rows"]:
                m, s = row["predictions"][model], row["predictions"]["stacked"]
                expected = scale_quantiles(row["intervals"][model], m, s, stacking.interval_scale)
                assert row["intervals"]["stacked"] == pytest.approx(expected)
    everything = stacking2.fit_history(report["studies"], SPEC)
    rows = stacking2.calibration_rows(report["studies"], SPEC, everything.weights)
    best = abs(stacking2._inside(rows, everything.interval_scale) / len(rows) - 0.8)
    for scale in SPEC.scales():
        assert abs(stacking2._inside(rows, float(scale)) / len(rows) - 0.8) >= best - 1e-12
    comparison = stacking2.development_comparison2(nested, SPEC)
    assert set(comparison["tasks"]) == {"next_observation", "landmark"}
    assert all(v["targets"] > 0 for v in comparison["tasks"].values())
    assert comparison["initial_state_model_own"]["targets"] > 0
    assert isinstance(comparison["adopted"], bool)
    unscaled = comparison["reported_not_in_rule"]["coverage_next_observation_moved_unscaled"]
    assert unscaled["80"]["n"] == comparison["coverage_next_observation"]["80"]["n"]


def test_per_task_scales_change_only_the_other_tasks_intervals() -> None:
    report = _report()
    model = SPEC.members.history_model
    per_task = HistoryStackingSpec(
        members=SPEC.members, scale_tasks=("next_observation", "landmark")
    )
    one = stacking2.fit_history(report["studies"], SPEC)
    two = stacking2.fit_history(report["studies"], per_task)
    assert two.weights == one.weights and two.interval_scale == one.interval_scale
    assert two.scale_for("next_observation") == one.interval_scale
    rows = stacking2.calibration_rows(report["studies"], per_task, two.weights, task="landmark")
    assert two.scale_for("landmark") == stacking2.calibrate_scale(rows, per_task)
    assert HistoryStacking.from_description(two.describe()) == two
    assert "task_scales" not in one.describe() and "scale_tasks" not in SPEC.describe()
    first = stacking2.stack_report2(report, one, SPEC)
    second = stacking2.stack_report2(report, two, per_task)
    for a, b in zip(first["studies"], second["studies"], strict=True):
        assert a["next_observation"] == b["next_observation"]
        for x, y in zip(a["landmark"], b["landmark"], strict=True):
            for r, s in zip(x["rows"], y["rows"], strict=True):
                assert r["predictions"] == s["predictions"]
                if "stacked" in s["intervals"]:
                    m, p = s["predictions"][model], s["predictions"]["stacked"]
                    expected = scale_quantiles(
                        s["intervals"][model], m, p, two.scale_for("landmark")
                    )
                    assert s["intervals"]["stacked"] == pytest.approx(expected)
    for bad in (("landmark",), ("next_observation", "next_observation")):
        with pytest.raises(InvalidRequest):
            HistoryStackingSpec(members=SPEC.members, scale_tasks=bad)
    with pytest.raises(InvalidRequest):
        HistoryStacking((1.0, 0.0, 0.0), 1.0, task_scales=(("landmark", 0.0),))
