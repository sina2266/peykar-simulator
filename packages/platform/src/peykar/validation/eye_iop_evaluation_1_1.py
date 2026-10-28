"""Leave-one-study-out and sealed-test evaluation of IOP model 1.1.

Same tasks, baselines, scores and summaries as the 1.0.0 harness
(``eye_iop_evaluation``, unchanged because its protocol pins it; its
model-independent parts are imported here). Only the population model differs:
``iop_population_1_1`` (early transient term and eye-count-scaled group-mean noise).
Every evaluated study is excluded from population fitting, noise estimation, pooled
curves and the analog library before any of its values are predicted. No result
here is a clinical validation claim.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Sequence
from typing import Any

from peykar.model_learning import iop_population_1_1 as population
from peykar.model_learning.corneal_population import NoiseModel
from peykar.validation.cornea_population_evaluation import _coverage, _errors, _passes
from peykar.validation.eye_iop_data import PressureSeries
from peykar.validation.eye_iop_evaluation import (
    FORECAST_TASKS,
    INITIAL_STATE_VARIANTS,
    MAX_LIMIT_PP,
    PERSISTENCE_MARGIN_PP,
    RMSE_LIMIT_PP,
    STUDENT_DF,
    _interval_coverage,
    _kind,
    _log_linear,
    _score_rows,
    analog_forecast,
    pooled_log_curve,
    summarize,
)
from peykar_core.errors import InvalidRequest

__all__ = ["evaluate_study", "leave_one_study_out", "summarize", "criteria"]

EVALUATION_ID = "eye.iop-evaluation"
EVALUATION_VERSION = "1.1.0"
MODEL_ID = population.MODEL_ID
MODEL_VERSION = population.MODEL_VERSION


def criteria() -> dict[str, Any]:
    return {
        "rmse_pp_max": RMSE_LIMIT_PP,
        "max_abs_pp_max": MAX_LIMIT_PP,
        "forecast_persistence_margin_pp": PERSISTENCE_MARGIN_PP,
        "unit": "percentage points of each series' preoperative IOP",
    }


def _fit(training: Sequence[PressureSeries]) -> population.PressureFit | None:
    controls = [s for s in training if s.role == "control" and s.reference_mmhg > 0]
    if len(controls) < 2 or len({s.study_id for s in controls}) < 2:
        return None
    return population.fit_population(controls, noise_series=training)


def _variants(prior: dict[str, Any] | None) -> list[tuple[str, population.Prior]]:
    variants: list[tuple[str, population.Prior]] = [("weak_prior", population.weak_prior())]
    if prior is None:
        return variants
    mean, covariance = prior["mean"], prior["covariance"]
    gaussian = population.gaussian(mean, covariance)
    t4 = population.student(mean, covariance, STUDENT_DF)
    return [
        ("population_assimilation", gaussian),
        ("population_assimilation_t4", t4),
        ("population_assimilation_robust", population.robust(gaussian)),
        ("population_anchored", population.anchored(gaussian)),
        ("population_anchored_t4", population.anchored(t4)),
        *variants,
    ]


def _series_prior(
    fit: population.PressureFit, item: PressureSeries
) -> tuple[dict[str, Any] | None, str | None]:
    try:
        covariate = population.level_covariate(item)
        return fit.prior(item.context, item.kind, covariate), fit.level_used(item.context)
    except InvalidRequest:
        return None, None


def _noise(fit: population.PressureFit, item: PressureSeries) -> NoiseModel:
    return fit.noise(item.kind, item.context, item.aggregate_n)


def _model_forecasts(
    fit: population.PressureFit | None,
    item: PressureSeries,
    prefix: PressureSeries,
    targets: Sequence[float],
) -> tuple[dict[str, list[float]], dict[str, dict[str, list[float]]], str | None]:
    predictions: dict[str, list[float]] = {}
    intervals: dict[str, dict[str, list[float]]] = {}
    if fit is None:
        return predictions, intervals, None
    noise = _noise(fit, item)
    prior, level = _series_prior(fit, item)
    for name, variant in _variants(prior):
        result = population.predictive(
            population.posterior(variant, series=prefix, noise=noise), months=list(targets)
        )
        predictions[name] = result["point_fraction"]
        intervals[name] = result["quantiles"]
    return predictions, intervals, level


def _initial_state_entry(
    item: PressureSeries,
    training: Sequence[PressureSeries],
    fit: population.PressureFit | None,
) -> dict[str, Any]:
    entry: dict[str, Any] = {
        "series_id": item.series_id,
        "context": item.context,
        "kind": _kind(item),
        "reference_mmhg": item.reference_mmhg,
        "months": list(item.months),
        "observed": list(item.fractions),
        "predictions": {},
    }
    pooled = pooled_log_curve(training, item.context, item.months)
    if pooled is not None:
        entry["predictions"]["pooled_training_curve"] = [math.exp(v) for v in pooled[0]]
        entry["pooled_level_used"] = pooled[1]
    if fit is not None:
        prior, level = _series_prior(fit, item)
        if prior is None:
            entry["population_unavailable"] = "no preoperative IOP or no family prior"
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
                    population.posterior(variant, series=None, noise=_noise(fit, item)),
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
    *, study_series: Sequence[PressureSeries], training: Sequence[PressureSeries]
) -> dict[str, Any]:
    """Score one excluded study against models built only from ``training``."""
    study_ids = {s.study_id for s in study_series}
    if len(study_ids) != 1:
        raise InvalidRequest("Evaluate exactly one study at a time")
    study_id = next(iter(study_ids))
    if any(s.study_id == study_id for s in training):
        raise InvalidRequest("The evaluated study must be absent from training")
    fit = _fit(training)
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
            result["initial_state"].append(_initial_state_entry(item, training, fit))
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
                pooled = pooled_log_curve(training, item.context, [origin, *targets])
                for position, target in enumerate(targets):
                    observed = item.fractions[item.months.index(target)]
                    predictions: dict[str, float] = {
                        "persistence": last,
                        "preoperative_level": 1.0,
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
                        library, context=item.context, history=history, target_month=target
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
    series: Sequence[PressureSeries],
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
        report = evaluate_study(study_series=held, training=training)
        report["training_exclusions"] = sorted(excluded - {study_id})
        reports.append(report)
    return {
        "evaluation_id": EVALUATION_ID,
        "evaluation_version": EVALUATION_VERSION,
        "model_id": MODEL_ID,
        "model_version": MODEL_VERSION,
        "criteria": criteria(),
        "clusters": [sorted(c) for c in clusters],
        "studies": reports,
        "summary": summarize(reports),
    }
