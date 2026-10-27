"""Leave-one-study-out and sealed-test evaluation of corneal wound forecasters.

Every evaluated study is excluded from population fitting, noise estimation and
the analog library before any of its values are predicted. Two tasks are scored:

* ``initial_state``: from wound size/species only, predict each control group's
  mean area fraction and each control animal's predictive interval;
* ``next_observation``: after each actual observation, predict the next one.

Errors are area-fraction percentage points (pp). No result here is a biological
validation claim; acceptance flags only restate declared numerical criteria.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Sequence
from typing import Any

import numpy as np

from peykar.model_learning import corneal_population as population
from peykar.model_learning.corneal_analog import displacement_at, forecast_analog
from peykar.model_learning.corneal_online import forecast_corneal_radius_trend
from peykar.model_learning.corneal_population import PopulationFit, WoundSeries
from peykar_core.errors import InvalidRequest

EVALUATION_ID = "cornea.population-evaluation"
EVALUATION_VERSION = "1.0.0"
RMSE_LIMIT_PP = 5.0
MAX_LIMIT_PP = 10.0
PERSISTENCE_MARGIN_PP = 0.5
CROSSON_SPEED_UM_H = 64.0
CROSSON_LAG_H = 5.5
INTERVALS = {"80": ("0.1", "0.9"), "90": ("0.05", "0.95")}
DEFAULT_COHORT_ANIMALS = 6
STUDENT_DF = 4.0
# A context absent from training borrows the pooled prior with doubled SDs.
FALLBACK_SD_FACTOR = 2.0
PROBABILISTIC = (
    "cohort_assimilation",
    "population_assimilation",
    "population_assimilation_t4",
    "radius_lag_flat_prior",
)


def crosson_fraction(hours: Sequence[float], radius0_um: float) -> list[float]:
    rho = population.normalized_radius(
        hours, speed_um_h=CROSSON_SPEED_UM_H, lag_h=CROSSON_LAG_H, radius0_um=radius0_um
    )[0]
    return [float(v) for v in rho**2]


def pooled_curve_fraction(
    library: Sequence[WoundSeries], hours: Sequence[float], radius0_um: float
) -> list[float]:
    """Mean physical displacement of training controls, mapped to this wound size."""
    controls = [s for s in library if s.role == "control"]
    if not controls:
        raise InvalidRequest("Pooled empirical curve needs training controls")
    displacement = np.mean([displacement_at(s, hours) for s in controls], axis=0)
    rho = np.maximum(0.0, 1.0 - displacement / radius0_um)
    return [float(v) for v in rho**2]


def _noise(fit: PopulationFit, series: WoundSeries) -> population.NoiseModel:
    return fit.noise(series.kind)


def _fit_with_fallback(
    controls: Sequence[WoundSeries], context: str, training: Sequence[WoundSeries]
) -> tuple[PopulationFit, str] | None:
    """Context-specific fit when that context is trained, otherwise a pooled fit."""
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


def _errors(predicted: Sequence[float], observed: Sequence[float]) -> dict[str, Any]:
    diff = 100.0 * (np.asarray(predicted) - np.asarray(observed))
    return {
        "n": int(diff.size),
        "rmse_pp": float(math.sqrt(float(np.mean(diff**2)))),
        "max_abs_pp": float(np.max(np.abs(diff))),
        "signed_errors_pp": [float(v) for v in diff],
    }


def _coverage(quantiles: dict[str, list[float]], observed: Sequence[float]) -> dict[str, Any]:
    result = {}
    for name, (low, high) in INTERVALS.items():
        inside = [
            lo <= value <= hi
            for lo, hi, value in zip(quantiles[low], quantiles[high], observed, strict=True)
        ]
        result[name] = {"inside": int(sum(inside)), "n": len(inside)}
    return result


def _cohort_targets(study: Sequence[WoundSeries]) -> list[dict[str, Any]]:
    """Observed control-group mean curves; individual groups use shared hours only."""
    targets = []
    groups: dict[str, list[WoundSeries]] = {}
    for item in study:
        if item.role == "control":
            groups.setdefault(item.group or item.series_id, []).append(item)
    for group, members in sorted(groups.items()):
        if len(members) == 1 and members[0].is_aggregate:
            item = members[0]
            targets.append(
                {
                    "group": group,
                    "kind": "published_group_mean",
                    "animals": item.aggregate_n or DEFAULT_COHORT_ANIMALS,
                    "animals_known": bool(item.aggregate_n),
                    "hours": list(item.hours),
                    "observed": list(item.fractions),
                    "radius0_um": item.radius0_um,
                    "context": item.context,
                }
            )
            continue
        individuals = [m for m in members if not m.is_aggregate]
        if not individuals:
            continue
        shared = sorted(set.intersection(*(set(m.hours) for m in individuals)))
        if not shared:
            continue
        observed = [
            float(np.mean([m.fractions[m.hours.index(h)] for m in individuals])) for h in shared
        ]
        radii = {m.radius0_um for m in individuals}
        targets.append(
            {
                "group": group,
                "kind": "mean_of_individual_series",
                "animals": len(individuals),
                "animals_known": True,
                "hours": shared,
                "observed": observed,
                "radius0_um": float(np.mean(sorted(radii))),
                "context": individuals[0].context,
            }
        )
    return targets


def oracle_floor(series: WoundSeries, sigma: float = 0.03) -> dict[str, Any]:
    """Best radius-lag fit to ALL of a series' points: a diagnostic floor, not a forecast.

    If even this in-sample fit misses the 5/10 pp targets, no forecaster within the
    radius-lag family can meet them without fitting noise.
    """
    best = int(np.argmax(population.grid_log_likelihood(series, sigma)))
    rho = population.normalized_radius(
        series.hours,
        speed_um_h=population.GRID_SPEED[best],
        lag_h=population.GRID_LAG[best],
        radius0_um=series.radius0_um,
    )[0]
    fitted = np.where(rho > series.detection_rho, rho**2, np.where(rho > 0, rho**2, 0.0))
    score = _errors(list(fitted), series.fractions)
    return {
        "speed_um_h": float(population.GRID_SPEED[best]),
        "lag_h": float(population.GRID_LAG[best]),
        "in_sample_rmse_pp": score["rmse_pp"],
        "in_sample_max_abs_pp": score["max_abs_pp"],
        "within_5_10_pp": _passes(score),
    }


def _passes(errors: dict[str, Any]) -> bool:
    return bool(errors["rmse_pp"] <= RMSE_LIMIT_PP and errors["max_abs_pp"] <= MAX_LIMIT_PP)


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
    widen = FALLBACK_SD_FACTOR**2 if used_context != context else 1.0
    group_prior = fit.group_prior(used_context)
    group_prior = {**group_prior, "covariance": group_prior["covariance"] * widen}
    # Task 1: group-mean curves from initial state only.
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
    # Individual-animal predictive intervals from initial state only.
    for item in study_series:
        if item.role != "control" or item.is_aggregate:
            continue
        prior = fit.prior(used_context, effective_n=1, new_study=True)
        prediction = population.predictive(
            population.posterior(prior, series=None, noise=_noise(fit, item)),
            hours=item.hours,
            radius0_um=item.radius0_um,
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
    # Task 2: causal next-observation forecasts.
    library = list(training)
    cohorts: dict[str, list[WoundSeries]] = {}
    for member in study_series:
        if not member.is_aggregate:
            cohorts.setdefault(member.group or member.series_id, []).append(member)
    cohort_cache: dict[tuple[str, float], dict[str, population.Posterior]] = {}

    def cohort_posterior(member: WoundSeries, origin: float) -> population.Posterior | None:
        group = member.group or member.series_id
        mates = cohorts.get(group, [])
        if member.is_aggregate or len(mates) < 2:
            return None
        key = (group, origin)
        if key not in cohort_cache:
            available = [m for m in mates if m.hours[0] <= origin]
            prefixes = [m.prefix(origin) for m in available]
            posteriors = population.cohort_posteriors(
                group_prior,
                within_study=fit.within_study,
                cohort=prefixes,
                noise=fit.noise("individual"),
                student_df=STUDENT_DF,
            )
            cohort_cache[key] = {m.series_id: p for m, p in zip(available, posteriors, strict=True)}
        return cohort_cache[key].get(member.series_id)

    for item in study_series:
        rows = []
        prior = fit.prior(used_context, effective_n=item.effective_n, new_study=True)
        prior = {**prior, "covariance": prior["covariance"] * widen}
        noise = _noise(fit, item)
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
            bayes = population.predictive(
                population.posterior(prior, series=prefix, noise=noise),
                hours=[target_hour],
                radius0_um=item.radius0_um,
                draws=draws,
                seed=seed,
            )
            preds["population_assimilation"] = bayes["mean_area_fraction"][0]
            row["population_assimilation_interval"] = {
                q: values[0] for q, values in bayes["quantiles"].items()
            }
            for name, variant_prior, df in (
                ("population_assimilation_t4", prior, STUDENT_DF),
                ("radius_lag_flat_prior", population.flat_prior(), None),
            ):
                variant = population.predictive(
                    population.posterior(variant_prior, series=prefix, noise=noise, student_df=df),
                    hours=[target_hour],
                    radius0_um=item.radius0_um,
                    draws=draws,
                    seed=seed,
                )
                preds[name] = variant["mean_area_fraction"][0]
                row[f"{name}_interval"] = {
                    q: values[0] for q, values in variant["quantiles"].items()
                }
            cohort_fit = cohort_posterior(item, prefix.hours[-1])
            if cohort_fit is not None:
                pooled = population.predictive(
                    cohort_fit,
                    hours=[target_hour],
                    radius0_um=item.radius0_um,
                    draws=draws,
                    seed=seed,
                )
                preds["cohort_assimilation"] = pooled["mean_area_fraction"][0]
                row["cohort_assimilation_interval"] = {
                    q: values[0] for q, values in pooled["quantiles"].items()
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
                "oracle_fit_floor": oracle_floor(item),
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
    """Run :func:`evaluate_study` for each study, refitting without it every time."""
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
        },
        "studies": reports,
        "summary": summarize(reports),
    }


def summarize(reports: Sequence[dict[str, Any]]) -> dict[str, Any]:
    """Pass counts and pooled errors per method, never replacing per-series results."""
    summary: dict[str, Any] = {"initial_state": {}, "next_observation": {}, "coverage": {}}
    for task in ("initial_state", "next_observation"):
        buckets: dict[str, dict[str, Any]] = {}
        for report in reports:
            for entry in report[task]:
                for name, score in entry["scores"].items():
                    bucket = buckets.setdefault(name, {"series": 0, "pass": 0, "errors": []})
                    bucket["series"] += 1
                    passed = score["meets_5_10_pp"]
                    if task == "next_observation":
                        passed = passed and score["improves_persistence_by_0_5_pp"]
                    bucket["pass"] += int(passed)
                    bucket["errors"].extend(score["signed_errors_pp"])
        summary[task] = {
            name: {
                "series": bucket["series"],
                "series_meeting_criteria": bucket["pass"],
                "pooled_rmse_pp": float(math.sqrt(np.mean(np.square(bucket["errors"])))),
                "pooled_max_abs_pp": float(np.max(np.abs(bucket["errors"]))),
                "targets": len(bucket["errors"]),
            }
            for name, bucket in sorted(buckets.items())
        }
    for name in INTERVALS:
        per_method = {}
        for method in PROBABILISTIC:
            inside = n = 0
            for report in reports:
                for entry in report["next_observation"]:
                    inside += entry["interval_coverage"][method][name]["inside"]
                    n += entry["interval_coverage"][method][name]["n"]
            per_method[method] = {"inside": inside, "n": n}
        animal_inside = animal_n = 0
        for report in reports:
            for entry in report["individual_intervals"]:
                animal_inside += entry["coverage"][name]["inside"]
                animal_n += entry["coverage"][name]["n"]
        summary["coverage"][name] = {
            "next_observation": per_method,
            "initial_state_individual": {"inside": animal_inside, "n": animal_n},
        }
    return summary
