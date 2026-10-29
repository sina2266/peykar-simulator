"""Leave-one-study-out and sealed-test evaluation with generic eye model version 3.

The tasks, baselines, scores, summaries and model calls are those of
``eye_recovery_evaluation2`` (unchanged because the retinal-thickness 1.1 and IOP 1.2
protocols pin it; its helpers are imported here). The only difference is the point
forecast of the population variants: it follows the configuration's ``point`` rule
(``eye_recovery_population3.point_forecast``), computed from the same predictive
distribution whose quantiles are reported. With ``point = "median"`` the reports equal
version 2's apart from the evaluation version. Every evaluated study is excluded from
population fitting, noise estimation, pooled curves and the analog library before any
of its values are predicted. No result here is a clinical validation claim.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Sequence
from typing import Any

from peykar.model_learning import eye_recovery_population2 as population
from peykar.model_learning.eye_recovery_population3 import point_forecast
from peykar.validation.cornea_population_evaluation import _coverage, _errors, _passes
from peykar.validation.eye_iop_evaluation import (
    FORECAST_TASKS,
    INITIAL_STATE_VARIANTS,
    STUDENT_DF,
    _interval_coverage,
    _log_linear,
    _score_rows,
    summarize,
)
from peykar.validation.eye_recovery_evaluation import (
    EvaluatedSeries,
    _kind,
    analog_forecast,
    criteria,
    pooled_log_curve,
)
from peykar.validation.eye_recovery_evaluation2 import _fit, _noise, _series_prior, _variants
from peykar_core.errors import InvalidRequest

__all__ = ["STUDENT_DF", "criteria", "evaluate_study", "leave_one_study_out", "summarize"]

EVALUATION_ID = "eye.recovery-evaluation"
EVALUATION_VERSION = "3.0.0"


def _point(config: population.RecoveryConfig) -> str:
    return str(getattr(config, "point", "median"))


def _points(config: population.RecoveryConfig, result: dict[str, Any]) -> list[float]:
    rule = _point(config)
    quantiles = result["quantiles"]
    return [
        point_forecast(m, a, b, rule)
        for m, a, b in zip(
            result["point_fraction"], quantiles["0.05"], quantiles["0.95"], strict=True
        )
    ]


def _model_forecasts(
    fit: population.RecoveryFit | None,
    item: EvaluatedSeries,
    prefix: EvaluatedSeries,
    targets: Sequence[float],
) -> tuple[dict[str, list[float]], dict[str, dict[str, list[float]]], str | None]:
    predictions: dict[str, list[float]] = {}
    intervals: dict[str, dict[str, list[float]]] = {}
    if fit is None:
        return predictions, intervals, None
    config = fit.config
    if not isinstance(config, population.RecoveryConfig):
        raise InvalidRequest("Version-3 evaluation needs a version-2 or version-3 configuration")
    noise = _noise(fit, item)
    extra = fit.forecast_inflation(item.kind, item.context, item.aggregate_n)
    prior, level = _series_prior(fit, item)
    for name, variant in _variants(config, prior):
        result = population.predictive(
            config,
            population.posterior(config, variant, series=prefix, noise=noise),
            months=list(targets),
            extra_noise=extra,
        )
        predictions[name] = _points(config, result)
        intervals[name] = result["quantiles"]
    return predictions, intervals, level


def _initial_state_entry(
    config: population.RecoveryConfig,
    item: EvaluatedSeries,
    training: Sequence[EvaluatedSeries],
    fit: population.RecoveryFit | None,
) -> dict[str, Any]:
    entry: dict[str, Any] = {
        "series_id": item.series_id,
        "context": item.context,
        "kind": _kind(item),
        "reference_value": item.reference_value,
        "months": list(item.months),
        "observed": list(item.fractions),
        "predictions": {},
    }
    pooled = pooled_log_curve(config, training, item.context, item.months)
    if pooled is not None:
        entry["predictions"]["pooled_training_curve"] = [math.exp(v) for v in pooled[0]]
        entry["pooled_level_used"] = pooled[1]
    if fit is not None:
        prior, level = _series_prior(fit, item)
        if prior is None:
            entry["population_unavailable"] = "no baseline value or no family prior"
        else:
            entry["population_level_used"] = level
            entry["population_interval_coverage"] = {}
            duration = population.effect_duration(item)
            extra = fit.forecast_inflation(item.kind, item.context, item.aggregate_n)
            for name, df in INITIAL_STATE_VARIANTS:
                variant = (
                    population.gaussian(prior["mean"], prior["covariance"])
                    if df is None
                    else population.student(prior["mean"], prior["covariance"], df)
                )
                prediction = population.predictive(
                    config,
                    population.posterior(
                        config, variant, series=None, noise=_noise(fit, item), duration=duration
                    ),
                    months=list(item.months),
                    extra_noise=extra,
                )
                entry["predictions"][name] = _points(config, prediction)
                entry["population_interval_coverage"][name] = _coverage(
                    prediction["quantiles"], item.fractions
                )
    entry["scores"] = {}
    for name, values in entry["predictions"].items():
        score = _errors(values, item.fractions)
        score["meets_5_10_pp"] = _passes(score)
        entry["scores"][name] = score
    return entry


def evaluate_study(
    config: population.RecoveryConfig,
    *,
    study_series: Sequence[EvaluatedSeries],
    training: Sequence[EvaluatedSeries],
) -> dict[str, Any]:
    """Score one excluded study against models built only from ``training``."""
    study_ids = {s.study_id for s in study_series}
    if len(study_ids) != 1:
        raise InvalidRequest("Evaluate exactly one study at a time")
    study_id = next(iter(study_ids))
    if any(s.study_id == study_id for s in training):
        raise InvalidRequest("The evaluated study must be absent from training")
    fit = _fit(config, training)
    result: dict[str, Any] = {
        "study_id": study_id,
        "contexts": sorted({s.context for s in study_series}),
        "evaluation_version": EVALUATION_VERSION,
        "training_studies": sorted({s.study_id for s in training}),
        "population_fit": fit.describe() if fit is not None else None,
        "initial_state": [],
        "next_observation": [],
        "landmark": [],
    }
    if fit is None:
        result["skipped_reason"] = "fewer than two training studies with controls"
    for item in study_series:
        if item.role == "control":
            result["initial_state"].append(_initial_state_entry(config, item, training, fit))
    library = list(training)
    for item in study_series:
        if len(item.months) < 2:
            continue
        for task in FORECAST_TASKS:
            rows = []
            origins = range(1, len(item.months)) if task == "next_observation" else [1]
            for k in origins:
                prefix = item.prefix(item.months[k - 1])
                targets = list(item.months[k:]) if task == "landmark" else [item.months[k]]
                history = [(0.0, 1.0), *zip(prefix.months, prefix.fractions, strict=True)]
                origin, last = history[-1]
                model, intervals, level = _model_forecasts(fit, item, prefix, targets)
                pooled = pooled_log_curve(config, training, item.context, [origin, *targets])
                for position, target in enumerate(targets):
                    observed = item.fractions[item.months.index(target)]
                    predictions: dict[str, float] = {
                        "persistence": last,
                        "baseline_level": 1.0,
                        "last_two_log_linear": _log_linear(history[-2:], target),
                        "reference_to_last_log_linear": _log_linear(
                            [history[0], history[-1]], target
                        ),
                    }
                    if pooled is not None:
                        predictions["pooled_curve_ratio"] = last * math.exp(
                            pooled[0][1 + position] - pooled[0][0]
                        )
                    analog = analog_forecast(
                        config, library, context=item.context, history=history, target_month=target
                    )
                    if analog is not None:
                        predictions["analog_knn"] = analog["fraction"]
                    for name, values in model.items():
                        predictions[name] = values[position]
                    rows.append(
                        {
                            "origin_month": origin,
                            "target_month": target,
                            "observed": observed,
                            "history_points": len(history),
                            "predictions": predictions,
                            "intervals": {
                                name: {q: v[position] for q, v in quantiles.items()}
                                for name, quantiles in intervals.items()
                            },
                            "population_level_used": level,
                        }
                    )
            result[task].append(
                {
                    "series_id": item.series_id,
                    "context": item.context,
                    "role": item.role,
                    "kind": _kind(item),
                    "rows": rows,
                    "scores": _score_rows(rows),
                    "interval_coverage": _interval_coverage(rows),
                }
            )
    return result


def leave_one_study_out(
    config: population.RecoveryConfig,
    series: Sequence[EvaluatedSeries],
    *,
    studies: Sequence[str] | None = None,
    clusters: Sequence[Sequence[str]] = (),
    progress: Callable[[str], None] | None = None,
) -> dict[str, Any]:
    """Hold out each study; studies of one declared cluster (possibly shared cohorts)
    leave training together."""
    ids = sorted({s.study_id for s in series}) if studies is None else list(studies)
    linked: dict[str, set[str]] = {}
    for cluster in clusters:
        for member in cluster:
            linked.setdefault(member, set()).update(cluster)
    reports = []
    for study_id in ids:
        if progress:
            progress(study_id)
        held = [s for s in series if s.study_id == study_id]
        if not held:
            raise InvalidRequest(f"Unknown study {study_id!r}")
        excluded = linked.get(study_id, set()) | {study_id}
        training = [s for s in series if s.study_id not in excluded]
        report = evaluate_study(config, study_series=held, training=training)
        report["training_exclusions"] = sorted(excluded - {study_id})
        reports.append(report)
    return {
        "evaluation_id": EVALUATION_ID,
        "evaluation_version": EVALUATION_VERSION,
        "model_id": config.model_id,
        "model_version": config.model_version,
        "model_configuration": config.describe(),
        "criteria": criteria(config),
        "clusters": [sorted(c) for c in clusters],
        "studies": reports,
        "summary": summarize(reports),
    }
