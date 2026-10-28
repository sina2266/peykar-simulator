"""Synthetic checks of the IOP-after-surgery population model, harness and publisher.

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

from peykar.model_learning import iop_population as population
from peykar.validation import eye_iop_evaluation as evaluation
from peykar.validation.eye_iop_data import PressureSeries
from peykar_core.errors import InvalidRequest

ROOT = Path(__file__).resolve().parents[2]
DAY = population.DAY_MONTHS
MONTHS = (DAY, 7 * DAY, 1.0, 3.0, 6.0, 12.0)
SLOPE = -0.75
LEVELS = {"human/trabeculectomy": -0.45, "human/tube": -0.5, "human/angle_mims": -0.3}
EXCESS = {"human/trabeculectomy": -0.35, "human/tube": -0.25, "human/angle_mims": 0.05}


def _series(
    study: str,
    name: str,
    context: str,
    *,
    reference_mmhg: float = 25.0,
    level: float | None = None,
    excess: float | None = None,
    tau_days: float = 20.0,
    months: tuple[float, ...] = MONTHS,
    jitter: float = 0.0,
    seed: int = 0,
    role: str = "control",
    n: int | None = 40,
) -> PressureSeries:
    x = math.log(reference_mmhg / population.REFERENCE_MMHG)
    ell = (LEVELS[context] if level is None else level) + SLOPE * x
    curve = population.pressure_fraction(
        months,
        level=ell,
        excess=EXCESS[context] if excess is None else excess,
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
                        jitter=0.02,
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


def test_kinetics_return_from_day_one_deviation_to_the_long_term_level() -> None:
    values = population.pressure_fraction(
        [DAY, 7 * DAY, 1.0, 12.0, 1200.0], level=-0.4, excess=-0.3, tau_months=0.5
    )
    assert values[0] == pytest.approx(math.exp(-0.7))
    assert values[0] < values[1] < values[2] < values[3]
    assert values[4] == pytest.approx(math.exp(-0.4), rel=1e-9)
    assert np.allclose(population.recovery_ratio([DAY], population.LOG_TAU_GRID), 1.0)


def test_one_family_per_species_and_the_preoperative_covariate() -> None:
    assert population.pressure_family("human/tube") == "human/all"
    assert population.pressure_family("human/other") == "human/all"
    assert population.pressure_family("rabbit/trabeculectomy") == "rabbit/all"
    with pytest.raises(InvalidRequest):
        population.pressure_family("human/lasik")
    item = _series("s", "a", "human/tube", reference_mmhg=40.0, months=(1.0,))
    assert population.level_covariate(item) == pytest.approx(math.log(2.0))
    blind = PressureSeries(
        series_id="s/x",
        study_id="s",
        context="human/tube",
        reference_mmhg=0.0,
        months=(1.0,),
        fractions=(0.6,),
    )
    with pytest.raises(InvalidRequest):
        population.level_covariate(blind)


def test_posterior_is_the_exact_linear_gaussian_update_at_a_fixed_tau() -> None:
    log_tau = float(population.LOG_TAU_GRID[40])
    mean = np.array([-0.4, -0.2, log_tau])
    covariance = np.diag([0.05**2, 0.1**2, 1e-10])
    item = _series(
        "s",
        "a",
        "human/trabeculectomy",
        tau_days=math.exp(log_tau) / DAY,
        months=MONTHS[:4],
        jitter=0.02,
        seed=3,
    )
    sigma = 0.03
    result = population.posterior(population.gaussian(mean, covariance), series=item, noise=sigma)
    got, _ = population.moments(result)
    design = np.stack(
        [np.ones(4), population.recovery_ratio(item.months, np.array([log_tau]))[0]], axis=1
    )
    prior_precision = np.linalg.inv(covariance[:2, :2])
    precision = prior_precision + design.T @ design / sigma**2
    expected = np.linalg.solve(
        precision, prior_precision @ mean[:2] + design.T @ np.log(item.fractions) / sigma**2
    )
    assert got[:2] == pytest.approx(expected, abs=1e-8)
    assert got[2] == pytest.approx(log_tau, abs=1e-6)


def test_fit_recovers_the_level_slope_and_context_levels(fitted: population.PressureFit) -> None:
    described = fitted.describe()["families"]
    assert set(described) == {"human/all"}
    family = described["human/all"]
    assert family["level_slope_on_ln_preop_iop"] == pytest.approx(SLOPE, abs=0.12)
    assert family["postoperative_iop_elasticity"] == pytest.approx(1.0 + SLOPE, abs=0.12)
    for context, level in LEVELS.items():
        got = family["contexts"][context]
        assert got["long_term_fraction_at_20_mmhg"] == pytest.approx(math.exp(level), abs=0.04)
        assert got["excess_day1"] == pytest.approx(EXCESS[context], abs=0.08)


def test_prior_mean_level_moves_with_the_preoperative_pressure(
    fitted: population.PressureFit,
) -> None:
    low = fitted.prior("human/tube", "aggregate", math.log(18 / 20))
    high = fitted.prior("human/tube", "aggregate", math.log(36 / 20))
    slope = fitted.families["human/all"].fixed[3]
    assert high["mean"][0] - low["mean"][0] == pytest.approx(slope * math.log(2.0), rel=1e-9)
    assert high["mean"][1:] == pytest.approx(low["mean"][1:])
    assert np.all(np.linalg.eigvalsh(low["covariance"]) > 0)


def test_unseen_procedures_fall_back_to_the_species_mean(fitted: population.PressureFit) -> None:
    assert fitted.level_used("human/tube") == "human/tube"
    assert fitted.level_used("human/cyclodestruction") == "family:human/all"
    seen = fitted.prior("human/tube", "aggregate", 0.0)
    unseen = fitted.prior("human/cyclodestruction", "aggregate", 0.0)
    assert np.all(np.diag(unseen["covariance"]) > np.diag(seen["covariance"]) - 1e-12)
    single = fitted.prior("human/tube", "individual", 0.0)
    assert np.all(np.diag(single["covariance"]) >= np.diag(seen["covariance"]) - 1e-12)
    with pytest.raises(InvalidRequest):
        fitted.prior("rabbit/tube", "aggregate", 0.0)
    with pytest.raises(InvalidRequest):
        fitted.prior("human/tube", "pooled", 0.0)


def test_fit_rejects_treated_and_duplicate_series() -> None:
    library = _library()
    treated = _series("x", "t", "human/tube", role="treated")
    with pytest.raises(InvalidRequest):
        population.fit_population([*library, treated])
    with pytest.raises(InvalidRequest):
        population.fit_population([library[0], library[0]])
    with pytest.raises(InvalidRequest):
        population.fit_population([s for s in library if s.study_id == "tube-0"][:1])


def test_assimilation_tracks_a_held_out_series_with_a_median_point(
    fitted: population.PressureFit,
) -> None:
    truth = _series("new", "a", "human/trabeculectomy", reference_mmhg=30.0, level=-0.5)
    prior = fitted.prior("human/trabeculectomy", "aggregate", population.level_covariate(truth))
    result = population.predictive(
        population.posterior(
            population.robust(population.gaussian(prior["mean"], prior["covariance"])),
            series=truth.prefix(3.0),
            noise=fitted.noise("aggregate", "human/trabeculectomy"),
        ),
        months=[6.0, 12.0],
    )
    assert result["point_fraction"] == result["quantiles"]["0.5"]
    assert result["point_fraction"][1] == pytest.approx(truth.fractions[-1], abs=0.03)
    quantiles = result["quantiles"]
    assert quantiles["0.05"][1] < quantiles["0.5"][1] < quantiles["0.95"][1]
    assert set(result["posterior_mean"]) == set(population.PARAMETERS)


def test_median_point_stays_finite_under_a_heavy_tailed_prior(
    fitted: population.PressureFit,
) -> None:
    prior = fitted.prior("human/angle_mims", "individual", 0.0)
    t4 = population.student(prior["mean"], prior["covariance"], 4.0)
    result = population.predictive(
        population.posterior(t4, series=None, noise=fitted.noise("individual", "human/tube")),
        months=[DAY, 1.0, 12.0],
    )
    assert all(math.isfinite(v) and 0 < v < 5 for v in result["point_fraction"])
    with pytest.raises(InvalidRequest):
        population.predictive(
            population.posterior(t4, series=None, noise=0.05), months=[1.0], quantiles=(0.1, 0.9)
        )


def test_anchored_forecast_follows_the_prefix_level(fitted: population.PressureFit) -> None:
    shifted = _series("new", "a", "human/angle_mims", level=-0.15)
    prior = fitted.prior("human/angle_mims", "aggregate", population.level_covariate(shifted))
    noise = fitted.noise("aggregate", "human/angle_mims")
    anchored = population.anchored(population.gaussian(prior["mean"], prior["covariance"]))
    later = population.predictive(
        population.posterior(anchored, series=shifted.prefix(3.0), noise=noise), months=[12.0]
    )
    assert later["point_fraction"][0] == pytest.approx(shifted.fractions[-1], abs=0.03)
    with pytest.raises(InvalidRequest):
        population.posterior(anchored, series=None, noise=noise)


def test_student_and_robust_priors_are_normalized_mixtures(
    fitted: population.PressureFit,
) -> None:
    prior = fitted.prior("human/tube", "aggregate", 0.0)
    t4 = population.student(prior["mean"], prior["covariance"], 4.0)
    assert sum(t4.weights) == pytest.approx(1.0)
    mixture = population.robust(population.gaussian(prior["mean"], prior["covariance"]))
    assert mixture.weights == pytest.approx((0.8, 0.2))
    for variant in (t4, mixture):
        weights = population.posterior(variant, series=None, noise=0.05).weights
        assert weights.sum() == pytest.approx(1.0)
        assert np.all(weights > 0)
    with pytest.raises(InvalidRequest):
        population.robust(mixture, weight=1.0)
    with pytest.raises(InvalidRequest):
        population.student(prior["mean"], prior["covariance"], 0.0)
    with pytest.raises(InvalidRequest):
        population.predictive(population.posterior(mixture, series=None, noise=0.05), months=[0.0])


def test_summaries_leave_uninformed_directions_without_weight() -> None:
    one_point = _series("s", "a", "human/tube", months=(DAY,))
    summary = population.series_summary(one_point, 0.05)
    _, weak = population.weak_prior_moments()
    variances = np.linalg.eigvalsh(summary["covariance"])
    assert variances.max() >= 0.9 / population.INFORMATION_FLOOR * min(np.diag(weak))
    assert np.all(np.isfinite(summary["mean"]))
    rich = _series("s", "b", "human/tube", level=-0.5)
    informed = population.series_summary(rich, 0.01)
    expected_level = -0.5 + SLOPE * math.log(25.0 / 20.0)
    assert informed["mean"][0] == pytest.approx(expected_level, abs=0.03)
    assert informed["covariate"] == pytest.approx(math.log(25.0 / 20.0))


def test_study_noise_needs_four_follow_ups() -> None:
    noisy = [
        _series("s", "a", "human/tube", jitter=0.05, seed=1),
        _series("s", "b", "human/tube", jitter=0.05, seed=2),
        _series("t", "a", "human/tube", months=MONTHS[:3]),
    ]
    estimates = population.study_noise(noisy)
    assert set(estimates) == {("s", "human/all", "aggregate")}
    assert 0.02 < estimates[("s", "human/all", "aggregate")] < 0.1
    centers = population.noise_centers(estimates)
    assert centers[("*", "individual")] >= population.NOISE_FLOOR
    with pytest.raises(InvalidRequest):
        population.noise_centers({})


def test_helpers_pool_by_context_or_species() -> None:
    library = _library()
    item = library[0]
    assert evaluation.log_fraction_at(item, 0.0) == 0.0
    assert evaluation.log_fraction_at(item, DAY) == pytest.approx(math.log(item.fractions[0]))
    assert evaluation.log_fraction_at(item, 24.0) is None
    curve = evaluation.pooled_log_curve(library, "human/cyclodestruction", [1.0])
    assert curve is not None and curve[1] == "family:human/all"
    assert evaluation.pooled_log_curve(library, "rabbit/tube", [1.0]) is None
    analog = evaluation.analog_forecast(
        library, context="human/tube", history=[(0.0, 1.0), (1.0, 0.6)], target_month=6.0
    )
    assert analog is not None and len(analog["neighbours"]) == evaluation.ANALOG_K


def test_leave_one_study_out_scores_every_task_without_leakage() -> None:
    library = _library()
    with pytest.raises(InvalidRequest):
        evaluation.evaluate_study(study_series=library[:1], training=library)
    report = evaluation.leave_one_study_out(library, studies=["tube-2"])
    study = report["studies"][0]
    assert "tube-2" not in study["training_studies"]
    assert study["initial_state"][0]["population_level_used"] == "human/tube"
    forecast = study["next_observation"][0]
    assert len(forecast["rows"]) == len(MONTHS) - 1
    expected = {"persistence", "preoperative_level", "population_assimilation_robust"}
    assert expected <= set(forecast["scores"])
    landmark = study["landmark"][0]
    assert [row["target_month"] for row in landmark["rows"]] == list(MONTHS[1:])
    summary = report["summary"]
    assert summary["next_observation_group_means"]["persistence"]["series"] == 2
    assert "human/tube" in summary["by_context"]
    coverage = summary["coverage"]["80"]["landmark"]["population_assimilation_robust"]
    assert coverage["n"] == 2 * (len(MONTHS) - 1)
    robust = summary["next_observation"]["population_assimilation_robust"]["pooled_rmse_pp"]
    assert robust < summary["next_observation"]["persistence"]["pooled_rmse_pp"]


def test_declared_clusters_leave_training_together() -> None:
    library = _library()
    report = evaluation.leave_one_study_out(
        library, studies=["tube-0"], clusters=[["tube-0", "tube-1"]]
    )
    study = report["studies"][0]
    assert study["training_exclusions"] == ["tube-1"]
    assert "tube-1" not in study["training_studies"]
    assert report["clusters"] == [["tube-0", "tube-1"]]


def test_publisher_republishes_values_only_from_cc_by_or_cc0_records() -> None:
    publish = _script("publish_iop_population")

    def record(statement: str, url: str | None = None) -> dict[str, object]:
        return {"license": {"statement": statement, "url": url}}

    assert publish.publishable(
        record("CC BY 4.0 (article)", "https://creativecommons.org/licenses/by/4.0/")
    )
    assert publish.publishable(record("CC BY, version not stated"))
    assert publish.publishable(record("Creative Commons Attribution License, version not stated"))
    assert publish.publishable(
        record("CC0 1.0", "https://creativecommons.org/publicdomain/zero/1.0/")
    )
    assert not publish.publishable(
        record("CC BY-NC 4.0", "https://creativecommons.org/licenses/by-nc/4.0/")
    )
    assert not publish.publishable(record("Dove terms incorporating CC BY-NC 3.0; not CC BY"))
    assert not publish.publishable(
        record(
            "Unclear: text names Creative Commons Attribution 4.0 but links by-nc",
            "https://creativecommons.org/licenses/by-nc/4.0/",
        )
    )
    assert not publish.publishable(
        record("CC BY 4.0", "https://creativecommons.org/licenses/by-nc/4.0/")
    )
    assert not publish.publishable(record("Publisher licence (Springer)"))
