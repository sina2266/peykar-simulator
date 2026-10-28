"""Hierarchical population model of intraocular pressure (IOP) after glaucoma procedures.

Kinetics, as the log fraction of the preoperative IOP over months ``t`` after the
procedure::

    ln f(t) = ell + e * r(t),    r(t) = exp(-(t - t1) / tau),    t1 = 1 day

``ell`` is the long-term log level, ``e`` the log deviation from it one day after
surgery (early hypotony below the long-term level after filtering surgery, or an
early pressure spike) and ``tau`` (months) the time scale of an exponential return.
On development series with at least four follow-ups one exponential phase fitted
group means slightly better than hyperbolic or power-law returns with the same
number of parameters.

Postoperative pressure regresses toward a common target: the long-term level depends
on the preoperative pressure through ``x = ln(IOP0 / 20 mmHg)``, with one slope ``b``
shared by all procedures and estimated as a fixed effect together with the
hierarchy (generalized least squares inside the REML fit). A series' prior mean
level is its context's level plus ``b * x``.

One hierarchy per species: species mean, procedure context effect, study effect and
series deviation (separate variances for group means and single eyes), diagonal
covariances by penalized restricted maximum likelihood on per-series likelihood
summaries (posterior moments under a weak prior with the weak prior's information
removed). A procedure without training studies falls back to the species mean.

Observation model: Gaussian noise in ``ln f`` with one SD per study and series kind;
a new study's SD is unknown and marginalized over a log-normal prior around the
median of training studies. Given ``ln tau`` the law is linear in ``(ell, e)``, so
posteriors under Gaussian, Student-t (scale mixture) or robust mixture priors are
exact Gaussian mixtures over a fixed ``tau`` grid and noise levels. An ``anchored``
variant leaves the level flat. Nothing here reads files, registers models or claims
clinical validity.
"""

from __future__ import annotations

import math
from collections.abc import Mapping, Sequence
from dataclasses import dataclass
from typing import Any

import numpy as np
from numpy.typing import NDArray

from peykar.model_learning.corneal_population import NoiseModel, nelder_mead
from peykar.validation.eye_iop_data import PROCEDURES, PressureSeries
from peykar_core.errors import InvalidRequest

MODEL_ID = "eye.iop-population"
MODEL_VERSION = "1.0.0"
FloatArray = NDArray[np.float64]

DAY_MONTHS = 1.0 / 30.4375
REFERENCE_MONTHS = DAY_MONTHS
REFERENCE_MMHG = 20.0
TAU_GRID_MONTHS: FloatArray = np.geomspace(0.01, 120.0, 80)
LOG_TAU_GRID: FloatArray = np.log(TAU_GRID_MONTHS)
PARAMETERS = ("level", "excess_day1", "log_tau_months")
LEVELS = ("context", "study", "group_mean_series", "single_eye_series")
# Declared constants (not fitted): Gamma(shape=2) modes of every level's SDs for
# (level, day-1 excess, ln tau); noise floor and the log-SD of a new study's noise
# around its centre; the weak prior of per-series summaries and of the unpooled
# baseline; the robust mixture's broad component and weight; Student-t nodes.
PENALTY_MODES: FloatArray = np.array([0.1, 0.1, 0.5])
NOISE_FLOOR = 0.01
NOISE_LOG_SD = 0.5
MIN_NOISE_FOLLOW_UPS = 4
WEAK_SD: FloatArray = np.array([1.0, 1.0])
WEAK_LOG_TAU_SD = 10.0
ROBUST_WEIGHT = 0.2
ROBUST_SD: FloatArray = np.array([0.3, 0.3, 2.0])
STUDENT_NODES = 16
SUMMARY_JITTER: FloatArray = np.array([1e-6, 1e-6, 1e-4])
INFORMATION_FLOOR = 0.05
PRUNE_WEIGHT = 1e-12
DESCRIBED_MONTHS = (DAY_MONTHS, 7 * DAY_MONTHS, 1.0, 6.0, 12.0)
MAX_SERIES = 5_000


def pressure_family(context: str) -> str:
    """``species/all``: every procedure shares one hierarchy per species."""
    species, _, procedure = context.partition("/")
    if procedure not in PROCEDURES:
        raise InvalidRequest(f"Unknown procedure context {context!r}")
    return f"{species}/all"


def series_family(series: PressureSeries) -> str:
    return pressure_family(series.context)


def level_covariate(series: PressureSeries) -> float:
    """``ln(IOP0 / 20 mmHg)``; series without a preoperative value get no prior."""
    if not math.isfinite(series.reference_mmhg) or series.reference_mmhg <= 0:
        raise InvalidRequest("Population priors need the preoperative IOP")
    return math.log(series.reference_mmhg / REFERENCE_MMHG)


def recovery_ratio(months: Sequence[float] | FloatArray, log_tau: FloatArray) -> FloatArray:
    """``r(t)`` for every ``ln tau`` (rows) at ``months`` (columns); ``r(1 day) = 1``."""
    t = np.asarray(months, dtype=np.float64)
    tau = np.exp(np.asarray(log_tau, dtype=np.float64))[:, None]
    result: FloatArray = np.exp(-(t[None, :] - REFERENCE_MONTHS) / tau)
    return result


def pressure_fraction(
    months: Sequence[float] | FloatArray, *, level: float, excess: float, tau_months: float
) -> FloatArray:
    """Kinetic law for scalar parameters."""
    ratio = recovery_ratio(months, np.array([math.log(tau_months)]))[0]
    result: FloatArray = np.exp(level + excess * ratio)
    return result


def best_fit_sum_squares(series: PressureSeries) -> float:
    """Least-squares residual sum of squares of ``ln f``, best over the ``tau`` grid."""
    y = np.log(np.asarray(series.fractions, dtype=np.float64))
    best = math.inf
    for row in recovery_ratio(series.months, LOG_TAU_GRID):
        design = np.stack([np.ones_like(row), row], axis=1)
        beta, *_ = np.linalg.lstsq(design, y, rcond=None)
        residual = y - design @ beta
        best = min(best, float(residual @ residual))
    return best


NoiseKey = tuple[str, str, str]  # (study, family, series kind)


def study_noise(series: Sequence[PressureSeries]) -> dict[NoiseKey, float]:
    """Residual SD of ``ln f`` per (study, family, kind) from best fits of series with
    at least four follow-ups (three kinetic parameters); others get no estimate."""
    groups: dict[NoiseKey, list[PressureSeries]] = {}
    for item in series:
        groups.setdefault((item.study_id, series_family(item), item.kind), []).append(item)
    estimates: dict[NoiseKey, float] = {}
    for key, members in groups.items():
        usable = [m for m in members if len(m.months) >= MIN_NOISE_FOLLOW_UPS]
        if not usable:
            continue
        sse = sum(best_fit_sum_squares(m) for m in usable)
        dof = sum(len(m.months) - 3 for m in usable)
        estimates[key] = max(NOISE_FLOOR, math.sqrt(sse / dof))
    return estimates


def noise_centers(estimates: Mapping[NoiseKey, float]) -> dict[tuple[str, str], float]:
    """Median study SD per (family, kind) with at least two studies, else per kind
    across families with at least two, else the median of every estimate."""
    if not estimates:
        raise InvalidRequest("Noise estimation needs a series with at least four follow-ups")
    every = float(np.median(sorted(estimates.values())))
    families = sorted({family for _, family, _ in estimates})
    centers: dict[tuple[str, str], float] = {}
    for kind in ("aggregate", "individual"):
        by_kind = [v for (_, _, k), v in estimates.items() if k == kind]
        kind_center = float(np.median(by_kind)) if len(by_kind) >= 2 else every
        centers[("*", kind)] = max(NOISE_FLOOR, kind_center)
        for family in families:
            values = [v for (_, f, k), v in estimates.items() if f == family and k == kind]
            value = float(np.median(values)) if len(values) >= 2 else kind_center
            centers[(family, kind)] = max(NOISE_FLOOR, value)
    return centers


# --- priors as Gaussian mixtures over theta -------------------------------------


@dataclass(frozen=True)
class Prior:
    """Mixture of Gaussians over ``theta``; ``flat_level`` drops the level's prior."""

    weights: tuple[float, ...]
    means: tuple[FloatArray, ...]
    covariances: tuple[FloatArray, ...]
    flat_level: bool = False


def gaussian(mean: FloatArray, covariance: FloatArray) -> Prior:
    return Prior(weights=(1.0,), means=(np.asarray(mean),), covariances=(np.asarray(covariance),))


def student(mean: FloatArray, covariance: FloatArray, df: float) -> Prior:
    """Multivariate Student-t as a scale mixture ``N(mean, covariance / w)``,
    ``w ~ Gamma(df/2, rate df/2)``, by generalized Gauss-Laguerre nodes."""
    if not math.isfinite(df) or df <= 0:
        raise InvalidRequest("Student-t degrees of freedom must be positive")
    shape = df / 2.0
    nodes, weights = np.polynomial.laguerre.laggauss(STUDENT_NODES)
    mass = weights * nodes ** (shape - 1.0) / math.gamma(shape)
    mass = mass / mass.sum()
    scales = nodes / shape
    return Prior(
        weights=tuple(float(m) for m in mass),
        means=tuple(np.asarray(mean) for _ in scales),
        covariances=tuple(np.asarray(covariance) / float(w) for w in scales),
    )


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


def anchored(prior: Prior) -> Prior:
    """Same shape prior with a flat (unknown) level: forecasts follow the prefix level."""
    return Prior(prior.weights, prior.means, prior.covariances, flat_level=True)


def weak_prior() -> Prior:
    """Unpooled baseline: broad Gaussian on (level, excess), nearly uniform ``ln tau``."""
    mean = np.array([0.0, 0.0, float(np.mean(LOG_TAU_GRID))])
    covariance = np.diag([WEAK_SD[0] ** 2, WEAK_SD[1] ** 2, WEAK_LOG_TAU_SD**2])
    return gaussian(mean, covariance)


@dataclass(frozen=True)
class Posterior:
    """Exact Gaussian mixture: per component, ``(level, excess)`` mean and covariance
    given ``ln tau`` and the noise SD."""

    weights: FloatArray
    means: FloatArray  # (components, 2)
    covariances: FloatArray  # (components, 2, 2)
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


def posterior(
    prior: Prior, *, series: PressureSeries | None, noise: NoiseModel | float
) -> Posterior:
    """Prior times the likelihood of ``series`` (a prefix; ``None`` = prior only)."""
    sigmas, log_noise = _noise_levels(noise)
    inverse_variance = 1.0 / sigmas**2
    if series is None:
        if prior.flat_level:
            raise InvalidRequest("An anchored prior needs at least one observation")
        z = np.zeros(0)
        ratio = np.zeros((LOG_TAU_GRID.size, 0))
    else:
        z = np.log(np.asarray(series.fractions, dtype=np.float64))
        ratio = recovery_ratio(series.months, LOG_TAU_GRID)
    n = z.size
    s01 = ratio.sum(axis=1)
    s11 = (ratio**2).sum(axis=1)
    t0 = float(z.sum())
    t1 = ratio @ z
    zz = float(z @ z)
    blocks = []
    for weight, mean, covariance in zip(prior.weights, prior.means, prior.covariances, strict=True):
        var_u = float(covariance[2, 2])
        u = LOG_TAU_GRID - float(mean[2])
        log_pi = _log_softmax(-0.5 * u**2 / var_u)
        if prior.flat_level:
            gain = float(covariance[1, 2]) / var_u
            mean0 = np.stack([np.zeros_like(u), float(mean[1]) + gain * u], axis=1)
            variance_e = float(covariance[1, 1]) - float(covariance[1, 2]) ** 2 / var_u
            precision0 = np.array([[0.0, 0.0], [0.0, 1.0 / variance_e]])
            log_det_prior = math.log(variance_e)
        else:
            gain2 = covariance[0:2, 2] / var_u
            mean0 = mean[0:2][None, :] + u[:, None] * gain2[None, :]
            conditional = (
                covariance[0:2, 0:2] - np.outer(covariance[0:2, 2], covariance[2, 0:2]) / var_u
            )
            precision0 = np.linalg.inv(conditional)
            sign, log_det_prior = np.linalg.slogdet(conditional)
            if sign <= 0:
                raise InvalidRequest("Prior covariance must be positive definite")
        b_prior = mean0 @ precision0.T
        quad_prior = np.einsum("ka,ab,kb->k", mean0, precision0, mean0)
        p00 = precision0[0, 0] + n * inverse_variance[None, :]
        p01 = precision0[0, 1] + s01[:, None] * inverse_variance[None, :]
        p11 = precision0[1, 1] + s11[:, None] * inverse_variance[None, :]
        p00 = np.broadcast_to(p00, p01.shape)
        det = p00 * p11 - p01**2
        if np.any(det <= 0):
            raise InvalidRequest("Posterior precision is not positive definite")
        b0 = b_prior[:, 0:1] + t0 * inverse_variance[None, :]
        b1 = b_prior[:, 1:2] + t1[:, None] * inverse_variance[None, :]
        m0 = (p11 * b0 - p01 * b1) / det
        m1 = (p00 * b1 - p01 * b0) / det
        quad_post = b0 * m0 + b1 * m1
        log_marginal = (
            -n * np.log(sigmas)[None, :]
            - 0.5 * log_det_prior
            - 0.5 * np.log(det)
            - 0.5 * (zz * inverse_variance[None, :] + quad_prior[:, None] - quad_post)
        )
        log_w = math.log(weight) + log_pi[:, None] + log_noise[None, :] + log_marginal
        covariances = (
            np.stack([np.stack([p11, -p01], axis=-1), np.stack([-p01, p00], axis=-1)], axis=-2)
            / det[..., None, None]
        )
        blocks.append(
            (
                log_w.ravel(),
                np.stack([m0, m1], axis=-1).reshape(-1, 2),
                covariances.reshape(-1, 2, 2),
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
    """Mean and covariance of ``theta = (level, excess, ln tau)`` under a posterior."""
    w = fitted.weights
    points = np.column_stack([fitted.means, fitted.log_tau])
    mean = w @ points
    centered = points - mean
    covariance = (centered * w[:, None]).T @ centered
    covariance[0:2, 0:2] += np.einsum("m,mab->ab", w, fitted.covariances)
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
    posterior mean of the model curve and observation quantiles (exact mixture).

    The median is the declared point forecast: the mean of a lognormal under a
    heavy-tailed (Student-t) prior is unstable or undefined.
    """
    if not months or any(not math.isfinite(m) or m <= 0 for m in months):
        raise InvalidRequest("Prediction months must be finite and positive")
    ratio = recovery_ratio(months, fitted.log_tau)
    level, excess = fitted.means[:, 0:1], fitted.means[:, 1:2]
    mean_log = level + excess * ratio
    c = fitted.covariances
    variance = c[:, 0, 0, None] + 2.0 * ratio * c[:, 0, 1, None] + ratio**2 * c[:, 1, 1, None]
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
    weights = _log_normalize(-0.5 * (LOG_TAU_GRID - mean[2]) ** 2 / covariance[2, 2])
    mean_u = float(weights @ LOG_TAU_GRID)
    var_u = float(weights @ (LOG_TAU_GRID - mean_u) ** 2)
    return (
        np.array([mean[0], mean[1], mean_u]),
        np.diag([covariance[0, 0], covariance[1, 1], var_u]),
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


FIXED = 4  # species mean of (level, excess, ln tau) and the level slope on x


def _design(covariate: float) -> FloatArray:
    design = np.zeros((3, FIXED))
    design[:, :3] = np.eye(3)
    design[0, 3] = covariate
    return design


@dataclass(frozen=True)
class FamilyFit:
    family: str
    fixed: FloatArray  # (mean level, mean excess, mean ln tau, slope on x)
    fixed_covariance: FloatArray
    # fixed + context effect, at x = 0
    context_effects: Mapping[str, FloatArray]
    # joint covariance of the 4+3 vector
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
    contexts = tuple(sorted({m["context"] for m in summaries}))
    context_index = {c: i for i, c in enumerate(contexts)}
    blocks = []
    for context in contexts:
        members = [m for m in summaries if m["context"] == context]
        n = len(members)
        y = np.concatenate([m["mean"] for m in members])
        design = np.concatenate([_design(m["covariate"]) for m in members])
        same_study = np.array(
            [[float(a["study_id"] == b["study_id"]) for b in members] for a in members]
        )
        series_cov = [m["covariance"] for m in members]
        aggregate = [m["kind"] == "aggregate" for m in members]
        blocks.append((context, n, y, design, same_study, series_cov, aggregate))

    def residual_covariance(params: FloatArray, block: tuple[Any, ...]) -> FloatArray:
        _, n, _, _, same_study, series_cov, aggregate = block
        group = _diagonal(params[6:9])
        eye = _diagonal(params[9:12])
        matrix = np.kron(same_study, _diagonal(params[3:6]))
        for row in range(n):
            deviation = group if aggregate[row] else eye
            matrix[3 * row : 3 * row + 3, 3 * row : 3 * row + 3] += deviation + series_cov[row]
        result: FloatArray = matrix
        return result

    def objective(params: FloatArray) -> float:
        if np.any(np.abs(params) > 12.0):
            return 1e12
        context_cov = _diagonal(params[0:3])
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
        for level in range(4):
            value -= _gamma2_log_penalty(np.exp(params[3 * level : 3 * level + 3]), modes)
        return value

    best, _ = nelder_mead(objective, np.tile(np.log(modes), 4), iterations=6000)
    for _ in range(3):  # deterministic restarts from the incumbent
        best, _ = nelder_mead(objective, best, step=0.2, iterations=6000)
    value = objective(best)
    if value >= 1e12:
        raise InvalidRequest(f"Population fit of {family!r} did not converge")
    # Joint Gaussian posterior of the fixed effects (flat prior) and context effects
    # (N(0, C)) given the fitted variances: exact BLUE/BLUP.
    context_precision = np.linalg.inv(_diagonal(best[0:3]))
    unknowns = FIXED + 3 * len(contexts)
    precision = np.zeros((unknowns, unknowns))
    rhs = np.zeros(unknowns)
    for block in blocks:
        context, n = block[0], block[1]
        design = np.zeros((3 * n, unknowns))
        design[:, :FIXED] = block[3]
        offset = FIXED + 3 * context_index[context]
        design[:, offset : offset + 3] = np.kron(np.ones((n, 1)), np.eye(3))
        inverse = np.linalg.inv(residual_covariance(best, block))
        precision += design.T @ inverse @ design
        rhs += design.T @ inverse @ block[2]
        precision[offset : offset + 3, offset : offset + 3] += context_precision
    covariance = np.linalg.inv(precision)
    solution = covariance @ rhs
    context_effects = {}
    context_cov = {}
    for context, i in context_index.items():
        selector = np.zeros((FIXED + 3, unknowns))
        selector[:FIXED, :FIXED] = np.eye(FIXED)
        selector[FIXED:, FIXED + 3 * i : FIXED + 3 * i + 3] = np.eye(3)
        context_effects[context] = selector @ solution
        context_cov[context] = selector @ covariance @ selector.T
    return FamilyFit(
        family=family,
        fixed=solution[:FIXED],
        fixed_covariance=covariance[:FIXED, :FIXED],
        context_effects=context_effects,
        context_covariance=context_cov,
        variances={level: _diagonal(best[3 * i : 3 * i + 3]) for i, level in enumerate(LEVELS)},
        studies=tuple(sorted({m["study_id"] for m in summaries})),
        series=len(summaries),
        objective=float(value),
    )


@dataclass(frozen=True)
class PressureFit:
    families: Mapping[str, FamilyFit]
    noise_sd: Mapping[tuple[str, str], float]
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
        row = np.zeros((3, FIXED + 3))
        row[:, :3] = np.eye(3)
        row[0, 3] = covariate
        if context in fitted.context_effects:
            row[:, FIXED:] = np.eye(3)
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

    def noise(self, kind: str, context: str) -> NoiseModel:
        family = pressure_family(context)
        center = self.noise_sd.get((family, kind), self.noise_sd[("*", kind)])
        return NoiseModel(center=center, log_sd=NOISE_LOG_SD)

    def describe(self) -> dict[str, Any]:
        def text(mean: FloatArray, covariance: FloatArray) -> dict[str, Any]:
            sd = np.sqrt(np.diag(covariance))
            curve = pressure_fraction(
                DESCRIBED_MONTHS,
                level=float(mean[0]),
                excess=float(mean[1]),
                tau_months=math.exp(mean[2]),
            )
            return {
                "long_term_fraction_at_20_mmhg": math.exp(mean[0]),
                "excess_day1": float(mean[1]),
                "median_tau_days": math.exp(mean[2]) / DAY_MONTHS,
                "estimate_sd": dict(zip(PARAMETERS, sd.tolist(), strict=True)),
                "fraction_at_mean_parameters_20_mmhg": {
                    f"{m:.4g}": float(v) for m, v in zip(DESCRIBED_MONTHS, curve, strict=True)
                },
            }

        families = {}
        for name, fitted in self.families.items():
            families[name] = {
                "mean": text(fitted.fixed[:3], fitted.fixed_covariance[:3, :3]),
                "level_slope_on_ln_preop_iop": float(fitted.fixed[3]),
                "level_slope_sd": float(math.sqrt(fitted.fixed_covariance[3, 3])),
                "postoperative_iop_elasticity": float(1.0 + fitted.fixed[3]),
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
            "noise_center_sd_log_fraction": {
                f"{family}/{kind}": value for (family, kind), value in self.noise_sd.items()
            },
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
        sigma = estimates.get(
            (item.study_id, family, item.kind),
            centers.get((family, item.kind), centers[("*", item.kind)]),
        )
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
        noise_sd=centers,
        study_noise_sd={f"{s}/{f}/{k}": v for (s, f, k), v in estimates.items()},
        training_series=tuple(s.series_id for s in series),
        training_studies=tuple(sorted({s.study_id for s in series})),
        unfitted_families=unfitted,
    )
