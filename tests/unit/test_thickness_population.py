"""Synthetic checks of the corneal thickness population model and its harness.

The data are generated from the declared kinetic law; passing these tests says the
code does what it declares, not that any biological or clinical claim holds.
"""

from __future__ import annotations

import math

import numpy as np
import pytest

from peykar.model_learning import thickness_population as population
from peykar.validation import cornea_thickness_evaluation as evaluation
from peykar.validation.cornea_thickness_data import ThicknessSeries
from peykar_core.errors import InvalidRequest

DAY = population.DAY_MONTHS
HOST_MONTHS = (DAY, 7 * DAY, 1.0, 3.0)
GRAFT_MONTHS = (1.0, 3.0, 6.0, 12.0)


def _series(
    study: str,
    name: str,
    context: str,
    *,
    level: float,
    excess: float,
    tau_days: float,
    months: tuple[float, ...],
    reference_um: float = 540.0,
    jitter: float = 0.0,
    seed: int = 0,
    role: str = "control",
    n: int | None = 40,
) -> ThicknessSeries:
    probe = ThicknessSeries(
        series_id="probe",
        study_id="probe",
        context=context,
        reference_um=reference_um,
        months=months,
        fractions=tuple(1.0 for _ in months),
    )
    curve = population.thickness_fraction(
        months,
        level=level,
        excess=excess,
        tau_months=tau_days * DAY,
        offset=population.level_offset(probe),
    )
    noise = np.exp(np.random.default_rng(seed).normal(0.0, jitter, len(months)))
    return ThicknessSeries(
        series_id=f"{study}/{name}",
        study_id=study,
        context=context,
        reference_um=reference_um,
        months=months,
        fractions=tuple(float(v) for v in curve * noise),
        aggregate_n=n,
        role=role,
        group=name,
    )


def _library() -> list[ThicknessSeries]:
    series = []
    for index in range(5):
        for arm, excess in (("a", 0.10), ("b", 0.14)):
            series.append(
                _series(
                    f"cat-{index}",
                    arm,
                    "human/cataract",
                    level=0.0,
                    excess=excess + 0.01 * index,
                    tau_days=4.0,
                    months=HOST_MONTHS,
                    jitter=0.004,
                    seed=index * 10 + len(arm),
                )
            )
        series.append(
            _series(
                f"dmek-{index}",
                "a",
                "human/dmek",
                level=math.log(540.0 / population.REFERENCE_UM) + 0.01 * (index - 2),
                excess=0.3,
                tau_days=10.0,
                months=GRAFT_MONTHS,
                reference_um=650.0 + 60.0 * index,
                jitter=0.01,
                seed=50 + index,
            )
        )
    return series


@pytest.fixture(scope="module")
def fitted() -> population.ThicknessFit:
    return population.fit_population(_library())


def test_kinetics_return_from_day_one_excess_to_the_long_term_level() -> None:
    values = population.thickness_fraction(
        [DAY, 7 * DAY, 1.0, 12.0, 1200.0], level=-0.01, excess=0.12, tau_months=4 * DAY
    )
    assert values[0] == pytest.approx(math.exp(-0.01 + 0.12))
    assert values[0] > values[1] > values[2] > values[3] > values[4]
    assert values[4] == pytest.approx(math.exp(-0.01), abs=2e-4)
    ratio = population.recovery_ratio([DAY], population.LOG_TAU_GRID)
    assert np.allclose(ratio, 1.0)
    graft = population.thickness_fraction(
        [1200.0], level=math.log(540 / 550), excess=0.3, tau_months=0.3, offset=math.log(800 / 550)
    )
    assert graft[0] == pytest.approx(540.0 / 800.0, rel=1e-3)


def test_families_offsets_and_unknown_procedures() -> None:
    assert population.thickness_family("human/cataract") == "human/host"
    assert population.thickness_family("human/vitreoretinal") == "human/host"
    assert population.thickness_family("human/dalk") == "human/graft"
    assert population.thickness_family("human/other") == "human/graft"
    with pytest.raises(InvalidRequest):
        population.thickness_family("human/lasik")
    host = _series("s", "h", "human/glaucoma", level=0, excess=0.05, tau_days=5, months=(1.0,))
    assert population.level_offset(host) == 0.0
    graft = _series(
        "s", "g", "human/dsaek", level=0, excess=0.0, tau_days=5, months=(1.0,), reference_um=825
    )
    assert population.level_offset(graft) == pytest.approx(math.log(1.5))
    blind = ThicknessSeries(
        series_id="s/x",
        study_id="s",
        context="human/pk",
        reference_um=0.0,
        months=(1.0,),
        fractions=(0.9,),
    )
    with pytest.raises(InvalidRequest):
        population.level_offset(blind)


def test_posterior_is_the_exact_linear_gaussian_update_at_a_fixed_tau() -> None:
    log_tau = float(population.LOG_TAU_GRID[40])
    mean = np.array([0.0, 0.1, log_tau])
    covariance = np.diag([0.01**2, 0.05**2, 1e-10])
    item = _series(
        "s",
        "a",
        "human/cataract",
        level=0.004,
        excess=0.13,
        tau_days=math.exp(log_tau) / DAY,
        months=HOST_MONTHS[:3],
        jitter=0.01,
        seed=3,
    )
    sigma = 0.01
    result = population.posterior(population.gaussian(mean, covariance), series=item, noise=sigma)
    got, _ = population.moments(result)
    design = np.stack(
        [np.ones(3), population.recovery_ratio(item.months, np.array([log_tau]))[0]], axis=1
    )
    prior_precision = np.linalg.inv(covariance[:2, :2])
    precision = prior_precision + design.T @ design / sigma**2
    expected = np.linalg.solve(
        precision, prior_precision @ mean[:2] + design.T @ np.log(item.fractions) / sigma**2
    )
    assert got[:2] == pytest.approx(expected, abs=1e-8)
    assert got[2] == pytest.approx(log_tau, abs=1e-6)


def test_fit_recovers_levels_and_falls_back_by_family(fitted: population.ThicknessFit) -> None:
    described = fitted.describe()["families"]
    assert set(described) == {"human/graft", "human/host"}
    cataract = described["human/host"]["contexts"]["human/cataract"]
    dmek = described["human/graft"]["contexts"]["human/dmek"]
    assert cataract["long_term_fraction"] == pytest.approx(1.0, abs=0.01)
    assert cataract["excess_day1"] == pytest.approx(0.14, abs=0.03)
    assert dmek["long_term_um"] == pytest.approx(540.0, abs=15.0)
    seen = fitted.prior("human/cataract", "aggregate")
    unseen = fitted.prior("human/glaucoma", "aggregate")
    assert fitted.level_used("human/glaucoma") == "family:human/host"
    assert np.all(np.diag(unseen["covariance"]) > np.diag(seen["covariance"]) - 1e-12)
    assert np.all(np.linalg.eigvalsh(seen["covariance"]) > 0)
    with pytest.raises(InvalidRequest):
        fitted.prior("rabbit/cataract", "aggregate")
    with pytest.raises(InvalidRequest):
        fitted.prior("human/cataract", "pooled")


def test_fit_rejects_treated_and_duplicate_series() -> None:
    library = _library()
    treated = _series(
        "x",
        "t",
        "human/cataract",
        level=0,
        excess=0.1,
        tau_days=4,
        months=HOST_MONTHS,
        role="treated",
    )
    with pytest.raises(InvalidRequest):
        population.fit_population([*library, treated])
    with pytest.raises(InvalidRequest):
        population.fit_population([library[0], library[0]])


def test_absolute_level_prior_uses_the_preoperative_thickness(
    fitted: population.ThicknessFit,
) -> None:
    prior = fitted.prior("human/dmek", "aggregate")
    noise = fitted.noise("aggregate", "human/dmek")
    for reference in (700.0, 950.0):
        item = _series(
            "new",
            "a",
            "human/dmek",
            level=math.log(540 / 550),
            excess=0.3,
            tau_days=10,
            months=(12.0,),
            reference_um=reference,
        )
        result = population.predictive(
            population.posterior(
                population.gaussian(prior["mean"], prior["covariance"]),
                series=None,
                noise=noise,
                offset=population.level_offset(item),
            ),
            months=[12.0],
        )
        assert result["mean_fraction"][0] == pytest.approx(540.0 / reference, abs=0.04)


def test_assimilation_tracks_a_held_out_series(fitted: population.ThicknessFit) -> None:
    truth = _series(
        "new",
        "a",
        "human/dmek",
        level=math.log(560 / 550),
        excess=0.3,
        tau_days=10,
        months=GRAFT_MONTHS,
        reference_um=820,
    )
    prior = fitted.prior("human/dmek", "aggregate")
    result = population.predictive(
        population.posterior(
            population.robust(population.gaussian(prior["mean"], prior["covariance"])),
            series=truth.prefix(3.0),
            noise=fitted.noise("aggregate", "human/dmek"),
            offset=population.level_offset(truth),
        ),
        months=[12.0],
    )
    assert result["mean_fraction"][0] == pytest.approx(truth.fractions[-1], abs=0.03)
    quantiles = result["quantiles"]
    assert quantiles["0.05"][0] < quantiles["0.5"][0] < quantiles["0.95"][0]


def test_anchored_forecast_follows_the_prefix_level(fitted: population.ThicknessFit) -> None:
    prior = fitted.prior("human/cataract", "aggregate")
    noise = fitted.noise("aggregate", "human/cataract")
    shifted = _series(
        "new", "a", "human/cataract", level=0.05, excess=0.12, tau_days=4, months=HOST_MONTHS
    )
    anchored = population.anchored(population.gaussian(prior["mean"], prior["covariance"]))
    one = population.predictive(
        population.posterior(anchored, series=shifted.prefix(DAY), noise=noise),
        months=[DAY],
    )
    assert one["mean_fraction"][0] == pytest.approx(shifted.fractions[0], rel=0.01)
    later = population.predictive(
        population.posterior(anchored, series=shifted.prefix(1.0), noise=noise), months=[3.0]
    )
    assert later["mean_fraction"][0] == pytest.approx(shifted.fractions[-1], abs=0.02)
    with pytest.raises(InvalidRequest):
        population.posterior(anchored, series=None, noise=noise)


def test_student_and_robust_priors_are_normalized_mixtures(
    fitted: population.ThicknessFit,
) -> None:
    prior = fitted.prior("human/cataract", "aggregate")
    t4 = population.student(prior["mean"], prior["covariance"], 4.0)
    assert sum(t4.weights) == pytest.approx(1.0)
    mixture = population.robust(population.gaussian(prior["mean"], prior["covariance"]))
    assert mixture.weights == pytest.approx((0.8, 0.2))
    for variant in (t4, mixture):
        weights = population.posterior(variant, series=None, noise=0.01).weights
        assert weights.sum() == pytest.approx(1.0)
        assert np.all(weights > 0)
    with pytest.raises(InvalidRequest):
        population.robust(mixture, weight=1.0)
    with pytest.raises(InvalidRequest):
        population.student(prior["mean"], prior["covariance"], 0.0)
    with pytest.raises(InvalidRequest):
        population.predictive(population.posterior(mixture, series=None, noise=0.01), months=[0.0])


def test_summaries_leave_uninformed_directions_without_weight() -> None:
    one_point = _series(
        "s", "a", "human/cataract", level=0.0, excess=0.1, tau_days=4, months=(DAY,)
    )
    summary = population.series_summary(one_point, 0.01)
    _, weak = population.weak_prior_moments()
    variances = np.linalg.eigvalsh(summary["covariance"])
    assert variances.max() >= 0.9 / population.INFORMATION_FLOOR * min(np.diag(weak))
    assert np.all(np.isfinite(summary["mean"]))
    assert abs(summary["mean"][2] - np.mean(population.LOG_TAU_GRID)) < 3 * math.sqrt(weak[2, 2])
    rich = _series(
        "s", "b", "human/cataract", level=0.0, excess=0.1, tau_days=4, months=HOST_MONTHS
    )
    informed = population.series_summary(rich, 0.005)
    assert informed["mean"][1] == pytest.approx(0.1, abs=0.02)


def test_normal_cdf_approximation() -> None:
    grid = np.linspace(-6, 6, 241)
    exact = np.array([0.5 * (1 + math.erf(x / math.sqrt(2))) for x in grid])
    assert np.max(np.abs(population._normal_cdf(grid) - exact)) < 2e-7


def test_helpers_pool_by_context_or_family() -> None:
    library = _library()
    item = library[0]
    assert evaluation.log_fraction_at(item, 0.0) == 0.0
    assert evaluation.log_fraction_at(item, DAY) == pytest.approx(math.log(item.fractions[0]))
    assert evaluation.log_fraction_at(item, 24.0) is None
    curve = evaluation.pooled_log_curve(library, "human/glaucoma", [1.0])
    assert curve is not None and curve[1] == "family:human/host"
    assert evaluation.pooled_log_curve(library, "rabbit/cataract", [1.0]) is None
    analog = evaluation.analog_forecast(
        library, context="human/dsaek", history=[(0.0, 1.0), (1.0, 0.8)], target_month=6.0
    )
    assert analog is not None and len(analog["neighbours"]) == evaluation.ANALOG_K


def test_leave_one_study_out_scores_every_task_without_leakage() -> None:
    library = _library()
    with pytest.raises(InvalidRequest):
        evaluation.evaluate_study(study_series=library[:1], training=library)
    report = evaluation.leave_one_study_out(library, studies=["dmek-2"])
    study = report["studies"][0]
    assert "dmek-2" not in study["training_studies"]
    assert study["initial_state"][0]["population_level_used"] == "human/dmek"
    forecast = study["next_observation"][0]
    assert len(forecast["rows"]) == len(GRAFT_MONTHS) - 1
    expected = {"persistence", "preoperative_level", "population_assimilation_robust"}
    assert expected <= set(forecast["scores"])
    landmark = study["landmark"][0]
    assert [row["target_month"] for row in landmark["rows"]] == list(GRAFT_MONTHS[1:])
    summary = report["summary"]
    assert summary["next_observation"]["persistence"]["series"] == 1
    assert "human/graft" in summary["by_family"]
    coverage = summary["coverage"]["80"]["landmark"]["population_assimilation_robust"]
    assert coverage["n"] == len(GRAFT_MONTHS) - 1


def test_declared_clusters_leave_training_together() -> None:
    library = _library()
    report = evaluation.leave_one_study_out(
        library, studies=["cat-0"], clusters=[["cat-0", "cat-1"]]
    )
    study = report["studies"][0]
    assert study["training_exclusions"] == ["cat-1"]
    assert "cat-1" not in study["training_studies"]
    assert report["clusters"] == [["cat-0", "cat-1"]]
