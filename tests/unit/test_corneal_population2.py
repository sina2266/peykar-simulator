"""Synthetic verification of population model 2.0.0 (closure tail); no experiment scored."""

import math

import numpy as np
import pytest

from peykar.model_learning import corneal_population as v1
from peykar.model_learning import corneal_population2 as v2
from peykar.validation.cornea_population_evaluation2 import evaluate_study
from peykar_core.errors import InvalidRequest

HOURS = (6.0, 12.0, 18.0, 24.0, 30.0, 36.0, 42.0, 48.0)


def synthetic(study, index, *, context, speed, lag, tail, radius=1000.0, noise=0.0, rng=None):
    rho = v2.normalized_radius(HOURS, speed_um_h=speed, lag_h=lag, radius0_um=radius, tail_um=tail)[
        0
    ]
    if noise:
        rho = rho + rng.normal(0.0, noise, rho.size)
    detection = min(0.5, v1.DETECTION_RADIUS_UM / radius)
    reported = np.where(rho > detection, rho**2, 0.0)
    return v1.WoundSeries(
        series_id=f"{study}/animal-{index}",
        study_id=study,
        context=context,
        radius0_um=radius,
        hours=HOURS,
        fractions=tuple(float(v) for v in reported),
        group="vehicle",
    )


def population(context, tail, *, seed=5, studies=4, animals=4):
    rng = np.random.default_rng(seed)
    series = []
    for s in range(studies):
        study_speed = math.log(30.0) + rng.normal(0, 0.1)
        study_lag = 6.0 + rng.normal(0, 1.5)
        for a in range(animals):
            series.append(
                synthetic(
                    f"{context}-study-{s}",
                    a,
                    context=context,
                    speed=math.exp(study_speed + rng.normal(0, 0.08)),
                    lag=max(0.0, study_lag + rng.normal(0, 0.8)),
                    tail=tail,
                    noise=0.015,
                    rng=rng,
                )
            )
    return series


def test_zero_tail_reproduces_version_one_exactly():
    hours = [2.0, 8.0, 20.0, 40.0]
    a = v2.normalized_radius(hours, speed_um_h=v1.GRID_SPEED, lag_h=v1.GRID_LAG, radius0_um=900.0)
    b = v1.normalized_radius(hours, speed_um_h=v1.GRID_SPEED, lag_h=v1.GRID_LAG, radius0_um=900.0)
    assert np.array_equal(a, b)
    series = synthetic("s", 0, context="mouse", speed=40.0, lag=5.0, tail=0.0)
    assert np.array_equal(
        v2.grid_log_likelihood(series, 0.02, 0.0), v1.grid_log_likelihood(series, 0.02)
    )


@pytest.mark.parametrize("tail", [50.0, 300.0, 800.0])
def test_tail_solution_satisfies_the_integrated_closure_law(tail):
    radius0, speed, lag = 1500.0, 45.0, 4.0
    hours = np.array([0.0, 4.0, 10.0, 30.0, 60.0, 120.0])
    rho = v2.normalized_radius(
        hours, speed_um_h=speed, lag_h=lag, radius0_um=radius0, tail_um=tail
    )[0]
    assert rho[0] == 1.0 and rho[1] == 1.0
    assert np.all(np.diff(rho) <= 0.0) and np.all(rho > 0.0)
    radius = rho * radius0
    # Integral of dr/dt = -v r / (r + r_c): r + r_c ln r = r0 + r_c ln r0 - v (t - L).
    moved = speed * np.maximum(0.0, hours - lag)
    lhs = radius + tail * np.log(radius)
    rhs = radius0 + tail * math.log(radius0) - moved
    visible = radius > 1e-6 * radius0
    assert np.allclose(lhs[visible], rhs[visible], rtol=0, atol=1e-6 * radius0)
    # A positive tail always leaves at least the version 1 radius.
    linear = v1.normalized_radius(hours, speed_um_h=speed, lag_h=lag, radius0_um=radius0)[0]
    assert np.all(rho >= linear - 1e-12)


def test_tail_rejects_invalid_radius():
    with pytest.raises(InvalidRequest):
        v2.normalized_radius([1.0], speed_um_h=10.0, lag_h=0.0, radius0_um=500.0, tail_um=-1.0)
    with pytest.raises(InvalidRequest):
        v2.normalized_radius(
            [1.0], speed_um_h=10.0, lag_h=0.0, radius0_um=500.0, tail_um=float("nan")
        )


def test_tail_selection_separates_contexts_by_marginal_likelihood():
    slow = population("mouse", 300.0)
    sharp = population("rat", 0.0, seed=9)
    selection = v2.select_tails([*slow, *sharp])
    assert selection["chosen"]["mouse"] >= 200.0
    assert selection["chosen"]["rat"] <= 100.0
    assert set(selection["log_marginal"]["mouse"]) == set(v2.TAIL_RADII_UM)


def test_fit_population_records_tails_and_version():
    series = population("mouse", 300.0, studies=3, animals=3)
    fit = v2.fit_population(series)
    described = fit.describe()
    assert described["model_version"] == "2.0.0"
    assert fit.tail("mouse") >= 200.0
    assert fit.tail("unknown-context") == fit.tails_um["pooled"]
    assert described["contexts"]["mouse"]["median_speed_um_h"] == pytest.approx(30.0, rel=0.3)


def test_v2_evaluation_refuses_leakage_and_uses_the_context_tail():
    series = population("mouse", 300.0, studies=4, animals=3)
    held = [s for s in series if s.study_id == "mouse-study-0"]
    with pytest.raises(InvalidRequest):
        evaluate_study(study_series=held, training=series, draws=200)
    report = evaluate_study(
        study_series=held,
        training=[s for s in series if s.study_id != "mouse-study-0"],
        draws=200,
    )
    assert report["evaluation_version"] == "2.0.0"
    assert report["tail_radius_um_used"] >= 200.0
    assert len(report["next_observation"]) == 3
    assert all(len(e["rows"]) == len(HOURS) - 1 for e in report["next_observation"])
