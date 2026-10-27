"""Synthetic verification of the corneal population model; no experiment is scored."""

import math

import numpy as np
import pytest

from peykar.model_learning import corneal_population as cp
from peykar.model_learning.corneal_analog import forecast_analog
from peykar.validation.cornea_population_evaluation import evaluate_study
from peykar_core.errors import InvalidRequest

HOURS = (6.0, 12.0, 18.0, 24.0, 30.0, 36.0, 42.0, 48.0)


def synthetic(study, index, *, speed, lag, radius=1000.0, noise=0.0, rng=None, role="control"):
    rho = cp.normalized_radius(HOURS, speed_um_h=speed, lag_h=lag, radius0_um=radius)[0]
    if noise:
        rho = np.where(rho > 0, np.maximum(0.0, rho + rng.normal(0, noise, rho.size)), 0.0)
    return cp.WoundSeries(
        series_id=f"{study}/animal-{index}",
        study_id=study,
        context="mouse",
        radius0_um=radius,
        hours=HOURS,
        fractions=tuple(float(v) for v in rho**2),
        role=role,
        group="vehicle",
    )


def population(seed=3, studies=6, animals=5):
    rng = np.random.default_rng(seed)
    series = []
    for s in range(studies):
        study_speed = math.log(30.0) + rng.normal(0, 0.15)
        study_lag = 6.0 + rng.normal(0, 2.0)
        for a in range(animals):
            series.append(
                synthetic(
                    f"study-{s}",
                    a,
                    speed=math.exp(study_speed + rng.normal(0, 0.1)),
                    lag=max(0.0, study_lag + rng.normal(0, 1.0)),
                    noise=0.02,
                    rng=rng,
                )
            )
    return series


def test_radius_law_is_exact_and_clipped_at_closure():
    rho = cp.normalized_radius([2, 6, 16, 30], speed_um_h=100.0, lag_h=6.0, radius0_um=1000.0)
    assert rho[0] == pytest.approx([1.0, 1.0, 0.0, 0.0])
    rho = cp.normalized_radius([8.0], speed_um_h=50.0, lag_h=6.0, radius0_um=1000.0)
    assert rho[0][0] == pytest.approx(0.9)


def test_series_summary_recovers_noise_free_parameters():
    series = synthetic("s", 0, speed=40.0, lag=7.0)
    summary = cp.series_summary(series, sigma=0.01)
    assert summary["map_speed_um_h"] == pytest.approx(40.0, rel=0.03)
    assert summary["map_lag_h"] == pytest.approx(7.0, abs=0.3)


def test_population_fit_recovers_synthetic_hierarchy():
    fit = cp.fit_population(population())
    described = fit.describe()["contexts"]["mouse"]
    assert described["median_speed_um_h"] == pytest.approx(30.0, rel=0.2)
    assert described["mean_lag_h"] == pytest.approx(6.0, abs=2.0)
    assert fit.noise_sd["individual"] == pytest.approx(0.02, rel=0.35)


def test_population_fit_rejects_treated_and_duplicate_series():
    series = population(studies=2, animals=2)
    with pytest.raises(InvalidRequest):
        cp.fit_population([*series, series[0]])
    treated = [synthetic("x", 0, speed=30, lag=5, role="treated"), *series]
    with pytest.raises(InvalidRequest):
        cp.fit_population(treated)


def test_assimilation_uses_only_the_prefix():
    fit = cp.fit_population(population())
    full = synthetic("new", 0, speed=45.0, lag=3.0)
    prefix = full.prefix(18.0)
    altered = cp.WoundSeries(
        series_id=full.series_id,
        study_id=full.study_id,
        context=full.context,
        radius0_um=full.radius0_um,
        hours=full.hours,
        fractions=(*full.fractions[:3], 0.9, 0.9, 0.9, 0.9, 0.9),
        group=full.group,
    ).prefix(18.0)
    prior = fit.prior("mouse")
    noise = fit.noise("individual")
    a = cp.predictive(
        cp.posterior(prior, series=prefix, noise=noise), hours=[24.0], radius0_um=1000.0
    )
    b = cp.predictive(
        cp.posterior(prior, series=altered, noise=noise), hours=[24.0], radius0_um=1000.0
    )
    assert a["mean_area_fraction"] == b["mean_area_fraction"]
    assert a["posterior_median_speed_um_h"] == pytest.approx(45.0, rel=0.25)


def test_zero_area_is_censored_below_the_detection_radius():
    closed = cp.WoundSeries(
        series_id="s/z",
        study_id="s",
        context="mouse",
        radius0_um=1000.0,
        hours=(18.0, 24.0),
        fractions=(0.25, 0.0),
    )
    log_likelihood = cp.grid_log_likelihood(closed, 0.02)
    # A speed that leaves a 100 µm residual at 24 h is not penalized by the zero.
    slow = np.argmin(np.abs(cp.GRID_SPEED - 75.0) + np.abs(cp.GRID_LAG - 11.5))
    fast = np.argmin(np.abs(cp.GRID_SPEED - 75.0) + np.abs(cp.GRID_LAG - 4.0))
    assert log_likelihood[slow] > log_likelihood[fast]
    assert cp._log_norm_cdf(np.array([0.0]))[0] == pytest.approx(math.log(0.5), abs=1e-6)
    assert cp._log_norm_cdf(np.array([-30.0]))[0] < -400


def test_noise_is_estimated_per_study_and_pooled_by_median():
    series = population()
    estimates = cp.study_noise(series)
    assert len(estimates) == 6
    centers = cp.noise_centers(estimates)
    assert centers["individual"] == pytest.approx(0.02, rel=0.4)
    sigmas, log_w = cp.NoiseModel(center=0.02).levels()
    assert sigmas.size == cp.NOISE_LEVEL_Z.size and np.exp(log_w).sum() == pytest.approx(1.0)


def test_analog_forecaster_excludes_query_study_and_needs_neighbors():
    library = population(studies=3, animals=3)
    query = library[0].prefix(18.0)
    result = forecast_analog(library, prefix=query, future_hours=[24.0])
    assert all(not n["series_id"].startswith("study-0/") for n in result["neighbors"])
    with pytest.raises(InvalidRequest):
        forecast_analog(library[:6], prefix=query, future_hours=[24.0])


def test_evaluation_refuses_training_leakage_and_scores_every_target():
    series = population(studies=4, animals=3)
    held = [s for s in series if s.study_id == "study-0"]
    with pytest.raises(InvalidRequest):
        evaluate_study(study_series=held, training=series, draws=200)
    report = evaluate_study(
        study_series=held, training=[s for s in series if s.study_id != "study-0"], draws=200
    )
    assert report["population_context_used"] == "mouse"
    assert len(report["next_observation"]) == 3
    assert all(len(e["rows"]) == len(HOURS) - 1 for e in report["next_observation"])
    assert report["initial_state"][0]["animals"] == 3
