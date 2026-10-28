"""Population model of an eye measurement after treatment, as a fraction of baseline.

Generic form of IOP model 1.1.0 (``iop_population_1_1``, unchanged because its
protocol pins it): the same kinetic law, hierarchy, exact Gaussian-mixture
posteriors and noise model, with every component-specific choice declared in a
:class:`RecoveryConfig`, so that other components (central retinal thickness first)
reuse one implementation instead of copies.

Kinetics, as the log fraction ``f`` of the pretreatment value over months ``t``::

    ln f(t) = ell + e * r(t) [+ s * q(t)],
    r(t) = exp(-(t - t1) / tau),   q(t) = exp(-(t - t1) / tq)

``t1`` is the reference time (``r = q = 1``), ``tq`` a fixed transient time
constant (``None`` drops the term) and ``ln tau`` lies on a fixed grid; given
``ln tau`` the law is linear in ``(ell, e[, s])``. The long-term level regresses on
``x = ln(baseline / reference)`` with one shared slope estimated inside the REML fit.

One hierarchy per species (``<species>/all``): species mean, then the declared
grouping levels (``("context",)`` as in IOP 1.1, or ``("treatment", "context")``
where the treatment is the last part of the context), study, and series (group
means and single units separately). Studies whose series span several groups stay
in one block, so their study effect is shared; with one group per study the blocks,
and the fit, are those of IOP 1.1. Group-mean noise scales with the number of units
``n`` as ``(n / n_ref)^-g`` with ``g`` estimated in each fit. Priors may be Gaussian,
Student-t or robust mixtures; point forecasts are posterior predictive medians.
Nothing here reads files, registers models or claims clinical validity.
"""

from __future__ import annotations

import math
from collections.abc import Mapping, Sequence
from dataclasses import dataclass, field, replace
from functools import cached_property
from typing import Any, Protocol, cast

import numpy as np
from numpy.typing import NDArray

from peykar.model_learning.corneal_population import NoiseModel, nelder_mead
from peykar.model_learning.iop_population import Prior, anchored, gaussian, student
from peykar_core.errors import InvalidRequest

__all__ = [
    "DAY_MONTHS",
    "Prior",
    "RecoveryConfig",
    "RecoveryFit",
    "anchored",
    "fit_population",
    "gaussian",
    "level_covariate",
    "posterior",
    "predictive",
    "robust",
    "student",
    "weak_prior",
]

FloatArray = NDArray[np.float64]
DAY_MONTHS = 1.0 / 30.4375
GROUPINGS = {("context",), ("treatment", "context")}
SERIES_LEVELS = ("study", "group_mean_series", "single_eye_series")
PRUNE_WEIGHT = 1e-12
MAX_SERIES = 5_000
REML_ITERATIONS = 8000
REML_RESTARTS = 3


class RecoverySeries(Protocol):
    """What the model needs from one series (for example ``RetinaSeries``)."""

    @property
    def series_id(self) -> str: ...

    @property
    def study_id(self) -> str: ...

    @property
    def context(self) -> str: ...

    @property
    def reference_value(self) -> float: ...

    @property
    def months(self) -> tuple[float, ...]: ...

    @property
    def fractions(self) -> tuple[float, ...]: ...

    @property
    def aggregate_n(self) -> int | None: ...

    @property
    def role(self) -> str: ...

    @property
    def kind(self) -> str: ...


@dataclass(frozen=True)
class RecoveryConfig:
    """Declared constants of one component's model (none is fitted).

    ``penalty_modes`` are the Gamma(shape=2) modes of every level's SDs, one per
    parameter ``(level, excess[, transient], ln tau)``; ``robust_sd`` the robust
    mixture's broad component; ``noise_log_sd`` the log-SD of a new study's noise
    around its centre; ``noise_reference_n`` and ``max_noise_exponent`` define the
    group-mean noise scaling; ``weak_*`` the unpooled baseline prior.
    """

    model_id: str
    model_version: str
    quantity: str
    covariate_reference: float
    covariate_unit: str
    reference_months: float = DAY_MONTHS
    transient_months: float | None = 7.0 * DAY_MONTHS
    tau_grid: tuple[float, float, int] = (0.01, 120.0, 80)
    grouping: tuple[str, ...] = ("context",)
    penalty_modes: tuple[float, ...] = (0.1, 0.1, 0.1, 0.5)
    robust_sd: tuple[float, ...] = (0.3, 0.3, 0.3, 2.0)
    robust_weight: float = 0.2
    noise_floor: float = 0.01
    noise_log_sd: float = 0.5
    noise_reference_n: float = 40.0
    max_noise_exponent: float = 0.5
    min_exponent_studies: int = 4
    weak_sd: float = 1.0
    weak_log_tau_sd: float = 10.0
    information_floor: float = 0.05
    described_months: tuple[float, ...] = (DAY_MONTHS, 7 * DAY_MONTHS, 1.0, 6.0, 12.0)
    # Persistent deviations from the law: a fraction ``deviation_fraction`` of each
    # series' residual variance follows an Ornstein-Uhlenbeck process with correlation
    # time ``deviation_months``; the rest is independent noise. 0 = IOP 1.1.
    deviation_fraction: float = 0.0
    deviation_months: float = 3.0
    # False: the population (series summaries, REML) is fitted under working
    # independence and the deviation process acts in forecasting only.
    deviation_in_population_fit: bool = True

    def __post_init__(self) -> None:
        if self.grouping not in GROUPINGS:
            raise InvalidRequest(f"Grouping must be one of {sorted(GROUPINGS)}")
        if not (0.0 <= self.deviation_fraction < 1.0 and self.deviation_months > 0):
            raise InvalidRequest("Deviation fraction must lie in [0, 1) with a positive time")
        if len(self.penalty_modes) != self.dimensions or len(self.robust_sd) != self.dimensions:
            raise InvalidRequest("Penalty modes and robust SDs need one value per parameter")
        if not all(math.isfinite(v) and v > 0 for v in (*self.penalty_modes, *self.robust_sd)):
            raise InvalidRequest("Penalty modes and robust SDs must be positive")
        if not (math.isfinite(self.covariate_reference) and self.covariate_reference > 0):
            raise InvalidRequest("The covariate reference must be positive")
        if self.transient_months is not None and not self.transient_months > 0:
            raise InvalidRequest("The transient time constant must be positive")
        low, high, count = self.tau_grid
        if not (0 < low < high and count >= 2):
            raise InvalidRequest("The tau grid needs 0 < low < high and two points")
        if not 0.0 < self.robust_weight < 1.0:
            raise InvalidRequest("Robust mixture weight must lie strictly between 0 and 1")

    @property
    def linear(self) -> int:
        return 2 if self.transient_months is None else 3

    @property
    def dimensions(self) -> int:
        return self.linear + 1

    @property
    def parameters(self) -> tuple[str, ...]:
        middle = () if self.transient_months is None else ("transient_at_t1",)
        return ("level", "excess_at_t1", *middle, "log_tau_months")

    @property
    def levels(self) -> tuple[str, ...]:
        return (*self.grouping, *SERIES_LEVELS)

    @cached_property
    def log_tau_grid(self) -> FloatArray:
        low, high, count = self.tau_grid
        grid: FloatArray = np.log(np.geomspace(low, high, count))
        return grid

    @property
    def min_noise_follow_ups(self) -> int:
        return self.dimensions + 1

    @property
    def summary_jitter(self) -> FloatArray:
        return np.array([*(1e-6 for _ in range(self.linear)), 1e-4])

    def describe(self) -> dict[str, Any]:
        return {
            "model_id": self.model_id,
            "model_version": self.model_version,
            "quantity": self.quantity,
            "covariate": f"ln(baseline / {self.covariate_reference:g} {self.covariate_unit})",
            "reference_days": self.reference_months / DAY_MONTHS,
            "transient_days": (
                None if self.transient_months is None else self.transient_months / DAY_MONTHS
            ),
            "tau_grid_months": list(self.tau_grid),
            "grouping": list(self.grouping),
            "penalty_modes": list(self.penalty_modes),
            "robust_sd": list(self.robust_sd),
            "robust_weight": self.robust_weight,
            "noise_floor": self.noise_floor,
            "noise_log_sd": self.noise_log_sd,
            "noise_reference_n": self.noise_reference_n,
            "max_noise_exponent": self.max_noise_exponent,
            "deviation_fraction": self.deviation_fraction,
            "deviation_months": self.deviation_months,
            "deviation_in_population_fit": self.deviation_in_population_fit,
        }


# --- context parts ----------------------------------------------------------------


def family_of(context: str) -> str:
    """``<species>/all``: every context of a species shares one hierarchy."""
    species, sep, rest = context.partition("/")
    if not species or not sep or not rest:
        raise InvalidRequest(f"Context {context!r} must look like species/...")
    return f"{species}/all"


def treatment_of(context: str) -> str:
    """The treatment is the last part of the context (``human/dme/anti_vegf``)."""
    family_of(context)
    return context.rsplit("/", 1)[1]


def group_key(level: str, context: str) -> str:
    if level == "context":
        return context
    if level == "treatment":
        return treatment_of(context)
    raise InvalidRequest(f"Unknown grouping level {level!r}")


def level_covariate(config: RecoveryConfig, series: RecoverySeries) -> float:
    """``ln(baseline / reference)``; series without a baseline value get no prior."""
    value = series.reference_value
    if not math.isfinite(value) or value <= 0:
        raise InvalidRequest("Population priors need the baseline value")
    return math.log(value / config.covariate_reference)


# --- kinetic law -------------------------------------------------------------------


def recovery_ratio(
    config: RecoveryConfig, months: Sequence[float] | FloatArray, log_tau: FloatArray
) -> FloatArray:
    """``r(t)`` for every ``ln tau`` (rows) at ``months`` (columns); ``r(t1) = 1``."""
    t = np.asarray(months, dtype=np.float64)
    tau = np.exp(np.asarray(log_tau, dtype=np.float64))[:, None]
    result: FloatArray = np.exp(-(t[None, :] - config.reference_months) / tau)
    return result


def transient_ratio(config: RecoveryConfig, months: Sequence[float] | FloatArray) -> FloatArray:
    """``q(t)``, equal to 1 at the reference time."""
    if config.transient_months is None:
        raise InvalidRequest("This configuration has no transient term")
    t = np.asarray(months, dtype=np.float64)
    result: FloatArray = np.exp(-(t - config.reference_months) / config.transient_months)
    return result


def design(
    config: RecoveryConfig, months: Sequence[float] | FloatArray, log_tau: FloatArray
) -> FloatArray:
    """Columns ``1, r(t)[, q(t)]`` for every ``ln tau`` (shape: tau, months, linear)."""
    r = recovery_ratio(config, months, log_tau)
    columns = [np.ones_like(r), r]
    if config.transient_months is not None:
        columns.append(np.broadcast_to(transient_ratio(config, months), r.shape))
    result: FloatArray = np.stack(columns, axis=-1)
    return result


def law_fraction(
    config: RecoveryConfig,
    months: Sequence[float] | FloatArray,
    *,
    level: float,
    excess: float,
    tau_months: float,
    transient: float = 0.0,
) -> FloatArray:
    """Kinetic law for scalar parameters."""
    x = design(config, months, np.array([math.log(tau_months)]))[0]
    beta = [level, excess] if config.transient_months is None else [level, excess, transient]
    result: FloatArray = np.exp(x @ np.array(beta))
    return result


def best_fit_sum_squares(config: RecoveryConfig, series: RecoverySeries) -> float:
    """Least-squares residual sum of squares of ``ln f``, best over the ``tau`` grid."""
    y = np.log(np.asarray(series.fractions, dtype=np.float64))
    best = math.inf
    for x in design(config, series.months, config.log_tau_grid):
        beta, *_ = np.linalg.lstsq(x, y, rcond=None)
        residual = y - x @ beta
        best = min(best, float(residual @ residual))
    return best


# --- noise --------------------------------------------------------------------------

NoiseKey = tuple[str, str, str]  # (study, family, series kind)


@dataclass(frozen=True)
class NoiseEstimate:
    sd: float
    units: float | None  # mean unit count of the contributing group means


def study_noise(
    config: RecoveryConfig, series: Sequence[RecoverySeries]
) -> dict[NoiseKey, NoiseEstimate]:
    """Residual SD of ``ln f`` per (study, family, kind) from best fits of series with
    at least ``dimensions + 1`` follow-ups; others get no estimate."""
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
        estimates[key] = NoiseEstimate(
            sd=max(config.noise_floor, math.sqrt(sse / dof)),
            units=float(np.mean(units)) if units else None,
        )
    return estimates


@dataclass(frozen=True)
class NoiseCenters:
    """Median study SD per (family, kind) at the reference unit count, and the
    group-mean exponent ``g``."""

    centers: Mapping[tuple[str, str], float]
    exponent: float
    reference_n: float
    floor: float

    def center(self, family: str, kind: str, units: int | None) -> float:
        value = self.centers.get((family, kind), self.centers[("*", kind)])
        if kind == "aggregate" and units:
            value *= (units / self.reference_n) ** (-self.exponent)
        return max(self.floor, value)


def noise_exponent(config: RecoveryConfig, estimates: Mapping[NoiseKey, NoiseEstimate]) -> float:
    """Least-squares slope of ``-ln SD`` on ``ln(units)`` over group-mean studies,
    clipped to [0, max_noise_exponent]; 0 with too few studies."""
    points = [
        (math.log(e.units), math.log(e.sd))
        for (_, _, kind), e in estimates.items()
        if kind == "aggregate" and e.units
    ]
    if len(points) < config.min_exponent_studies:
        return 0.0
    x = np.array([p[0] for p in points])
    y = np.array([p[1] for p in points])
    if float(np.ptp(x)) <= 0:
        return 0.0
    slope = float(np.polyfit(x, y, 1)[0])
    return float(min(config.max_noise_exponent, max(0.0, -slope)))


def noise_centers(
    config: RecoveryConfig, estimates: Mapping[NoiseKey, NoiseEstimate]
) -> NoiseCenters:
    """Median per (family, kind) with at least two studies, else per kind across
    families with at least two, else the median of every estimate; group-mean SDs
    are first rescaled to the reference unit count."""
    if not estimates:
        raise InvalidRequest("Noise estimation needs a series with enough follow-ups")
    exponent = noise_exponent(config, estimates)

    def scaled(key: NoiseKey, estimate: NoiseEstimate) -> float:
        if key[2] == "aggregate" and estimate.units:
            return float(estimate.sd * (estimate.units / config.noise_reference_n) ** exponent)
        return estimate.sd

    values = {key: scaled(key, e) for key, e in estimates.items()}
    every = float(np.median(sorted(values.values())))
    families = sorted({family for _, family, _ in values})
    centers: dict[tuple[str, str], float] = {}
    for kind in ("aggregate", "individual"):
        by_kind = [v for (_, _, k), v in values.items() if k == kind]
        kind_center = float(np.median(by_kind)) if len(by_kind) >= 2 else every
        centers[("*", kind)] = max(config.noise_floor, kind_center)
        for family in families:
            chosen = [v for (_, f, k), v in values.items() if f == family and k == kind]
            value = float(np.median(chosen)) if len(chosen) >= 2 else kind_center
            centers[(family, kind)] = max(config.noise_floor, value)
    return NoiseCenters(
        centers=centers,
        exponent=exponent,
        reference_n=config.noise_reference_n,
        floor=config.noise_floor,
    )


# --- priors and exact mixture posteriors ----------------------------------------------


def robust(config: RecoveryConfig, prior: Prior, weight: float | None = None) -> Prior:
    """Mixture of ``prior`` and a broad component around its first mean."""
    weight = config.robust_weight if weight is None else weight
    if not 0.0 < weight < 1.0:
        raise InvalidRequest("Robust mixture weight must lie strictly between 0 and 1")
    broad = prior.covariances[0] + np.diag(np.asarray(config.robust_sd) ** 2)
    return Prior(
        weights=(*(w * (1.0 - weight) for w in prior.weights), weight),
        means=(*prior.means, prior.means[0]),
        covariances=(*prior.covariances, broad),
        flat_level=prior.flat_level,
    )


def weak_prior(config: RecoveryConfig) -> Prior:
    """Unpooled baseline: broad Gaussian on the linear part, nearly uniform ``ln tau``."""
    mean = np.array([*(0.0 for _ in range(config.linear)), float(np.mean(config.log_tau_grid))])
    covariance = np.diag(
        [*(config.weak_sd**2 for _ in range(config.linear)), config.weak_log_tau_sd**2]
    )
    return gaussian(mean, covariance)


@dataclass(frozen=True)
class Posterior:
    """Exact Gaussian mixture: per component, the linear part's mean and covariance
    given ``ln tau`` and the noise SD."""

    weights: FloatArray
    means: FloatArray  # (components, linear)
    covariances: FloatArray  # (components, linear, linear)
    log_tau: FloatArray
    sigmas: FloatArray
    # Observed months and log fractions, kept when deviations are persistent so that
    # forecasts can carry the current deviation forward.
    observed_months: FloatArray | None = None
    observed_logs: FloatArray | None = None


def deviation_correlation(
    config: RecoveryConfig,
    first: Sequence[float] | FloatArray,
    second: Sequence[float] | FloatArray,
) -> FloatArray:
    """``exp(-|t - t'| / deviation_months)`` between two sets of months."""
    a = np.asarray(first, dtype=np.float64)[:, None]
    b = np.asarray(second, dtype=np.float64)[None, :]
    result: FloatArray = np.exp(-np.abs(a - b) / config.deviation_months)
    return result


def residual_correlation(
    config: RecoveryConfig, months: Sequence[float] | FloatArray
) -> FloatArray:
    """Correlation of one series' residuals: ``(1 - phi) I + phi R``."""
    phi = config.deviation_fraction
    t = np.asarray(months, dtype=np.float64)
    result: FloatArray = (1.0 - phi) * np.eye(t.size) + phi * deviation_correlation(config, t, t)
    return result


def _noise_levels(noise: NoiseModel | float, floor: float) -> tuple[FloatArray, FloatArray]:
    if isinstance(noise, NoiseModel):
        sigmas, log_weights = noise.levels()
        return np.maximum(floor, sigmas), log_weights
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
    config: RecoveryConfig, mean: FloatArray, covariance: FloatArray, flat_level: bool
) -> tuple[FloatArray, FloatArray, float, FloatArray]:
    """Prior of the linear part given ``ln tau`` on the grid: means (tau, linear),
    precision (linear, linear), log-determinant of the proper part and log grid
    weights."""
    linear = config.linear
    grid = config.log_tau_grid
    var_u = float(covariance[linear, linear])
    u = grid - float(mean[linear])
    log_pi = _log_softmax(-0.5 * u**2 / var_u)
    first = 1 if flat_level else 0
    block = slice(first, linear)
    gain = covariance[block, linear] / var_u
    conditional = (
        covariance[block, block]
        - np.outer(covariance[block, linear], covariance[linear, block]) / var_u
    )
    sign, log_det = np.linalg.slogdet(conditional)
    if sign <= 0:
        raise InvalidRequest("Prior covariance must be positive definite")
    means = np.zeros((grid.size, linear))
    means[:, block] = mean[block][None, :] + u[:, None] * gain[None, :]
    precision = np.zeros((linear, linear))
    precision[block, block] = np.linalg.inv(conditional)
    return means, precision, float(log_det), log_pi


def posterior(
    config: RecoveryConfig,
    prior: Prior,
    *,
    series: RecoverySeries | None,
    noise: NoiseModel | float,
) -> Posterior:
    """Prior times the likelihood of ``series`` (a prefix; ``None`` = prior only)."""
    linear = config.linear
    grid = config.log_tau_grid
    if any(len(m) != config.dimensions for m in prior.means):
        raise InvalidRequest("Prior dimension does not match the configuration")
    sigmas, log_noise = _noise_levels(noise, config.noise_floor)
    inverse_variance = 1.0 / sigmas**2
    observed: tuple[FloatArray, FloatArray] | None = None
    log_det_correlation = 0.0
    if series is None:
        if prior.flat_level:
            raise InvalidRequest("An anchored prior needs at least one observation")
        n = 0
        xtx = np.zeros((grid.size, linear, linear))
        xtz = np.zeros((grid.size, linear))
        zz = 0.0
    else:
        z = np.log(np.asarray(series.fractions, dtype=np.float64))
        x = design(config, series.months, grid)
        n = z.size
        if config.deviation_fraction > 0:
            # Whiten by the residual correlation, so that every component below sees
            # independent residuals with the noise SD.
            observed = (np.asarray(series.months, dtype=np.float64), z.copy())
            chol = np.linalg.cholesky(residual_correlation(config, series.months))
            log_det_correlation = 2.0 * float(np.sum(np.log(np.diag(chol))))
            x = np.linalg.solve(chol[None], x)
            z = np.linalg.solve(chol, z)
        xtx = np.einsum("kna,knb->kab", x, x)
        xtz = np.einsum("kna,n->ka", x, z)
        zz = float(z @ z)
    blocks = []
    for weight, mean, covariance in zip(prior.weights, prior.means, prior.covariances, strict=True):
        mean0, precision0, log_det_prior, log_pi = _conditional_prior(
            config, np.asarray(mean), np.asarray(covariance), prior.flat_level
        )
        b_prior = mean0 @ precision0.T  # (tau, linear)
        quad_prior = np.einsum("ka,ab,kb->k", mean0, precision0, mean0)
        precision = (
            precision0[None, None] + xtx[:, None] * inverse_variance[None, :, None, None]
        )  # (tau, sigma, linear, linear)
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
        if observed is not None:
            log_marginal = log_marginal - 0.5 * log_det_correlation
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
    weights = _log_normalize(np.concatenate([b[0] for b in blocks]))
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
    )


def moments(fitted: Posterior) -> tuple[FloatArray, FloatArray]:
    """Mean and covariance of ``theta = (level, excess[, transient], ln tau)``."""
    linear = fitted.means.shape[1]
    w = fitted.weights
    points = np.column_stack([fitted.means, fitted.log_tau])
    mean = w @ points
    centered = points - mean
    covariance = (centered * w[:, None]).T @ centered
    covariance[:linear, :linear] += np.einsum("m,mab->ab", w, fitted.covariances)
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
    config: RecoveryConfig,
    fitted: Posterior,
    *,
    months: Sequence[float],
    quantiles: Sequence[float] = (0.05, 0.1, 0.5, 0.9, 0.95),
) -> dict[str, Any]:
    """Point forecast (median of the observation's posterior predictive distribution),
    posterior mean of the model curve and observation quantiles (exact mixture)."""
    if not months or any(not math.isfinite(m) or m <= 0 for m in months):
        raise InvalidRequest("Prediction months must be finite and positive")
    x = design(config, months, fitted.log_tau)  # (components, months, linear)
    w = fitted.weights
    if fitted.observed_months is None or fitted.observed_logs is None:
        mean_log = np.einsum("kta,ka->kt", x, fitted.means)
        variance = np.einsum("kta,kab,ktb->kt", x, fitted.covariances, x)
        mean_fraction = w @ np.exp(mean_log + 0.5 * variance)
        spread = np.sqrt(variance + fitted.sigmas[:, None] ** 2)
    else:
        # Persistent deviations: the target's deviation is conditioned on the observed
        # residuals. With M the residual correlation and r the correlation of target
        # and observed deviations, g = phi M^-1 r; the law's design becomes x - X'g,
        # the observations add z'g, and the noise variance shrinks by phi r'g.
        phi = config.deviation_fraction
        cross = deviation_correlation(config, fitted.observed_months, months)  # (n, months)
        gain = phi * np.linalg.solve(
            residual_correlation(config, fitted.observed_months), cross
        )  # (n, months)
        observed_design = design(config, fitted.observed_months, fitted.log_tau)
        a = x - np.einsum("kna,nt->kta", observed_design, gain)
        mean_log = np.einsum("kta,ka->kt", a, fitted.means) + (fitted.observed_logs @ gain)
        variance = np.einsum("kta,kab,ktb->kt", a, fitted.covariances, a)
        explained = phi * np.einsum("nt,nt->t", cross, gain)  # phi^2 r'M^-1 r
        latent = variance + fitted.sigmas[:, None] ** 2 * (phi - explained)[None, :]
        mean_fraction = w @ np.exp(mean_log + 0.5 * latent)
        spread = np.sqrt(variance + fitted.sigmas[:, None] ** 2 * (1.0 - explained)[None, :])
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
        "posterior_mean": dict(zip(config.parameters, (float(v) for v in mean), strict=True)),
    }


# --- per-series likelihood summaries --------------------------------------------------


def weak_prior_moments(config: RecoveryConfig) -> tuple[FloatArray, FloatArray]:
    """Mean and covariance of the weak prior as it acts on the ``tau`` grid."""
    prior = weak_prior(config)
    linear = config.linear
    grid = config.log_tau_grid
    mean, covariance = prior.means[0], prior.covariances[0]
    weights = _log_normalize(-0.5 * (grid - mean[linear]) ** 2 / covariance[linear, linear])
    mean_u = float(weights @ grid)
    var_u = float(weights @ (grid - mean_u) ** 2)
    return (
        np.array([*mean[:linear], mean_u]),
        np.diag([*np.diag(covariance)[:linear], var_u]),
    )


def series_summary(config: RecoveryConfig, series: RecoverySeries, sigma: float) -> dict[str, Any]:
    """Gaussian likelihood summary of ``theta`` for one series (fixed noise SD).

    Posterior moments under the weak prior, with the weak prior's precision and
    information subtracted in coordinates standardized by its SDs. Directions with
    less than ``information_floor`` weak-prior precisions keep the posterior mean's
    component and get variance ``1 / information_floor`` weak-prior variances.
    """
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


# --- penalized REML of the hierarchy --------------------------------------------------


def _gamma2_log_penalty(sd: FloatArray, mode: FloatArray) -> float:
    return float(np.sum(np.log(sd) - sd / mode))


def _diagonal(log_sd: FloatArray) -> FloatArray:
    result: FloatArray = np.diag(np.exp(2.0 * log_sd))
    return result


def _fixed_design(d: int, covariate: float) -> FloatArray:
    matrix = np.zeros((d, d + 1))
    matrix[:, :d] = np.eye(d)
    matrix[0, d] = covariate
    return matrix


@dataclass(frozen=True)
class _Block:
    members: tuple[int, ...]
    y: FloatArray
    fixed_design: FloatArray
    same_group: tuple[FloatArray, ...]  # per grouping level
    same_study: FloatArray
    series_cov: tuple[FloatArray, ...]
    aggregate: tuple[bool, ...]


def _blocks(config: RecoveryConfig, summaries: Sequence[Mapping[str, Any]]) -> list[_Block]:
    """Connected components of summaries sharing their top grouping key or a study,
    ordered by their smallest top key (one block per context in IOP-like data)."""
    top = config.grouping[0]
    parent = list(range(len(summaries)))

    def find(i: int) -> int:
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i

    first_of: dict[tuple[str, str], int] = {}
    for i, m in enumerate(summaries):
        for key in (("top", group_key(top, m["context"])), ("study", m["study_id"])):
            if key in first_of:
                parent[find(i)] = find(first_of[key])
            else:
                first_of[key] = i
    components: dict[int, list[int]] = {}
    for i in range(len(summaries)):
        components.setdefault(find(i), []).append(i)
    ordered = sorted(
        components.values(),
        key=lambda idx: min(group_key(top, summaries[i]["context"]) for i in idx),
    )
    d = config.dimensions
    blocks = []
    for idx in ordered:
        members = [summaries[i] for i in idx]
        same_group = tuple(
            np.array(
                [
                    [
                        float(group_key(level, a["context"]) == group_key(level, b["context"]))
                        for b in members
                    ]
                    for a in members
                ]
            )
            for level in config.grouping
        )
        blocks.append(
            _Block(
                members=tuple(idx),
                y=np.concatenate([m["mean"] for m in members]),
                fixed_design=np.concatenate([_fixed_design(d, m["covariate"]) for m in members]),
                same_group=same_group,
                same_study=np.array(
                    [[float(a["study_id"] == b["study_id"]) for b in members] for a in members]
                ),
                series_cov=tuple(m["covariance"] for m in members),
                aggregate=tuple(m["kind"] == "aggregate" for m in members),
            )
        )
    return blocks


@dataclass(frozen=True)
class FamilyFit:
    family: str
    grouping: tuple[str, ...]
    fixed: FloatArray  # species mean of theta and the level slope on x
    variances: Mapping[str, FloatArray]  # per hierarchy level
    # (level, group) -> offset in unknowns
    effect_index: Mapping[tuple[str, str], int]
    solution: FloatArray  # fixed effects then grouping-level effects
    covariance: FloatArray
    studies: tuple[str, ...]
    series: int
    objective: float

    @property
    def fixed_covariance(self) -> FloatArray:
        n = self.fixed.size
        result: FloatArray = self.covariance[:n, :n]
        return result


def fit_family(
    config: RecoveryConfig,
    family: str,
    summaries: Sequence[Mapping[str, Any]],
    modes: FloatArray | None = None,
) -> FamilyFit:
    """Penalized REML of diagonal grouping/study/series covariances with a fixed level
    slope on the baseline covariate, then the joint BLUE/BLUP of fixed and grouping
    effects given those covariances."""
    d = config.dimensions
    fixed_n = d + 1
    levels = config.levels
    groups = len(config.grouping)
    penalty = np.asarray(config.penalty_modes if modes is None else modes, dtype=np.float64)
    blocks = _blocks(config, summaries)

    def residual_covariance(params: FloatArray, block: _Block) -> FloatArray:
        study = _diagonal(params[groups * d : (groups + 1) * d])
        group_mean = _diagonal(params[(groups + 1) * d : (groups + 2) * d])
        single = _diagonal(params[(groups + 2) * d : (groups + 3) * d])
        matrix = cast(FloatArray, np.kron(block.same_study, study))
        for row in range(len(block.members)):
            deviation = group_mean if block.aggregate[row] else single
            matrix[d * row : d * row + d, d * row : d * row + d] += (
                deviation + block.series_cov[row]
            )
        return matrix

    def marginal(params: FloatArray, block: _Block) -> FloatArray:
        matrix = residual_covariance(params, block)
        for level in range(groups):
            matrix += cast(
                FloatArray,
                np.kron(block.same_group[level], _diagonal(params[level * d : level * d + d])),
            )
        return matrix

    def objective(params: FloatArray) -> float:
        if np.any(np.abs(params) > 12.0):
            return 1e12
        a_total = np.zeros((fixed_n, fixed_n))
        b_total = np.zeros(fixed_n)
        pieces = []
        for block in blocks:
            try:
                chol = np.linalg.cholesky(marginal(params, block))
            except np.linalg.LinAlgError:
                return 1e12
            inv_design = np.linalg.solve(chol, block.fixed_design)
            inv_y = np.linalg.solve(chol, block.y)
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
        for level in range(len(levels)):
            value -= _gamma2_log_penalty(np.exp(params[d * level : d * level + d]), penalty)
        return value

    best, _ = nelder_mead(
        objective, np.tile(np.log(penalty), len(levels)), iterations=REML_ITERATIONS
    )
    for _ in range(REML_RESTARTS):  # deterministic restarts from the incumbent
        best, _ = nelder_mead(objective, best, step=0.2, iterations=REML_ITERATIONS)
    value = objective(best)
    if value >= 1e12:
        raise InvalidRequest(f"Population fit of {family!r} did not converge")
    # Joint Gaussian posterior of the fixed effects (flat prior) and grouping effects
    # (N(0, V_level)) given the fitted variances: exact BLUE/BLUP.
    effect_index: dict[tuple[str, str], int] = {}
    offset = fixed_n
    for level in config.grouping:
        for key in sorted({group_key(level, m["context"]) for m in summaries}):
            effect_index[(level, key)] = offset
            offset += d
    unknowns = offset
    precision = np.zeros((unknowns, unknowns))
    rhs = np.zeros(unknowns)
    for block in blocks:
        n = len(block.members)
        matrix = np.zeros((d * n, unknowns))
        matrix[:, :fixed_n] = block.fixed_design
        for row, index in enumerate(block.members):
            context = summaries[index]["context"]
            for level in config.grouping:
                start = effect_index[(level, group_key(level, context))]
                matrix[d * row : d * row + d, start : start + d] = np.eye(d)
        inverse = np.linalg.inv(residual_covariance(best, block))
        precision += matrix.T @ inverse @ matrix
        rhs += matrix.T @ inverse @ block.y
    for (level, _), start in effect_index.items():
        position = config.grouping.index(level)
        group_precision = np.linalg.inv(_diagonal(best[position * d : position * d + d]))
        precision[start : start + d, start : start + d] += group_precision
    covariance = np.linalg.inv(precision)
    solution = covariance @ rhs
    return FamilyFit(
        family=family,
        grouping=config.grouping,
        fixed=solution[:fixed_n],
        variances={level: _diagonal(best[d * i : d * i + d]) for i, level in enumerate(levels)},
        effect_index=effect_index,
        solution=solution,
        covariance=covariance,
        studies=tuple(sorted({m["study_id"] for m in summaries})),
        series=len(summaries),
        objective=float(value),
    )


@dataclass(frozen=True)
class RecoveryFit:
    config: RecoveryConfig
    families: Mapping[str, FamilyFit]
    noise_centers: NoiseCenters
    study_noise_sd: Mapping[str, float]
    training_series: tuple[str, ...]
    training_studies: tuple[str, ...]
    unfitted_families: Mapping[str, str] = field(default_factory=dict)

    def _family(self, context: str) -> FamilyFit:
        family = family_of(context)
        if family not in self.families:
            reason = self.unfitted_families.get(family, "no training study")
            raise InvalidRequest(f"No population prior for family {family!r}: {reason}")
        return self.families[family]

    def _known(self, fitted: FamilyFit, context: str) -> list[tuple[str, int]]:
        found = []
        for level in fitted.grouping:
            start = fitted.effect_index.get((level, group_key(level, context)))
            if start is not None:
                found.append((level, start))
        return found

    def level_used(self, context: str) -> str:
        """The most specific trained grouping level the prior of ``context`` uses."""
        fitted = self._family(context)
        known = dict(self._known(fitted, context))
        if "context" in known:
            return context
        if "treatment" in known:
            return f"treatment:{treatment_of(context)}"
        return f"family:{fitted.family}"

    def group_location(self, family: str, level: str, key: str) -> tuple[FloatArray, FloatArray]:
        """Species mean plus one trained group's effect (and, for a context, the
        effect of its treatment when that level exists), at covariate 0."""
        fitted = self.families[family]
        if level == "context":
            return self.location(key, 0.0)
        d = self.config.dimensions
        row = np.zeros((d, fitted.solution.size))
        row[:, :d] = np.eye(d)
        start = fitted.effect_index[(level, key)]
        row[:, start : start + d] = np.eye(d)
        return row @ fitted.solution, row @ fitted.covariance @ row.T

    def location(self, context: str, covariate: float) -> tuple[FloatArray, FloatArray]:
        """Mean and covariance of the context-level ``theta`` at covariate ``x``."""
        fitted = self._family(context)
        d = self.config.dimensions
        row = np.zeros((d, fitted.solution.size))
        row[:, :d] = np.eye(d)
        row[0, d] = covariate
        known = self._known(fitted, context)
        for _, start in known:
            row[:, start : start + d] = np.eye(d)
        mean = row @ fitted.solution
        covariance = row @ fitted.covariance @ row.T
        for level in fitted.grouping:
            if level not in {name for name, _ in known}:
                covariance = covariance + fitted.variances[level]
        return mean, covariance

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

    def noise(self, kind: str, context: str, units: int | None = None) -> NoiseModel:
        center = self.noise_centers.center(family_of(context), kind, units)
        return NoiseModel(center=center, log_sd=self.config.noise_log_sd)

    def describe(self) -> dict[str, Any]:
        config = self.config
        d = config.dimensions

        def text(mean: FloatArray, covariance: FloatArray) -> dict[str, Any]:
            sd = np.sqrt(np.diag(covariance))
            transient = 0.0 if config.transient_months is None else float(mean[2])
            curve = law_fraction(
                config,
                config.described_months,
                level=float(mean[0]),
                excess=float(mean[1]),
                transient=transient,
                tau_months=math.exp(mean[config.linear]),
            )
            return {
                "long_term_fraction_at_reference": math.exp(mean[0]),
                "parameters": dict(zip(config.parameters, mean.tolist(), strict=True)),
                "median_tau_days": math.exp(mean[config.linear]) / DAY_MONTHS,
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
                "level_slope_on_ln_baseline": float(fitted.fixed[d]),
                "level_slope_sd": float(math.sqrt(fixed_cov[d, d])),
                "elasticity": float(1.0 + fitted.fixed[d]),
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
    config: RecoveryConfig,
    series: Sequence[RecoverySeries],
    *,
    noise_series: Sequence[RecoverySeries] | None = None,
) -> RecoveryFit:
    """Fit one hierarchy per species to control series of several studies.

    ``noise_series`` (default: the controls) supplies study noise estimates; treated
    series may be added because noise belongs to the measurement. Series without a
    baseline value are skipped. A family with fewer than two training studies gets no
    population prior.
    """
    if not 2 <= len(series) <= MAX_SERIES:
        raise InvalidRequest("Population fitting needs 2..5000 series")
    if len({s.series_id for s in series}) != len(series):
        raise InvalidRequest("Population fitting requires unique series identities")
    if any(s.role != "control" for s in series):
        raise InvalidRequest("Population priors are fitted to control series only")
    estimates = study_noise(config, noise_series if noise_series is not None else series)
    centers = noise_centers(config, estimates)
    summary_config = (
        config if config.deviation_in_population_fit else replace(config, deviation_fraction=0.0)
    )
    summaries = []
    for item in series:
        if item.reference_value <= 0:
            continue
        family = family_of(item.context)
        own = estimates.get((item.study_id, family, item.kind))
        sigma = own.sd if own is not None else centers.center(family, item.kind, item.aggregate_n)
        summaries.append({**series_summary(summary_config, item, sigma), "family": family})
    fitted: dict[str, FamilyFit] = {}
    unfitted: dict[str, str] = {}
    for family in sorted({m["family"] for m in summaries}):
        members = [m for m in summaries if m["family"] == family]
        if len({m["study_id"] for m in members}) < 2:
            unfitted[family] = "fewer than two training studies"
            continue
        fitted[family] = fit_family(config, family, members)
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
    )
