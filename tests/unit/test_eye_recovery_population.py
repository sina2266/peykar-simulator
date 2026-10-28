"""Generic eye recovery population model: equivalence with IOP 1.1 and new grouping."""

from __future__ import annotations

import math
from dataclasses import dataclass

import numpy as np
import pytest

from peykar.model_learning import eye_recovery_population as generic
from peykar.model_learning import iop_population_1_1 as iop11
from peykar.validation.eye_iop_data import PressureSeries
from peykar_core.errors import InvalidRequest

DAY = 1.0 / 30.4375
IOP = generic.RecoveryConfig(
    model_id="eye.iop-population",
    model_version="1.1.0",
    quantity="intraocular pressure",
    covariate_reference=20.0,
    covariate_unit="mmHg",
)


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
        )


MONTHS = (DAY, 7 * DAY, 1.0, 3.0, 6.0, 12.0)


def _synthetic(contexts: dict[str, tuple[float, float, float, float]], seed: int) -> list[Series]:
    """Two studies per context, two group means and one single eye per study."""
    rng = np.random.default_rng(seed)
    out = []
    for c_index, (context, (level, excess, transient, tau)) in enumerate(sorted(contexts.items())):
        for s in range(2):
            study = f"st-{c_index}-{s}"
            shift = rng.normal(0, 0.05, 3)
            for k, n in enumerate((30, 60, None)):
                reference = float(rng.uniform(15, 35))
                x = math.log(reference / 20.0)
                base = generic.law_fraction(
                    IOP,
                    MONTHS,
                    level=level + shift[0] - 0.5 * x,
                    excess=excess + shift[1],
                    transient=transient + shift[2],
                    tau_months=tau,
                )
                sd = 0.05 if n is None else 0.03
                values = base * np.exp(rng.normal(0, sd, len(MONTHS)))
                out.append(
                    Series(
                        series_id=f"{study}/{k}",
                        study_id=study,
                        context=context,
                        reference_value=reference,
                        months=MONTHS,
                        fractions=tuple(float(v) for v in values),
                        aggregate_n=n,
                    )
                )
    return out


def _pressure(item: Series) -> PressureSeries:
    return PressureSeries(
        series_id=item.series_id,
        study_id=item.study_id,
        context=item.context,
        reference_mmhg=item.reference_value,
        months=item.months,
        fractions=item.fractions,
        aggregate_n=item.aggregate_n,
        role=item.role,
    )


IOP_CONTEXTS = {
    "human/trabeculectomy": (-0.45, -0.3, 0.1, 3.0),
    "human/tube": (-0.35, -0.2, 0.2, 6.0),
    "human/angle_mims": (-0.2, -0.1, 0.25, 1.0),
}


def test_iop_configuration_reproduces_model_1_1() -> None:
    data = _synthetic(IOP_CONTEXTS, seed=3)
    ours = generic.fit_population(IOP, data)
    theirs = iop11.fit_population([_pressure(s) for s in data])
    a, b = ours.families["human/all"], theirs.families["human/all"]
    np.testing.assert_allclose(a.fixed, b.fixed, rtol=1e-9, atol=1e-12)
    np.testing.assert_allclose(a.fixed_covariance, b.fixed_covariance, rtol=1e-9, atol=1e-12)
    for level in iop11.LEVELS:
        np.testing.assert_allclose(a.variances[level], b.variances[level], rtol=1e-9, atol=1e-14)
    assert a.objective == pytest.approx(b.objective, rel=1e-12)
    assert ours.noise_centers.exponent == pytest.approx(theirs.noise_centers.exponent)
    assert dict(ours.noise_centers.centers) == pytest.approx(dict(theirs.noise_centers.centers))
    for context in (*IOP_CONTEXTS, "human/cyclodestruction"):
        assert ours.level_used(context) == theirs.level_used(context)
        for kind in ("aggregate", "individual"):
            p, q = ours.prior(context, kind, 0.3), theirs.prior(context, kind, 0.3)
            np.testing.assert_allclose(p["mean"], q["mean"], rtol=1e-9, atol=1e-12)
            np.testing.assert_allclose(p["covariance"], q["covariance"], rtol=1e-9, atol=1e-12)
    item = data[0]
    prior = ours.prior(item.context, item.kind, generic.level_covariate(IOP, item))
    prefix = item.prefix(1.0)
    for make_ours, make_theirs in (
        (generic.gaussian, iop11.gaussian),
        (lambda m, c: generic.student(m, c, 4.0), lambda m, c: iop11.student(m, c, 4.0)),
        (
            lambda m, c: generic.robust(IOP, generic.gaussian(m, c)),
            lambda m, c: iop11.robust(iop11.gaussian(m, c)),
        ),
        (
            lambda m, c: generic.anchored(generic.gaussian(m, c)),
            lambda m, c: iop11.anchored(iop11.gaussian(m, c)),
        ),
    ):
        mine = generic.predictive(
            IOP,
            generic.posterior(
                IOP,
                make_ours(prior["mean"], prior["covariance"]),
                series=prefix,
                noise=ours.noise(item.kind, item.context, item.aggregate_n),
            ),
            months=[3.0, 6.0, 12.0],
        )
        reference = iop11.predictive(
            iop11.posterior(
                make_theirs(prior["mean"], prior["covariance"]),
                series=_pressure(prefix),
                noise=theirs.noise(item.kind, item.context, item.aggregate_n),
            ),
            months=[3.0, 6.0, 12.0],
        )
        np.testing.assert_allclose(mine["point_fraction"], reference["point_fraction"], rtol=1e-9)
        for q, values in reference["quantiles"].items():
            np.testing.assert_allclose(mine["quantiles"][q], values, rtol=1e-9)
    weak = generic.posterior(IOP, generic.weak_prior(IOP), series=prefix, noise=0.05)
    weak_ref = iop11.posterior(iop11.weak_prior(), series=_pressure(prefix), noise=0.05)
    np.testing.assert_allclose(generic.moments(weak)[0], iop11.moments(weak_ref)[0], rtol=1e-9)


RETINA = generic.RecoveryConfig(
    model_id="eye.retina-thickness-population",
    model_version="test",
    quantity="central retinal thickness",
    covariate_reference=300.0,
    covariate_unit="um",
    transient_months=14 * DAY,
    grouping=("treatment", "context"),
)


def test_treatment_grouping_shares_kinetics_across_indications() -> None:
    contexts = {
        "human/dme/anti_vegf": (-0.25, 0.05, -0.1, 2.0),
        "human/rvo/anti_vegf": (-0.35, 0.05, -0.1, 2.0),
        "human/dme/steroid_implant": (-0.05, -0.3, 0.2, 1.5),
    }
    data = [
        Series(
            s.series_id,
            s.study_id,
            s.context,
            s.reference_value * 15.0,
            s.months,
            s.fractions,
            s.aggregate_n,
        )
        for s in _synthetic(contexts, seed=5)
    ]
    fit = generic.fit_population(RETINA, data)
    family = fit.families["human/all"]
    assert set(family.effect_index) == {
        ("treatment", "anti_vegf"),
        ("treatment", "steroid_implant"),
        *(("context", c) for c in contexts),
    }
    assert fit.level_used("human/dme/anti_vegf") == "human/dme/anti_vegf"
    assert fit.level_used("human/uveitic/steroid_implant") == "treatment:steroid_implant"
    assert fit.level_used("human/csc/laser") == "family:human/all"
    _, seen_treatment = fit.location("human/uveitic/steroid_implant", 0.0)
    _, unseen = fit.location("human/csc/laser", 0.0)
    assert np.all(np.diag(seen_treatment) < np.diag(unseen))
    described = fit.describe()["families"]["human/all"]
    assert set(described["groups"]) == {"treatment", "context"}
    assert described["level_sd"].keys() == {
        "treatment",
        "context",
        "study",
        "group_mean_series",
        "single_eye_series",
    }
    item = data[0]
    prior = fit.prior(item.context, item.kind, generic.level_covariate(RETINA, item))
    result = generic.predictive(
        RETINA,
        generic.posterior(
            RETINA,
            generic.gaussian(prior["mean"], prior["covariance"]),
            series=item.prefix(1.0),
            noise=fit.noise(item.kind, item.context, item.aggregate_n),
        ),
        months=[3.0, 12.0],
    )
    assert all(0.2 < v < 2.0 for v in result["point_fraction"])
    assert set(result["posterior_mean"]) == set(RETINA.parameters)


def test_blocks_join_studies_that_span_groups() -> None:
    summaries = [
        {
            "context": "human/dme/anti_vegf",
            "study_id": "a",
            "mean": np.zeros(4),
            "covariate": 0.0,
            "covariance": np.eye(4),
            "kind": "aggregate",
        },
        {
            "context": "human/dme/laser",
            "study_id": "a",
            "mean": np.zeros(4),
            "covariate": 0.0,
            "covariance": np.eye(4),
            "kind": "aggregate",
        },
        {
            "context": "human/rvo/laser",
            "study_id": "b",
            "mean": np.zeros(4),
            "covariate": 0.0,
            "covariance": np.eye(4),
            "kind": "individual",
        },
        {
            "context": "human/csc/pdt",
            "study_id": "c",
            "mean": np.zeros(4),
            "covariate": 0.0,
            "covariance": np.eye(4),
            "kind": "aggregate",
        },
    ]
    blocks = generic._blocks(RETINA, summaries)
    assert [b.members for b in blocks] == [(0, 1, 2), (3,)]
    same_treatment, same_context = blocks[0].same_group
    assert same_treatment[1, 2] == 1.0 and same_context[1, 2] == 0.0
    assert blocks[0].same_study[0, 1] == 1.0 and blocks[0].same_study[0, 2] == 0.0


def test_persistent_deviations_match_the_exact_joint_gaussian() -> None:
    config = generic.RecoveryConfig(
        model_id="m",
        model_version="v",
        quantity="q",
        covariate_reference=300.0,
        covariate_unit="um",
        transient_months=14 * DAY,
        deviation_fraction=0.6,
        deviation_months=4.0,
    )
    log_tau = float(config.log_tau_grid[40])
    mean = np.array([-0.2, 0.1, 0.05, log_tau])
    covariance = np.diag([0.04, 0.02, 0.03, 1e-10])
    prior = generic.gaussian(mean, covariance)
    item = Series("s/1", "s", "human/dme/anti_vegf", 450.0, (1.0, 3.0, 6.0), (0.72, 0.70, 0.78), 30)
    sigma = 0.05
    fitted = generic.posterior(config, prior, series=item, noise=sigma)
    assert fitted.weights.size == 1
    targets = [9.0, 12.0]
    result = generic.predictive(config, fitted, months=targets, quantiles=(0.1, 0.5, 0.9))
    # Exact conditional of the target log fractions given the prefix.
    x_obs = generic.design(config, item.months, np.array([log_tau]))[0]
    x_new = generic.design(config, targets, np.array([log_tau]))[0]
    m0, c0 = mean[:3], covariance[:3, :3]
    z = np.log(np.asarray(item.fractions))
    cov_zz = x_obs @ c0 @ x_obs.T + sigma**2 * generic.residual_correlation(config, item.months)
    cross = x_new @ c0 @ x_obs.T + sigma**2 * 0.6 * generic.deviation_correlation(
        config, targets, item.months
    )
    gain = np.linalg.solve(cov_zz, cross.T)
    expected_mean = x_new @ m0 + gain.T @ (z - x_obs @ m0)
    expected_sd = np.sqrt(np.diag(x_new @ c0 @ x_new.T + sigma**2 - cross @ gain))
    np.testing.assert_allclose(np.log(result["point_fraction"]), expected_mean, rtol=1e-6)
    np.testing.assert_allclose(
        np.log(result["quantiles"]["0.9"]), expected_mean + 1.2815516 * expected_sd, rtol=1e-5
    )
    # Without deviations the same prior and data give a different (independent) forecast,
    # and the prior-only forecast does not depend on the deviation fraction.
    plain = generic.RecoveryConfig(
        model_id="m",
        model_version="v",
        quantity="q",
        covariate_reference=300.0,
        covariate_unit="um",
        transient_months=14 * DAY,
    )
    independent = generic.predictive(
        plain, generic.posterior(plain, prior, series=item, noise=sigma), months=targets
    )
    assert not np.allclose(independent["point_fraction"], result["point_fraction"])
    alone = generic.predictive(
        config, generic.posterior(config, prior, series=None, noise=sigma), months=targets
    )
    alone_plain = generic.predictive(
        plain, generic.posterior(plain, prior, series=None, noise=sigma), months=targets
    )
    np.testing.assert_allclose(alone["quantiles"]["0.9"], alone_plain["quantiles"]["0.9"])


def test_working_independence_fit_keeps_the_population_of_independent_residuals() -> None:
    data = _synthetic(IOP_CONTEXTS, seed=7)
    forecast_only = generic.RecoveryConfig(
        model_id="m",
        model_version="v",
        quantity="q",
        covariate_reference=20.0,
        covariate_unit="mmHg",
        deviation_fraction=0.8,
        deviation_months=12.0,
        deviation_in_population_fit=False,
    )
    a = generic.fit_population(forecast_only, data).families["human/all"]
    b = generic.fit_population(IOP, data).families["human/all"]
    np.testing.assert_allclose(a.fixed, b.fixed)
    np.testing.assert_allclose(a.covariance, b.covariance)


def test_law_and_configuration_checks() -> None:
    value = generic.law_fraction(
        RETINA, [RETINA.reference_months], level=-0.2, excess=0.1, transient=0.05, tau_months=2.0
    )
    assert value[0] == pytest.approx(math.exp(-0.05))
    plain = generic.RecoveryConfig(
        model_id="m",
        model_version="v",
        quantity="q",
        covariate_reference=1.0,
        covariate_unit="u",
        transient_months=None,
        penalty_modes=(0.1, 0.1, 0.5),
        robust_sd=(0.3, 0.3, 2.0),
    )
    assert plain.parameters == ("level", "excess_at_t1", "log_tau_months")
    assert generic.design(plain, [1.0, 2.0], plain.log_tau_grid).shape == (80, 2, 2)
    with pytest.raises(InvalidRequest):
        generic.transient_ratio(plain, [1.0])
    with pytest.raises(InvalidRequest):
        generic.RecoveryConfig(
            model_id="m",
            model_version="v",
            quantity="q",
            covariate_reference=1.0,
            covariate_unit="u",
            grouping=("indication",),
        )
    with pytest.raises(InvalidRequest):
        generic.RecoveryConfig(
            model_id="m",
            model_version="v",
            quantity="q",
            covariate_reference=1.0,
            covariate_unit="u",
            penalty_modes=(0.1, 0.5),
        )
    with pytest.raises(InvalidRequest):
        generic.family_of("human")
    assert generic.treatment_of("human/dme/anti_vegf") == "anti_vegf"
