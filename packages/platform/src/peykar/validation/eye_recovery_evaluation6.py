"""Leave-one-study-out and sealed-test evaluation that keeps initial-state intervals
(harness 6).

Harness 2 (``eye_recovery_evaluation2``, unchanged because frozen protocols pin it)
with one addition: every initial-state entry with a population forecast also carries
``initial_state_intervals``, the predictive quantiles (0.05, 0.1, 0.5, 0.9, 0.95) of
each initial-state variant at the entry's months. They come from the same fit, prior,
noise, effect duration and forecast inflation as harness 2's initial states, and the
recomputed point forecasts must equal harness 2's. The forecast tasks call harness 2's
functions with one fit per study; reports equal harness 2's apart from this field and
the evaluation version. The stacking layer (``eye_stacking_evaluation``) needs these
quantiles to give stacked initial states an interval. Every evaluated study is
excluded from population fitting, noise estimation, pooled curves and the analog
library before any of its values are predicted. No result here is a clinical
validation claim.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Sequence
from typing import Any

from peykar.model_learning import eye_recovery_population2 as population
from peykar.validation import eye_recovery_evaluation2 as harness2
from peykar.validation.eye_iop_evaluation import (
    FORECAST_TASKS,
    INITIAL_STATE_VARIANTS,
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
    "initial_state_intervals",
    "leave_one_study_out",
    "summarize",
]

EVALUATION_ID = "eye.recovery-evaluation"
EVALUATION_VERSION = "6.0.0"
INTERVAL_FIELD = "initial_state_intervals"


def initial_state_intervals(
    config: population.RecoveryConfig,
    item: EvaluatedSeries,
    fit: population.RecoveryFit,
    entry: dict[str, Any],
) -> dict[str, dict[str, list[float]]] | None:
    """Quantiles of every initial-state variant of ``entry`` (computed as harness 2
    computes its initial states), or ``None`` without a population forecast."""
    if "population_level_used" not in entry:
        return None
    prior, _ = harness2._series_prior(fit, item)
    if prior is None:
        return None
    duration = population.effect_duration(item)
    extra = fit.forecast_inflation(item.kind, item.context, item.aggregate_n)
    intervals: dict[str, dict[str, list[float]]] = {}
    for name, df in INITIAL_STATE_VARIANTS:
        variant = (
            population.gaussian(prior["mean"], prior["covariance"])
            if df is None
            else population.student(prior["mean"], prior["covariance"], df)
        )
        prediction = population.predictive(
            config,
            population.posterior(
                config, variant, series=None, noise=harness2._noise(fit, item), duration=duration
            ),
            months=list(item.months),
            extra_noise=extra,
        )
        if list(prediction["point_fraction"]) != list(entry["predictions"][name]):
            raise InvalidRequest(f"Recomputed initial state of {item.series_id} differs")
        intervals[name] = {q: list(v) for q, v in prediction["quantiles"].items()}
    return intervals


def _initial_state_entry(
    config: population.RecoveryConfig,
    item: EvaluatedSeries,
    training: Sequence[EvaluatedSeries],
    fit: population.RecoveryFit | None,
) -> dict[str, Any]:
    entry = harness2._initial_state_entry(config, item, training, fit)
    intervals = None if fit is None else initial_state_intervals(config, item, fit, entry)
    if intervals is not None:
        entry[INTERVAL_FIELD] = intervals
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
    fit = harness2._fit(config, training)
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
                model, intervals, level = harness2._model_forecasts(fit, item, prefix, targets)
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
