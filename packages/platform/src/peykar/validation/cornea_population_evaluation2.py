"""Evaluation harness 2.0.0 for population model 2.0.0 (tail kinematics).

Same tasks, criteria, comparators and summaries as the frozen 1.0.0 harness, whose
helpers are reused unchanged; only the population-model calls carry the context's
tail radius. Kept separate so the 1.0.0 sealed replay stays byte-identical.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Sequence
from typing import Any

import numpy as np

from peykar.model_learning import corneal_population2 as population
from peykar.model_learning.corneal_analog import forecast_analog
from peykar.model_learning.corneal_online import forecast_corneal_radius_trend
from peykar.model_learning.corneal_population import (
    GRID_LAG,
    GRID_SPEED,
    NoiseModel,
    Posterior,
    WoundSeries,
    flat_prior,
)
from peykar.validation.cornea_population_evaluation import (
    FALLBACK_SD_FACTOR,
    INTERVALS,
    MAX_LIMIT_PP,
    PERSISTENCE_MARGIN_PP,
    RMSE_LIMIT_PP,
    STUDENT_DF,
    _cohort_targets,
    _coverage,
    _errors,
    _passes,
    crosson_fraction,
    pooled_curve_fraction,
    summarize,
)
from peykar_core.errors import InvalidRequest

EVALUATION_ID = "cornea.population-evaluation"
EVALUATION_VERSION = "2.0.0"
CRITERIA_VERSION_NOTE = "criteria unchanged from evaluation 1.0.0"
PROBABILISTIC = (
    "cohort_assimilation",
    "population_assimilation",
    "population_assimilation_t4",
    "radius_lag_flat_prior",
)


def _fit_with_fallback(
    controls: Sequence[WoundSeries], context: str, training: Sequence[WoundSeries]
) -> tuple[population.PopulationFit2, str] | None:
    if len(controls) < 2 or len({s.study_id for s in controls}) < 2:
        return None
    if any(s.context == context for s in controls):
        return population.fit_population(controls, noise_series=training), context
    pooled = [
        WoundSeries(
            series_id=s.series_id,
            study_id=s.study_id,
            context="pooled-all-contexts",
            radius0_um=s.radius0_um,
            hours=s.hours,
            fractions=s.fractions,
            aggregate_n=s.aggregate_n,
            role=s.role,
            group=s.group,
            baseline_observed=s.baseline_observed,
        )
        for s in controls
    ]
    return population.fit_population(pooled, noise_series=training), "pooled-all-contexts"


def oracle_floor(series: WoundSeries, tail_um: float, sigma: float = 0.03) -> dict[str, Any]:
    best = int(np.argmax(population.grid_log_likelihood(series, sigma, tail_um)))
    rho = population.normalized_radius(
        series.hours,
        speed_um_h=GRID_SPEED[best],
        lag_h=GRID_LAG[best],
        radius0_um=series.radius0_um,
        tail_um=tail_um,
    )[0]
    fitted = np.where(rho > series.detection_rho, rho**2, 0.0)
    score = _errors(list(fitted), series.fractions)
    return {
        "speed_um_h": float(GRID_SPEED[best]),
        "lag_h": float(GRID_LAG[best]),
        "tail_um": tail_um,
        "in_sample_rmse_pp": score["rmse_pp"],
        "in_sample_max_abs_pp": score["max_abs_pp"],
        "within_5_10_pp": _passes(score),
    }


def evaluate_study(
    *,
    study_series: Sequence[WoundSeries],
    training: Sequence[WoundSeries],
    draws: int = 2000,
    seed: int = 0,
) -> dict[str, Any]:
    """Score one excluded study against models built only from ``training``."""
    study_ids = {s.study_id for s in study_series}
    if len(study_ids) != 1:
        raise InvalidRequest("Evaluate exactly one study at a time")
    study_id = next(iter(study_ids))
    if any(s.study_id == study_id for s in training):
        raise InvalidRequest("The evaluated study must be absent from training")
    controls = [s for s in training if s.role == "control"]
    context = study_series[0].context
    fitted = _fit_with_fallback(controls, context, training)
    result: dict[str, Any] = {
        "study_id": study_id,
        "context": context,
        "evaluation_version": EVALUATION_VERSION,
        "training_studies": sorted({s.study_id for s in training}),
        "training_control_series": len(controls),
        "population_context_used": fitted[1] if fitted else None,
        "population_fit": fitted[0].describe() if fitted else None,
        "initial_state": [],
        "individual_intervals": [],
        "next_observation": [],
    }
    if fitted is None:
        result["skipped_reason"] = "fewer than two training studies with controls"
        return result
    fit, used_context = fitted
    tail = fit.tail(used_context)
    result["tail_radius_um_used"] = tail
    widen = FALLBACK_SD_FACTOR**2 if used_context != context else 1.0
    group_prior = fit.group_prior(used_context)
    group_prior = {**group_prior, "covariance": group_prior["covariance"] * widen}
    for target in _cohort_targets(study_series):
        hours = target["hours"]
        observed = target["observed"]
        cohort = population.cohort_mean_prediction(
            fit,
            context=used_context,
            hours=hours,
            radius0_um=target["radius0_um"],
            animals=target["animals"],
            draws=draws,
            seed=seed,
        )
        methods = {
            "population_initial_state": cohort["mean_area_fraction"],
            "crosson1986_fixed": crosson_fraction(hours, target["radius0_um"]),
            "pooled_training_curve": pooled_curve_fraction(training, hours, target["radius0_um"]),
        }
        scores = {name: _errors(values, observed) for name, values in methods.items()}
        for score in scores.values():
            score["meets_5_10_pp"] = _passes(score)
        result["initial_state"].append(
            {
                **{k: v for k, v in target.items() if k != "observed"},
                "observed": observed,
                "predictions": methods,
                "scores": scores,
                "population_interval_coverage": _coverage(cohort["quantiles"], observed),
            }
        )
    for item in study_series:
        if item.role != "control" or item.is_aggregate:
            continue
        prior = fit.prior(used_context, effective_n=1, new_study=True)
        prior = {**prior, "covariance": prior["covariance"] * widen}
        prediction = population.predictive(
            population.posterior(prior, series=None, noise=fit.noise(item.kind), tail_um=tail),
            hours=item.hours,
            radius0_um=item.radius0_um,
            tail_um=tail,
            draws=draws,
            seed=seed,
        )
        score = _errors(prediction["mean_area_fraction"], item.fractions)
        score["meets_5_10_pp"] = _passes(score)
        result["individual_intervals"].append(
            {
                "series_id": item.series_id,
                "score": score,
                "coverage": _coverage(prediction["quantiles"], item.fractions),
            }
        )
    library = list(training)
    cohorts: dict[str, list[WoundSeries]] = {}
    for member in study_series:
        if not member.is_aggregate:
            cohorts.setdefault(member.group or member.series_id, []).append(member)
    cache: dict[tuple[str, float], dict[str, Posterior]] = {}

    def cohort_posterior(member: WoundSeries, origin: float) -> Posterior | None:
        group = member.group or member.series_id
        mates = cohorts.get(group, [])
        if member.is_aggregate or len(mates) < 2:
            return None
        key = (group, origin)
        if key not in cache:
            available = [m for m in mates if m.hours[0] <= origin]
            posteriors = population.cohort_posteriors(
                group_prior,
                within_study=fit.within_study,
                cohort=[m.prefix(origin) for m in available],
                noise=fit.noise("individual"),
                tail_um=tail,
                student_df=STUDENT_DF,
            )
            cache[key] = {m.series_id: p for m, p in zip(available, posteriors, strict=True)}
        return cache[key].get(member.series_id)

    for item in study_series:
        rows = []
        prior = fit.prior(used_context, effective_n=item.effective_n, new_study=True)
        prior = {**prior, "covariance": prior["covariance"] * widen}
        noise: NoiseModel = fit.noise(item.kind)
        for k in range(1, len(item.hours)):
            prefix = item.prefix(item.hours[k - 1])
            target_hour = item.hours[k]
            history = ([(0.0, 1.0)] if item.baseline_observed else []) + list(
                zip(prefix.hours, prefix.fractions, strict=True)
            )
            row: dict[str, Any] = {
                "origin_hour": prefix.hours[-1],
                "target_hour": target_hour,
                "observed": item.fractions[k],
                "actual_history_points": len(history),
                "predictions": {},
            }
            preds = row["predictions"]
            preds["persistence"] = prefix.fractions[-1]
            if len(history) >= 2:
                trend = forecast_corneal_radius_trend(
                    history=history, origin_hour=prefix.hours[-1], future_hours=[target_hour]
                )
                preds["last_two_radius_trend"] = trend["predictions"][0]["area_fraction"]
                first_hour, first_value = history[0]
                last_hour, last_value = history[-1]
                slope = (math.sqrt(last_value) - math.sqrt(first_value)) / (last_hour - first_hour)
                preds["first_to_current_trend"] = (
                    max(0.0, math.sqrt(last_value) + slope * (target_hour - last_hour)) ** 2
                )
            preds["crosson1986_fixed"] = crosson_fraction([target_hour], item.radius0_um)[0]
            for name, variant_prior, df in (
                ("population_assimilation", prior, None),
                ("population_assimilation_t4", prior, STUDENT_DF),
                ("radius_lag_flat_prior", flat_prior(), None),
            ):
                variant = population.predictive(
                    population.posterior(
                        variant_prior, series=prefix, noise=noise, tail_um=tail, student_df=df
                    ),
                    hours=[target_hour],
                    radius0_um=item.radius0_um,
                    tail_um=tail,
                    draws=draws,
                    seed=seed,
                )
                preds[name] = variant["mean_area_fraction"][0]
                row[f"{name}_interval"] = {q: v[0] for q, v in variant["quantiles"].items()}
            cohort_fit = cohort_posterior(item, prefix.hours[-1])
            if cohort_fit is not None:
                pooled = population.predictive(
                    cohort_fit,
                    hours=[target_hour],
                    radius0_um=item.radius0_um,
                    tail_um=tail,
                    draws=draws,
                    seed=seed,
                )
                preds["cohort_assimilation"] = pooled["mean_area_fraction"][0]
                row["cohort_assimilation_interval"] = {
                    q: v[0] for q, v in pooled["quantiles"].items()
                }
            try:
                analog = forecast_analog(library, prefix=prefix, future_hours=[target_hour])
                preds["analog_knn"] = analog["area_fraction"][0]
            except InvalidRequest as error:
                row["analog_unavailable"] = str(error)
            rows.append(row)
        if not rows:
            continue
        method_names = sorted({name for row in rows for name in row["predictions"]})
        series_scores: dict[str, dict[str, Any]] = {}
        for name in method_names:
            pairs = [
                (row["predictions"][name], row["observed"])
                for row in rows
                if name in row["predictions"]
            ]
            series_score = _errors([p for p, _ in pairs], [o for _, o in pairs])
            series_score["meets_5_10_pp"] = _passes(series_score)
            series_scores[name] = series_score
        persistence = series_scores["persistence"]["rmse_pp"]
        for series_score in series_scores.values():
            series_score["improves_persistence_by_0_5_pp"] = bool(
                series_score["rmse_pp"] <= persistence - PERSISTENCE_MARGIN_PP
            )
        coverage = {
            method: {
                interval: {
                    "inside": sum(
                        row[f"{method}_interval"][low]
                        <= row["observed"]
                        <= row[f"{method}_interval"][high]
                        for row in rows
                        if f"{method}_interval" in row
                    ),
                    "n": sum(f"{method}_interval" in row for row in rows),
                }
                for interval, (low, high) in INTERVALS.items()
            }
            for method in PROBABILISTIC
        }
        result["next_observation"].append(
            {
                "oracle_fit_floor": oracle_floor(item, tail),
                "series_id": item.series_id,
                "role": item.role,
                "aggregate": item.is_aggregate,
                "rows": rows,
                "scores": series_scores,
                "interval_coverage": coverage,
            }
        )
    return result


def leave_one_study_out(
    series: Sequence[WoundSeries],
    *,
    studies: Sequence[str] | None = None,
    progress: Callable[[str], None] | None = None,
    draws: int = 2000,
) -> dict[str, Any]:
    ids = sorted({s.study_id for s in series}) if studies is None else list(studies)
    reports = []
    for study_id in ids:
        if progress:
            progress(study_id)
        held = [s for s in series if s.study_id == study_id]
        if not held:
            raise InvalidRequest(f"Unknown study {study_id!r}")
        training = [s for s in series if s.study_id != study_id]
        reports.append(evaluate_study(study_series=held, training=training, draws=draws))
    return {
        "evaluation_id": EVALUATION_ID,
        "evaluation_version": EVALUATION_VERSION,
        "criteria": {
            "rmse_pp_max": RMSE_LIMIT_PP,
            "max_abs_pp_max": MAX_LIMIT_PP,
            "next_observation_persistence_margin_pp": PERSISTENCE_MARGIN_PP,
            "note": CRITERIA_VERSION_NOTE,
        },
        "studies": reports,
        "summary": summarize(reports),
    }
