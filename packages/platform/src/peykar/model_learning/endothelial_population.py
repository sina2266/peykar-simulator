"""Hierarchical population model of corneal endothelial cell density after surgery.

Kinetics, as a fraction of the reference ECD (preoperative or donor count), over
months ``t`` after surgery::

    f(t) = (1 - A) * (1 + t / tau) ** (-p)

``A`` is the loss already present at the first postoperative count (surgical and
graft-handling loss, and any offset between the reference and follow-up counts),
followed by a decelerating decline with onset time scale ``tau`` (months) and
exponent ``p``. A power law approximates the mixture of fast and slow exponential
phases described for graft endothelium (Armitage, Dick & Bourne 2003, IOVS
44:3326) and the slow continued loss after lens implantation (Bourne, Nelson &
Hodge 1994, Ophthalmology 101:1014); on development series it fitted keratoplasty
trajectories better than one exponential phase plus a constant chronic rate with
the same number of parameters. Human endothelium is treated as non-regenerating:
``0 <= A <= 0.95`` and ``p > 0``.

Observation model: Gaussian noise in ``ln f`` with one SD per study, tissue family
and series kind (published group mean or single eye); a new study's SD is unknown
and marginalized over a log-normal prior centred on the median of training studies
of the same family and kind.

Hierarchy on ``theta = (ln(1 - A), ln tau, ln p)``, fitted separately per tissue
family (species and host versus graft endothelium, whose variability differs): a
family mean, a context effect shared by the studies of one procedure (for grafts,
a recipient-referenced series is its own context), a study effect and a series
deviation with separate variances for group means and single eyes. Each series is
summarized by flat-prior grid posterior moments; penalized restricted maximum
likelihood estimates the diagonal variances. Predictions use exact grid posteriors
with a Gaussian, Student-t or robust mixture (population + uniform) prior, either
on the observed levels or anchored to them by an unknown constant log offset.
Nothing here reads files, registers models or claims biological validity.
"""

from __future__ import annotations

import math
from collections.abc import Mapping, Sequence
from dataclasses import dataclass
from typing import Any

import numpy as np
from numpy.typing import NDArray

from peykar.model_learning.corneal_population import NoiseModel, nelder_mead
from peykar.validation.cornea_endothelium_data import DensitySeries
from peykar_core.errors import InvalidRequest

MODEL_ID = "cornea.endothelial-density-population"
MODEL_VERSION = "1.0.0"
FloatArray = NDArray[np.float64]

# u = ln(1 - A): log of the fraction retained at the first postoperative count,
# uniform in the observation (log-fraction) scale; A spans 0..0.95.
LOG_RETAINED_GRID: FloatArray = np.linspace(math.log(0.05), 0.0, 201)
TAU_GRID_MONTHS: FloatArray = np.geomspace(0.01, 36.0, 40)
EXPONENT_GRID: FloatArray = np.geomspace(0.002, 1.0, 30)
_RETAINED, _LOG_TAU, _LOG_EXPONENT = np.meshgrid(
    LOG_RETAINED_GRID, np.log(TAU_GRID_MONTHS), np.log(EXPONENT_GRID), indexing="ij"
)
GRID_LOG_RETAINED: FloatArray = _RETAINED.ravel()
GRID_LOG_TAU: FloatArray = _LOG_TAU.ravel()
GRID_LOG_EXPONENT: FloatArray = _LOG_EXPONENT.ravel()
GRID_A: FloatArray = 1.0 - np.exp(GRID_LOG_RETAINED)
GRID_TAU: FloatArray = np.exp(GRID_LOG_TAU)
GRID_EXPONENT: FloatArray = np.exp(GRID_LOG_EXPONENT)
GRID_POINTS: FloatArray = np.stack([GRID_LOG_RETAINED, GRID_LOG_TAU, GRID_LOG_EXPONENT], axis=1)
PARAMETERS = ("log_retained_immediate", "log_tau_months", "log_exponent")

GRAFT_PROCEDURES = frozenset({"pk", "dsaek", "dmek"})
RECIPIENT_REFERENCE_SUFFIX = "~recipient-reference"
# Declared constants (not fitted): Gamma(shape=2) modes of every level's SDs for
# (ln(1 - A), ln tau, ln p), the noise floor and the log-SD of a new study's noise
# level around its centre (specular microscopy noise is known within a factor of
# about 1.6), a floor for observed fractions and the weight of the uniform
# component of the robust mixture prior.
PENALTY_MODES: FloatArray = np.array([0.1, 0.5, 0.5])
LEVELS = ("context", "study", "group_mean_series", "single_eye_series")
NOISE_FLOOR = 0.005
NOISE_LOG_SD = 0.5
MIN_FRACTION = 1e-3
ROBUST_WEIGHT = 0.2
SUMMARY_JITTER: FloatArray = np.array([1e-4, 1e-4, 1e-4])
DESCRIBED_MONTHS = (1.0, 6.0, 12.0, 60.0)
MAX_SERIES = 5_000


def model_context(series: DensitySeries) -> str:
    """Context key: species/procedure, with recipient-referenced grafts separated."""
    procedure = series.context.partition("/")[2]
    if procedure in GRAFT_PROCEDURES and series.reference_kind == "patient_preoperative":
        return series.context + RECIPIENT_REFERENCE_SUFFIX
    return series.context


def tissue_family(context: str) -> str:
    """``species/graft`` for keratoplasty with donor endothelium, else ``species/host``."""
    species, _, rest = context.partition("/")
    procedure = rest.split("~")[0]
    return f"{species}/{'graft' if procedure in GRAFT_PROCEDURES else 'host'}"


def series_family(series: DensitySeries) -> str:
    return tissue_family(model_context(series))


def density_fraction(
    months: Sequence[float] | FloatArray,
    *,
    immediate_loss: float | FloatArray,
    tau_months: float | FloatArray,
    exponent: float | FloatArray,
) -> FloatArray:
    """Kinetic law for scalar parameters or every grid point (rows) at ``months``."""
    t = np.asarray(months, dtype=np.float64)
    a = np.atleast_1d(np.asarray(immediate_loss, dtype=np.float64))[:, None]
    tau = np.atleast_1d(np.asarray(tau_months, dtype=np.float64))[:, None]
    p = np.atleast_1d(np.asarray(exponent, dtype=np.float64))[:, None]
    result: FloatArray = (1.0 - a) * np.exp(-p * np.log1p(t[None, :] / tau))
    return result


def grid_log_fraction(months: Sequence[float]) -> FloatArray:
    t = np.asarray(months, dtype=np.float64)
    result: FloatArray = GRID_LOG_RETAINED[:, None] - GRID_EXPONENT[:, None] * np.log1p(
        t[None, :] / GRID_TAU[:, None]
    )
    return result


def grid_sum_squares(series: DensitySeries) -> FloatArray:
    """Sum over observations of squared ``ln`` residuals for every grid point."""
    observed = np.log(np.maximum(MIN_FRACTION, np.asarray(series.fractions, dtype=np.float64)))
    residual = grid_log_fraction(series.months) - observed[None, :]
    result: FloatArray = np.sum(residual**2, axis=1)
    return result


def _log_likelihood(sum_squares: FloatArray, points: int, sigma: float) -> FloatArray:
    if not math.isfinite(sigma) or sigma <= 0:
        raise InvalidRequest("Noise SD must be finite and positive")
    result: FloatArray = -0.5 * sum_squares / sigma**2 - points * math.log(sigma)
    return result


def grid_log_likelihood(series: DensitySeries, sigma: float) -> FloatArray:
    return _log_likelihood(grid_sum_squares(series), len(series.months), sigma)


def shape_terms(series: DensitySeries) -> tuple[FloatArray, FloatArray]:
    """Per grid point: mean log residual (the anchor) and the residual sum of squares
    around it (the shape misfit), for a constant unknown log offset."""
    observed = np.log(np.maximum(MIN_FRACTION, np.asarray(series.fractions, dtype=np.float64)))
    residual = observed[None, :] - grid_log_fraction(series.months)
    anchor = residual.mean(axis=1)
    shape: FloatArray = np.sum((residual - anchor[:, None]) ** 2, axis=1)
    return anchor, shape


def _normalize(log_weights: FloatArray) -> FloatArray:
    peak = float(np.max(log_weights))
    weights = np.exp(log_weights - peak)
    total = float(np.sum(weights))
    if not math.isfinite(total) or total <= 0:
        raise InvalidRequest("Grid posterior is degenerate")
    normalized: FloatArray = weights / total
    return normalized


NoiseKey = tuple[str, str, str]  # (study, tissue family, series kind)


def study_noise(series: Sequence[DensitySeries]) -> dict[NoiseKey, float]:
    """Residual SD of ``ln f`` per (study, family, kind) from best grid fits.

    Only series with at least four follow-ups (three kinetic parameters) count;
    other study/family/kind combinations get no estimate.
    """
    estimates: dict[NoiseKey, float] = {}
    groups: dict[NoiseKey, list[DensitySeries]] = {}
    for item in series:
        groups.setdefault((item.study_id, series_family(item), item.kind), []).append(item)
    for key, members in groups.items():
        usable = [m for m in members if len(m.months) >= 4]
        if not usable:
            continue
        sse = sum(float(np.min(grid_sum_squares(m))) for m in usable)
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


def series_summary(series: DensitySeries, sigma: float) -> dict[str, Any]:
    """Flat-prior grid posterior moments of ``(ln(1 - A), ln tau, ln p)`` for one series."""
    weights = _normalize(grid_log_likelihood(series, sigma))
    mean = weights @ GRID_POINTS
    centered = GRID_POINTS - mean
    covariance = (centered * weights[:, None]).T @ centered
    best = int(np.argmax(weights))
    return {
        "series_id": series.series_id,
        "study_id": series.study_id,
        "context": model_context(series),
        "kind": series.kind,
        "mean": mean,
        "covariance": covariance + np.diag(SUMMARY_JITTER),
        "map": {
            "immediate_loss": float(GRID_A[best]),
            "tau_months": float(GRID_TAU[best]),
            "exponent": float(GRID_EXPONENT[best]),
        },
    }


def _gamma2_log_penalty(sd: FloatArray, mode: FloatArray) -> float:
    # Sum of log Gamma(shape=2, rate=1/mode) densities up to constants.
    return float(np.sum(np.log(sd) - sd / mode))


def _diagonal(log_sd: FloatArray) -> FloatArray:
    result: FloatArray = np.diag(np.exp(2.0 * log_sd))
    return result


@dataclass(frozen=True)
class FamilyFit:
    family: str
    mean: FloatArray
    mean_covariance: FloatArray
    context_means: Mapping[str, FloatArray]
    context_mean_covariance: Mapping[str, FloatArray]
    variances: Mapping[str, FloatArray]
    studies: tuple[str, ...]
    series: int
    objective: float


def _fit_family(family: str, summaries: Sequence[Mapping[str, Any]]) -> FamilyFit:
    contexts = tuple(sorted({m["context"] for m in summaries}))
    context_index = {c: i for i, c in enumerate(contexts)}
    blocks = []
    for context in contexts:
        members = [m for m in summaries if m["context"] == context]
        n = len(members)
        y = np.concatenate([m["mean"] for m in members])
        design = np.kron(np.ones((n, 1)), np.eye(3))
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
        if np.any(np.abs(params) > 8.0):
            return 1e12
        context_cov = _diagonal(params[0:3])
        a_total = np.zeros((3, 3))
        b_total = np.zeros(3)
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
            value -= _gamma2_log_penalty(np.exp(params[3 * level : 3 * level + 3]), PENALTY_MODES)
        return value

    best, _ = nelder_mead(objective, np.tile(np.log(PENALTY_MODES), 4), iterations=6000)
    for _ in range(3):  # deterministic restarts from the incumbent
        best, _ = nelder_mead(objective, best, step=0.2, iterations=6000)
    value = objective(best)
    if value >= 1e12:
        raise InvalidRequest(f"Population fit of {family!r} did not converge")
    # Joint Gaussian posterior of the family mean (flat prior) and context effects
    # (N(0, C)) given the fitted variances: exact BLUE/BLUP of every context mean.
    context_precision = np.linalg.inv(_diagonal(best[0:3]))
    unknowns = 3 + 3 * len(contexts)
    precision = np.zeros((unknowns, unknowns))
    rhs = np.zeros(unknowns)
    for block in blocks:
        context, n = block[0], block[1]
        design = np.zeros((3 * n, unknowns))
        design[:, :3] = block[3]
        offset = 3 + 3 * context_index[context]
        design[:, offset : offset + 3] = block[3]
        inverse = np.linalg.inv(residual_covariance(best, block))
        precision += design.T @ inverse @ design
        rhs += design.T @ inverse @ block[2]
        precision[offset : offset + 3, offset : offset + 3] += context_precision
    covariance = np.linalg.inv(precision)
    solution = covariance @ rhs
    context_means = {}
    context_cov = {}
    for context, i in context_index.items():
        selector = np.zeros((3, unknowns))
        selector[:, :3] = np.eye(3)
        selector[:, 3 + 3 * i : 6 + 3 * i] += np.eye(3)
        context_means[context] = selector @ solution
        context_cov[context] = selector @ covariance @ selector.T
    return FamilyFit(
        family=family,
        mean=solution[:3],
        mean_covariance=covariance[:3, :3],
        context_means=context_means,
        context_mean_covariance=context_cov,
        variances={level: _diagonal(best[3 * i : 3 * i + 3]) for i, level in enumerate(LEVELS)},
        studies=tuple(sorted({m["study_id"] for m in summaries})),
        series=len(summaries),
        objective=float(value),
    )


@dataclass(frozen=True)
class EndothelialFit:
    families: Mapping[str, FamilyFit]
    noise_sd: Mapping[tuple[str, str], float]
    study_noise_sd: Mapping[str, float]
    training_series: tuple[str, ...]
    training_studies: tuple[str, ...]
    unfitted_families: Mapping[str, str]

    def _location(self, context: str) -> tuple[FamilyFit, FloatArray, FloatArray, str]:
        family = tissue_family(context)
        if family not in self.families:
            reason = self.unfitted_families.get(family, "no training study")
            raise InvalidRequest(f"No population prior for tissue family {family!r}: {reason}")
        fitted = self.families[family]
        if context in fitted.context_means:
            mean = fitted.context_means[context]
            return fitted, mean, fitted.context_mean_covariance[context], context
        covariance = fitted.mean_covariance + fitted.variances["context"]
        return fitted, fitted.mean, covariance, f"family:{family}"

    def level_used(self, context: str) -> str:
        return self._location(context)[3]

    def prior(self, context: str, kind: str) -> dict[str, Any]:
        """Distribution of a new study's series parameters (``kind`` aggregate/individual)."""
        if kind not in {"aggregate", "individual"}:
            raise InvalidRequest("Series kind must be aggregate or individual")
        fitted, mean, covariance, _ = self._location(context)
        series_level = "group_mean_series" if kind == "aggregate" else "single_eye_series"
        return {
            "mean": mean,
            "covariance": covariance + fitted.variances["study"] + fitted.variances[series_level],
        }

    def noise(self, kind: str, context: str) -> NoiseModel:
        family = tissue_family(context)
        center = self.noise_sd.get((family, kind), self.noise_sd[("*", kind)])
        return NoiseModel(center=center, log_sd=NOISE_LOG_SD)

    def describe(self) -> dict[str, Any]:
        def location(mean: FloatArray, covariance: FloatArray) -> dict[str, Any]:
            sd = np.sqrt(np.diag(covariance))
            curve = density_fraction(
                DESCRIBED_MONTHS,
                immediate_loss=1.0 - math.exp(mean[0]),
                tau_months=math.exp(mean[1]),
                exponent=math.exp(mean[2]),
            )[0]
            return {
                "immediate_loss": float(1.0 - math.exp(mean[0])),
                "median_tau_months": float(math.exp(mean[1])),
                "median_exponent": float(math.exp(mean[2])),
                "estimate_sd_log_retained": float(sd[0]),
                "estimate_sd_log_tau": float(sd[1]),
                "estimate_sd_log_exponent": float(sd[2]),
                "fraction_at_mean_parameters": {
                    f"{m:g}": float(v) for m, v in zip(DESCRIBED_MONTHS, curve, strict=True)
                },
            }

        families = {}
        for name, fitted in self.families.items():
            families[name] = {
                "mean": location(fitted.mean, fitted.mean_covariance),
                "contexts": {
                    c: location(fitted.context_means[c], fitted.context_mean_covariance[c])
                    for c in sorted(fitted.context_means)
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
    series: Sequence[DensitySeries], *, noise_series: Sequence[DensitySeries] | None = None
) -> EndothelialFit:
    """Fit one hierarchy per tissue family to control series of several studies.

    ``noise_series`` (default: the controls) supplies study noise estimates; treated
    series of training studies may be added because noise belongs to the measurement.
    A family with fewer than two training studies gets no population prior.
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
        fitted[family] = _fit_family(family, members)
    if not fitted:
        raise InvalidRequest("No tissue family has two training studies")
    return EndothelialFit(
        families=fitted,
        noise_sd=centers,
        study_noise_sd={f"{s}/{f}/{k}": v for (s, f, k), v in estimates.items()},
        training_series=tuple(s.series_id for s in series),
        training_studies=tuple(sorted({s.study_id for s in series})),
        unfitted_families=unfitted,
    )


def flat_prior() -> dict[str, Any]:
    """Marker for a uniform prior over the bounded grid (no population pooling)."""
    return {"flat": True}


def _prior_log_density(prior: Mapping[str, Any], *, student_df: float | None) -> FloatArray:
    if prior.get("flat"):
        return np.zeros(GRID_A.size)
    inverse = np.linalg.inv(prior["covariance"])
    centered = GRID_POINTS - prior["mean"]
    mahalanobis = np.einsum("ij,jk,ik->i", centered, inverse, centered)
    if student_df is None:
        log_density: FloatArray = -0.5 * mahalanobis
    else:
        log_density = -0.5 * (student_df + 3.0) * np.log1p(mahalanobis / student_df)
    weight = float(prior.get("robust_weight", 0.0))
    if weight <= 0.0:
        return log_density
    # Mixture of the normalized population density and a uniform grid density.
    population = _normalize(log_density)
    mixture: FloatArray = np.log((1.0 - weight) * population + weight / GRID_A.size)
    return mixture


def robust(prior: Mapping[str, Any], weight: float = ROBUST_WEIGHT) -> dict[str, Any]:
    if not 0.0 < weight < 1.0:
        raise InvalidRequest("Robust mixture weight must lie strictly between 0 and 1")
    return {**prior, "robust_weight": weight}


def _noise_levels(noise: NoiseModel | float) -> tuple[FloatArray, FloatArray]:
    if isinstance(noise, NoiseModel):
        sigmas, log_noise = noise.levels()
        return np.maximum(NOISE_FLOOR, sigmas), log_noise
    return np.array([float(noise)]), np.zeros(1)


@dataclass(frozen=True)
class Posterior:
    """Joint grid posterior over ``(ln(1 - A), ln tau, ln p)`` and the noise level.

    ``log_anchor`` is ``None`` for level forecasts; for anchored forecasts it holds
    each grid point's mean log residual of the prefix (the unknown constant offset at
    its posterior mean), and ``prefix_points`` the number of prefix observations.
    """

    joint: FloatArray  # shape (grid points, noise levels), sums to one
    sigmas: FloatArray
    log_anchor: FloatArray | None = None
    prefix_points: int = 0

    @property
    def parameter_weights(self) -> FloatArray:
        weights: FloatArray = self.joint.sum(axis=1)
        return weights


def posterior(
    prior: Mapping[str, Any],
    *,
    series: DensitySeries | None,
    noise: NoiseModel | float,
    student_df: float | None = None,
) -> Posterior:
    """Population prior times the prefix likelihood, marginalizing the noise level."""
    log_prior = _prior_log_density(prior, student_df=student_df)
    sigmas, log_noise = _noise_levels(noise)
    sum_squares = grid_sum_squares(series) if series is not None else None
    columns = []
    for sigma, log_weight in zip(sigmas, log_noise, strict=True):
        column = log_prior + log_weight
        if sum_squares is not None and series is not None:
            column = column + _log_likelihood(sum_squares, len(series.months), float(sigma))
        columns.append(column)
    joint = _normalize(np.stack(columns, axis=1).ravel()).reshape(GRID_A.size, len(sigmas))
    return Posterior(joint=joint, sigmas=sigmas)


def anchored_posterior(
    prior: Mapping[str, Any],
    *,
    series: DensitySeries,
    noise: NoiseModel | float,
    student_df: float | None = None,
) -> Posterior:
    """Population prior times the offset-marginalized (shape) prefix likelihood.

    Observed ``ln y_i = ln f(t_i; theta) + delta + e_i`` with a flat prior on the
    constant ``delta`` (reference-count and device offsets persist over follow-ups):
    the prefix informs ``theta`` only through its shape, and forecasts are anchored
    to the prefix level.
    """
    log_prior = _prior_log_density(prior, student_df=student_df)
    sigmas, log_noise = _noise_levels(noise)
    anchor, shape = shape_terms(series)
    points = len(series.months)
    columns = []
    for sigma, log_weight in zip(sigmas, log_noise, strict=True):
        column = log_prior + log_weight
        if points > 1:
            column = column - 0.5 * shape / sigma**2 - (points - 1) * math.log(float(sigma))
        columns.append(column)
    joint = _normalize(np.stack(columns, axis=1).ravel()).reshape(GRID_A.size, len(sigmas))
    return Posterior(joint=joint, sigmas=sigmas, log_anchor=anchor, prefix_points=points)


def predictive(
    fitted: Posterior,
    *,
    months: Sequence[float],
    draws: int = 4000,
    seed: int = 0,
    quantiles: Sequence[float] = (0.05, 0.1, 0.5, 0.9, 0.95),
) -> dict[str, Any]:
    """Posterior mean fraction (exact grid average) and observation quantiles at ``months``.

    Anchored posteriors shift each grid curve by its prefix offset and widen the
    observation noise by the offset's uncertainty ``sigma**2 / prefix_points``.
    """
    if not months or any(not math.isfinite(m) or m <= 0 for m in months):
        raise InvalidRequest("Prediction months must be finite and positive")
    log_model = grid_log_fraction(months)
    spread_factor = 1.0
    if fitted.log_anchor is not None:
        log_model = log_model + fitted.log_anchor[:, None]
        spread_factor = math.sqrt(1.0 + 1.0 / max(1, fitted.prefix_points))
    model = np.exp(log_model)
    weights = fitted.parameter_weights
    mean = weights @ model
    rng = np.random.default_rng(seed)
    flat = fitted.joint.ravel()
    picks = rng.choice(flat.size, size=draws, p=flat / flat.sum())
    grid_index, noise_index = np.divmod(picks, fitted.sigmas.size)
    spread = fitted.sigmas[noise_index] * spread_factor
    noise = rng.standard_normal((draws, len(months))) * spread[:, None]
    observed = model[grid_index] * np.exp(noise)
    return {
        "months": [float(m) for m in months],
        "mean_fraction": [float(v) for v in mean],
        "quantiles": {
            f"{q:g}": [float(v) for v in np.quantile(observed, q, axis=0)] for q in quantiles
        },
        "posterior_mean": {
            "immediate_loss": float(weights @ GRID_A),
            "log_tau_months": float(weights @ GRID_LOG_TAU),
            "log_exponent": float(weights @ GRID_LOG_EXPONENT),
        },
    }
