"""Population model 2.0.0: radius–lag kinematics with a saturating closure tail.

Version 1.0.0 (``corneal_population``) stays frozen for its sealed-test replay.
Its consumed sealed test missed the 90% interval floor because every miss was a
residual defect larger than predicted near closure: a constant-speed front closes
too abruptly. Version 2 lets the front slow down when the defect becomes small:

``dr/dt = -v * r / (r + r_c)`` after the lag ``L``; ``r_c = 0`` recovers 1.0.0.

The solution is ``r = r_c * W((r0/r_c) * exp((r0 - v*(t-L))/r_c))`` (Lambert W),
so the defect never reaches exactly zero; reported zeros are censored below the
same 125 µm detection radius. ``r_c`` is one structural parameter per
species/injury context, chosen from a fixed set by the training series' flat-prior
marginal likelihood. Everything else (noise model, hierarchy, cohort assimilation)
matches 1.0.0 and reuses its tested building blocks.
"""

from __future__ import annotations

import math
from collections.abc import Mapping, Sequence
from dataclasses import dataclass, field
from typing import Any

import numpy as np

from peykar.model_learning import corneal_population as v1
from peykar.model_learning.corneal_population import (
    DETECTION_RADIUS_UM,
    GRID_LAG,
    GRID_LOG_SPEED,
    GRID_SPEED,
    NOISE_FLOOR,
    PENALTY_MODES,
    FloatArray,
    NoiseModel,
    Posterior,
    WoundSeries,
    nelder_mead,
    noise_centers,
)
from peykar_core.errors import InvalidRequest

MODEL_ID = "cornea.population-radius-lag-tail"
MODEL_VERSION = "2.0.0"
TAIL_RADII_UM: tuple[float, ...] = (0.0, 100.0, 200.0, 300.0, 500.0, 800.0)
_NEWTON_STEPS = 40


def normalized_radius(
    hours: Sequence[float] | FloatArray,
    *,
    speed_um_h: float | FloatArray,
    lag_h: float | FloatArray,
    radius0_um: float,
    tail_um: float = 0.0,
) -> FloatArray:
    """Normalized defect radius for scalar parameters or every grid point (rows)."""
    if tail_um < 0 or not math.isfinite(tail_um):
        raise InvalidRequest("Tail radius must be finite and nonnegative")
    if tail_um == 0.0:
        return v1.normalized_radius(
            hours, speed_um_h=speed_um_h, lag_h=lag_h, radius0_um=radius0_um
        )
    t = np.asarray(hours, dtype=np.float64)
    speed = np.atleast_1d(np.asarray(speed_um_h, dtype=np.float64))
    lag = np.atleast_1d(np.asarray(lag_h, dtype=np.float64))
    moved = speed[:, None] * np.maximum(0.0, t[None, :] - lag[:, None])
    log_z = math.log(radius0_um / tail_um) + (radius0_um - moved) / tail_um
    # Solve w + ln(w) = ln(z) for the principal Lambert W branch in log space.
    safe = np.minimum(log_z, 1.0)
    w = np.where(log_z > 1.0, log_z - np.log(np.maximum(log_z, 1.0)), np.exp(safe))
    w = np.maximum(w, 1e-300)
    for _ in range(_NEWTON_STEPS):
        w = np.maximum(w * (1.0 + log_z - np.log(w)) / (1.0 + w), 1e-300)
    rho = np.minimum(1.0, tail_um * w / radius0_um)
    result: FloatArray = np.where(moved > 0.0, rho, 1.0)
    return result


def grid_log_likelihood(series: WoundSeries, sigma: float, tail_um: float) -> FloatArray:
    """Log likelihood of every grid point; zeros are censored below detection."""
    if not math.isfinite(sigma) or sigma <= 0:
        raise InvalidRequest("Noise SD must be finite and positive")
    fractions = np.asarray(series.fractions, dtype=np.float64)
    model = normalized_radius(
        series.hours,
        speed_um_h=GRID_SPEED,
        lag_h=GRID_LAG,
        radius0_um=series.radius0_um,
        tail_um=tail_um,
    )
    positive = fractions > 0.0
    log_likelihood = np.zeros(GRID_LAG.size)
    if np.any(positive):
        observed = np.sqrt(fractions[positive])
        residual = (model[:, positive] - observed[None, :]) / sigma
        log_likelihood += np.sum(-0.5 * residual**2, axis=1) - positive.sum() * math.log(sigma)
    if np.any(~positive):
        bound = (series.detection_rho - model[:, ~positive]) / sigma
        log_likelihood += np.sum(v1._log_norm_cdf(bound), axis=1)
    result: FloatArray = log_likelihood
    return result


def _log_marginal(log_likelihood: FloatArray) -> float:
    peak = float(np.max(log_likelihood))
    return peak + math.log(float(np.mean(np.exp(log_likelihood - peak))))


def study_noise(
    series: Sequence[WoundSeries], tails: Mapping[str, float]
) -> dict[tuple[str, str], float]:
    """Residual SD per (study, kind) from best grid fits of uncensored points."""
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
                tail = tails.get(item.context, 0.0)
                best = int(np.argmax(grid_log_likelihood(item, sigma, tail)))
                model = normalized_radius(
                    item.hours,
                    speed_um_h=GRID_SPEED[best],
                    lag_h=GRID_LAG[best],
                    radius0_um=item.radius0_um,
                    tail_um=tail,
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


def select_tails(series: Sequence[WoundSeries]) -> dict[str, Any]:
    """Choose ``r_c`` per context (and pooled) by flat-prior marginal likelihood."""
    contexts = sorted({s.context for s in series})
    scores: dict[str, dict[float, float]] = {c: {} for c in [*contexts, "pooled"]}
    provisional = noise_centers(v1.study_noise(series))
    for tail in TAIL_RADII_UM:
        total = 0.0
        for context in contexts:
            members = [s for s in series if s.context == context]
            value = sum(
                _log_marginal(grid_log_likelihood(s, provisional[s.kind], tail)) for s in members
            )
            scores[context][tail] = value
            total += value
        scores["pooled"][tail] = total
    chosen = {name: max(values, key=lambda t: (values[t], -t)) for name, values in scores.items()}
    return {"chosen": chosen, "log_marginal": scores}


def series_summary(series: WoundSeries, sigma: float, tail_um: float) -> dict[str, Any]:
    weights = v1._normalize(grid_log_likelihood(series, sigma, tail_um))
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
        "map_speed_um_h": float(GRID_SPEED[best]),
        "map_lag_h": float(GRID_LAG[best]),
    }


@dataclass(frozen=True)
class PopulationFit2(v1.PopulationFit):
    tails_um: Mapping[str, float] = field(default_factory=dict)
    tail_selection: Mapping[str, Any] = field(default_factory=dict)

    def tail(self, context: str) -> float:
        return float(self.tails_um.get(context, self.tails_um["pooled"]))

    def describe(self) -> dict[str, Any]:
        described = super().describe()
        described.update(
            model_id=MODEL_ID,
            model_version=MODEL_VERSION,
            tail_radius_um=dict(self.tails_um),
            tail_log_marginal={
                name: {str(k): v for k, v in values.items()}
                for name, values in self.tail_selection["log_marginal"].items()
            },
        )
        return described


def fit_population(
    series: Sequence[WoundSeries],
    *,
    noise_series: Sequence[WoundSeries] | None = None,
) -> PopulationFit2:
    """Fit tails, study noise and the nested random-effects model (as 1.0.0)."""
    if not 2 <= len(series) <= v1.MAX_SERIES:
        raise InvalidRequest("Population fitting needs 2..5000 series")
    if len({s.series_id for s in series}) != len(series):
        raise InvalidRequest("Population fitting requires unique series identities")
    if any(s.role != "control" for s in series):
        raise InvalidRequest("Population priors are fitted to control/vehicle series only")
    structural = list(noise_series) if noise_series is not None else list(series)
    selection = select_tails(structural)
    tails: dict[str, float] = {k: float(v) for k, v in selection["chosen"].items()}
    estimates = study_noise(structural, tails)
    centers = noise_centers(estimates)
    summaries = [
        series_summary(
            s,
            estimates.get((s.study_id, s.kind), centers[s.kind]),
            tails.get(s.context, tails["pooled"]),
        )
        for s in series
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
        within = v1._covariance(params[:2])
        between = v1._covariance(params[2:4])
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
            value -= v1._gamma2_log_penalty(math.sqrt(matrix[0, 0]), PENALTY_MODES["log_speed_sd"])
            value -= v1._gamma2_log_penalty(math.sqrt(matrix[1, 1]), PENALTY_MODES["lag_sd_h"])
        return value, beta, np.linalg.inv(a_total)

    def objective(params: FloatArray) -> float:
        if np.any(np.abs(params) > 8.0):
            return 1e12
        result = assemble(params)
        return 1e12 if result is None else result[0]

    start = np.array([math.log(0.15), math.log(2.0), math.log(0.15), math.log(3.0)])
    best, _ = nelder_mead(objective, start)
    for _ in range(3):
        best, _ = nelder_mead(objective, best, step=0.2)
    final = assemble(best)
    if final is None:
        raise InvalidRequest("Population fit did not converge to a positive-definite model")
    value, beta, beta_covariance = final
    return PopulationFit2(
        contexts=contexts,
        context_means={c: beta[2 * index[c] : 2 * index[c] + 2] for c in contexts},
        context_mean_covariance={
            c: beta_covariance[2 * index[c] : 2 * index[c] + 2, 2 * index[c] : 2 * index[c] + 2]
            for c in contexts
        },
        within_study=v1._covariance(best[:2]),
        between_study=v1._covariance(best[2:4]),
        noise_sd=centers,
        study_noise_sd={f"{study}/{kind}": v for (study, kind), v in estimates.items()},
        training_series=tuple(s.series_id for s in series),
        training_studies=tuple(sorted(studies)),
        objective=value,
        tails_um=tails,
        tail_selection=selection,
    )


def posterior(
    prior: Mapping[str, Any],
    *,
    series: WoundSeries | None,
    noise: NoiseModel | float,
    tail_um: float,
    student_df: float | None = None,
) -> Posterior:
    if prior.get("flat"):
        log_prior = np.zeros(GRID_LAG.size)
    else:
        log_prior = v1._prior_log_density(prior["mean"], prior["covariance"], student_df=student_df)
    if isinstance(noise, NoiseModel):
        sigmas, log_noise = noise.levels()
    else:
        sigmas, log_noise = np.array([float(noise)]), np.zeros(1)
    columns = []
    for sigma, log_weight in zip(sigmas, log_noise, strict=True):
        column = log_prior + log_weight
        if series is not None:
            column = column + grid_log_likelihood(series, float(sigma), tail_um)
        columns.append(column)
    joint = v1._normalize(np.stack(columns, axis=1).ravel()).reshape(GRID_LAG.size, len(sigmas))
    return Posterior(joint=joint, sigmas=sigmas)


def predictive(
    fitted: Posterior,
    *,
    hours: Sequence[float],
    radius0_um: float,
    tail_um: float,
    draws: int = 4000,
    seed: int = 0,
    quantiles: Sequence[float] = (0.05, 0.1, 0.5, 0.9, 0.95),
) -> dict[str, Any]:
    """Observed-value predictive: noisy radius, reported 0 below detection."""
    if not hours:
        raise InvalidRequest("Predictive queries need at least one hour")
    rho = normalized_radius(
        hours, speed_um_h=GRID_SPEED, lag_h=GRID_LAG, radius0_um=radius0_um, tail_um=tail_um
    )
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
    }


def cohort_mean_prediction(
    fit: PopulationFit2,
    *,
    context: str,
    hours: Sequence[float],
    radius0_um: float,
    animals: int = 6,
    draws: int = 2000,
    seed: int = 0,
    quantiles: Sequence[float] = (0.05, 0.1, 0.5, 0.9, 0.95),
) -> dict[str, Any]:
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
    rho = normalized_radius(
        hours, speed_um_h=speed, lag_h=lag, radius0_um=radius0_um, tail_um=fit.tail(context)
    )
    detection = min(0.5, DETECTION_RADIUS_UM / radius0_um)
    reported = np.where(rho > detection, rho**2, 0.0)
    means = reported.reshape(draws, animals, len(hours)).mean(axis=1)
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


def cohort_posteriors(
    group_prior: Mapping[str, Any],
    *,
    within_study: FloatArray,
    cohort: Sequence[WoundSeries],
    noise: NoiseModel,
    tail_um: float,
    student_df: float | None = None,
) -> list[Posterior]:
    """As 1.0.0 ``cohort_posteriors`` with the tail kinematics."""
    if not cohort:
        raise InvalidRequest("Cohort assimilation needs at least one animal prefix")
    kernel = v1._gaussian_kernel(within_study)
    log_prior = v1._prior_log_density(
        group_prior["mean"], group_prior["covariance"], student_df=student_df
    )
    sigmas, log_noise = noise.levels()
    per_level = []
    evidence = []
    for sigma, log_weight in zip(sigmas, log_noise, strict=True):
        own = [grid_log_likelihood(item, float(sigma), tail_um) for item in cohort]
        blurred = [v1._blur_log(log_likelihood, kernel) for log_likelihood in own]
        total = log_prior + np.sum(blurred, axis=0)
        peak = float(np.max(total))
        evidence.append(log_weight + peak + math.log(float(np.sum(np.exp(total - peak)))))
        animals = []
        for index, log_likelihood in enumerate(own):
            animal_prior = v1._blur_log(total - blurred[index], kernel)
            animals.append(animal_prior + log_likelihood)
        per_level.append(animals)
    level_weights = v1._normalize(np.asarray(evidence))
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
