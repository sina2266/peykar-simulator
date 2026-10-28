"""Generic eye model version 2: equivalence with version 1 and its three extensions."""

from __future__ import annotations

import math
from collections.abc import Mapping
from dataclasses import dataclass, field
from typing import Any

import numpy as np
import pytest

from peykar.model_learning import eye_recovery_population as v1
from peykar.model_learning import eye_recovery_population2 as v2
from peykar_core.errors import InvalidRequest

DAY = 1.0 / 30.4375
MONTHS = (DAY, 7 * DAY, 1.0, 2.0, 4.0, 6.0)
COMMON: dict[str, Any] = {
    "model_id": "m",
    "model_version": "v",
    "quantity": "central retinal thickness",
    "covariate_reference": 300.0,
    "covariate_unit": "um",
    "transient_months": 14 * DAY,
}


@dataclass(frozen=True)
class Series:
    series_id: str
    study_id: str
    context: str
    reference_value: float
    months: tuple[float, ...]
    fractions: tuple[float, ...]
    aggregate_n: int | None = None
    role: str = "control"
    metadata: Mapping[str, Any] = field(default_factory=dict)

    @property
    def kind(self) -> str:
        return "aggregate" if self.aggregate_n is not None else "individual"

    def prefix(self, last: float) -> Series:
        keep = [i for i, m in enumerate(self.months) if m <= last]
        return Series(
            self.series_id,
            self.study_id,
            self.context,
            self.reference_value,
            tuple(self.months[i] for i in keep),
            tuple(self.fractions[i] for i in keep),
            self.aggregate_n,
            self.role,
            self.metadata,
        )


CONTEXTS = {
    "human/dme/anti_vegf": (-0.25, 0.05, -0.1, 2.0),
    "human/rvo/anti_vegf": (-0.35, 0.05, -0.1, 1.5),
    "human/dme/steroid_implant": (-0.3, 0.1, -0.05, 0.5),
}


def _synthetic(seed: int, duration: float | None = None) -> list[Series]:
    """Two studies per context, two group means and one single eye per study."""
    rng = np.random.default_rng(seed)
    config = v2.RecoveryConfig(**COMMON, wear_off_loss=0.8)
    out = []
    for c_index, (context, (level, excess, transient, tau)) in enumerate(sorted(CONTEXTS.items())):
        own = duration if context.endswith("steroid_implant") else None
        for s in range(2):
            study = f"st-{c_index}-{s}"
            shift = rng.normal(0, 0.05, 3)
            for k, n in enumerate((30, 60, None)):
                reference = float(rng.uniform(250, 550))
                x = math.log(reference / 300.0)
                log_curve = v2.design(config, MONTHS, np.array([math.log(tau)]), own)[0] @ (
                    np.array([level + shift[0] - 0.5 * x, excess + shift[1], transient + shift[2]])
                )
                sd = 0.05 if n is None else 0.03
                values = np.exp(log_curve + rng.normal(0, sd, len(MONTHS)))
                out.append(
                    Series(
                        series_id=f"{study}/{k}",
                        study_id=study,
                        context=context,
                        reference_value=reference,
                        months=MONTHS,
                        fractions=tuple(float(v) for v in values),
                        aggregate_n=n,
                        metadata={"effect_duration_months": own},
                    )
                )
    return out


@pytest.mark.parametrize(
    "options",
    [
        {},
        {"deviation_fraction": 0.8, "deviation_months": 12.0, "deviation_in_population_fit": False},
    ],
)
def test_default_extensions_reproduce_version_1(options: dict[str, Any]) -> None:
    data = _synthetic(seed=3, duration=3.5)
    one = v1.RecoveryConfig(**COMMON, **options)
    two = v2.RecoveryConfig(**COMMON, **options)
    a = v1.fit_population(one, data)
    b = v2.fit_population(two, data)
    fa, fb = a.families["human/all"], b.families["human/all"]
    np.testing.assert_array_equal(fa.solution, fb.solution)
    np.testing.assert_array_equal(fa.covariance, fb.covariance)
    assert fa.objective == fb.objective
    assert dict(a.noise_centers.centers) == dict(b.noise_centers.centers)
    for context in (*CONTEXTS, "human/uveitic/steroid_implant", "human/csc/laser"):
        assert a.level_used(context) == b.level_used(context)
        p, q = a.prior(context, "aggregate", 0.2), b.prior(context, "aggregate", 0.2)
        np.testing.assert_array_equal(p["mean"], q["mean"])
        np.testing.assert_array_equal(p["covariance"], q["covariance"])
    item = data[-1]
    prior = b.prior(item.context, item.kind, v2.level_covariate(two, item))
    for make in (
        lambda: (v1.gaussian(prior["mean"], prior["covariance"]),) * 2,
        lambda: (v1.robust(one, v1.gaussian(prior["mean"], prior["covariance"])),) * 2,
    ):
        p1, p2 = make()
        noise = b.noise(item.kind, item.context, item.aggregate_n)
        r1 = v1.predictive(
            one, v1.posterior(one, p1, series=item.prefix(2.0), noise=noise), months=[4.0, 6.0]
        )
        r2 = v2.predictive(
            two, v2.posterior(two, p2, series=item.prefix(2.0), noise=noise), months=[4.0, 6.0]
        )
        assert r1["point_fraction"] == r2["point_fraction"]
        assert r1["quantiles"] == r2["quantiles"]


def test_wear_off_returns_part_of_the_effect() -> None:
    config = v2.RecoveryConfig(**COMMON, wear_off_loss=0.8, wear_off_width_months=0.5)
    log_tau = np.array([math.log(0.5)])
    beta = np.array([-0.4, 0.3, 0.1])
    months = [1.0, 2.0, 3.5, 6.0, 12.0]
    plain = v2.design(config, months, log_tau)[0] @ beta
    worn = v2.design(config, months, log_tau, 3.5)[0] @ beta
    np.testing.assert_allclose(worn[2], plain[2] * 0.6)  # half worn off at the duration
    assert worn[-1] == pytest.approx(plain[-1] * 0.2, rel=1e-6)
    assert abs(worn[0] - plain[0]) < 0.01 * abs(plain[0])
    off = v2.RecoveryConfig(**COMMON)
    np.testing.assert_array_equal(
        v2.design(off, months, log_tau, 3.5), v2.design(off, months, log_tau)
    )
    # A prior-only forecast of a series with a duration rises after the duration.
    prior = v2.gaussian(np.array([-0.4, 0.3, 0.1, math.log(0.5)]), np.diag([1e-4] * 3 + [1e-8]))
    forecast = v2.predictive(
        config, v2.posterior(config, prior, series=None, noise=0.03, duration=3.5), months=months
    )
    assert forecast["point_fraction"][-1] > forecast["point_fraction"][1] + 0.2
    item = Series("s/1", "s", "human/dme/steroid_implant", 450.0, (1.0, 2.0), (0.7, 0.68))
    with pytest.raises(InvalidRequest):
        v2.posterior(config, prior, series=item, noise=0.03, duration=3.5)


def test_treatment_fallback_averages_trained_contexts_of_the_treatment() -> None:
    data = [s for s in _synthetic(seed=5) if s.context != "human/dme/steroid_implant"]
    plain = v2.fit_population(v2.RecoveryConfig(**COMMON), data)
    fallback = v2.fit_population(v2.RecoveryConfig(**COMMON, treatment_fallback=True), data)
    for context in ("human/dme/anti_vegf", "human/rvo/anti_vegf"):
        p, q = plain.location(context, 0.1), fallback.location(context, 0.1)
        np.testing.assert_array_equal(p[0], q[0])
        np.testing.assert_array_equal(p[1], q[1])
    new = "human/myopic_cnv/anti_vegf"
    assert plain.level_used(new) == "family:human/all"
    assert fallback.level_used(new) == "treatment:anti_vegf"
    mean, covariance = fallback.location(new, 0.1)
    known = [fallback.location(c, 0.1)[0] for c in ("human/dme/anti_vegf", "human/rvo/anti_vegf")]
    np.testing.assert_allclose(mean, np.mean(known, axis=0), atol=1e-12)
    family = fallback.families["human/all"]
    assert np.all(np.diag(covariance) >= np.diag(family.variances["context"]))
    assert fallback.level_used("human/csc/laser") == "family:human/all"
    with pytest.raises(InvalidRequest):
        v2.RecoveryConfig(**COMMON, treatment_fallback=True, grouping=("treatment", "context"))


def _exact_conditional(
    config: v2.RecoveryConfig,
    prior_mean: np.ndarray,
    prior_cov: np.ndarray,
    log_tau: float,
    item: Series,
    targets: list[float],
    sigma: float,
) -> tuple[np.ndarray, np.ndarray]:
    """Joint Gaussian of observed and target log fractions for one fixed ``tau``."""
    x_obs = v2.design(config, item.months, np.array([log_tau]))[0]
    x_new = v2.design(config, targets, np.array([log_tau]))[0]
    s_obs, s_new = v2.noise_scale(config, item.months), v2.noise_scale(config, targets)
    phi = config.deviation_fraction
    corr_obs = v1.residual_correlation(config, item.months)
    cov_zz = x_obs @ prior_cov @ x_obs.T + sigma**2 * np.outer(s_obs, s_obs) * corr_obs
    cross = x_new @ prior_cov @ x_obs.T + sigma**2 * phi * np.outer(
        s_new, s_obs
    ) * v1.deviation_correlation(config, targets, item.months)
    z = np.log(np.asarray(item.fractions))
    gain = np.linalg.solve(cov_zz, cross.T)
    mean = x_new @ prior_mean + gain.T @ (z - x_obs @ prior_mean)
    variance = np.diag(x_new @ prior_cov @ x_new.T + sigma**2 * np.diag(s_new**2) - cross @ gain)
    return mean, np.sqrt(variance)


@pytest.mark.parametrize("deviation", [0.0, 0.6])
def test_early_noise_matches_the_exact_joint_gaussian(deviation: float) -> None:
    config = v2.RecoveryConfig(
        **COMMON, early_noise=(3.0, 0.5), deviation_fraction=deviation, deviation_months=4.0
    )
    log_tau = float(config.log_tau_grid[40])
    mean = np.array([-0.2, 0.1, 0.05, log_tau])
    covariance = np.diag([0.04, 0.02, 0.03, 1e-10])
    prior = v2.gaussian(mean, covariance)
    item = Series("s/1", "s", "human/dme/anti_vegf", 450.0, (0.25, 1.0, 3.0), (0.8, 0.72, 0.70), 30)
    sigma = 0.05
    fitted = v2.posterior(config, prior, series=item, noise=sigma)
    assert fitted.weights.size == 1
    targets = [0.5, 6.0]
    result = v2.predictive(config, fitted, months=targets, quantiles=(0.1, 0.5, 0.9))
    expected_mean, expected_sd = _exact_conditional(
        config, mean[:3], covariance[:3, :3], log_tau, item, targets, sigma
    )
    np.testing.assert_allclose(np.log(result["point_fraction"]), expected_mean, rtol=1e-6)
    np.testing.assert_allclose(
        np.log(result["quantiles"]["0.9"]), expected_mean + 1.2815516 * expected_sd, rtol=1e-5
    )
    plain = v2.RecoveryConfig(**COMMON, deviation_fraction=deviation, deviation_months=4.0)
    narrow = v2.predictive(
        plain, v2.posterior(plain, prior, series=item, noise=sigma), months=targets
    )
    width = np.log(np.array(result["quantiles"]["0.9"]) / np.array(result["point_fraction"]))
    narrow_width = np.log(np.array(narrow["quantiles"]["0.9"]) / np.array(narrow["point_fraction"]))
    assert width[0] > narrow_width[0] * 1.3
    assert width[1] == pytest.approx(narrow_width[1], rel=0.05)
    with pytest.raises(InvalidRequest):
        v2.RecoveryConfig(**COMMON, early_noise=(-1.0, 0.5))
    with pytest.raises(InvalidRequest):
        v2.RecoveryConfig(**COMMON, wear_off_loss=1.5)


def test_effect_duration_reads_metadata() -> None:
    assert v2.effect_duration(Series("a", "s", "human/x/y", 1.0, (1.0,), (1.0,))) is None
    item = Series(
        "a", "s", "human/x/y", 1.0, (1.0,), (1.0,), metadata={"effect_duration_months": 3}
    )
    assert v2.effect_duration(item) == 3.0
    bad = Series("a", "s", "human/x/y", 1.0, (1.0,), (1.0,), metadata={"effect_duration_months": 0})
    with pytest.raises(InvalidRequest):
        v2.effect_duration(bad)
    described = v2.RecoveryConfig(**COMMON, wear_off_loss=0.8).describe()
    assert described["wear_off_loss"] == 0.8 and described["generic_model_version"] == "2.0.0"


def test_forecast_only_early_noise_widens_early_intervals_only() -> None:
    log_tau = float(v2.RecoveryConfig(**COMMON).log_tau_grid[40])
    prior = v2.gaussian(np.array([-0.2, 0.1, 0.05, log_tau]), np.diag([0.04, 0.02, 0.03, 1e-10]))
    item = Series("s/1", "s", "human/dme/anti_vegf", 450.0, (0.25, 1.0), (0.8, 0.72), 30)
    plain = v2.RecoveryConfig(**COMMON)
    widened = v2.RecoveryConfig(**COMMON, early_noise=(3.0, 0.5), early_noise_in_likelihood=False)
    a = v2.posterior(plain, prior, series=item, noise=0.05)
    b = v2.posterior(widened, prior, series=item, noise=0.05)
    np.testing.assert_array_equal(a.means, b.means)
    np.testing.assert_array_equal(a.covariances, b.covariances)
    targets = [0.5, 12.0]
    ra = v2.predictive(plain, a, months=targets, quantiles=(0.1, 0.5, 0.9))
    rb = v2.predictive(widened, b, months=targets, quantiles=(0.1, 0.5, 0.9))
    np.testing.assert_allclose(ra["point_fraction"], rb["point_fraction"], rtol=1e-12)
    x = v2.design(plain, targets, a.log_tau)[0]
    variance = np.einsum("ta,ab,tb->t", x, a.covariances[0], x)
    expected = np.sqrt(variance + 0.05**2 * v2.noise_scale(widened, targets) ** 2)
    width = np.log(np.array(rb["quantiles"]["0.9"]) / np.array(rb["point_fraction"]))
    np.testing.assert_allclose(width, 1.2815516 * expected, rtol=1e-5)
    assert v2.noise_scale(widened, [12.0])[0] == pytest.approx(1.0, abs=1e-6)


def test_pooled_noise_centre_weights_studies_by_degrees_of_freedom() -> None:
    key = ("human/all", "aggregate")
    estimates = {
        ("a", "human/all", "aggregate"): v2.PooledNoiseEstimate(sd=0.10, units=40.0, dof=6),
        ("b", "human/all", "aggregate"): v2.PooledNoiseEstimate(sd=0.01, units=40.0, dof=1),
        ("c", "human/all", "aggregate"): v2.PooledNoiseEstimate(sd=0.01, units=40.0, dof=1),
    }
    median = v2.pooled_noise_centers(v2.RecoveryConfig(**COMMON), estimates)
    pooled = v2.pooled_noise_centers(v2.RecoveryConfig(**COMMON, noise_pooling="pooled"), estimates)
    assert median.centers[key] == pytest.approx(0.01)
    assert pooled.centers[key] == pytest.approx(math.sqrt((6 * 0.01 + 2 * 0.0001) / 8))
    assert dict(median.centers) == dict(
        v1.noise_centers(v1.RecoveryConfig(**COMMON), estimates).centers
    )
    with pytest.raises(InvalidRequest):
        v2.RecoveryConfig(**COMMON, noise_pooling="mean")
    data = _synthetic(seed=11)
    fit = v2.fit_population(v2.RecoveryConfig(**COMMON, noise_pooling="pooled"), data)
    assert fit.noise_centers.centers[key] > 0.01


def test_forecast_only_pooling_inflates_intervals_not_the_fit() -> None:
    data = _synthetic(seed=11)
    plain = v2.fit_population(v2.RecoveryConfig(**COMMON), data)
    wide = v2.fit_population(v2.RecoveryConfig(**COMMON, forecast_noise_pooling="pooled"), data)
    np.testing.assert_array_equal(
        plain.families["human/all"].solution, wide.families["human/all"].solution
    )
    assert plain.forecast_inflation("aggregate", "human/dme/anti_vegf", 30) == 0.0
    pooled = v2.fit_population(v2.RecoveryConfig(**COMMON, noise_pooling="pooled"), data)
    family = "human/all"
    ratio = pooled.noise_centers.center(family, "aggregate", 30) / plain.noise_centers.center(
        family, "aggregate", 30
    )
    assert wide.forecast_inflation("aggregate", "human/dme/anti_vegf", 30) == pytest.approx(
        max(0.0, ratio**2 - 1.0)
    )
    item = data[0]
    prior = v2.gaussian(np.array([-0.2, 0.1, 0.05, 0.0]), np.diag([0.04, 0.02, 0.03, 1.0]))
    fitted = v2.posterior(v2.RecoveryConfig(**COMMON), prior, series=item.prefix(1.0), noise=0.05)
    base = v2.predictive(v2.RecoveryConfig(**COMMON), fitted, months=[6.0])
    inflated = v2.predictive(v2.RecoveryConfig(**COMMON), fitted, months=[6.0], extra_noise=1.0)
    assert inflated["quantiles"]["0.95"][0] > base["quantiles"]["0.95"][0]
    with pytest.raises(InvalidRequest):
        v2.predictive(v2.RecoveryConfig(**COMMON), fitted, months=[6.0], extra_noise=-1.0)


def test_per_group_study_effects_reproduce_iop_1_1_with_multi_context_studies() -> None:
    from peykar.model_learning import iop_population_1_1 as iop11
    from peykar.validation.eye_iop_data import PressureSeries

    rng = np.random.default_rng(21)
    iop = v2.RecoveryConfig(
        model_id="eye.iop-population",
        model_version="1.1.0",
        quantity="intraocular pressure",
        covariate_reference=20.0,
        covariate_unit="mmHg",
        study_effects="per_group",
    )
    contexts = {
        "human/trabeculectomy": (-0.45, -0.3, 0.1, 3.0),
        "human/tube": (-0.35, -0.2, 0.2, 6.0),
        "human/angle_mims": (-0.2, -0.1, 0.25, 1.0),
    }
    months = (DAY, 7 * DAY, 1.0, 3.0, 6.0, 12.0)
    data = []
    for c, (context, (level, excess, transient, tau)) in enumerate(sorted(contexts.items())):
        for s in range(2):
            # Study "st-x" has arms in two contexts.
            study = "st-x" if (c, s) in {(0, 0), (1, 0)} else f"st-{c}-{s}"
            for k, n in enumerate((30, 60, None)):
                reference = float(rng.uniform(15, 35))
                x = math.log(reference / 20.0)
                base_curve = v1.law_fraction(
                    iop,
                    months,
                    level=level - 0.5 * x + rng.normal(0, 0.05),
                    excess=excess,
                    transient=transient,
                    tau_months=tau,
                )
                values = base_curve * np.exp(rng.normal(0, 0.03, len(months)))
                data.append(
                    Series(
                        f"{study}/{context}/{k}",
                        study,
                        context,
                        reference,
                        months,
                        tuple(float(v) for v in values),
                        n,
                    )
                )
    ours = v2.fit_population(iop, data)
    pressure = [
        PressureSeries(
            series_id=s.series_id,
            study_id=s.study_id,
            context=s.context,
            reference_mmhg=s.reference_value,
            months=s.months,
            fractions=s.fractions,
            aggregate_n=s.aggregate_n,
        )
        for s in data
    ]
    theirs = iop11.fit_population(pressure)
    a, b = ours.families["human/all"], theirs.families["human/all"]
    np.testing.assert_allclose(a.fixed, b.fixed, rtol=1e-8, atol=1e-11)
    for level in iop11.LEVELS:
        np.testing.assert_allclose(a.variances[level], b.variances[level], rtol=1e-7, atol=1e-12)
    for context in contexts:
        p, q = ours.prior(context, "aggregate", 0.3), theirs.prior(context, "aggregate", 0.3)
        np.testing.assert_allclose(p["mean"], q["mean"], rtol=1e-8, atol=1e-11)
        np.testing.assert_allclose(p["covariance"], q["covariance"], rtol=1e-7, atol=1e-12)
    assert a.studies == b.studies
    shared = v2.fit_population(replace_config(iop, study_effects="shared"), data)
    assert not np.allclose(shared.families["human/all"].fixed, b.fixed, rtol=1e-6)


def replace_config(config: v2.RecoveryConfig, **changes: Any) -> v2.RecoveryConfig:
    from dataclasses import replace

    return replace(config, **changes)
