"""Leave-one-study-out and sealed-test evaluation of a generic eye recovery model.

Same tasks, baselines, scores and summaries as the IOP harnesses
(``eye_iop_evaluation`` and ``eye_iop_evaluation_1_1``, unchanged because their
protocols pin them; their model-independent parts are imported here), with the
population model ``eye_recovery_population`` and its :class:`RecoveryConfig`
passed explicitly. Pooled curves and analog libraries fall back from the exact
context to the treatment (when the model groups by treatment) and then to the
species family. Every evaluated study is excluded from population fitting, noise
estimation, pooled curves and the analog library before any of its values are
predicted. No result here is a clinical validation claim.

Tasks (fractions of each series' baseline value):

- ``initial_state``: every follow-up of a control series from its context, baseline
  value and number of units only;
- ``next_observation``: each follow-up from all earlier ones;
- ``landmark``: every later follow-up from the first one.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Sequence
from typing import Any, Protocol

import numpy as np

from peykar.model_learning import eye_recovery_population as population
from peykar.model_learning.corneal_population import NoiseModel
from peykar.validation.cornea_population_evaluation import _coverage, _errors, _passes
from peykar.validation.eye_iop_evaluation import (
    ANALOG_K,
    FORECAST_TASKS,
    INITIAL_STATE_VARIANTS,
    MAX_LIMIT_PP,
    MIN_FRACTION,
    PERSISTENCE_MARGIN_PP,
    RMSE_LIMIT_PP,
    STUDENT_DF,
    _interval_coverage,
    _log_linear,
    _score_rows,
    summarize,
)
from peykar_core.errors import InvalidRequest

__all__ = ["criteria", "evaluate_study", "leave_one_study_out", "summarize"]

EVALUATION_ID = "eye.recovery-evaluation"
EVALUATION_VERSION = "1.0.0"


class EvaluatedSeries(population.RecoverySeries, Protocol):
    @property
    def is_aggregate(self) -> bool: ...

    def prefix(self, last_month: float) -> EvaluatedSeries: ...


def criteria(config: population.RecoveryConfig) -> dict[str, Any]:
    return {
        "rmse_pp_max": RMSE_LIMIT_PP,
        "max_abs_pp_max": MAX_LIMIT_PP,
        "forecast_persistence_margin_pp": PERSISTENCE_MARGIN_PP,
        "unit": f"percentage points of each series' baseline {config.quantity}",
    }


def _log(value: float) -> float:
    return math.log(max(MIN_FRACTION, value))


def _log_points(series: EvaluatedSeries) -> tuple[list[float], list[float]]:
    return [0.0, *series.months], [0.0, *(_log(f) for f in series.fractions)]


def log_fraction_at(series: EvaluatedSeries, month: float) -> float | None:
    """Linear interpolation of ``ln f`` over the baseline and follow-ups; ``None``
    beyond the last follow-up (no extrapolation)."""
    months, logs = _log_points(series)
    if month < 0 or month > months[-1]:
        return None
    return float(np.interp(month, months, logs))


def _fallbacks(
    config: population.RecoveryConfig, context: str
) -> list[tuple[str, Callable[[str], bool]]]:
    """Library levels from the most specific: context, treatment (when grouped), family."""
    levels: list[tuple[str, Callable[[str], bool]]] = [(context, lambda c: c == context)]
    if "treatment" in config.grouping:
        treatment = population.treatment_of(context)
        family = population.family_of(context)
        levels.append(
            (
                f"treatment:{treatment}",
                lambda c: (
                    population.family_of(c) == family and population.treatment_of(c) == treatment
                ),
            )
        )
    family = population.family_of(context)
    levels.append((f"family:{family}", lambda c: population.family_of(c) == family))
    return levels


def _library_level(
    config: population.RecoveryConfig, library: Sequence[EvaluatedSeries], context: str
) -> tuple[list[EvaluatedSeries], str] | None:
    for label, matches in _fallbacks(config, context):
        members = [s for s in library if matches(s.context)]
        if members:
            return members, label
    return None


def pooled_log_curve(
    config: population.RecoveryConfig,
    library: Sequence[EvaluatedSeries],
    context: str,
    months: Sequence[float],
) -> tuple[list[float], str] | None:
    """Mean ``ln f`` of training controls (per study first, then across studies), of
    the most specific library level; beyond every series' end, last values are used."""
    level = _library_level(config, [s for s in library if s.role == "control"], context)
    if level is None:
        return None
    members, used = level
    curve = []
    for month in months:
        by_study: dict[str, list[float]] = {}
        for item in members:
            value = log_fraction_at(item, month)
            if value is not None:
                by_study.setdefault(item.study_id, []).append(value)
        if not by_study:
            for item in members:
                by_study.setdefault(item.study_id, []).append(_log_points(item)[1][-1])
        curve.append(float(np.mean([np.mean(v) for v in by_study.values()])))
    return curve, used


def analog_forecast(
    config: population.RecoveryConfig,
    library: Sequence[EvaluatedSeries],
    *,
    context: str,
    history: Sequence[tuple[float, float]],
    target_month: float,
    k: int = ANALOG_K,
) -> dict[str, Any] | None:
    """Nearest training series of the same family by ``ln f`` at the history months;
    the forecast applies their mean log change from the origin to the target."""
    observed = [(m, f) for m, f in history if m > 0]
    if not observed:
        return None
    origin, last = observed[-1]
    family = population.family_of(context)
    candidates = []
    for item in library:
        if population.family_of(item.context) != family or item.months[-1] < target_month:
            continue
        values = [log_fraction_at(item, m) for m, _ in observed]
        target = log_fraction_at(item, target_month)
        start = log_fraction_at(item, origin)
        if target is None or start is None or any(v is None for v in values):
            continue
        distance = math.sqrt(
            float(
                np.mean(
                    [
                        (float(v) - _log(f)) ** 2
                        for v, (_, f) in zip(values, observed, strict=True)
                        if v is not None
                    ]
                )
            )
        )
        candidates.append((distance, item.series_id, target - start))
    if not candidates:
        return None
    nearest = sorted(candidates)[:k]
    change = float(np.mean([c[2] for c in nearest]))
    return {
        "fraction": last * math.exp(change),
        "neighbours": [c[1] for c in nearest],
        "mean_distance_log": float(np.mean([c[0] for c in nearest])),
    }


def _fit(
    config: population.RecoveryConfig, training: Sequence[EvaluatedSeries]
) -> population.RecoveryFit | None:
    controls = [s for s in training if s.role == "control" and s.reference_value > 0]
    if len(controls) < 2 or len({s.study_id for s in controls}) < 2:
        return None
    return population.fit_population(config, controls, noise_series=training)


def _variants(
    config: population.RecoveryConfig, prior: dict[str, Any] | None
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


def _kind(item: EvaluatedSeries) -> str:
    return "group_mean" if item.is_aggregate else "single_eye"


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
    noise = _noise(fit, item)
    prior, level = _series_prior(fit, item)
    for name, variant in _variants(config, prior):
        result = population.predictive(
            config,
            population.posterior(config, variant, series=prefix, noise=noise),
            months=list(targets),
        )
        predictions[name] = result["point_fraction"]
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
            for name, df in INITIAL_STATE_VARIANTS:
                variant = (
                    population.gaussian(prior["mean"], prior["covariance"])
                    if df is None
                    else population.student(prior["mean"], prior["covariance"], df)
                )
                prediction = population.predictive(
                    config,
                    population.posterior(config, variant, series=None, noise=_noise(fit, item)),
                    months=list(item.months),
                )
                entry["predictions"][name] = prediction["point_fraction"]
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
