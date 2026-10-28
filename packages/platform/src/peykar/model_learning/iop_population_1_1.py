"""IOP after glaucoma procedures, population model 1.1 (early transient, eye-count noise).

Kinetics, as the log fraction of the preoperative IOP over months ``t``::

    ln f(t) = ell + e * r(t) + s * q(t),
    r(t) = exp(-(t - t1) / tau),   q(t) = exp(-(t - t1) / tq),   t1 = 1 day, tq = 7 days

Compared with model 1.0.0 (``iop_population``, unchanged because its protocol pins
it), ``s`` adds a fast early transient with a fixed one-week time constant, so a
series can dip or rise in the first weeks and then follow the slower return ``e``
toward the long-term level ``ell`` (for example a week-1 pressure rise after angle
surgery, or early hypotony before a tube's hypertensive phase). On the 72 training
records, group means with at least five follow-ups had a dof-adjusted residual SD of
0.0621 with the term against 0.0755 without it, and the systematic early residuals
of angle surgery and tubes disappeared.

The noise SD of a group mean depends on its number of eyes ``n`` as ``(n/40)^-g``,
with ``g`` estimated in each fit from the study noise estimates (clipped to
[0, 0.5]); single eyes have no such scaling.

Everything else follows 1.0.0: the long-term level regresses on
``x = ln(IOP0 / 20 mmHg)`` with one shared slope estimated inside the REML fit; one
hierarchy per species (species mean, procedure context, study, series); exact
Gaussian-mixture posteriors over a fixed ``tau`` grid and noise levels (given
``ln tau`` the law is linear in ``(ell, e, s)``); Student-t and robust mixture
priors; the point forecast is the median of the posterior predictive distribution.
Nothing here reads files, registers models or claims clinical validity.
"""

from __future__ import annotations

import math
from collections.abc import Mapping, Sequence
from dataclasses import dataclass
from typing import Any

import numpy as np
from numpy.typing import NDArray

from peykar.model_learning.corneal_population import NoiseModel, nelder_mead
from peykar.model_learning.iop_population import (
    LOG_TAU_GRID,
    REFERENCE_MMHG,
    TAU_GRID_MONTHS,
    Prior,
    anchored,
    gaussian,
    level_covariate,
    pressure_family,
    recovery_ratio,
    series_family,
    student,
)
from peykar.validation.eye_iop_data import PressureSeries
from peykar_core.errors import InvalidRequest

__all__ = [
    "LOG_TAU_GRID",
    "REFERENCE_MMHG",
    "TAU_GRID_MONTHS",
    "Prior",
    "anchored",
    "gaussian",
    "level_covariate",
    "pressure_family",
    "series_family",
    "student",
]

MODEL_ID = "eye.iop-population"
MODEL_VERSION = "1.1.0"
FloatArray = NDArray[np.float64]

DAY_MONTHS = 1.0 / 30.4375
REFERENCE_MONTHS = DAY_MONTHS
TRANSIENT_MONTHS = 7.0 * DAY_MONTHS
PARAMETERS = ("level", "excess_day1", "transient_day1", "log_tau_months")
LINEAR = 3
DIMENSIONS = 4
LEVELS = ("context", "study", "group_mean_series", "single_eye_series")
# Declared constants (not fitted): Gamma(shape=2) modes of every level's SDs for
# (level, day-1 excess, day-1 transient, ln tau); noise floor and the log-SD of a new
# study's noise around its centre; the reference eye count and the largest exponent
# of the group-mean noise scaling; the weak prior of per-series summaries and of the
# unpooled baseline; the robust mixture's broad component and weight.
PENALTY_MODES: FloatArray = np.array([0.1, 0.1, 0.1, 0.5])
NOISE_FLOOR = 0.01
NOISE_LOG_SD = 0.5
MIN_NOISE_FOLLOW_UPS = 5
NOISE_REFERENCE_EYES = 40.0
MAX_NOISE_EXPONENT = 0.5
MIN_EXPONENT_STUDIES = 4
WEAK_SD: FloatArray = np.array([1.0, 1.0, 1.0])
WEAK_LOG_TAU_SD = 10.0
ROBUST_WEIGHT = 0.2
ROBUST_SD: FloatArray = np.array([0.3, 0.3, 0.3, 2.0])
SUMMARY_JITTER: FloatArray = np.array([1e-6, 1e-6, 1e-6, 1e-4])
INFORMATION_FLOOR = 0.05
PRUNE_WEIGHT = 1e-12
DESCRIBED_MONTHS = (DAY_MONTHS, 7 * DAY_MONTHS, 1.0, 6.0, 12.0)
MAX_SERIES = 5_000
REML_ITERATIONS = 8000
REML_RESTARTS = 3


def transient_ratio(months: Sequence[float] | FloatArray) -> FloatArray:
    """``q(t)``, equal to 1 one day after surgery."""
    t = np.asarray(months, dtype=np.float64)
    result: FloatArray = np.exp(-(t - REFERENCE_MONTHS) / TRANSIENT_MONTHS)
    return result


def design(months: Sequence[float] | FloatArray, log_tau: FloatArray) -> FloatArray:
    """Columns ``1, r(t), q(t)`` for every ``ln tau`` (shape: tau, months, 3)."""
    r = recovery_ratio(months, log_tau)
    q = np.broadcast_to(transient_ratio(months), r.shape)
    result: FloatArray = np.stack([np.ones_like(r), r, q], axis=-1)
    return result


def pressure_fraction(
    months: Sequence[float] | FloatArray,
    *,
    level: float,
    excess: float,
    transient: float,
    tau_months: float,
) -> FloatArray:
    """Kinetic law for scalar parameters."""
    x = design(months, np.array([math.log(tau_months)]))[0]
    result: FloatArray = np.exp(x @ np.array([level, excess, transient]))
    return result


def best_fit_sum_squares(series: PressureSeries) -> float:
    """Least-squares residual sum of squares of ``ln f``, best over the ``tau`` grid."""
    y = np.log(np.asarray(series.fractions, dtype=np.float64))
    best = math.inf
    for x in design(series.months, LOG_TAU_GRID):
        beta, *_ = np.linalg.lstsq(x, y, rcond=None)
        residual = y - x @ beta
        best = min(best, float(residual @ residual))
    return best


NoiseKey = tuple[str, str, str]  # (study, family, series kind)


@dataclass(frozen=True)
class NoiseEstimate:
    sd: float
    eyes: float | None  # mean eye count of the contributing group means


def study_noise(series: Sequence[PressureSeries]) -> dict[NoiseKey, NoiseEstimate]:
    """Residual SD of ``ln f`` per (study, family, kind) from best fits of series with
    at least five follow-ups (four kinetic parameters); others get no estimate."""
    groups: dict[NoiseKey, list[PressureSeries]] = {}
    for item in series:
        groups.setdefault((item.study_id, series_family(item), item.kind), []).append(item)
    estimates: dict[NoiseKey, NoiseEstimate] = {}
    for key, members in groups.items():
        usable = [m for m in members if len(m.months) >= MIN_NOISE_FOLLOW_UPS]
        if not usable:
            continue
        sse = sum(best_fit_sum_squares(m) for m in usable)
        dof = sum(len(m.months) - DIMENSIONS for m in usable)
        eyes = [float(m.aggregate_n) for m in usable if m.aggregate_n]
        estimates[key] = NoiseEstimate(
            sd=max(NOISE_FLOOR, math.sqrt(sse / dof)),
            eyes=float(np.mean(eyes)) if eyes else None,
        )
    return estimates


@dataclass(frozen=True)
class NoiseCenters:
    """Median study SD per (family, kind) at the reference eye count, and the
    group-mean eye-count exponent ``g``."""

    centers: Mapping[tuple[str, str], float]
    exponent: float

    def center(self, family: str, kind: str, eyes: int | None) -> float:
        value = self.centers.get((family, kind), self.centers[("*", kind)])
        if kind == "aggregate" and eyes:
            value *= (eyes / NOISE_REFERENCE_EYES) ** (-self.exponent)
        return max(NOISE_FLOOR, value)


def noise_exponent(estimates: Mapping[NoiseKey, NoiseEstimate]) -> float:
    """Least-squares slope of ``-ln SD`` on ``ln(eyes)`` over group-mean studies,
    clipped to [0, MAX_NOISE_EXPONENT]; 0 with fewer than four studies."""
    points = [
        (math.log(e.eyes), math.log(e.sd))
        for (_, _, kind), e in estimates.items()
        if kind == "aggregate" and e.eyes
    ]
    if len(points) < MIN_EXPONENT_STUDIES:
        return 0.0
    x = np.array([p[0] for p in points])
    y = np.array([p[1] for p in points])
    if float(np.ptp(x)) <= 0:
        return 0.0
    slope = float(np.polyfit(x, y, 1)[0])
    return float(min(MAX_NOISE_EXPONENT, max(0.0, -slope)))


def noise_centers(estimates: Mapping[NoiseKey, NoiseEstimate]) -> NoiseCenters:
    """Median per (family, kind) with at least two studies, else per kind across
    families with at least two, else the median of every estimate; group-mean SDs
    are first rescaled to the reference eye count."""
    if not estimates:
        raise InvalidRequest("Noise estimation needs a series with at least five follow-ups")
    exponent = noise_exponent(estimates)

    def scaled(key: NoiseKey, estimate: NoiseEstimate) -> float:
        if key[2] == "aggregate" and estimate.eyes:
            return float(estimate.sd * (estimate.eyes / NOISE_REFERENCE_EYES) ** exponent)
        return estimate.sd

    values = {key: scaled(key, e) for key, e in estimates.items()}
    every = float(np.median(sorted(values.values())))
    families = sorted({family for _, family, _ in values})
    centers: dict[tuple[str, str], float] = {}
    for kind in ("aggregate", "individual"):
        by_kind = [v for (_, _, k), v in values.items() if k == kind]
        kind_center = float(np.median(by_kind)) if len(by_kind) >= 2 else every
        centers[("*", kind)] = max(NOISE_FLOOR, kind_center)
        for family in families:
            chosen = [v for (_, f, k), v in values.items() if f == family and k == kind]
            value = float(np.median(chosen)) if len(chosen) >= 2 else kind_center
            centers[(family, kind)] = max(NOISE_FLOOR, value)
    return NoiseCenters(centers=centers, exponent=exponent)


# --- priors as Gaussian mixtures over theta -------------------------------------


def robust(prior: Prior, weight: float = ROBUST_WEIGHT) -> Prior:
    """Mixture of ``prior`` and a broad component around its first mean."""
    if not 0.0 < weight < 1.0:
        raise InvalidRequest("Robust mixture weight must lie strictly between 0 and 1")
    broad = prior.covariances[0] + np.diag(ROBUST_SD**2)
    return Prior(
        weights=(*(w * (1.0 - weight) for w in prior.weights), weight),
        means=(*prior.means, prior.means[0]),
        covariances=(*prior.covariances, broad),
        flat_level=prior.flat_level,
    )


def weak_prior() -> Prior:
    """Unpooled baseline: broad Gaussian on the linear part, nearly uniform ``ln tau``."""
    mean = np.array([0.0, 0.0, 0.0, float(np.mean(LOG_TAU_GRID))])
    covariance = np.diag([*(WEAK_SD**2), WEAK_LOG_TAU_SD**2])
    return gaussian(mean, covariance)


@dataclass(frozen=True)
class Posterior:
    """Exact Gaussian mixture: per component, the linear part's mean and covariance
    given ``ln tau`` and the noise SD."""

    weights: FloatArray
    means: FloatArray  # (components, 3)
    covariances: FloatArray  # (components, 3, 3)
    log_tau: FloatArray
    sigmas: FloatArray


def _noise_levels(noise: NoiseModel | float) -> tuple[FloatArray, FloatArray]:
    if isinstance(noise, NoiseModel):
        sigmas, log_weights = noise.levels()
        return np.maximum(NOISE_FLOOR, sigmas), log_weights
    sigma = float(noise)
    if not math.isfinite(sigma) or sigma <= 0:
        raise InvalidRequest("Noise SD must be finite and positive")
    return np.array([sigma]), np.zeros(1)


def _log_normalize(log_weights: FloatArray) -> FloatArray:
    peak = float(np.max(log_weights))
    if not math.isfinite(peak):
        raise InvalidRequest("Posterior is degenerate")
    shifted = np.exp(log_weights - peak)
    normalized: FloatArray = shifted / float(shifted.sum())
    return normalized


def _log_softmax(log_weights: FloatArray) -> FloatArray:
    peak = float(np.max(log_weights))
    result: FloatArray = log_weights - peak - math.log(float(np.exp(log_weights - peak).sum()))
    return result


def _conditional_prior(
    mean: FloatArray, covariance: FloatArray, flat_level: bool
) -> tuple[FloatArray, FloatArray, float, FloatArray]:
    """Prior of the linear part given ``ln tau`` on the grid: means (tau, 3), precision
    (3, 3), log-determinant of the proper part and the log grid weights."""
    var_u = float(covariance[3, 3])
    u = LOG_TAU_GRID - float(mean[3])
    log_pi = _log_softmax(-0.5 * u**2 / var_u)
    first = 1 if flat_level else 0
    block = slice(first, LINEAR)
    gain = covariance[block, 3] / var_u
    conditional = (
        covariance[block, block] - np.outer(covariance[block, 3], covariance[3, block]) / var_u
    )
    sign, log_det = np.linalg.slogdet(conditional)
    if sign <= 0:
        raise InvalidRequest("Prior covariance must be positive definite")
    means = np.zeros((LOG_TAU_GRID.size, LINEAR))
    means[:, block] = mean[block][None, :] + u[:, None] * gain[None, :]
    precision = np.zeros((LINEAR, LINEAR))
    precision[block, block] = np.linalg.inv(conditional)
    return means, precision, float(log_det), log_pi


def posterior(
    prior: Prior, *, series: PressureSeries | None, noise: NoiseModel | float
) -> Posterior:
    """Prior times the likelihood of ``series`` (a prefix; ``None`` = prior only)."""
    sigmas, log_noise = _noise_levels(noise)
    inverse_variance = 1.0 / sigmas**2
    if series is None:
        if prior.flat_level:
            raise InvalidRequest("An anchored prior needs at least one observation")
        n = 0
        xtx = np.zeros((LOG_TAU_GRID.size, LINEAR, LINEAR))
        xtz = np.zeros((LOG_TAU_GRID.size, LINEAR))
        zz = 0.0
    else:
        z = np.log(np.asarray(series.fractions, dtype=np.float64))
        x = design(series.months, LOG_TAU_GRID)
        n = z.size
        xtx = np.einsum("kna,knb->kab", x, x)
        xtz = np.einsum("kna,n->ka", x, z)
        zz = float(z @ z)
    blocks = []
    for weight, mean, covariance in zip(prior.weights, prior.means, prior.covariances, strict=True):
        mean0, precision0, log_det_prior, log_pi = _conditional_prior(
            np.asarray(mean), np.asarray(covariance), prior.flat_level
        )
        b_prior = mean0 @ precision0.T  # (tau, 3)
        quad_prior = np.einsum("ka,ab,kb->k", mean0, precision0, mean0)
        precision = (
            precision0[None, None] + xtx[:, None] * inverse_variance[None, :, None, None]
        )  # (tau, sigma, 3, 3)
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
        )
        log_w = math.log(weight) + log_pi[:, None] + log_noise[None, :] + log_marginal
        blocks.append(
            (
                log_w.ravel(),
                means.reshape(-1, LINEAR),
                covariances.reshape(-1, LINEAR, LINEAR),
                np.repeat(LOG_TAU_GRID, sigmas.size),
                np.tile(sigmas, LOG_TAU_GRID.size),
            )
        )
    weights = _log_normalize(np.concatenate([b[0] for b in blocks]))
    keep = weights >= PRUNE_WEIGHT * float(weights.max())
    kept = weights[keep]
    return Posterior(
        weights=kept / kept.sum(),
        means=np.concatenate([b[1] for b in blocks])[keep],
        covariances=np.concatenate([b[2] for b in blocks])[keep],
        log_tau=np.concatenate([b[3] for b in blocks])[keep],
        sigmas=np.concatenate([b[4] for b in blocks])[keep],
    )


def moments(fitted: Posterior) -> tuple[FloatArray, FloatArray]:
    """Mean and covariance of ``theta = (level, excess, transient, ln tau)``."""
    w = fitted.weights
    points = np.column_stack([fitted.means, fitted.log_tau])
    mean = w @ points
    centered = points - mean
    covariance = (centered * w[:, None]).T @ centered
    covariance[:LINEAR, :LINEAR] += np.einsum("m,mab->ab", w, fitted.covariances)
    return mean, covariance


def _normal_cdf(x: FloatArray) -> FloatArray:
    # Abramowitz & Stegun 7.1.26 (absolute error below 1.5e-7).
    z = np.abs(x) / math.sqrt(2.0)
    t = 1.0 / (1.0 + 0.3275911 * z)
    poly = t * (
        0.254829592 + t * (-0.284496736 + t * (1.421413741 + t * (-1.453152027 + t * 1.061405429)))
    )
    erf = 1.0 - poly * np.exp(-(z**2))
    result: FloatArray = 0.5 * (1.0 + np.sign(x) * erf)
    return result


def predictive(
    fitted: Posterior,
    *,
    months: Sequence[float],
    quantiles: Sequence[float] = (0.05, 0.1, 0.5, 0.9, 0.95),
) -> dict[str, Any]:
    """Point forecast (median of the observation's posterior predictive distribution),
    posterior mean of the model curve and observation quantiles (exact mixture)."""
    if not months or any(not math.isfinite(m) or m <= 0 for m in months):
        raise InvalidRequest("Prediction months must be finite and positive")
    x = design(months, fitted.log_tau)  # (components, months, 3)
    mean_log = np.einsum("kta,ka->kt", x, fitted.means)
    variance = np.einsum("kta,kab,ktb->kt", x, fitted.covariances, x)
    w = fitted.weights
    mean_fraction = w @ np.exp(mean_log + 0.5 * variance)
    spread = np.sqrt(variance + fitted.sigmas[:, None] ** 2)
    low = np.min(mean_log - 12.0 * spread, axis=0)
    high = np.max(mean_log + 12.0 * spread, axis=0)
    result_quantiles: dict[str, list[float]] = {}
    for q in quantiles:
        lo, hi = low.copy(), high.copy()
        for _ in range(60):
            mid = 0.5 * (lo + hi)
            below = w @ _normal_cdf((mid[None, :] - mean_log) / spread) < q
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
        "posterior_mean": dict(zip(PARAMETERS, (float(v) for v in mean), strict=True)),
    }


# --- population fit -------------------------------------------------------------


def weak_prior_moments() -> tuple[FloatArray, FloatArray]:
    """Mean and covariance of the weak prior as it acts on the ``tau`` grid."""
    prior = weak_prior()
    mean, covariance = prior.means[0], prior.covariances[0]
    weights = _log_normalize(-0.5 * (LOG_TAU_GRID - mean[3]) ** 2 / covariance[3, 3])
    mean_u = float(weights @ LOG_TAU_GRID)
    var_u = float(weights @ (LOG_TAU_GRID - mean_u) ** 2)
    return (
        np.array([*mean[:LINEAR], mean_u]),
        np.diag([*np.diag(covariance)[:LINEAR], var_u]),
    )


def series_summary(series: PressureSeries, sigma: float) -> dict[str, Any]:
    """Gaussian likelihood summary of ``theta`` for one series (fixed noise SD).

    Posterior moments under the weak prior, with the weak prior's precision and
    information subtracted in coordinates standardized by its SDs. Directions with
    less than ``INFORMATION_FLOOR`` weak-prior precisions keep the posterior mean's
    component and get variance ``1 / INFORMATION_FLOOR`` weak-prior variances.
    """
    post_mean, post_cov = moments(posterior(weak_prior(), series=series, noise=sigma))
    prior_mean, prior_cov = weak_prior_moments()
    scale = np.sqrt(np.diag(prior_cov))
    standard = np.outer(scale, scale)
    post_precision = np.linalg.inv(post_cov / standard)
    prior_precision = np.linalg.inv(prior_cov / standard)
    information = post_precision - prior_precision
    values, vectors = np.linalg.eigh(0.5 * (information + information.T))
    eta = post_precision @ (post_mean / scale) - prior_precision @ (prior_mean / scale)
    informed = values >= INFORMATION_FLOOR
    safe = np.where(informed, values, 1.0)
    components = np.where(informed, (vectors.T @ eta) / safe, vectors.T @ (post_mean / scale))
    values = np.where(informed, values, INFORMATION_FLOOR)
    covariance_std = vectors @ np.diag(1.0 / values) @ vectors.T
    return {
        "series_id": series.series_id,
        "study_id": series.study_id,
        "context": series.context,
        "kind": series.kind,
        "covariate": level_covariate(series),
        "mean": (vectors @ components) * scale,
        "covariance": covariance_std * standard + np.diag(SUMMARY_JITTER),
    }


def _gamma2_log_penalty(sd: FloatArray, mode: FloatArray) -> float:
    return float(np.sum(np.log(sd) - sd / mode))


def _diagonal(log_sd: FloatArray) -> FloatArray:
    result: FloatArray = np.diag(np.exp(2.0 * log_sd))
    return result


FIXED = DIMENSIONS + 1  # species mean of theta and the level slope on x


def _fixed_design(covariate: float) -> FloatArray:
    matrix = np.zeros((DIMENSIONS, FIXED))
    matrix[:, :DIMENSIONS] = np.eye(DIMENSIONS)
    matrix[0, DIMENSIONS] = covariate
    return matrix


@dataclass(frozen=True)
class FamilyFit:
    family: str
    fixed: FloatArray  # (mean level, excess, transient, ln tau, slope on x)
    fixed_covariance: FloatArray
    # fixed + context effect, at x = 0
    context_effects: Mapping[str, FloatArray]
    context_covariance: Mapping[str, FloatArray]
    variances: Mapping[str, FloatArray]
    studies: tuple[str, ...]
    series: int
    objective: float


def fit_family(
    family: str, summaries: Sequence[Mapping[str, Any]], modes: FloatArray = PENALTY_MODES
) -> FamilyFit:
    """Penalized REML of diagonal context/study/series covariances with a fixed
    level slope on the preoperative-pressure covariate."""
    d = DIMENSIONS
    contexts = tuple(sorted({m["context"] for m in summaries}))
    context_index = {c: i for i, c in enumerate(contexts)}
    blocks = []
    for context in contexts:
        members = [m for m in summaries if m["context"] == context]
        y = np.concatenate([m["mean"] for m in members])
        fixed_design = np.concatenate([_fixed_design(m["covariate"]) for m in members])
        same_study = np.array(
            [[float(a["study_id"] == b["study_id"]) for b in members] for a in members]
        )
        series_cov = [m["covariance"] for m in members]
        aggregate = [m["kind"] == "aggregate" for m in members]
        blocks.append((context, len(members), y, fixed_design, same_study, series_cov, aggregate))

    def residual_covariance(params: FloatArray, block: tuple[Any, ...]) -> FloatArray:
        _, n, _, _, same_study, series_cov, aggregate = block
        group = _diagonal(params[2 * d : 3 * d])
        eye = _diagonal(params[3 * d : 4 * d])
        matrix = np.kron(same_study, _diagonal(params[d : 2 * d]))
        for row in range(n):
            deviation = group if aggregate[row] else eye
            matrix[d * row : d * row + d, d * row : d * row + d] += deviation + series_cov[row]
        result: FloatArray = matrix
        return result

    def objective(params: FloatArray) -> float:
        if np.any(np.abs(params) > 12.0):
            return 1e12
        context_cov = _diagonal(params[0:d])
        a_total = np.zeros((FIXED, FIXED))
        b_total = np.zeros(FIXED)
        pieces = []
        for block in blocks:
            n = block[1]
            matrix = residual_covariance(params, block) + np.kron(np.ones((n, n)), context_cov)
            try:
                chol = np.linalg.cholesky(matrix)
            except np.linalg.LinAlgError:
                return 1e12
            inv_design = np.linalg.solve(chol, block[3])
            inv_y = np.linalg.solve(chol, block[2])
            a_total += inv_design.T @ inv_design
            b_total += inv_design.T @ inv_y
            pieces.append((chol, inv_design, inv_y))
        sign, logdet_a = np.linalg.slogdet(a_total)
        if sign <= 0:
            return 1e12
        beta = np.linalg.solve(a_total, b_total)
        value = 0.5 * logdet_a
        for chol, inv_design, inv_y in pieces:
            residual = inv_y - inv_design @ beta
            value += float(np.sum(np.log(np.diag(chol)))) + 0.5 * float(residual @ residual)
        for level in range(len(LEVELS)):
            value -= _gamma2_log_penalty(np.exp(params[d * level : d * level + d]), modes)
        return value

    best, _ = nelder_mead(
        objective, np.tile(np.log(modes), len(LEVELS)), iterations=REML_ITERATIONS
    )
    for _ in range(REML_RESTARTS):  # deterministic restarts from the incumbent
        best, _ = nelder_mead(objective, best, step=0.2, iterations=REML_ITERATIONS)
    value = objective(best)
    if value >= 1e12:
        raise InvalidRequest(f"Population fit of {family!r} did not converge")
    # Joint Gaussian posterior of the fixed effects (flat prior) and context effects
    # (N(0, C)) given the fitted variances: exact BLUE/BLUP.
    context_precision = np.linalg.inv(_diagonal(best[0:d]))
    unknowns = FIXED + d * len(contexts)
    precision = np.zeros((unknowns, unknowns))
    rhs = np.zeros(unknowns)
    for block in blocks:
        context, n = block[0], block[1]
        matrix = np.zeros((d * n, unknowns))
        matrix[:, :FIXED] = block[3]
        offset = FIXED + d * context_index[context]
        matrix[:, offset : offset + d] = np.kron(np.ones((n, 1)), np.eye(d))
        inverse = np.linalg.inv(residual_covariance(best, block))
        precision += matrix.T @ inverse @ matrix
        rhs += matrix.T @ inverse @ block[2]
        precision[offset : offset + d, offset : offset + d] += context_precision
    covariance = np.linalg.inv(precision)
    solution = covariance @ rhs
    context_effects = {}
    context_cov = {}
    for context, i in context_index.items():
        selector = np.zeros((FIXED + d, unknowns))
        selector[:FIXED, :FIXED] = np.eye(FIXED)
        selector[FIXED:, FIXED + d * i : FIXED + d * i + d] = np.eye(d)
        context_effects[context] = selector @ solution
        context_cov[context] = selector @ covariance @ selector.T
    return FamilyFit(
        family=family,
        fixed=solution[:FIXED],
        fixed_covariance=covariance[:FIXED, :FIXED],
        context_effects=context_effects,
        context_covariance=context_cov,
        variances={level: _diagonal(best[d * i : d * i + d]) for i, level in enumerate(LEVELS)},
        studies=tuple(sorted({m["study_id"] for m in summaries})),
        series=len(summaries),
        objective=float(value),
    )


@dataclass(frozen=True)
class PressureFit:
    families: Mapping[str, FamilyFit]
    noise_centers: NoiseCenters
    study_noise_sd: Mapping[str, float]
    training_series: tuple[str, ...]
    training_studies: tuple[str, ...]
    unfitted_families: Mapping[str, str]

    def _family(self, context: str) -> FamilyFit:
        family = pressure_family(context)
        if family not in self.families:
            reason = self.unfitted_families.get(family, "no training study")
            raise InvalidRequest(f"No population prior for family {family!r}: {reason}")
        return self.families[family]

    def level_used(self, context: str) -> str:
        fitted = self._family(context)
        return context if context in fitted.context_effects else f"family:{fitted.family}"

    def location(self, context: str, covariate: float) -> tuple[FloatArray, FloatArray]:
        """Mean and covariance of the context-level ``theta`` at covariate ``x``."""
        fitted = self._family(context)
        d = DIMENSIONS
        row = np.zeros((d, FIXED + d))
        row[:, :d] = np.eye(d)
        row[0, d] = covariate
        if context in fitted.context_effects:
            row[:, FIXED:] = np.eye(d)
            vector, covariance = fitted.context_effects[context], fitted.context_covariance[context]
            return row @ vector, row @ covariance @ row.T
        fixed_row = row[:, :FIXED]
        mean = fixed_row @ fitted.fixed
        covariance = fixed_row @ fitted.fixed_covariance @ fixed_row.T
        return mean, covariance + fitted.variances["context"]

    def prior(self, context: str, kind: str, covariate: float) -> dict[str, Any]:
        """Distribution of a new study's series ``theta`` (``kind`` aggregate/individual)."""
        if kind not in {"aggregate", "individual"}:
            raise InvalidRequest("Series kind must be aggregate or individual")
        fitted = self._family(context)
        mean, covariance = self.location(context, covariate)
        series_level = "group_mean_series" if kind == "aggregate" else "single_eye_series"
        return {
            "mean": mean,
            "covariance": covariance + fitted.variances["study"] + fitted.variances[series_level],
        }

    def noise(self, kind: str, context: str, eyes: int | None = None) -> NoiseModel:
        center = self.noise_centers.center(pressure_family(context), kind, eyes)
        return NoiseModel(center=center, log_sd=NOISE_LOG_SD)

    def describe(self) -> dict[str, Any]:
        def text(mean: FloatArray, covariance: FloatArray) -> dict[str, Any]:
            sd = np.sqrt(np.diag(covariance))
            curve = pressure_fraction(
                DESCRIBED_MONTHS,
                level=float(mean[0]),
                excess=float(mean[1]),
                transient=float(mean[2]),
                tau_months=math.exp(mean[3]),
            )
            return {
                "long_term_fraction_at_20_mmhg": math.exp(mean[0]),
                "excess_day1": float(mean[1]),
                "transient_day1": float(mean[2]),
                "median_tau_days": math.exp(mean[3]) / DAY_MONTHS,
                "estimate_sd": dict(zip(PARAMETERS, sd.tolist(), strict=True)),
                "fraction_at_mean_parameters_20_mmhg": {
                    f"{m:.4g}": float(v) for m, v in zip(DESCRIBED_MONTHS, curve, strict=True)
                },
            }

        families = {}
        for name, fitted in self.families.items():
            families[name] = {
                "mean": text(
                    fitted.fixed[:DIMENSIONS], fitted.fixed_covariance[:DIMENSIONS, :DIMENSIONS]
                ),
                "level_slope_on_ln_preop_iop": float(fitted.fixed[DIMENSIONS]),
                "level_slope_sd": float(math.sqrt(fitted.fixed_covariance[DIMENSIONS, DIMENSIONS])),
                "postoperative_iop_elasticity": float(1.0 + fitted.fixed[DIMENSIONS]),
                "contexts": {
                    c: text(*self.location(c, 0.0)) for c in sorted(fitted.context_effects)
                },
                "level_sd": {
                    level: dict(zip(PARAMETERS, np.sqrt(np.diag(matrix)).tolist(), strict=True))
                    for level, matrix in fitted.variances.items()
                },
                "training_studies": len(fitted.studies),
                "training_series": fitted.series,
                "penalized_reml_objective": fitted.objective,
            }
        return {
            "model_id": MODEL_ID,
            "model_version": MODEL_VERSION,
            "families": families,
            "unfitted_families": dict(self.unfitted_families),
            "noise_center_sd_log_fraction_at_40_eyes": {
                f"{family}/{kind}": value
                for (family, kind), value in self.noise_centers.centers.items()
            },
            "group_mean_noise_eye_exponent": self.noise_centers.exponent,
            "study_noise_sd_log_fraction": dict(self.study_noise_sd),
            "training_series": len(self.training_series),
            "training_studies": len(self.training_studies),
        }


def fit_population(
    series: Sequence[PressureSeries], *, noise_series: Sequence[PressureSeries] | None = None
) -> PressureFit:
    """Fit one hierarchy per species to control series of several studies.

    ``noise_series`` (default: the controls) supplies study noise estimates; treated
    series may be added because noise belongs to the measurement. Series without a
    preoperative IOP are skipped. A family with fewer than two training studies gets
    no population prior.
    """
    if not 2 <= len(series) <= MAX_SERIES:
        raise InvalidRequest("Population fitting needs 2..5000 series")
    if len({s.series_id for s in series}) != len(series):
        raise InvalidRequest("Population fitting requires unique series identities")
    if any(s.role != "control" for s in series):
        raise InvalidRequest("Population priors are fitted to control series only")
    estimates = study_noise(noise_series if noise_series is not None else series)
    centers = noise_centers(estimates)
    summaries = []
    for item in series:
        if item.reference_mmhg <= 0:
            continue
        family = series_family(item)
        own = estimates.get((item.study_id, family, item.kind))
        sigma = own.sd if own is not None else centers.center(family, item.kind, item.aggregate_n)
        summaries.append({**series_summary(item, sigma), "family": family})
    fitted: dict[str, FamilyFit] = {}
    unfitted: dict[str, str] = {}
    for family in sorted({m["family"] for m in summaries}):
        members = [m for m in summaries if m["family"] == family]
        if len({m["study_id"] for m in members}) < 2:
            unfitted[family] = "fewer than two training studies"
            continue
        fitted[family] = fit_family(family, members)
    if not fitted:
        raise InvalidRequest("No family has two training studies")
    return PressureFit(
        families=fitted,
        noise_centers=centers,
        study_noise_sd={f"{s}/{f}/{k}": v.sd for (s, f, k), v in estimates.items()},
        training_series=tuple(s.series_id for s in series),
        training_studies=tuple(sorted({s.study_id for s in series})),
        unfitted_families=unfitted,
    )
