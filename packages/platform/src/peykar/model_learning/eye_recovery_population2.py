"""Population model of an eye measurement after treatment, generic version 2.

Generic model 1 (``eye_recovery_population``, unchanged because retinal-thickness
protocol 1.0.0 pins it) with three optional extensions. Each is off by default, and
a default configuration reproduces model 1 exactly; unchanged parts are imported
from model 1 rather than copied.

- **Wear-off of a single dose.** A series whose ``metadata`` declares an effect
  duration ``D`` (``effect_duration_months``; for example one dexamethasone implant)
  follows::

      ln f(t) = L(t) * (1 - rho * W(t)),   W(t) = 1 / (1 + exp(-(t - D) / w))

  where ``L`` is model 1's law, ``w`` the declared wear-off width and ``rho``
  (``wear_off_loss``) the declared fraction of the treatment effect that is lost
  once the dose has worn off. Given ``ln tau`` the law stays linear in
  ``(ell, e[, s])``, so the exact mixture posteriors are unchanged in form; series
  without a duration, and every series when ``rho = 0``, follow model 1.
- **Treatment fallback.** With grouping ``("context",)``, a context without training
  series takes the mean location of the trained contexts of the same treatment (plus
  the between-context variance) instead of the species mean. Trained contexts are
  unchanged.
- **Early noise.** The residual SD at month ``t`` is
  ``sigma * sqrt(1 + k * exp(-(t - t1) / T))``: early observations count as noisier
  and early forecasts get wider intervals. ``k = 0`` (``early_noise=None``) is
  model 1. With persistent deviations the whole residual process is scaled. With
  ``early_noise_in_likelihood=False`` every likelihood ignores it and it only adds
  independent noise to forecasts (point forecasts then equal model 1's).

Nothing here reads files, registers models or claims clinical validity.
"""

from __future__ import annotations

import math
from collections.abc import Mapping, Sequence
from dataclasses import dataclass, replace
from typing import Any

import numpy as np

from peykar.model_learning import eye_recovery_population as base
from peykar.model_learning.corneal_population import NoiseModel
from peykar.model_learning.eye_recovery_population import (
    DAY_MONTHS,
    MAX_SERIES,
    PRUNE_WEIGHT,
    FloatArray,
    NoiseCenters,
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
    noise_centers,
    noise_exponent,
    residual_correlation,
    robust,
    student,
    treatment_of,
    weak_prior,
    weak_prior_moments,
)
from peykar_core.errors import InvalidRequest

__all__ = [
    "DAY_MONTHS",
    "Posterior",
    "Prior",
    "RecoveryConfig",
    "RecoveryFit",
    "anchored",
    "design",
    "effect_duration",
    "fit_population",
    "gaussian",
    "level_covariate",
    "noise_scale",
    "posterior",
    "predictive",
    "robust",
    "student",
    "wear_off_factor",
    "weak_prior",
]

GENERIC_VERSION = "2.0.0"
NOISE_POOLINGS = {"median", "pooled"}
STUDY_EFFECTS = {"shared", "per_group"}


@dataclass(frozen=True)
class RecoveryConfig(base.RecoveryConfig):
    """Model 1's declared constants plus the three optional extensions (none fitted).

    ``wear_off_loss`` (``rho``) 0 disables wear-off; ``early_noise`` is ``(k, T)``
    with ``T`` in months, or ``None``.
    """

    wear_off_width_months: float = 0.5
    wear_off_loss: float = 0.0
    treatment_fallback: bool = False
    early_noise: tuple[float, float] | None = None
    # False: every likelihood (population fit and per-series posteriors) ignores the
    # early noise; it only widens forecasts as extra independent observation noise.
    early_noise_in_likelihood: bool = True
    # "median": model 1's median of study residual SDs; "pooled": the pooled residual
    # variance (summed squares over summed degrees of freedom), which is unbiased when
    # each study has only a few degrees of freedom.
    noise_pooling: str = "median"
    # Pooling of the noise centre used for forecast intervals only (None: the same as
    # ``noise_pooling``). Forecasts then add independent noise so that their noise
    # matches that centre; likelihoods and posteriors keep ``noise_pooling``.
    forecast_noise_pooling: str | None = None
    # "shared" (model 1): a study's series in several top-level groups share one study
    # effect, and those groups are fitted in one REML block. "per_group" (IOP 1.1): each
    # top-level group of a study has its own study effect, and blocks stay per group.
    study_effects: str = "shared"

    def __post_init__(self) -> None:
        super().__post_init__()
        if not (math.isfinite(self.wear_off_width_months) and self.wear_off_width_months > 0):
            raise InvalidRequest("The wear-off width must be positive")
        if not 0.0 <= self.wear_off_loss <= 1.0:
            raise InvalidRequest("The wear-off loss must lie in [0, 1]")
        if self.treatment_fallback and self.grouping != ("context",):
            raise InvalidRequest("The treatment fallback needs grouping ('context',)")
        if self.noise_pooling not in NOISE_POOLINGS or self.forecast_noise_pooling not in (
            None,
            *NOISE_POOLINGS,
        ):
            raise InvalidRequest(f"Noise pooling must be one of {sorted(NOISE_POOLINGS)}")
        if self.study_effects not in STUDY_EFFECTS:
            raise InvalidRequest(f"Study effects must be one of {sorted(STUDY_EFFECTS)}")
        if self.early_noise is not None:
            k, months = self.early_noise
            if not (math.isfinite(k) and k >= 0 and math.isfinite(months) and months > 0):
                raise InvalidRequest("Early noise needs k >= 0 and a positive time")

    def describe(self) -> dict[str, Any]:
        return {
            **super().describe(),
            "generic_model_version": GENERIC_VERSION,
            "wear_off_width_months": self.wear_off_width_months,
            "wear_off_loss": self.wear_off_loss,
            "treatment_fallback": self.treatment_fallback,
            "early_noise": None if self.early_noise is None else list(self.early_noise),
            "early_noise_in_likelihood": self.early_noise_in_likelihood,
            "noise_pooling": self.noise_pooling,
            "forecast_noise_pooling": self.forecast_noise_pooling,
            "study_effects": self.study_effects,
        }

    @property
    def likelihood_config(self) -> RecoveryConfig:
        """The configuration every likelihood uses (early noise removed when it acts in
        forecasting only)."""
        if self.early_noise is None or self.early_noise_in_likelihood:
            return self
        return replace(self, early_noise=None)


# --- series attributes, wear-off and noise scale --------------------------------------


def effect_duration(series: object) -> float | None:
    """The series' declared single-dose effect duration in months, or ``None``."""
    metadata = getattr(series, "metadata", None) or {}
    value = metadata.get("effect_duration_months") if isinstance(metadata, Mapping) else None
    if value is None:
        return None
    duration = float(value)
    if not (math.isfinite(duration) and duration > 0):
        raise InvalidRequest("An effect duration must be finite and positive")
    return duration


def wear_off_factor(
    config: RecoveryConfig, months: Sequence[float] | FloatArray, duration: float | None
) -> FloatArray:
    """``1 - rho * W(t)``; ones without a duration or with ``rho = 0``."""
    t = np.asarray(months, dtype=np.float64)
    if duration is None or config.wear_off_loss == 0.0:
        return np.ones_like(t)
    worn: FloatArray = 1.0 / (1.0 + np.exp(-(t - duration) / config.wear_off_width_months))
    result: FloatArray = 1.0 - config.wear_off_loss * worn
    return result


def noise_scale(config: RecoveryConfig, months: Sequence[float] | FloatArray) -> FloatArray:
    """Residual SD multiplier ``sqrt(1 + k exp(-(t - t1) / T))``; ones without early noise."""
    t = np.asarray(months, dtype=np.float64)
    if config.early_noise is None:
        return np.ones_like(t)
    k, months_scale = config.early_noise
    result: FloatArray = np.sqrt(1.0 + k * np.exp(-(t - config.reference_months) / months_scale))
    return result


def design(
    config: RecoveryConfig,
    months: Sequence[float] | FloatArray,
    log_tau: FloatArray,
    duration: float | None = None,
) -> FloatArray:
    """Model 1's columns times the wear-off factor (shape: tau, months, linear)."""
    x = base.design(config, months, log_tau)
    result: FloatArray = x * wear_off_factor(config, months, duration)[None, :, None]
    return result


def _whitening(config: RecoveryConfig, months: Sequence[float]) -> FloatArray | None:
    """Lower Cholesky factor of the residual covariance in noise-variance units:
    ``S chol((1 - phi) I + phi R)`` with ``S`` the early-noise scale; ``None`` when it
    is the identity."""
    scale = noise_scale(config, months)
    if config.deviation_fraction > 0:
        factor: FloatArray = scale[:, None] * np.linalg.cholesky(
            residual_correlation(config, months)
        )
        return factor
    if config.early_noise is not None:
        diagonal: FloatArray = np.diag(scale)
        return diagonal
    return None


@dataclass(frozen=True)
class PooledNoiseEstimate(NoiseEstimate):
    """Model 1's study noise estimate with its residual degrees of freedom."""

    dof: int = 1


# --- exact mixture posteriors ----------------------------------------------------------


@dataclass(frozen=True)
class Posterior(base.Posterior):
    duration: float | None = None


def posterior(
    config: RecoveryConfig,
    prior: Prior,
    *,
    series: RecoverySeries | None,
    noise: NoiseModel | float,
    duration: float | None = None,
) -> Posterior:
    """Prior times the likelihood of ``series`` (a prefix; ``None`` = prior only).

    The effect duration comes from the series' metadata; a prior-only posterior takes
    it from ``duration`` (the series to be forecast)."""
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
        factor = _whitening(config.likelihood_config, series.months)
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
    config: RecoveryConfig,
    fitted: Posterior,
    *,
    months: Sequence[float],
    quantiles: Sequence[float] = (0.05, 0.1, 0.5, 0.9, 0.95),
    extra_noise: float = 0.0,
) -> dict[str, Any]:
    """Point forecast (median of the observation's posterior predictive distribution),
    posterior mean of the model curve and observation quantiles (exact mixture).

    ``extra_noise`` adds independent forecast noise of variance ``extra_noise * sigma^2``
    (see :meth:`RecoveryFit.forecast_inflation`)."""
    if not months or any(not math.isfinite(m) or m <= 0 for m in months):
        raise InvalidRequest("Prediction months must be finite and positive")
    if not (math.isfinite(extra_noise) and extra_noise >= 0):
        raise InvalidRequest("Extra forecast noise must be finite and non-negative")
    duration = fitted.duration
    x = design(config, months, fitted.log_tau, duration)
    # Variance multiplier of the likelihood's residual process at the targets, plus the
    # forecast-only extra (independent) early noise.
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
        # Persistent deviations of a scaled residual process (see model 1): with S the
        # observed scales and m the target scale, the gain is m S^-1 phi M^-1 r and
        # every explained variance carries m^2.
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


def best_fit_sum_squares(config: RecoveryConfig, series: RecoverySeries) -> float:
    """Weighted least-squares residual sum of squares of ``ln f`` (residuals divided by
    the early-noise scale), best over the ``tau`` grid."""
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
    config: RecoveryConfig, series: Sequence[RecoverySeries]
) -> dict[NoiseKey, NoiseEstimate]:
    """Model 1's per-(study, family, kind) residual SD with this version's law."""
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


def pooled_noise_centers(
    config: RecoveryConfig, estimates: Mapping[NoiseKey, NoiseEstimate]
) -> NoiseCenters:
    """Model 1's noise centres with ``noise_pooling``: the median of study SDs, or the
    pooled SD ``sqrt(sum(dof * sd^2) / sum(dof))`` over the same groups (per family and
    kind with at least two studies, else per kind, else every estimate), group-mean SDs
    first rescaled to the reference unit count with model 1's exponent."""
    if config.noise_pooling == "median":
        return noise_centers(config, estimates)
    if not estimates:
        raise InvalidRequest("Noise estimation needs a series with enough follow-ups")
    exponent = noise_exponent(config, estimates)

    def scaled(key: NoiseKey, estimate: NoiseEstimate) -> float:
        if key[2] == "aggregate" and estimate.units:
            return float(estimate.sd * (estimate.units / config.noise_reference_n) ** exponent)
        return estimate.sd

    def pooled(keys: list[NoiseKey]) -> float:
        weights = [float(getattr(estimates[k], "dof", 1)) for k in keys]
        total = sum(w * scaled(k, estimates[k]) ** 2 for k, w in zip(keys, weights, strict=True))
        return math.sqrt(total / sum(weights))

    every = pooled(list(estimates))
    families = sorted({family for _, family, _ in estimates})
    centers: dict[tuple[str, str], float] = {}
    for kind in ("aggregate", "individual"):
        by_kind = [key for key in estimates if key[2] == kind]
        kind_center = pooled(by_kind) if len(by_kind) >= 2 else every
        centers[("*", kind)] = max(config.noise_floor, kind_center)
        for family in families:
            chosen = [key for key in by_kind if key[1] == family]
            value = pooled(chosen) if len(chosen) >= 2 else kind_center
            centers[(family, kind)] = max(config.noise_floor, value)
    return NoiseCenters(
        centers=centers,
        exponent=exponent,
        reference_n=config.noise_reference_n,
        floor=config.noise_floor,
    )


def series_summary(config: RecoveryConfig, series: RecoverySeries, sigma: float) -> dict[str, Any]:
    """Model 1's Gaussian likelihood summary of ``theta`` with this version's law."""
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
class RecoveryFit(base.RecoveryFit):
    """Model 1's fitted population with the optional treatment fallback and forecast
    noise centres."""

    forecast_noise_centers: NoiseCenters | None = None

    def forecast_inflation(self, kind: str, context: str, units: int | None = None) -> float:
        """Extra forecast noise variance in units of ``sigma^2``: the squared ratio of
        the forecast centre to the likelihood centre minus one, at least zero."""
        if self.forecast_noise_centers is None:
            return 0.0
        family = family_of(context)
        ratio = self.forecast_noise_centers.center(family, kind, units) / (
            self.noise_centers.center(family, kind, units)
        )
        return max(0.0, ratio**2 - 1.0)

    def _fallback_starts(self, fitted: base.FamilyFit, context: str) -> list[int]:
        config = self.config
        if not isinstance(config, RecoveryConfig) or not config.treatment_fallback:
            return []
        if any(level == "context" for level, _ in self._known(fitted, context)):
            return []
        treatment = treatment_of(context)
        return [
            start
            for (level, key), start in sorted(fitted.effect_index.items())
            if level == "context" and treatment_of(key) == treatment
        ]

    def level_used(self, context: str) -> str:
        fitted = self._family(context)
        if self._fallback_starts(fitted, context):
            return f"treatment:{treatment_of(context)}"
        return super().level_used(context)

    def location(self, context: str, covariate: float) -> tuple[FloatArray, FloatArray]:
        """Context-level ``theta`` at covariate ``x``; an untrained context with the
        fallback averages the trained contexts of its treatment."""
        fitted = self._family(context)
        starts = self._fallback_starts(fitted, context)
        if not starts:
            return super().location(context, covariate)
        d = self.config.dimensions
        row = np.zeros((d, fitted.solution.size))
        row[:, :d] = np.eye(d)
        row[0, d] = covariate
        for start in starts:
            row[:, start : start + d] += np.eye(d) / len(starts)
        mean = row @ fitted.solution
        covariance = row @ fitted.covariance @ row.T + fitted.variances["context"]
        return mean, covariance


def fit_population(
    config: RecoveryConfig,
    series: Sequence[RecoverySeries],
    *,
    noise_series: Sequence[RecoverySeries] | None = None,
) -> RecoveryFit:
    """Model 1's population fit (one hierarchy per species, control series only) with
    this version's law, noise scale and fallback."""
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
