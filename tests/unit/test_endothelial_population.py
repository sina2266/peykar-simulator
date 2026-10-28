"""Synthetic checks of the endothelial density population model and its harness.

The data are generated from the declared kinetic law; passing these tests says the
code does what it declares, not that any biological or clinical claim holds.
"""

from __future__ import annotations

import math

import numpy as np
import pytest

from peykar.model_learning import endothelial_population as population
from peykar.validation import cornea_endothelium_evaluation as evaluation
from peykar.validation.cornea_endothelium_data import DensitySeries
from peykar_core.errors import InvalidRequest

MONTHS = (1.0, 3.0, 6.0, 12.0)


def _series(
    study: str,
    name: str,
    context: str,
    *,
    loss: float,
    tau: float,
    exponent: float,
    months: tuple[float, ...] = MONTHS,
    offset: float = 1.0,
    jitter: float = 0.0,
    seed: int = 0,
    reference_kind: str = "patient_preoperative",
    role: str = "control",
    n: int | None = 40,
) -> DensitySeries:
    curve = population.density_fraction(
        months, immediate_loss=loss, tau_months=tau, exponent=exponent
    )[0]
    noise = np.exp(np.random.default_rng(seed).normal(0.0, jitter, len(months)))
    return DensitySeries(
        series_id=f"{study}/{name}",
        study_id=study,
        context=context,
        reference_ecd=2500.0,
        months=months,
        fractions=tuple(float(v) for v in curve * noise * offset),
        aggregate_n=n,
        role=role,
        group=name,
        reference_kind=reference_kind,
    )


def _library() -> list[DensitySeries]:
    series = []
    for index in range(5):
        series.append(
            _series(
                f"cat-{index}",
                "a",
                "human/cataract",
                loss=0.08 + 0.01 * index,
                tau=0.5,
                exponent=0.01,
                jitter=0.01,
                seed=index,
            )
        )
        series.append(
            _series(
                f"cat-{index}",
                "b",
                "human/cataract",
                loss=0.1 + 0.01 * index,
                tau=0.5,
                exponent=0.01,
                jitter=0.01,
                seed=10 + index,
            )
        )
        series.append(
            _series(
                f"dmek-{index}",
                "a",
                "human/dmek",
                loss=0.3 + 0.02 * index,
                tau=0.8,
                exponent=0.08,
                jitter=0.02,
                seed=20 + index,
                reference_kind="donor_eye_bank",
            )
        )
    for index in range(3):
        series.append(
            _series(
                f"glau-{index}",
                "a",
                "human/glaucoma",
                loss=0.03,
                tau=1.0,
                exponent=0.02,
                jitter=0.01,
                seed=30 + index,
            )
        )
    return series


@pytest.fixture(scope="module")
def fitted() -> population.EndothelialFit:
    return population.fit_population(_library())


def test_kinetics_keep_the_immediate_loss_and_decelerate() -> None:
    values = population.density_fraction(
        [1e-9, 1.0, 12.0, 120.0], immediate_loss=0.3, tau_months=0.5, exponent=0.1
    )[0]
    assert values[0] == pytest.approx(0.7, abs=1e-6)
    assert values[0] > values[1] > values[2] > values[3]
    early = math.log(values[1] / values[2]) / 11.0
    late = math.log(values[2] / values[3]) / 108.0
    assert early > late > 0
    grid = population.grid_log_fraction([12.0])[:, 0]
    assert np.allclose(
        np.exp(grid[:5]),
        population.density_fraction(
            [12.0],
            immediate_loss=population.GRID_A[:5],
            tau_months=population.GRID_TAU[:5],
            exponent=population.GRID_EXPONENT[:5],
        )[:, 0],
    )
    assert population.GRID_A.min() == pytest.approx(0.0, abs=1e-12)
    assert population.GRID_A.max() == pytest.approx(0.95)


def test_contexts_and_families_separate_recipient_referenced_grafts() -> None:
    graft = _series("s", "g", "human/dmek", loss=0.3, tau=1, exponent=0.05)
    assert population.model_context(graft) == "human/dmek~recipient-reference"
    assert population.tissue_family(population.model_context(graft)) == "human/graft"
    donor = _series(
        "s", "d", "human/dmek", loss=0.3, tau=1, exponent=0.05, reference_kind="donor_other"
    )
    assert population.model_context(donor) == "human/dmek"
    assert population.tissue_family("human/dalk") == "human/host"
    assert population.tissue_family("rabbit/cataract") == "rabbit/host"


def test_fit_recovers_context_losses_and_falls_back_by_family(
    fitted: population.EndothelialFit,
) -> None:
    described = fitted.describe()["families"]
    assert set(described) == {"human/graft", "human/host"}
    cataract = described["human/host"]["contexts"]["human/cataract"]["immediate_loss"]
    dmek = described["human/graft"]["contexts"]["human/dmek"]["immediate_loss"]
    assert 0.06 < cataract < 0.16
    assert 0.25 < dmek < 0.45
    seen = fitted.prior("human/cataract", "aggregate")
    unseen = fitted.prior("human/vitreoretinal", "aggregate")
    assert fitted.level_used("human/vitreoretinal") == "family:human/host"
    assert np.all(np.diag(unseen["covariance"]) > np.diag(seen["covariance"]) - 1e-12)
    assert np.all(np.linalg.eigvalsh(seen["covariance"]) > 0)
    with pytest.raises(InvalidRequest):
        fitted.prior("rabbit/cataract", "aggregate")
    with pytest.raises(InvalidRequest):
        fitted.prior("human/cataract", "pooled")


def test_fit_rejects_treated_and_duplicate_series() -> None:
    library = _library()
    with pytest.raises(InvalidRequest):
        population.fit_population(
            [
                *library,
                _series("x", "t", "human/cataract", loss=0.1, tau=1, exponent=0.01, role="treated"),
            ]
        )
    with pytest.raises(InvalidRequest):
        population.fit_population([library[0], library[0]])


def test_assimilation_tracks_a_held_out_series(fitted: population.EndothelialFit) -> None:
    truth = _series(
        "new", "a", "human/dmek", loss=0.34, tau=0.8, exponent=0.08, reference_kind="donor_other"
    )
    prior = fitted.prior("human/dmek", "aggregate")
    prefix = truth.prefix(6.0)
    result = population.predictive(
        population.posterior(prior, series=prefix, noise=fitted.noise("aggregate", "human/dmek")),
        months=[12.0],
        draws=800,
    )
    assert result["mean_fraction"][0] == pytest.approx(truth.fractions[-1], abs=0.03)
    low, high = result["quantiles"]["0.05"][0], result["quantiles"]["0.95"][0]
    assert low < result["quantiles"]["0.5"][0] < high


def test_anchored_forecast_reproduces_the_conditioning_point_and_offsets(
    fitted: population.EndothelialFit,
) -> None:
    shifted = _series(
        "new",
        "a",
        "human/dmek",
        loss=0.34,
        tau=0.8,
        exponent=0.08,
        offset=0.7,
        reference_kind="donor_other",
    )
    prior = fitted.prior("human/dmek", "individual")
    noise = fitted.noise("individual", "human/dmek")
    one = population.anchored_posterior(prior, series=shifted.prefix(1.0), noise=noise)
    same = population.predictive(one, months=[1.0], draws=200)
    assert same["mean_fraction"][0] == pytest.approx(shifted.fractions[0], rel=1e-9)
    three = population.anchored_posterior(prior, series=shifted.prefix(6.0), noise=noise)
    anchored = population.predictive(three, months=[12.0], draws=400)
    level = population.predictive(
        population.posterior(prior, series=shifted.prefix(6.0), noise=noise),
        months=[12.0],
        draws=400,
    )
    target = shifted.fractions[-1]
    assert abs(anchored["mean_fraction"][0] - target) < 0.03
    assert abs(level["mean_fraction"][0] - target) < 0.03
    quantiles = anchored["quantiles"]
    assert quantiles["0.05"][0] < quantiles["0.5"][0] < quantiles["0.95"][0]


def test_robust_and_student_priors_are_proper_on_the_grid(
    fitted: population.EndothelialFit,
) -> None:
    prior = fitted.prior("human/cataract", "aggregate")
    for variant, df in ((prior, 4.0), (population.robust(prior), None)):
        weights = population.posterior(
            variant, series=None, noise=0.02, student_df=df
        ).parameter_weights
        assert weights.sum() == pytest.approx(1.0)
        assert np.all(weights > 0)
    with pytest.raises(InvalidRequest):
        population.robust(prior, weight=1.0)
    with pytest.raises(InvalidRequest):
        population.predictive(population.posterior(prior, series=None, noise=0.02), months=[0.0])


def test_interpolation_pooled_curve_and_analog_helpers() -> None:
    item = _series("s", "a", "human/cataract", loss=0.1, tau=0.5, exponent=0.01)
    assert evaluation.log_fraction_at(item, 0.0) == 0.0
    assert evaluation.log_fraction_at(item, 3.0) == pytest.approx(math.log(item.fractions[1]))
    assert evaluation.log_fraction_at(item, 24.0) is None
    library = _library()
    curve = evaluation.pooled_log_curve(library, "human/vitreoretinal", [3.0])
    assert curve is not None and curve[1] == "family:human/host"
    assert evaluation.pooled_log_curve(library, "rabbit/cataract", [3.0]) is None
    analog = evaluation.analog_forecast(
        library, context="human/dmek", history=[(0.0, 1.0), (1.0, 0.7)], target_month=6.0
    )
    assert analog is not None and len(analog["neighbours"]) == evaluation.ANALOG_K
    assert (
        evaluation.analog_forecast(
            library, context="human/dmek", history=[(0.0, 1.0), (1.0, 0.7)], target_month=60.0
        )
        is None
    )


def test_leave_one_study_out_scores_every_task_without_leakage() -> None:
    library = _library()
    with pytest.raises(InvalidRequest):
        evaluation.evaluate_study(study_series=library[:1], training=library)
    report = evaluation.leave_one_study_out(library, studies=["dmek-2"], draws=200)
    study = report["studies"][0]
    assert "dmek-2" not in study["training_studies"]
    assert study["initial_state"][0]["population_level_used"] == "human/dmek"
    forecast = study["next_observation"][0]
    assert len(forecast["rows"]) == len(MONTHS) - 1
    assert {"persistence", "population_anchored", "pooled_curve_ratio"} <= set(forecast["scores"])
    landmark = study["landmark"][0]
    assert [row["target_month"] for row in landmark["rows"]] == list(MONTHS[1:])
    summary = report["summary"]
    assert summary["next_observation"]["persistence"]["series"] == 1
    assert "human/graft" in summary["by_family"]
    assert summary["coverage"]["80"]["landmark"]["population_anchored"]["n"] == len(MONTHS) - 1


def test_declared_clusters_leave_training_together() -> None:
    library = _library()
    report = evaluation.leave_one_study_out(
        library, studies=["cat-0"], clusters=[["cat-0", "cat-1"]], draws=100
    )
    study = report["studies"][0]
    assert study["training_exclusions"] == ["cat-1"]
    assert "cat-1" not in study["training_studies"]
    assert report["clusters"] == [["cat-0", "cat-1"]]
