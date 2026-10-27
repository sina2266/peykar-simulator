"""Hierarchical population model of corneal epithelial wound closure (numpy only).

Kinematics: after a latent lag the projected wound radius decreases at a constant
physical speed (Crosson et al. 1986, rabbit, size-independent lag and speed):

``rho(t) = max(0, 1 - v * max(0, t - L) / r0)`` and ``area_fraction = rho**2``.

Observation model (declared before sealed-test evaluation):

* noise is Gaussian in normalized radius ``sqrt(area_fraction)`` with a study-level
  SD; a new study's SD is unknown and marginalized over a log-normal prior centred on
  the median SD of training studies;
* a reported area of exactly 0 is left-censored: the residual defect may be any size
  below a detection radius of 125 µm (a ~0.25 mm pinpoint defect), not exactly zero.

Each series (one animal/eye, or one published group mean) is summarized on a fixed
``(ln v, L)`` grid. A nested normal random-effects model (series within study within
context) pools those summaries by restricted maximum likelihood with weakly
informative, boundary-avoiding variance penalties. Predictions are population draws
(initial state only) or exact grid posteriors that condition on an observed prefix.
Nothing here reads files, registers models or claims biological validity.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Mapping, Sequence
from dataclasses import dataclass, field
from typing import Any

import numpy as np
from numpy.typing import NDArray

from peykar_core.errors import InvalidRequest

MODEL_ID = "cornea.population-radius-lag"
MODEL_VERSION = "1.0.0"
FloatArray = NDArray[np.float64]

SPEED_GRID_UM_H: FloatArray = np.geomspace(2.0, 400.0, 121)
LAG_GRID_H: FloatArray = np.arange(0.0, 60.0 + 1e-9, 0.25)
_LOG_SPEED, _LAG = np.meshgrid(np.log(SPEED_GRID_UM_H), LAG_GRID_H, indexing="ij")
GRID_LOG_SPEED: FloatArray = _LOG_SPEED.ravel()
GRID_LAG: FloatArray = _LAG.ravel()
GRID_SPEED: FloatArray = np.exp(GRID_LOG_SPEED)

# Declared constants (not fitted): Gamma(shape=2) variance-penalty modes, detection
# radius for reported zero areas, noise floor and spread of a new study's noise level.
PENALTY_MODES = {"log_speed_sd": 0.2, "lag_sd_h": 5.0}
DETECTION_RADIUS_UM = 125.0
NOISE_FLOOR = 0.005
NOISE_LOG_SD = 1.0
NOISE_LEVEL_Z: FloatArray = np.linspace(-2.5, 2.5, 11)
MAX_SERIES = 5_000
MAX_POINTS = 200


@dataclass(frozen=True)
class WoundSeries:
    """One longitudinal projected wound-area series normalized to its own hour-0 area.

    ``hours``/``fractions`` exclude the hour-0 normalizer. ``aggregate_n`` is
    ``None`` for one animal/eye, otherwise the reported group size (0 if unknown).
    """

    series_id: str
    study_id: str
    context: str
    radius0_um: float
    hours: tuple[float, ...]
    fractions: tuple[float, ...]
    aggregate_n: int | None = None
    role: str = "control"
    group: str = ""
    baseline_observed: bool = True
    metadata: Mapping[str, Any] = field(default_factory=dict)

    def __post_init__(self) -> None:
        if not self.series_id or not self.study_id or not self.context:
            raise InvalidRequest("Wound series require series, study and context identities")
        if not math.isfinite(self.radius0_um) or not 50.0 <= self.radius0_um <= 10_000.0:
            raise InvalidRequest("Initial wound radius must be within 50..10000 µm")
        if len(self.hours) != len(self.fractions) or not 1 <= len(self.hours) <= MAX_POINTS:
            raise InvalidRequest("Wound series need 1..200 paired nonbaseline observations")
        previous = 0.0
        for hour, fraction in zip(self.hours, self.fractions, strict=True):
            if not math.isfinite(hour) or hour <= previous:
                raise InvalidRequest("Observation hours must be finite, positive and increasing")
            if not math.isfinite(fraction) or not 0.0 <= fraction <= 3.0:
                raise InvalidRequest("Area fractions must be finite within 0..3")
            previous = hour
        if self.aggregate_n is not None and self.aggregate_n < 0:
            raise InvalidRequest("Aggregate group size must be nonnegative")
        if self.role not in {"control", "treated"}:
            raise InvalidRequest("Series role must be control or treated")

    @property
    def is_aggregate(self) -> bool:
        return self.aggregate_n is not None

    @property
    def kind(self) -> str:
        return "aggregate" if self.is_aggregate else "individual"

    @property
    def effective_n(self) -> int:
        """Animals behind the series; unknown aggregates count conservatively as one."""
        return max(1, self.aggregate_n or 1)

    @property
    def detection_rho(self) -> float:
        return min(0.5, DETECTION_RADIUS_UM / self.radius0_um)

    def prefix(self, last_hour: float) -> WoundSeries:
        """Return the causal prefix ending at ``last_hour`` (inclusive)."""
        keep = [i for i, hour in enumerate(self.hours) if hour <= last_hour]
        if not keep:
            raise InvalidRequest("A causal prefix needs at least one nonbaseline observation")
        return WoundSeries(
            series_id=self.series_id,
            study_id=self.study_id,
            context=self.context,
            radius0_um=self.radius0_um,
            hours=tuple(self.hours[i] for i in keep),
            fractions=tuple(self.fractions[i] for i in keep),
            aggregate_n=self.aggregate_n,
            role=self.role,
            group=self.group,
            baseline_observed=self.baseline_observed,
            metadata=self.metadata,
        )


def normalized_radius(
    hours: Sequence[float] | FloatArray,
    *,
    speed_um_h: float | FloatArray,
    lag_h: float | FloatArray,
    radius0_um: float,
) -> FloatArray:
    """Radius law evaluated for scalar parameters or every grid point (rows)."""
    t = np.asarray(hours, dtype=np.float64)
    speed = np.atleast_1d(np.asarray(speed_um_h, dtype=np.float64))
    lag = np.atleast_1d(np.asarray(lag_h, dtype=np.float64))
    moved = speed[:, None] * np.maximum(0.0, t[None, :] - lag[:, None])
    result: FloatArray = np.maximum(0.0, 1.0 - moved / radius0_um)
    return result


def _log_norm_cdf(x: FloatArray) -> FloatArray:
    """log Φ(x) with an Abramowitz–Stegun 7.1.26 erf (|error| < 1.5e-7), stable tails."""
    z = np.clip(x, -37.0, 37.0) / math.sqrt(2.0)
    a = np.abs(z)
    t = 1.0 / (1.0 + 0.3275911 * a)
    poly = t * (
        0.254829592 + t * (-0.284496736 + t * (1.421413741 + t * (-1.453152027 + t * 1.061405429)))
    )
    erfc_abs = poly * np.exp(-a * a)  # erfc(|z|)
    lower_tail = np.log(np.maximum(0.5 * erfc_abs, 1e-300))  # log Φ(-|x|)
    upper = np.log1p(-0.5 * erfc_abs)  # log Φ(|x|)
    result: FloatArray = np.where(z < 0, lower_tail, upper)
    return result


def grid_log_likelihood(series: WoundSeries, sigma: float) -> FloatArray:
    """Log likelihood of every grid point; zeros are censored below detection."""
    if not math.isfinite(sigma) or sigma <= 0:
        raise InvalidRequest("Noise SD must be finite and positive")
    fractions = np.asarray(series.fractions, dtype=np.float64)
    model = normalized_radius(
        series.hours, speed_um_h=GRID_SPEED, lag_h=GRID_LAG, radius0_um=series.radius0_um
    )
    positive = fractions > 0.0
    log_likelihood = np.zeros(GRID_LAG.size)
    if np.any(positive):
        observed = np.sqrt(fractions[positive])
        residual = (model[:, positive] - observed[None, :]) / sigma
        log_likelihood += np.sum(-0.5 * residual**2, axis=1) - positive.sum() * math.log(sigma)
    if np.any(~positive):
        bound = (series.detection_rho - model[:, ~positive]) / sigma
        log_likelihood += np.sum(_log_norm_cdf(bound), axis=1)
    result: FloatArray = log_likelihood
    return result


def _normalize(log_weights: FloatArray) -> FloatArray:
    peak = float(np.max(log_weights))
    weights = np.exp(log_weights - peak)
    total = float(np.sum(weights))
    if not math.isfinite(total) or total <= 0:
        raise InvalidRequest("Grid posterior is degenerate")
    normalized: FloatArray = weights / total
    return normalized


def study_noise(series: Sequence[WoundSeries]) -> dict[tuple[str, str], float]:
    """Residual SD per (study, series kind) from best grid fits of uncensored points.

    Two passes: fit with a provisional SD, then refit with the study's estimate.
    Studies without any series of >2 uncensored points get no estimate.
    """
    estimates: dict[tuple[str, str], float] = {}
    groups: dict[tuple[str, str], list[WoundSeries]] = {}
    for item in series:
        groups.setdefault((item.study_id, item.kind), []).append(item)
    for key, members in groups.items():
        sigma = 0.03
        estimated = False
        for _ in range(2):
            sse = 0.0
            dof = 0
            for item in members:
                mask = np.asarray(item.fractions) > 0.0
                if int(mask.sum()) <= 2:
                    continue
                best = int(np.argmax(grid_log_likelihood(item, sigma)))
                model = normalized_radius(
                    item.hours,
                    speed_um_h=GRID_SPEED[best],
                    lag_h=GRID_LAG[best],
                    radius0_um=item.radius0_um,
                )[0]
                observed = np.sqrt(np.asarray(item.fractions))
                sse += float(np.sum((model[mask] - observed[mask]) ** 2))
                dof += int(mask.sum()) - 2
            if dof <= 0:
                break
            sigma = max(NOISE_FLOOR, math.sqrt(sse / dof))
            estimated = True
        if estimated:
            estimates[key] = sigma
    return estimates


def noise_centers(estimates: Mapping[tuple[str, str], float]) -> dict[str, float]:
    """Robust prior centres for a new study's noise SD.

    Individual series use the median over all training studies of both kinds unless
    at least three individual-data studies exist, so one coarse study cannot set the
    centre. Aggregates use the median of aggregate studies when available.
    """
    if not estimates:
        raise InvalidRequest("Noise estimation needs at least one series with >2 observations")
    every = float(np.median(sorted(estimates.values())))
    individual = sorted(v for (_, k), v in estimates.items() if k == "individual")
    aggregate = sorted(v for (_, k), v in estimates.items() if k == "aggregate")
    centers = {
        "individual": float(np.median(individual)) if len(individual) >= 3 else every,
        "aggregate": float(np.median(aggregate)) if aggregate else every,
    }
    return {kind: max(NOISE_FLOOR, value) for kind, value in centers.items()}


def series_summary(series: WoundSeries, sigma: float) -> dict[str, Any]:
    """Flat-prior grid posterior moments of ``(ln v, L)`` for one series."""
    weights = _normalize(grid_log_likelihood(series, sigma))
    points = np.stack([GRID_LOG_SPEED, GRID_LAG], axis=1)
    mean = weights @ points
    centered = points - mean
    covariance = (centered * weights[:, None]).T @ centered
    best = int(np.argmax(weights))
    return {
        "series_id": series.series_id,
        "study_id": series.study_id,
        "context": series.context,
        "mean": mean,
        "covariance": covariance + np.diag([1e-6, 1e-4]),
        "effective_n": series.effective_n,
        "is_aggregate": series.is_aggregate,
        "map_speed_um_h": float(GRID_SPEED[best]),
        "map_lag_h": float(GRID_LAG[best]),
        "posterior_sd_log_speed": float(math.sqrt(covariance[0, 0])),
        "posterior_sd_lag_h": float(math.sqrt(covariance[1, 1])),
    }


def _covariance(params: FloatArray) -> FloatArray:
    """Two log-SDs plus optional correlation logit; two entries mean a diagonal matrix."""
    sd = np.exp(params[:2])
    correlation = math.tanh(float(params[2])) * 0.95 if params.size > 2 else 0.0
    return np.array(
        [[sd[0] ** 2, correlation * sd[0] * sd[1]], [correlation * sd[0] * sd[1], sd[1] ** 2]]
    )


def _gamma2_log_penalty(sd: float, mode: float) -> float:
    # log Gamma(shape=2, rate=1/mode) density up to a constant: log(sd) - sd/mode.
    return math.log(sd) - sd / mode


def nelder_mead(
    function: Callable[[FloatArray], float],
    start: FloatArray,
    *,
    step: float = 0.4,
    iterations: int = 4000,
    tolerance: float = 1e-9,
) -> tuple[FloatArray, float]:
    """Deterministic Nelder–Mead minimizer (no randomness, bounded iterations)."""
    dimension = start.size
    simplex = [start.astype(np.float64)]
    for index in range(dimension):
        vertex = start.astype(np.float64).copy()
        vertex[index] += step
        simplex.append(vertex)
    values = [function(v) for v in simplex]
    for _ in range(iterations):
        order = np.argsort(values)
        simplex = [simplex[i] for i in order]
        values = [values[i] for i in order]
        if abs(values[-1] - values[0]) <= tolerance * (1.0 + abs(values[0])):
            break
        centroid = np.mean(simplex[:-1], axis=0)
        reflected = centroid + (centroid - simplex[-1])
        value = function(reflected)
        if value < values[0]:
            expanded = centroid + 2.0 * (centroid - simplex[-1])
            expanded_value = function(expanded)
            if expanded_value < value:
                simplex[-1], values[-1] = expanded, expanded_value
            else:
                simplex[-1], values[-1] = reflected, value
        elif value < values[-2]:
            simplex[-1], values[-1] = reflected, value
        else:
            contracted = centroid + 0.5 * (simplex[-1] - centroid)
            contracted_value = function(contracted)
            if contracted_value < values[-1]:
                simplex[-1], values[-1] = contracted, contracted_value
            else:
                best = simplex[0]
                simplex = [best + 0.5 * (v - best) for v in simplex]
                values = [function(v) for v in simplex]
    index = int(np.argmin(values))
    return simplex[index], float(values[index])


@dataclass(frozen=True)
class NoiseModel:
    """Unknown study noise SD: log-normal prior, marginalized on fixed levels."""

    center: float
    log_sd: float = NOISE_LOG_SD

    def levels(self) -> tuple[FloatArray, FloatArray]:
        sigmas = self.center * np.exp(self.log_sd * NOISE_LEVEL_Z)
        log_weights = -0.5 * NOISE_LEVEL_Z**2
        return np.maximum(NOISE_FLOOR, sigmas), log_weights - np.log(np.exp(log_weights).sum())


@dataclass(frozen=True)
class PopulationFit:
    contexts: tuple[str, ...]
    context_means: Mapping[str, FloatArray]
    context_mean_covariance: Mapping[str, FloatArray]
    within_study: FloatArray
    between_study: FloatArray
    noise_sd: Mapping[str, float]
    study_noise_sd: Mapping[str, float]
    training_series: tuple[str, ...]
    training_studies: tuple[str, ...]
    objective: float

    def prior(
        self, context: str, *, effective_n: int = 1, new_study: bool = True
    ) -> dict[str, Any]:
        """Predictive distribution of ``(ln v, L)`` for one new series."""
        if context not in self.context_means:
            raise InvalidRequest(f"Population fit has no training data for context {context!r}")
        covariance = self.within_study / max(1, effective_n)
        if new_study:
            covariance = covariance + self.between_study + self.context_mean_covariance[context]
        return {"mean": self.context_means[context], "covariance": covariance}

    def group_prior(self, context: str) -> dict[str, Any]:
        """Prior of a new experimental group's mean ``(ln v, L)`` (no animal term)."""
        if context not in self.context_means:
            raise InvalidRequest(f"Population fit has no training data for context {context!r}")
        return {
            "mean": self.context_means[context],
            "covariance": self.between_study + self.context_mean_covariance[context],
        }

    def noise(self, kind: str) -> NoiseModel:
        return NoiseModel(center=self.noise_sd[kind])

    def describe(self) -> dict[str, Any]:
        def sd(matrix: FloatArray) -> dict[str, float]:
            return {
                "log_speed_sd": float(math.sqrt(matrix[0, 0])),
                "lag_sd_h": float(math.sqrt(matrix[1, 1])),
                "correlation": float(matrix[0, 1] / math.sqrt(matrix[0, 0] * matrix[1, 1])),
            }

        return {
            "model_id": MODEL_ID,
            "model_version": MODEL_VERSION,
            "contexts": {
                context: {
                    "median_speed_um_h": float(math.exp(self.context_means[context][0])),
                    "mean_lag_h": float(self.context_means[context][1]),
                    "estimate_sd_log_speed": float(
                        math.sqrt(self.context_mean_covariance[context][0, 0])
                    ),
                    "estimate_sd_lag_h": float(
                        math.sqrt(self.context_mean_covariance[context][1, 1])
                    ),
                }
                for context in self.contexts
            },
            "within_study_variability": sd(self.within_study),
            "between_study_variability": sd(self.between_study),
            "noise_center_sd_normalized_radius": dict(self.noise_sd),
            "study_noise_sd_normalized_radius": dict(self.study_noise_sd),
            "training_series": len(self.training_series),
            "training_studies": len(self.training_studies),
            "penalized_reml_objective": self.objective,
        }


def fit_population(
    series: Sequence[WoundSeries], *, noise_series: Sequence[WoundSeries] | None = None
) -> PopulationFit:
    """Fit the nested random-effects model to control series of several studies.

    ``noise_series`` (default: the control series) supplies study noise estimates;
    treated series of training studies may be added because noise is a property of
    the measurement, not of the treatment.
    """
    if not 2 <= len(series) <= MAX_SERIES:
        raise InvalidRequest("Population fitting needs 2..5000 series")
    if len({s.series_id for s in series}) != len(series):
        raise InvalidRequest("Population fitting requires unique series identities")
    if any(s.role != "control" for s in series):
        raise InvalidRequest("Population priors are fitted to control/vehicle series only")
    estimates = study_noise(noise_series if noise_series is not None else series)
    centers = noise_centers(estimates)
    summaries = [
        series_summary(s, estimates.get((s.study_id, s.kind), centers[s.kind])) for s in series
    ]
    contexts = tuple(sorted({s.context for s in series}))
    index = {context: position for position, context in enumerate(contexts)}
    studies: dict[str, list[dict[str, Any]]] = {}
    for summary in summaries:
        studies.setdefault(summary["study_id"], []).append(summary)
    for members in studies.values():
        if len({m["context"] for m in members}) != 1:
            raise InvalidRequest("Each study must belong to exactly one context")
    p = 2 * len(contexts)
    blocks = []
    for members in studies.values():
        n = len(members)
        y = np.concatenate([m["mean"] for m in members])
        design = np.zeros((2 * n, p))
        column = 2 * index[members[0]["context"]]
        for row in range(n):
            design[2 * row, column] = 1.0
            design[2 * row + 1, column + 1] = 1.0
        blocks.append((members, y, design))

    def assemble(params: FloatArray) -> tuple[float, FloatArray, FloatArray] | None:
        # Both levels are declared diagonal: per-series posteriors already carry the
        # speed/lag estimation trade-off, and few studies cannot identify a correlation.
        within = _covariance(params[:2])
        between = _covariance(params[2:4])
        a_total = np.zeros((p, p))
        b_total = np.zeros(p)
        pieces = []
        for members, y, design in blocks:
            n = len(members)
            c = np.kron(np.ones((n, n)), between)
            for row, member in enumerate(members):
                c[2 * row : 2 * row + 2, 2 * row : 2 * row + 2] += (
                    within / member["effective_n"] + member["covariance"]
                )
            try:
                chol = np.linalg.cholesky(c)
            except np.linalg.LinAlgError:
                return None
            inv_design = np.linalg.solve(chol, design)
            inv_y = np.linalg.solve(chol, y)
            a_total += inv_design.T @ inv_design
            b_total += inv_design.T @ inv_y
            pieces.append((chol, inv_design, inv_y))
        try:
            beta = np.linalg.solve(a_total, b_total)
            sign, logdet_a = np.linalg.slogdet(a_total)
        except np.linalg.LinAlgError:
            return None
        if sign <= 0:
            return None
        value = 0.5 * logdet_a
        for chol, inv_design, inv_y in pieces:
            residual = inv_y - inv_design @ beta
            value += float(np.sum(np.log(np.diag(chol)))) + 0.5 * float(residual @ residual)
        for matrix in (within, between):
            value -= _gamma2_log_penalty(math.sqrt(matrix[0, 0]), PENALTY_MODES["log_speed_sd"])
            value -= _gamma2_log_penalty(math.sqrt(matrix[1, 1]), PENALTY_MODES["lag_sd_h"])
        return value, beta, np.linalg.inv(a_total)

    def objective(params: FloatArray) -> float:
        if np.any(np.abs(params) > 8.0):
            return 1e12
        result = assemble(params)
        return 1e12 if result is None else result[0]

    start = np.array([math.log(0.15), math.log(2.0), math.log(0.15), math.log(3.0)])
    best, _ = nelder_mead(objective, start)
    for _ in range(3):  # deterministic restarts from the incumbent
        best, _ = nelder_mead(objective, best, step=0.2)
    final = assemble(best)
    if final is None:
        raise InvalidRequest("Population fit did not converge to a positive-definite model")
    value, beta, beta_covariance = final
    return PopulationFit(
        contexts=contexts,
        context_means={c: beta[2 * index[c] : 2 * index[c] + 2] for c in contexts},
        context_mean_covariance={
            c: beta_covariance[2 * index[c] : 2 * index[c] + 2, 2 * index[c] : 2 * index[c] + 2]
            for c in contexts
        },
        within_study=_covariance(best[:2]),
        between_study=_covariance(best[2:4]),
        noise_sd=centers,
        study_noise_sd={f"{study}/{kind}": value for (study, kind), value in estimates.items()},
        training_series=tuple(s.series_id for s in series),
        training_studies=tuple(sorted(studies)),
        objective=value,
    )


def _prior_log_density(
    mean: FloatArray, covariance: FloatArray, *, student_df: float | None = None
) -> FloatArray:
    """Gaussian (default) or bivariate Student-t log density on the fixed grid."""
    inverse = np.linalg.inv(covariance)
    centered = np.stack([GRID_LOG_SPEED - mean[0], GRID_LAG - mean[1]], axis=1)
    mahalanobis = np.einsum("ij,jk,ik->i", centered, inverse, centered)
    if student_df is None:
        gaussian: FloatArray = -0.5 * mahalanobis
        return gaussian
    heavy: FloatArray = -0.5 * (student_df + 2.0) * np.log1p(mahalanobis / student_df)
    return heavy


def flat_prior() -> dict[str, Any]:
    """Marker for a uniform prior over the bounded grid (no population pooling)."""
    return {"flat": True}


@dataclass(frozen=True)
class Posterior:
    """Joint grid posterior over ``(ln v, L)`` and the study noise level."""

    joint: FloatArray  # shape (grid points, noise levels), sums to one
    sigmas: FloatArray

    @property
    def parameter_weights(self) -> FloatArray:
        weights: FloatArray = self.joint.sum(axis=1)
        return weights


def posterior(
    prior: Mapping[str, Any],
    *,
    series: WoundSeries | None,
    noise: NoiseModel | float,
    student_df: float | None = None,
) -> Posterior:
    """Population prior × prefix likelihood, marginalizing an unknown noise level."""
    if prior.get("flat"):
        log_prior = np.zeros(GRID_LAG.size)
    else:
        log_prior = _prior_log_density(prior["mean"], prior["covariance"], student_df=student_df)
    if isinstance(noise, NoiseModel):
        sigmas, log_noise = noise.levels()
    else:
        sigmas, log_noise = np.array([float(noise)]), np.zeros(1)
    columns = []
    for sigma, log_weight in zip(sigmas, log_noise, strict=True):
        log_column = log_prior + log_weight
        if series is not None:
            log_column = log_column + grid_log_likelihood(series, float(sigma))
        columns.append(log_column)
    joint = _normalize(np.stack(columns, axis=1).ravel()).reshape(GRID_LAG.size, len(sigmas))
    return Posterior(joint=joint, sigmas=sigmas)


def predictive(
    fitted: Posterior,
    *,
    hours: Sequence[float],
    radius0_um: float,
    draws: int = 4000,
    seed: int = 0,
    quantiles: Sequence[float] = (0.05, 0.1, 0.5, 0.9, 0.95),
) -> dict[str, Any]:
    """Predicted observed area fractions (mean and quantiles) at future hours.

    The observed value is ``(rho + noise)**2`` and is reported as 0 when the
    noisy radius falls below the detection radius, mirroring the likelihood.
    """
    if not hours:
        raise InvalidRequest("Predictive queries need at least one hour")
    rho = normalized_radius(hours, speed_um_h=GRID_SPEED, lag_h=GRID_LAG, radius0_um=radius0_um)
    true_mean = fitted.parameter_weights @ (rho**2)
    rng = np.random.default_rng(seed)
    flat = fitted.joint.ravel()
    picks = rng.choice(flat.size, size=draws, p=flat)
    grid_index, level = np.divmod(picks, fitted.sigmas.size)
    sampled = rho[grid_index]
    noisy = sampled + rng.normal(0.0, 1.0, size=sampled.shape) * fitted.sigmas[level][:, None]
    detection = min(0.5, DETECTION_RADIUS_UM / radius0_um)
    observed = np.where((sampled > 0.0) & (noisy > detection), noisy**2, 0.0)
    return {
        "hours": [float(h) for h in hours],
        "mean_area_fraction": [float(v) for v in observed.mean(axis=0)],
        "true_area_fraction": [float(v) for v in true_mean],
        "quantiles": {
            f"{q:g}": [float(v) for v in np.quantile(observed, q, axis=0)] for q in quantiles
        },
        "posterior_median_speed_um_h": float(np.exp(np.quantile(GRID_LOG_SPEED[grid_index], 0.5))),
        "posterior_median_lag_h": float(np.quantile(GRID_LAG[grid_index], 0.5)),
        "posterior_median_noise_sd": float(np.quantile(fitted.sigmas[level], 0.5)),
    }


def cohort_mean_prediction(
    fit: PopulationFit,
    *,
    context: str,
    hours: Sequence[float],
    radius0_um: float,
    animals: int = 6,
    draws: int = 2000,
    seed: int = 0,
    quantiles: Sequence[float] = (0.05, 0.1, 0.5, 0.9, 0.95),
) -> dict[str, Any]:
    """Initial-state-only prediction of a new study's group-mean area fraction."""
    if animals < 1 or draws < 10:
        raise InvalidRequest("Cohort prediction needs >=1 animal and >=10 draws")
    rng = np.random.default_rng(seed)
    study_prior = fit.prior(context, effective_n=1, new_study=True)
    study_covariance = study_prior["covariance"] - fit.within_study
    study_means = rng.multivariate_normal(study_prior["mean"], study_covariance, size=draws)
    animal = rng.multivariate_normal(np.zeros(2), fit.within_study, size=(draws, animals))
    parameters = study_means[:, None, :] + animal
    speed = np.exp(parameters[..., 0]).ravel()
    lag = np.maximum(0.0, parameters[..., 1]).ravel()
    rho = normalized_radius(hours, speed_um_h=speed, lag_h=lag, radius0_um=radius0_um)
    means = (rho**2).reshape(draws, animals, len(hours)).mean(axis=1)
    return {
        "hours": [float(h) for h in hours],
        "mean_area_fraction": [float(v) for v in means.mean(axis=0)],
        "quantiles": {
            f"{q:g}": [float(v) for v in np.quantile(means, q, axis=0)] for q in quantiles
        },
        "animals_per_draw": animals,
        "draws": draws,
        "class": "initial_state_only_population_prediction",
    }


GRID_SHAPE = (SPEED_GRID_UM_H.size, LAG_GRID_H.size)
_STEP_LOG_SPEED = float(np.log(SPEED_GRID_UM_H[1] / SPEED_GRID_UM_H[0]))
_STEP_LAG = float(LAG_GRID_H[1] - LAG_GRID_H[0])


def _gaussian_kernel(covariance: FloatArray) -> FloatArray:
    """Discrete 2D Gaussian kernel on the grid spacing, truncated at 4 SD."""
    sd = np.sqrt(np.diag(covariance))
    half_x = max(1, int(math.ceil(4.0 * sd[0] / _STEP_LOG_SPEED)))
    half_y = max(1, int(math.ceil(4.0 * sd[1] / _STEP_LAG)))
    half_x = min(half_x, GRID_SHAPE[0] - 1)
    half_y = min(half_y, GRID_SHAPE[1] - 1)
    dx = np.arange(-half_x, half_x + 1) * _STEP_LOG_SPEED
    dy = np.arange(-half_y, half_y + 1) * _STEP_LAG
    gx, gy = np.meshgrid(dx, dy, indexing="ij")
    points = np.stack([gx.ravel(), gy.ravel()], axis=1)
    inverse = np.linalg.inv(covariance)
    log_kernel = -0.5 * np.einsum("ij,jk,ik->i", points, inverse, points)
    kernel = np.exp(log_kernel - log_kernel.max()).reshape(gx.shape)
    result: FloatArray = kernel / kernel.sum()
    return result


def _blur_log(log_field: FloatArray, kernel: FloatArray) -> FloatArray:
    """log((exp(log_field) ⊛ kernel)) by zero-padded FFT convolution, same shape."""
    field_2d = log_field.reshape(GRID_SHAPE)
    peak = float(np.max(field_2d))
    values = np.exp(field_2d - peak)
    shape = (GRID_SHAPE[0] + kernel.shape[0] - 1, GRID_SHAPE[1] + kernel.shape[1] - 1)
    spectrum = np.fft.rfft2(values, shape) * np.fft.rfft2(kernel, shape)
    full = np.fft.irfft2(spectrum, shape)
    ox, oy = kernel.shape[0] // 2, kernel.shape[1] // 2
    same = full[ox : ox + GRID_SHAPE[0], oy : oy + GRID_SHAPE[1]]
    result: FloatArray = (np.log(np.maximum(same, 1e-300)) + peak).ravel()
    return result


def cohort_posteriors(
    group_prior: Mapping[str, Any],
    *,
    within_study: FloatArray,
    cohort: Sequence[WoundSeries],
    noise: NoiseModel,
    student_df: float | None = None,
) -> list[Posterior]:
    """Hierarchical grid posteriors for every animal of one experimental group.

    The group mean ``theta`` has the population prior; each animal ``phi ~ N(theta,
    within_study)``. Each animal's posterior uses its own prefix plus the prefixes
    of its cohort-mates (leave-one-out for the group mean). All prefixes must end at
    the same forecast origin, so no future value of any animal is used.
    """
    if not cohort:
        raise InvalidRequest("Cohort assimilation needs at least one animal prefix")
    kernel = _gaussian_kernel(within_study)
    log_prior = _prior_log_density(
        group_prior["mean"], group_prior["covariance"], student_df=student_df
    )
    sigmas, log_noise = noise.levels()
    per_level = []
    evidence = []
    for sigma, log_weight in zip(sigmas, log_noise, strict=True):
        own = [grid_log_likelihood(item, float(sigma)) for item in cohort]
        blurred = [_blur_log(log_likelihood, kernel) for log_likelihood in own]
        total = log_prior + np.sum(blurred, axis=0)
        peak = float(np.max(total))
        evidence.append(log_weight + peak + math.log(float(np.sum(np.exp(total - peak)))))
        animals = []
        for index, log_likelihood in enumerate(own):
            leave_out = total - blurred[index]
            animal_prior = _blur_log(leave_out, kernel)
            animals.append(animal_prior + log_likelihood)
        per_level.append(animals)
    level_weights = _normalize(np.asarray(evidence))
    results = []
    for index in range(len(cohort)):
        columns = []
        for level, weight in enumerate(level_weights):
            column = per_level[level][index]
            peak = float(np.max(column))
            normalized = np.exp(column - peak)
            columns.append(weight * normalized / normalized.sum())
        joint = np.stack(columns, axis=1)
        results.append(Posterior(joint=joint / joint.sum(), sigmas=sigmas))
    return results
