"""Population model of an eye measurement after treatment, generic version 5.

Generic model 2 (``eye_recovery_population2``, unchanged because frozen protocols pin
it) with one option for the start of the kinetic law, ``start``:

- ``"free"`` (versions 1-4): ``ln f(t) = ell + e r(t) [+ s q(t)]`` with
  ``r(t) = exp(-(t - t1) / tau)``. The value at the reference time ``t1`` (one day),
  ``ell + e [+ s]``, is a free parameter of every series and of the population.
- ``"zero_level"``: the measurement is unchanged at ``t1``, ``ln f(t) = ell (1 - r(t))``,
  with parameters ``(level, ln tau)``: the long-term change and its time constant.
- ``"zero_rate"``: the same law written with its initial rate,
  ``ln f(t) = v (tau / R) (1 - r(t))``, parameters ``(initial_rate, ln tau)``: ``v`` is the
  change per ``R = rate_months`` months at ``t1`` and ``v tau / R`` the long-term change.
  The population hierarchy, the level covariate and the anchored variants then act on
  the rate, which series of a year or two identify well, rather than on a long-term
  change that only long series identify.

Both zero starts are meant for a quantity recorded as its change from a baseline at
which nothing has happened yet, such as the axial length of a growing eye. With a free
start and no visit before month 1, the value at ``t1`` is read from the curvature of
later visits: in the myopia model 1.0 development the population law gave a change of
-0.66 mm at day 1, a shortening no eye shows.

Every law is multiplied by model 2's wear-off factor. Given ``ln tau`` a zero-start law
is linear in its one remaining parameter, so the exact mixture posterior, the
likelihood summaries, the noise estimates, the REML population fit and the predictive
distribution keep model 2's form with a one-column design (``1 - r(t)`` or
``(tau / R)(1 - r(t))``). The functions below are model 2's with this design; with
``start = "free"`` they give model 2's results exactly (unit-tested). A zero start has
no early transient. Nothing here reads files, registers models or claims clinical
validity.
"""

from __future__ import annotations

import math
from collections.abc import Sequence
from dataclasses import dataclass, replace
from typing import Any

import numpy as np

from peykar.model_learning import eye_recovery_population as base
from peykar.model_learning import eye_recovery_population2 as version2
from peykar.model_learning.corneal_population import NoiseModel
from peykar.model_learning.eye_recovery_population import (
    DAY_MONTHS,
    MAX_SERIES,
    PRUNE_WEIGHT,
    FloatArray,
    NoiseEstimate,
    NoiseKey,
    Prior,
    RecoverySeries,
    anchored,
    deviation_correlation,
    family_of,
    fit_family,
    gaussian,
    group_key,
    level_covariate,
    moments,
    residual_correlation,
    robust,
    student,
    weak_prior,
    weak_prior_moments,
)
from peykar.model_learning.eye_recovery_population2 import (
    PooledNoiseEstimate,
    Posterior,
    effect_duration,
    noise_scale,
    pooled_noise_centers,
    wear_off_factor,
)
from peykar_core.errors import InvalidRequest

__all__ = [
    "GENERIC_VERSION",
    "STARTS",
    "Posterior",
    "Prior",
    "RecoveryConfig",
    "RecoveryFit",
    "anchored",
    "best_fit_sum_squares",
    "design",
    "effect_duration",
    "fit_population",
    "gaussian",
    "law_fraction",
    "level_covariate",
    "posterior",
    "predictive",
    "robust",
    "series_summary",
    "student",
    "study_noise",
    "weak_prior",
]

GENERIC_VERSION = "5.0.0"
STARTS = ("free", "zero_level", "zero_rate")
LAWS = {
    "free": "ln f(t) = ell + e r(t) [+ s q(t)], r(t) = exp(-(t - t1)/tau)",
    "zero_level": "ln f(t) = ell (1 - r(t)), unchanged at t1",
    "zero_rate": "ln f(t) = v (tau / R) (1 - r(t)), unchanged at t1, initial rate v per R months",
}


@dataclass(frozen=True)
class RecoveryConfig(version2.RecoveryConfig):
    """Version 2's configuration plus the start of the law (``start``) and, for
    ``"zero_rate"``, the period ``rate_months`` of its rate."""

    start: str = "free"
    rate_months: float = 12.0

    def __post_init__(self) -> None:
        if self.start not in STARTS:
            raise InvalidRequest(f"start must be one of {list(STARTS)}")
        if not (math.isfinite(self.rate_months) and self.rate_months > 0):
            raise InvalidRequest("rate_months must be finite and positive")
        if self.zero_start and self.transient_months is not None:
            raise InvalidRequest("A zero start needs transient_months = None")
        super().__post_init__()

    @property
    def zero_start(self) -> bool:
        return self.start != "free"

    @property
    def linear(self) -> int:
        if self.zero_start:
            return 1
        return super().linear

    @property
    def parameters(self) -> tuple[str, ...]:
        if self.start == "zero_level":
            return ("level", "log_tau_months")
        if self.start == "zero_rate":
            return ("initial_rate", "log_tau_months")
        return super().parameters

    def describe(self) -> dict[str, Any]:
        return {
            **super().describe(),
            "generic_model_version": GENERIC_VERSION,
            "start": self.start,
            "law": LAWS[self.start],
            "rate_months": self.rate_months if self.start == "zero_rate" else None,
        }


def _start(config: base.RecoveryConfig) -> str:
    return config.start if isinstance(config, RecoveryConfig) else "free"


# --- kinetic law ---------------------------------------------------------------------


def design(
    config: version2.RecoveryConfig,
    months: Sequence[float] | FloatArray,
    log_tau: FloatArray,
    duration: float | None = None,
) -> FloatArray:
    """Model 2's columns for a free start; for a zero start the one column
    ``1 - r(t)`` (``zero_level``) or ``(tau / R)(1 - r(t))`` (``zero_rate``), times the
    wear-off factor (shape: tau, months, linear)."""
    start = _start(config)
    if start == "free":
        free: FloatArray = version2.design(config, months, log_tau, duration)
        return free
    rise: FloatArray = 1.0 - base.recovery_ratio(config, months, log_tau)
    if start == "zero_rate":
        assert isinstance(config, RecoveryConfig)
        tau = np.exp(np.asarray(log_tau, dtype=np.float64))[:, None]
        rise = rise * tau / config.rate_months
    result: FloatArray = rise[:, :, None] * wear_off_factor(config, months, duration)[None, :, None]
    return result


def law_fraction(
    config: version2.RecoveryConfig,
    months: Sequence[float] | FloatArray,
    *,
    parameters: Sequence[float] | FloatArray,
    duration: float | None = None,
) -> FloatArray:
    """The law for one parameter vector ``theta`` (its linear part, then ``ln tau``)."""
    theta = np.asarray(parameters, dtype=np.float64)
    if theta.size != config.dimensions:
        raise InvalidRequest("Parameter vector does not match the configuration")
    x = design(config, months, theta[config.linear :], duration)[0]
    result: FloatArray = np.exp(x @ theta[: config.linear])
    return result


# --- exact mixture posteriors ----------------------------------------------------------


def posterior(
    config: version2.RecoveryConfig,
    prior: Prior,
    *,
    series: RecoverySeries | None,
    noise: NoiseModel | float,
    duration: float | None = None,
) -> Posterior:
    """Model 2's posterior (prior times the likelihood of the prefix ``series``; ``None``
    = prior only) with this version's design."""
    linear = config.linear
    grid = config.log_tau_grid
    if any(len(m) != config.dimensions for m in prior.means):
        raise InvalidRequest("Prior dimension does not match the configuration")
    sigmas, log_noise = base._noise_levels(noise, config.noise_floor)
    inverse_variance = 1.0 / sigmas**2
    observed: tuple[FloatArray, FloatArray] | None = None
    if series is not None:
        own = effect_duration(series)
        if duration is not None and duration != own:
            raise InvalidRequest("The given duration differs from the series' own duration")
        duration = own
    log_det_whitening = 0.0
    if series is None:
        if prior.flat_level:
            raise InvalidRequest("An anchored prior needs at least one observation")
        n = 0
        xtx = np.zeros((grid.size, linear, linear))
        xtz = np.zeros((grid.size, linear))
        zz = 0.0
    else:
        z = np.log(np.asarray(series.fractions, dtype=np.float64))
        x = design(config, series.months, grid, duration)
        n = z.size
        if config.deviation_fraction > 0:
            observed = (np.asarray(series.months, dtype=np.float64), z.copy())
        factor = version2._whitening(config.likelihood_config, series.months)
        if factor is not None:
            log_det_whitening = 2.0 * float(np.sum(np.log(np.diag(factor))))
            x = np.linalg.solve(factor[None], x)
            z = np.linalg.solve(factor, z)
        xtx = np.einsum("kna,knb->kab", x, x)
        xtz = np.einsum("kna,n->ka", x, z)
        zz = float(z @ z)
    blocks = []
    for weight, mean, covariance in zip(prior.weights, prior.means, prior.covariances, strict=True):
        mean0, precision0, log_det_prior, log_pi = base._conditional_prior(
            config, np.asarray(mean), np.asarray(covariance), prior.flat_level
        )
        b_prior = mean0 @ precision0.T
        quad_prior = np.einsum("ka,ab,kb->k", mean0, precision0, mean0)
        precision = precision0[None, None] + xtx[:, None] * inverse_variance[None, :, None, None]
        rhs = b_prior[:, None, :] + xtz[:, None, :] * inverse_variance[None, :, None]
        sign, log_det = np.linalg.slogdet(precision)
        if np.any(sign <= 0):
            raise InvalidRequest("Posterior precision is not positive definite")
        covariances = np.linalg.inv(precision)
        means = np.einsum("ksab,ksb->ksa", covariances, rhs)
        quad_post = np.einsum("ksa,ksa->ks", rhs, means)
        log_marginal = (
            -n * np.log(sigmas)[None, :]
            - 0.5 * log_det_prior
            - 0.5 * log_det
            - 0.5 * (zz * inverse_variance[None, :] + quad_prior[:, None] - quad_post)
            - 0.5 * log_det_whitening
        )
        log_w = math.log(weight) + log_pi[:, None] + log_noise[None, :] + log_marginal
        blocks.append(
            (
                log_w.ravel(),
                means.reshape(-1, linear),
                covariances.reshape(-1, linear, linear),
                np.repeat(grid, sigmas.size),
                np.tile(sigmas, grid.size),
            )
        )
    weights = base._log_normalize(np.concatenate([b[0] for b in blocks]))
    keep = weights >= PRUNE_WEIGHT * float(weights.max())
    kept = weights[keep]
    return Posterior(
        weights=kept / kept.sum(),
        means=np.concatenate([b[1] for b in blocks])[keep],
        covariances=np.concatenate([b[2] for b in blocks])[keep],
        log_tau=np.concatenate([b[3] for b in blocks])[keep],
        sigmas=np.concatenate([b[4] for b in blocks])[keep],
        observed_months=None if observed is None else observed[0],
        observed_logs=None if observed is None else observed[1],
        duration=duration,
    )


def predictive(
    config: version2.RecoveryConfig,
    fitted: Posterior,
    *,
    months: Sequence[float],
    quantiles: Sequence[float] = (0.05, 0.1, 0.5, 0.9, 0.95),
    extra_noise: float = 0.0,
) -> dict[str, Any]:
    """Model 2's predictive distribution (median point forecast, posterior mean curve and
    observation quantiles of the exact mixture) with this version's design."""
    if not months or any(not math.isfinite(m) or m <= 0 for m in months):
        raise InvalidRequest("Prediction months must be finite and positive")
    if not (math.isfinite(extra_noise) and extra_noise >= 0):
        raise InvalidRequest("Extra forecast noise must be finite and non-negative")
    duration = fitted.duration
    x = design(config, months, fitted.log_tau, duration)
    likelihood = config.likelihood_config
    target_scale = noise_scale(likelihood, months) ** 2
    extra = noise_scale(config, months) ** 2 - target_scale + extra_noise
    w = fitted.weights
    if fitted.observed_months is None or fitted.observed_logs is None:
        mean_log = np.einsum("kta,ka->kt", x, fitted.means)
        variance = np.einsum("kta,kab,ktb->kt", x, fitted.covariances, x)
        mean_fraction = w @ np.exp(mean_log + 0.5 * variance)
        spread = np.sqrt(variance + fitted.sigmas[:, None] ** 2 * (target_scale + extra)[None, :])
    else:
        phi = config.deviation_fraction
        cross = deviation_correlation(config, fitted.observed_months, months)
        plain_gain = phi * np.linalg.solve(
            residual_correlation(config, fitted.observed_months), cross
        )
        scale_obs = noise_scale(likelihood, fitted.observed_months)
        gain = plain_gain * np.sqrt(target_scale)[None, :] / scale_obs[:, None]
        observed_design = design(config, fitted.observed_months, fitted.log_tau, duration)
        a = x - np.einsum("kna,nt->kta", observed_design, gain)
        mean_log = np.einsum("kta,ka->kt", a, fitted.means) + (fitted.observed_logs @ gain)
        variance = np.einsum("kta,kab,ktb->kt", a, fitted.covariances, a)
        explained = target_scale * phi * np.einsum("nt,nt->t", cross, plain_gain)
        latent = variance + fitted.sigmas[:, None] ** 2 * (phi * target_scale - explained)[None, :]
        mean_fraction = w @ np.exp(mean_log + 0.5 * latent)
        spread = np.sqrt(
            variance + fitted.sigmas[:, None] ** 2 * (target_scale + extra - explained)[None, :]
        )
    low = np.min(mean_log - 12.0 * spread, axis=0)
    high = np.max(mean_log + 12.0 * spread, axis=0)
    result_quantiles: dict[str, list[float]] = {}
    for q in quantiles:
        lo, hi = low.copy(), high.copy()
        for _ in range(60):
            mid = 0.5 * (lo + hi)
            below = w @ base._normal_cdf((mid[None, :] - mean_log) / spread) < q
            lo = np.where(below, mid, lo)
            hi = np.where(below, hi, mid)
        result_quantiles[f"{q:g}"] = [float(v) for v in np.exp(0.5 * (lo + hi))]
    mean, _ = moments(fitted)
    point = result_quantiles.get("0.5")
    if point is None:
        raise InvalidRequest("Predictive quantiles must include the median (0.5)")
    return {
        "months": [float(m) for m in months],
        "point_fraction": point,
        "mean_fraction": [float(v) for v in mean_fraction],
        "quantiles": result_quantiles,
        "posterior_mean": dict(zip(config.parameters, (float(v) for v in mean), strict=True)),
    }


# --- noise and per-series likelihood summaries -----------------------------------------


def best_fit_sum_squares(config: version2.RecoveryConfig, series: RecoverySeries) -> float:
    """Model 2's weighted least-squares residual sum of squares of ``ln f``, best over
    the ``tau`` grid, with this version's design."""
    scale = noise_scale(config, series.months)
    y = np.log(np.asarray(series.fractions, dtype=np.float64)) / scale
    best = math.inf
    for x in design(config, series.months, config.log_tau_grid, effect_duration(series)):
        weighted = x / scale[:, None]
        beta, *_ = np.linalg.lstsq(weighted, y, rcond=None)
        residual = y - weighted @ beta
        best = min(best, float(residual @ residual))
    return best


def study_noise(
    config: version2.RecoveryConfig, series: Sequence[RecoverySeries]
) -> dict[NoiseKey, NoiseEstimate]:
    """Model 2's per-(study, family, kind) residual SD with this version's law; the
    degrees of freedom count the law's parameters (two with a zero start)."""
    groups: dict[NoiseKey, list[RecoverySeries]] = {}
    for item in series:
        groups.setdefault((item.study_id, family_of(item.context), item.kind), []).append(item)
    estimates: dict[NoiseKey, NoiseEstimate] = {}
    for key, members in groups.items():
        usable = [m for m in members if len(m.months) >= config.min_noise_follow_ups]
        if not usable:
            continue
        sse = sum(best_fit_sum_squares(config, m) for m in usable)
        dof = sum(len(m.months) - config.dimensions for m in usable)
        units = [float(m.aggregate_n) for m in usable if m.aggregate_n]
        estimates[key] = PooledNoiseEstimate(
            sd=max(config.noise_floor, math.sqrt(sse / dof)),
            units=float(np.mean(units)) if units else None,
            dof=dof,
        )
    return estimates


def series_summary(
    config: version2.RecoveryConfig, series: RecoverySeries, sigma: float
) -> dict[str, Any]:
    """Model 2's Gaussian likelihood summary of ``theta`` with this version's law."""
    post_mean, post_cov = moments(posterior(config, weak_prior(config), series=series, noise=sigma))
    prior_mean, prior_cov = weak_prior_moments(config)
    scale = np.sqrt(np.diag(prior_cov))
    standard = np.outer(scale, scale)
    post_precision = np.linalg.inv(post_cov / standard)
    prior_precision = np.linalg.inv(prior_cov / standard)
    information = post_precision - prior_precision
    values, vectors = np.linalg.eigh(0.5 * (information + information.T))
    eta = post_precision @ (post_mean / scale) - prior_precision @ (prior_mean / scale)
    informed = values >= config.information_floor
    safe = np.where(informed, values, 1.0)
    components = np.where(informed, (vectors.T @ eta) / safe, vectors.T @ (post_mean / scale))
    values = np.where(informed, values, config.information_floor)
    covariance_std = vectors @ np.diag(1.0 / values) @ vectors.T
    return {
        "series_id": series.series_id,
        "study_id": series.study_id,
        "context": series.context,
        "kind": series.kind,
        "covariate": level_covariate(config, series),
        "mean": (vectors @ components) * scale,
        "covariance": covariance_std * standard + np.diag(config.summary_jitter),
    }


# --- fitted population ------------------------------------------------------------------


@dataclass(frozen=True)
class RecoveryFit(version2.RecoveryFit):
    """Model 2's fitted population; with a zero start its description gives the
    start's parameters and evaluates this version's law."""

    def describe(self) -> dict[str, Any]:
        config = self.config
        if not isinstance(config, RecoveryConfig) or not config.zero_start:
            free: dict[str, Any] = super().describe()
            return free
        d = config.dimensions

        def text(mean: FloatArray, covariance: FloatArray) -> dict[str, Any]:
            sd = np.sqrt(np.diag(covariance))
            curve = law_fraction(config, config.described_months, parameters=mean)
            tau = math.exp(float(mean[config.linear]))
            long_term = float(mean[0])
            if config.start == "zero_rate":
                long_term *= tau / config.rate_months
            return {
                "long_term_fraction_at_reference": math.exp(long_term),
                "parameters": dict(zip(config.parameters, mean.tolist(), strict=True)),
                "median_tau_days": tau / DAY_MONTHS,
                "estimate_sd": dict(zip(config.parameters, sd.tolist(), strict=True)),
                "fraction_at_mean_parameters": {
                    f"{m:.4g}": float(v)
                    for m, v in zip(config.described_months, curve, strict=True)
                },
            }

        families = {}
        for name, fitted in self.families.items():
            fixed_cov = fitted.fixed_covariance
            groups: dict[str, Any] = {}
            for level, key in sorted(fitted.effect_index):
                groups.setdefault(level, {})[key] = text(*self.group_location(name, level, key))
            families[name] = {
                "mean": text(fitted.fixed[:d], fixed_cov[:d, :d]),
                "first_parameter_slope_on_covariate": float(fitted.fixed[d]),
                "first_parameter_slope_sd": float(math.sqrt(fixed_cov[d, d])),
                "groups": groups,
                "level_sd": {
                    level: dict(
                        zip(config.parameters, np.sqrt(np.diag(matrix)).tolist(), strict=True)
                    )
                    for level, matrix in fitted.variances.items()
                },
                "training_studies": len(fitted.studies),
                "training_series": fitted.series,
                "penalized_reml_objective": fitted.objective,
            }
        return {
            **config.describe(),
            "families": families,
            "unfitted_families": dict(self.unfitted_families),
            "noise_center_sd_log_fraction_at_reference_n": {
                f"{family}/{kind}": value
                for (family, kind), value in self.noise_centers.centers.items()
            },
            "group_mean_noise_exponent": self.noise_centers.exponent,
            "study_noise_sd_log_fraction": dict(self.study_noise_sd),
            "training_series": len(self.training_series),
            "training_studies": len(self.training_studies),
        }


def fit_population(
    config: version2.RecoveryConfig,
    series: Sequence[RecoverySeries],
    *,
    noise_series: Sequence[RecoverySeries] | None = None,
) -> RecoveryFit:
    """Model 2's population fit (one hierarchy per species, control series only) with
    this version's law."""
    if not isinstance(config, version2.RecoveryConfig):
        raise InvalidRequest("Version-5 fitting needs a version-2 or version-5 configuration")
    if not 2 <= len(series) <= MAX_SERIES:
        raise InvalidRequest("Population fitting needs 2..5000 series")
    if len({s.series_id for s in series}) != len(series):
        raise InvalidRequest("Population fitting requires unique series identities")
    if any(s.role != "control" for s in series):
        raise InvalidRequest("Population priors are fitted to control series only")
    summary_config = config.likelihood_config
    if not config.deviation_in_population_fit:
        summary_config = replace(summary_config, deviation_fraction=0.0)
    estimates = study_noise(summary_config, noise_series if noise_series is not None else series)
    centers = pooled_noise_centers(config, estimates)
    forecast_centers = None
    pooling = config.forecast_noise_pooling
    if pooling is not None and pooling != config.noise_pooling:
        forecast_centers = pooled_noise_centers(replace(config, noise_pooling=pooling), estimates)
    summaries = []
    for item in series:
        if item.reference_value <= 0:
            continue
        family = family_of(item.context)
        own = estimates.get((item.study_id, family, item.kind))
        sigma = own.sd if own is not None else centers.center(family, item.kind, item.aggregate_n)
        summaries.append({**series_summary(summary_config, item, sigma), "family": family})
    fitted: dict[str, base.FamilyFit] = {}
    unfitted: dict[str, str] = {}
    for family in sorted({m["family"] for m in summaries}):
        members = [m for m in summaries if m["family"] == family]
        studies = tuple(sorted({m["study_id"] for m in members}))
        if len(studies) < 2:
            unfitted[family] = "fewer than two training studies"
            continue
        if config.study_effects == "per_group":
            top = config.grouping[0]
            members = [
                {**m, "study_id": f"{m['study_id']}@{group_key(top, m['context'])}"}
                for m in members
            ]
        fitted[family] = replace(fit_family(config, family, members), studies=studies)
    if not fitted:
        raise InvalidRequest("No family has two training studies")
    return RecoveryFit(
        config=config,
        families=fitted,
        noise_centers=centers,
        study_noise_sd={f"{s}/{f}/{k}": v.sd for (s, f, k), v in estimates.items()},
        training_series=tuple(s.series_id for s in series),
        training_studies=tuple(sorted({s.study_id for s in series})),
        unfitted_families=unfitted,
        forecast_noise_centers=forecast_centers,
    )
