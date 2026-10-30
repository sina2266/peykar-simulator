"""Leave-one-study-out and sealed-test evaluation with generic eye model version 5
(harness 7).

Harness 6 (``eye_recovery_evaluation6``, unchanged because frozen protocols pin it) with
every model call going to ``eye_recovery_population5``: population fits, posteriors and
predictive distributions use version 5's law, so a configuration that declares a zero
start is evaluated with it. The tasks, comparators, scores, summaries and initial-state
intervals are harness 6's (its model-independent parts are imported from harness 1 and
the IOP harness, as harness 2 and 6 import them). With a configuration whose start is
``"free"`` a report equals harness 6's apart from the evaluation version and the
configuration description (unit-tested). Every evaluated study is excluded from
population fitting, noise estimation, pooled curves and the analog library before any
of its values are predicted. No result here is a clinical validation claim.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Sequence
from typing import Any

from peykar.model_learning import eye_recovery_population2 as version2
from peykar.model_learning import eye_recovery_population5 as population
from peykar.model_learning.corneal_population import NoiseModel
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
from peykar_core.errors import InvalidRequest

__all__ = [
    "INTERVAL_FIELD",
    "criteria",
    "evaluate_study",
    "leave_one_study_out",
    "summarize",
]

EVALUATION_ID = "eye.recovery-evaluation"
EVALUATION_VERSION = "7.0.0"
INTERVAL_FIELD = "initial_state_intervals"


def _fit(
    config: version2.RecoveryConfig, training: Sequence[EvaluatedSeries]
) -> population.RecoveryFit | None:
    controls = [s for s in training if s.role == "control" and s.reference_value > 0]
    if len(controls) < 2 or len({s.study_id for s in controls}) < 2:
        return None
    return population.fit_population(config, controls, noise_series=training)


def _variants(
    config: version2.RecoveryConfig, prior: dict[str, Any] | None
) -> list[tuple[str, population.Prior]]:
    variants: list[tuple[str, population.Prior]] = [("weak_prior", population.weak_prior(config))]
    if prior is None:
        return variants
    mean, covariance = prior["mean"], prior["covariance"]
    gaussian = population.gaussian(mean, covariance)
    t4 = population.student(mean, covariance, STUDENT_DF)
    return [
        ("population_assimilation", gaussian),
        ("population_assimilation_t4", t4),
        ("population_assimilation_robust", population.robust(config, gaussian)),
        ("population_anchored", population.anchored(gaussian)),
        ("population_anchored_t4", population.anchored(t4)),
        *variants,
    ]


def _series_prior(
    fit: population.RecoveryFit, item: EvaluatedSeries
) -> tuple[dict[str, Any] | None, str | None]:
    try:
        covariate = population.level_covariate(fit.config, item)
        return fit.prior(item.context, item.kind, covariate), fit.level_used(item.context)
    except InvalidRequest:
        return None, None


def _noise(fit: population.RecoveryFit, item: EvaluatedSeries) -> NoiseModel:
    return fit.noise(item.kind, item.context, item.aggregate_n)


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
    if not isinstance(config, version2.RecoveryConfig):
        raise InvalidRequest("Version-5 evaluation needs a version-2 or version-5 configuration")
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
        predictions[name] = result["point_fraction"]
        intervals[name] = result["quantiles"]
    return predictions, intervals, level


def _initial_state_entry(
    config: version2.RecoveryConfig,
    item: EvaluatedSeries,
    training: Sequence[EvaluatedSeries],
    fit: population.RecoveryFit | None,
) -> dict[str, Any]:
    """Harness 2's initial-state entry plus harness 6's predictive quantiles."""
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
    intervals: dict[str, dict[str, list[float]]] = {}
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
                entry["predictions"][name] = prediction["point_fraction"]
                entry["population_interval_coverage"][name] = _coverage(
                    prediction["quantiles"], item.fractions
                )
                intervals[name] = {q: list(v) for q, v in prediction["quantiles"].items()}
    entry["scores"] = {}
    for name, values in entry["predictions"].items():
        score = _errors(values, item.fractions)
        score["meets_5_10_pp"] = _passes(score)
        entry["scores"][name] = score
    if intervals:
        entry[INTERVAL_FIELD] = intervals
    return entry


def evaluate_study(
    config: version2.RecoveryConfig,
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
    config: version2.RecoveryConfig,
    series: Sequence[EvaluatedSeries],
    *,
    studies: Sequence[str] | None = None,
    clusters: Sequence[Sequence[str]] = (),
    progress: Callable[[str], None] | None = None,
) -> dict[str, Any]:
    """Hold out each study; studies of one declared cluster leave training together."""
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
