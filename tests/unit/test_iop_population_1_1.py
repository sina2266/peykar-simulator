"""Synthetic checks of IOP model 1.1 (early transient, eye-count noise) and its harness.

The data are generated from the declared kinetic law; passing these tests says the
code does what it declares, not that any biological or clinical claim holds.
"""

from __future__ import annotations

import importlib.util
import math
from pathlib import Path
from types import ModuleType

import numpy as np
import pytest

from peykar.model_learning import iop_population_1_1 as population
from peykar.validation import eye_iop_evaluation_1_1 as evaluation
from peykar.validation.eye_iop_data import PressureSeries
from peykar_core.errors import InvalidRequest

ROOT = Path(__file__).resolve().parents[2]
DAY = population.DAY_MONTHS
MONTHS = (DAY, 7 * DAY, 14 * DAY, 1.0, 3.0, 6.0, 12.0)
SLOPE = -0.75
LEVELS = {"human/trabeculectomy": -0.45, "human/tube": -0.5, "human/angle_mims": -0.3}
EXCESS = {"human/trabeculectomy": -0.35, "human/tube": -0.1, "human/angle_mims": 0.0}
TRANSIENT = {"human/trabeculectomy": 0.0, "human/tube": -0.3, "human/angle_mims": 0.25}


def _series(
    study: str,
    name: str,
    context: str,
    *,
    reference_mmhg: float = 25.0,
    level: float | None = None,
    tau_days: float = 30.0,
    months: tuple[float, ...] = MONTHS,
    jitter: float = 0.0,
    seed: int = 0,
    role: str = "control",
    n: int | None = 40,
) -> PressureSeries:
    x = math.log(reference_mmhg / population.REFERENCE_MMHG)
    curve = population.pressure_fraction(
        months,
        level=(LEVELS[context] if level is None else level) + SLOPE * x,
        excess=EXCESS[context],
        transient=TRANSIENT[context],
        tau_months=tau_days * DAY,
    )
    noise = np.exp(np.random.default_rng(seed).normal(0.0, jitter, len(months)))
    return PressureSeries(
        series_id=f"{study}/{name}",
        study_id=study,
        context=context,
        reference_mmhg=reference_mmhg,
        months=months,
        fractions=tuple(float(v) for v in curve * noise),
        aggregate_n=n,
        role=role,
        group=name,
    )


def _library() -> list[PressureSeries]:
    series = []
    for index in range(4):
        for position, context in enumerate(LEVELS):
            for arm, reference in (("a", 18.0 + 3 * index), ("b", 26.0 + 4 * index)):
                series.append(
                    _series(
                        f"{context.split('/')[1]}-{index}",
                        arm,
                        context,
                        reference_mmhg=reference,
                        level=LEVELS[context] + 0.02 * (index - 1.5),
                        jitter=0.015,
                        seed=100 * position + 10 * index + len(arm),
                    )
                )
    return series


@pytest.fixture(scope="module")
def fitted() -> population.PressureFit:
    return population.fit_population(_library())


def _script(name: str) -> ModuleType:
    spec = importlib.util.spec_from_file_location(name, ROOT / "scripts" / f"{name}.py")
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_transient_allows_an_early_rise_before_the_slow_return() -> None:
    assert population.transient_ratio([DAY])[0] == pytest.approx(1.0)
    values = population.pressure_fraction(
        [DAY, 7 * DAY, 1.0, 12.0, 1200.0], level=-0.4, excess=-0.3, transient=0.25, tau_months=0.5
    )
    assert values[0] == pytest.approx(math.exp(-0.4 - 0.3 + 0.25))
    assert values[4] == pytest.approx(math.exp(-0.4), rel=1e-9)
    dip = population.pressure_fraction(
        [DAY, 7 * DAY, 1.0, 3.0], level=-0.4, excess=0.2, transient=-0.4, tau_months=1.0
    )
    # rise after a low day 1, then a return
    assert dip[0] < dip[1] < dip[2] and dip[2] > dip[3]
    x = population.design([DAY, 1.0], population.LOG_TAU_GRID)
    assert x.shape == (population.LOG_TAU_GRID.size, 2, 3)
    assert np.allclose(x[:, 0, :], 1.0)


def test_posterior_is_the_exact_linear_gaussian_update_at_a_fixed_tau() -> None:
    log_tau = float(population.LOG_TAU_GRID[45])
    mean = np.array([-0.4, -0.2, 0.0, log_tau])
    covariance = np.diag([0.05**2, 0.1**2, 0.1**2, 1e-10])
    item = _series(
        "s", "a", "human/angle_mims", tau_days=math.exp(log_tau) / DAY, jitter=0.02, seed=3
    )
    sigma = 0.03
    result = population.posterior(population.gaussian(mean, covariance), series=item, noise=sigma)
    got, _ = population.moments(result)
    x = population.design(item.months, np.array([log_tau]))[0]
    prior_precision = np.linalg.inv(covariance[:3, :3])
    precision = prior_precision + x.T @ x / sigma**2
    expected = np.linalg.solve(
        precision, prior_precision @ mean[:3] + x.T @ np.log(item.fractions) / sigma**2
    )
    assert got[:3] == pytest.approx(expected, abs=1e-8)
    assert got[3] == pytest.approx(log_tau, abs=1e-6)


def test_fit_recovers_slope_levels_and_transients(fitted: population.PressureFit) -> None:
    family = fitted.describe()["families"]["human/all"]
    assert family["level_slope_on_ln_preop_iop"] == pytest.approx(SLOPE, abs=0.12)
    for context, level in LEVELS.items():
        got = family["contexts"][context]
        assert got["long_term_fraction_at_20_mmhg"] == pytest.approx(math.exp(level), abs=0.04)
        assert got["transient_day1"] == pytest.approx(TRANSIENT[context], abs=0.1)
    angle = family["contexts"]["human/angle_mims"]["transient_day1"]
    tube = family["contexts"]["human/tube"]["transient_day1"]
    assert angle > 0.1 > -0.1 > tube


def test_group_mean_noise_scales_with_the_number_of_eyes() -> None:
    estimates = {
        (f"s{k}", "human/all", "aggregate"): population.NoiseEstimate(
            sd=0.06 * (eyes / 40.0) ** -0.3, eyes=eyes
        )
        for k, eyes in enumerate((10.0, 20.0, 40.0, 80.0, 160.0))
    }
    assert population.noise_exponent(estimates) == pytest.approx(0.3, abs=1e-9)
    centers = population.noise_centers(estimates)
    assert centers.center("human/all", "aggregate", 40) == pytest.approx(0.06, rel=1e-9)
    assert centers.center("human/all", "aggregate", 160) < centers.center(
        "human/all", "aggregate", 10
    )
    assert centers.center("human/all", "individual", None) > 0
    steep = {k: population.NoiseEstimate(sd=v.sd**3, eyes=v.eyes) for k, v in estimates.items()}
    assert population.noise_exponent(steep) == population.MAX_NOISE_EXPONENT
    few = dict(list(estimates.items())[:3])
    assert population.noise_exponent(few) == 0.0
    with pytest.raises(InvalidRequest):
        population.noise_centers({})


def test_study_noise_needs_five_follow_ups() -> None:
    items = [
        _series("s", "a", "human/tube", jitter=0.05, seed=1),
        _series("t", "a", "human/tube", months=MONTHS[:4]),
    ]
    estimates = population.study_noise(items)
    assert set(estimates) == {("s", "human/all", "aggregate")}
    assert estimates[("s", "human/all", "aggregate")].eyes == 40.0


def test_prior_moves_with_preoperative_pressure_and_falls_back(
    fitted: population.PressureFit,
) -> None:
    low = fitted.prior("human/tube", "aggregate", math.log(18 / 20))
    high = fitted.prior("human/tube", "aggregate", math.log(36 / 20))
    slope = fitted.families["human/all"].fixed[population.DIMENSIONS]
    assert high["mean"][0] - low["mean"][0] == pytest.approx(slope * math.log(2.0), rel=1e-9)
    assert np.all(np.linalg.eigvalsh(low["covariance"]) > 0)
    assert fitted.level_used("human/cyclodestruction") == "family:human/all"
    seen = fitted.prior("human/tube", "aggregate", 0.0)
    unseen = fitted.prior("human/cyclodestruction", "aggregate", 0.0)
    assert np.all(np.diag(unseen["covariance"]) > np.diag(seen["covariance"]) - 1e-12)
    with pytest.raises(InvalidRequest):
        fitted.prior("rabbit/tube", "aggregate", 0.0)


def test_fit_rejects_treated_and_duplicate_series() -> None:
    library = _library()
    with pytest.raises(InvalidRequest):
        population.fit_population([*library, _series("x", "t", "human/tube", role="treated")])
    with pytest.raises(InvalidRequest):
        population.fit_population([library[0], library[0]])


def test_assimilation_follows_a_week_one_rise(fitted: population.PressureFit) -> None:
    truth = _series("new", "a", "human/angle_mims", reference_mmhg=22.0)
    prior = fitted.prior("human/angle_mims", "aggregate", population.level_covariate(truth))
    noise = fitted.noise("aggregate", "human/angle_mims", 40)
    result = population.predictive(
        population.posterior(
            population.robust(population.gaussian(prior["mean"], prior["covariance"])),
            series=truth.prefix(1.0),
            noise=noise,
        ),
        months=[3.0, 12.0],
    )
    assert result["point_fraction"] == result["quantiles"]["0.5"]
    assert result["point_fraction"][1] == pytest.approx(truth.fractions[-1], abs=0.03)
    t4 = population.student(prior["mean"], prior["covariance"], 4.0)
    early = population.predictive(
        population.posterior(t4, series=None, noise=noise), months=[DAY, 7 * DAY, 12.0]
    )
    assert all(math.isfinite(v) and 0 < v < 5 for v in early["point_fraction"])
    assert set(result["posterior_mean"]) == set(population.PARAMETERS)


def test_anchored_and_robust_mixtures(fitted: population.PressureFit) -> None:
    prior = fitted.prior("human/trabeculectomy", "aggregate", 0.0)
    gaussian = population.gaussian(prior["mean"], prior["covariance"])
    mixture = population.robust(gaussian)
    assert mixture.weights == pytest.approx((0.8, 0.2))
    weights = population.posterior(mixture, series=None, noise=0.05).weights
    assert weights.sum() == pytest.approx(1.0) and np.all(weights > 0)
    with pytest.raises(InvalidRequest):
        population.posterior(population.anchored(gaussian), series=None, noise=0.05)
    shifted = _series("new", "a", "human/trabeculectomy", level=-0.25)
    later = population.predictive(
        population.posterior(population.anchored(gaussian), series=shifted.prefix(3.0), noise=0.02),
        months=[12.0],
    )
    assert later["point_fraction"][0] == pytest.approx(shifted.fractions[-1], abs=0.04)
    with pytest.raises(InvalidRequest):
        population.robust(mixture, weight=0.0)


def test_summaries_leave_uninformed_directions_without_weight() -> None:
    one_point = _series("s", "a", "human/tube", months=(DAY,))
    summary = population.series_summary(one_point, 0.05)
    _, weak = population.weak_prior_moments()
    assert np.linalg.eigvalsh(
        summary["covariance"]
    ).max() >= 0.9 / population.INFORMATION_FLOOR * min(np.diag(weak))
    assert np.all(np.isfinite(summary["mean"]))


def test_harness_scores_every_task_without_leakage() -> None:
    library = _library()
    with pytest.raises(InvalidRequest):
        evaluation.evaluate_study(study_series=library[:1], training=library)
    report = evaluation.leave_one_study_out(
        library, studies=["tube-2"], clusters=[["tube-2", "tube-3"]]
    )
    study = report["studies"][0]
    assert {"tube-2", "tube-3"}.isdisjoint(study["training_studies"])
    assert study["training_exclusions"] == ["tube-3"]
    assert report["model_version"] == "1.1.0"
    forecast = study["next_observation"][0]
    assert len(forecast["rows"]) == len(MONTHS) - 1
    assert {"persistence", "analog_knn", "population_assimilation_robust"} <= set(
        forecast["scores"]
    )
    summary = report["summary"]
    robust = summary["next_observation_group_means"]["population_assimilation_robust"]
    assert robust["pooled_rmse_pp"] < summary["next_observation"]["persistence"]["pooled_rmse_pp"]
    assert study["initial_state"][0]["population_level_used"] == "human/tube"


def test_operator_loads_tranche_one_training_records() -> None:
    operator = _script("evaluate_iop_population_1_1")
    assert set(operator.HARNESSES) == {"1.0.0", "1.1.0"}
    assert (
        operator.check_protocol(
            {
                "frozen_files_sha256": {"missing/file": "0"},
                "training_records_sha256": {},
                "sealed_records_sha256": {},
            }
        )[0]
        == "missing missing/file"
    )
