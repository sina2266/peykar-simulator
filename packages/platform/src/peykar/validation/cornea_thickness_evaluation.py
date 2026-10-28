"""Leave-one-study-out and sealed-test evaluation of corneal thickness forecasters.

Every evaluated study is excluded from population fitting, noise estimation,
pooled curves and the analog library before any of its values are predicted.
Values are fractions of each series' preoperative CCT; errors are percentage
points (pp) of that reference. Three tasks are scored:

* ``initial_state``: from procedure context, series kind and preoperative CCT
  only, predict each control series (group mean or single eye) at its follow-ups;
* ``next_observation``: after each actual follow-up, predict the next one;
* ``landmark``: from the first follow-up only, predict every later follow-up.

No result here is a biological or clinical validation claim; acceptance flags only
restate declared numerical criteria.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Sequence
from typing import Any

import numpy as np

from peykar.model_learning import thickness_population as population
from peykar.model_learning.corneal_population import NoiseModel
from peykar.validation.cornea_population_evaluation import _coverage, _errors, _passes
from peykar.validation.cornea_thickness_data import ThicknessSeries
from peykar_core.errors import InvalidRequest

EVALUATION_ID = "cornea.central-thickness-evaluation"
EVALUATION_VERSION = "1.0.0"
RMSE_LIMIT_PP = 5.0
MAX_LIMIT_PP = 10.0
PERSISTENCE_MARGIN_PP = 0.5
INTERVALS = {"80": ("0.1", "0.9"), "90": ("0.05", "0.95")}
STUDENT_DF = 4.0
ANALOG_K = 5
MIN_FRACTION = 1e-3
PROBABILISTIC = (
    "population_assimilation",
    "population_assimilation_t4",
    "population_assimilation_robust",
    "population_anchored",
    "population_anchored_t4",
    "weak_prior",
)
INITIAL_STATE_VARIANTS = (("population_initial_state", None), ("population_initial_state_t4", 4.0))
FORECAST_TASKS = ("next_observation", "landmark")


def _log(value: float) -> float:
    return math.log(max(MIN_FRACTION, value))


def _log_points(series: ThicknessSeries) -> tuple[list[float], list[float]]:
    return [0.0, *series.months], [0.0, *(_log(f) for f in series.fractions)]


def log_fraction_at(series: ThicknessSeries, month: float) -> float | None:
    """Linear interpolation of ``ln f`` over the reference and follow-ups; ``None``
    beyond the last follow-up (no extrapolation)."""
    months, logs = _log_points(series)
    if month < 0 or month > months[-1]:
        return None
    return float(np.interp(month, months, logs))


def _library_level(
    library: Sequence[ThicknessSeries], context: str
) -> tuple[list[ThicknessSeries], str] | None:
    exact = [s for s in library if s.context == context]
    if exact:
        return exact, context
    family = population.thickness_family(context)
    members = [s for s in library if population.series_family(s) == family]
    return (members, f"family:{family}") if members else None


def pooled_log_curve(
    library: Sequence[ThicknessSeries], context: str, months: Sequence[float]
) -> tuple[list[float], str] | None:
    """Mean ``ln f`` of training controls (per study first, then across studies), of
    the context or else its family; beyond every series' end, last values are used."""
    level = _library_level([s for s in library if s.role == "control"], context)
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
    library: Sequence[ThicknessSeries],
    *,
    context: str,
    history: Sequence[tuple[float, float]],
    target_month: float,
    k: int = ANALOG_K,
) -> dict[str, Any] | None:
    """Nearest training series (same family) by ``ln f`` at the history months; the
    forecast applies their mean log change from the origin to the target."""
    observed = [(m, f) for m, f in history if m > 0]
    if not observed:
        return None
    origin, last = observed[-1]
    family = population.thickness_family(context)
    candidates = []
    for item in library:
        if population.series_family(item) != family or item.months[-1] < target_month:
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


def _log_linear(points: Sequence[tuple[float, float]], target: float) -> float:
    (h1, v1), (h2, v2) = points
    slope = (_log(v2) - _log(v1)) / (h2 - h1)
    return float(min(2.0, math.exp(_log(v2) + slope * (target - h2))))


def _fit(training: Sequence[ThicknessSeries]) -> population.ThicknessFit | None:
    controls = [s for s in training if s.role == "control"]
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


def _model_forecasts(
    fit: population.ThicknessFit | None,
    item: ThicknessSeries,
    prefix: ThicknessSeries,
    targets: Sequence[float],
) -> tuple[dict[str, list[float]], dict[str, dict[str, list[float]]], str | None]:
    predictions: dict[str, list[float]] = {}
    intervals: dict[str, dict[str, list[float]]] = {}
    if fit is None:
        return predictions, intervals, None
    try:
        offset = population.level_offset(item)
    except InvalidRequest:
        return predictions, intervals, None
    noise: NoiseModel = fit.noise(item.kind, item.context)
    try:
        prior = fit.prior(item.context, item.kind)
    except InvalidRequest:
        prior = None
    for name, variant in _variants(prior):
        result = population.predictive(
            population.posterior(variant, series=prefix, noise=noise, offset=offset),
            months=list(targets),
        )
        predictions[name] = result["mean_fraction"]
        intervals[name] = result["quantiles"]
    level = fit.level_used(item.context) if prior is not None else None
    return predictions, intervals, level


def _score_rows(rows: Sequence[dict[str, Any]]) -> dict[str, dict[str, Any]]:
    names = sorted({name for row in rows for name in row["predictions"]})
    scores: dict[str, dict[str, Any]] = {}
    for name in names:
        pairs = [
            (row["predictions"][name], row["observed"])
            for row in rows
            if name in row["predictions"]
        ]
        score = _errors([p for p, _ in pairs], [o for _, o in pairs])
        score["meets_5_10_pp"] = _passes(score)
        scores[name] = score
    persistence = scores["persistence"]["rmse_pp"]
    complete = {name for name in names if all(name in row["predictions"] for row in rows)}
    for name, score in scores.items():
        score["covers_all_targets"] = name in complete
        score["improves_persistence_by_0_5_pp"] = bool(
            name in complete and score["rmse_pp"] <= persistence - PERSISTENCE_MARGIN_PP
        )
    return scores


def _interval_coverage(rows: Sequence[dict[str, Any]]) -> dict[str, Any]:
    coverage: dict[str, Any] = {}
    for method in PROBABILISTIC:
        coverage[method] = {}
        for interval, (low, high) in INTERVALS.items():
            hits = [
                row["intervals"][method][low] <= row["observed"] <= row["intervals"][method][high]
                for row in rows
                if method in row.get("intervals", {})
            ]
            coverage[method][interval] = {"inside": int(sum(hits)), "n": len(hits)}
    return coverage


def _kind(item: ThicknessSeries) -> str:
    return "group_mean" if item.is_aggregate else "single_eye"


def _initial_state_entry(
    item: ThicknessSeries,
    training: Sequence[ThicknessSeries],
    fit: population.ThicknessFit | None,
) -> dict[str, Any]:
    entry: dict[str, Any] = {
        "series_id": item.series_id,
        "context": item.context,
        "kind": _kind(item),
        "reference_um": item.reference_um,
        "months": list(item.months),
        "observed": list(item.fractions),
        "predictions": {},
    }
    pooled = pooled_log_curve(training, item.context, item.months)
    if pooled is not None:
        entry["predictions"]["pooled_training_curve"] = [math.exp(v) for v in pooled[0]]
        entry["pooled_level_used"] = pooled[1]
    if fit is not None:
        try:
            prior = fit.prior(item.context, item.kind)
            offset = population.level_offset(item)
        except InvalidRequest as error:
            entry["population_unavailable"] = str(error)
        else:
            entry["population_level_used"] = fit.level_used(item.context)
            entry["population_interval_coverage"] = {}
            for name, df in INITIAL_STATE_VARIANTS:
                variant = (
                    population.gaussian(prior["mean"], prior["covariance"])
                    if df is None
                    else population.student(prior["mean"], prior["covariance"], df)
                )
                prediction = population.predictive(
                    population.posterior(
                        variant,
                        series=None,
                        noise=fit.noise(item.kind, item.context),
                        offset=offset,
                    ),
                    months=list(item.months),
                )
                entry["predictions"][name] = prediction["mean_fraction"]
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
    *,
    study_series: Sequence[ThicknessSeries],
    training: Sequence[ThicknessSeries],
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


def _stats(entries: Sequence[dict[str, Any]], *, forecast: bool) -> dict[str, Any]:
    buckets: dict[str, dict[str, Any]] = {}
    for entry in entries:
        for name, score in entry["scores"].items():
            bucket = buckets.setdefault(
                name, {"series": 0, "within_5_10": 0, "pass": 0, "errors": []}
            )
            bucket["series"] += 1
            bucket["within_5_10"] += int(score["meets_5_10_pp"])
            passed = score["meets_5_10_pp"]
            if forecast:
                passed = passed and score["improves_persistence_by_0_5_pp"]
            bucket["pass"] += int(passed)
            bucket["errors"].extend(score["signed_errors_pp"])
    return {
        name: {
            "series": b["series"],
            "series_within_5_10_pp": b["within_5_10"],
            "series_meeting_criteria": b["pass"],
            "pooled_rmse_pp": float(math.sqrt(np.mean(np.square(b["errors"])))),
            "pooled_max_abs_pp": float(np.max(np.abs(b["errors"]))),
            "targets": len(b["errors"]),
        }
        for name, b in sorted(buckets.items())
    }


def summarize(reports: Sequence[dict[str, Any]]) -> dict[str, Any]:
    """Pass counts and pooled errors per method, never replacing per-series results."""
    initial = [e for r in reports for e in r["initial_state"]]
    summary: dict[str, Any] = {"initial_state": {}, "coverage": {}}
    for kind in ("group_mean", "single_eye"):
        members = [e for e in initial if e["kind"] == kind]
        summary["initial_state"][kind] = _stats(members, forecast=False)
        seen = [e for e in members if e.get("population_level_used") == e["context"]]
        summary["initial_state"][f"{kind}_trained_context"] = _stats(seen, forecast=False)
    for task in FORECAST_TASKS:
        entries = [e for r in reports for e in r[task]]
        summary[task] = _stats(entries, forecast=True)
        summary[f"{task}_controls"] = _stats(
            [e for e in entries if e["role"] == "control"], forecast=True
        )
    families = sorted(
        {population.thickness_family(e["context"]) for e in initial}
        | {
            population.thickness_family(e["context"])
            for r in reports
            for t in FORECAST_TASKS
            for e in r[t]
        }
    )
    by_family: dict[str, Any] = {}
    for family in families:

        def of(entries: Sequence[dict[str, Any]], family: str = family) -> list[dict[str, Any]]:
            return [e for e in entries if population.thickness_family(e["context"]) == family]

        by_family[family] = {
            "initial_state_group_mean": _stats(
                of([e for e in initial if e["kind"] == "group_mean"]), forecast=False
            ),
            **{
                task: _stats(of([e for r in reports for e in r[task]]), forecast=True)
                for task in FORECAST_TASKS
            },
        }
    summary["by_family"] = by_family
    for interval in INTERVALS:
        block: dict[str, Any] = {}
        for kind in ("group_mean", "single_eye"):
            per_variant = {}
            for name, _ in INITIAL_STATE_VARIANTS:
                inside = n = 0
                for entry in initial:
                    coverage = entry.get("population_interval_coverage", {})
                    if entry["kind"] == kind and name in coverage:
                        inside += coverage[name][interval]["inside"]
                        n += coverage[name][interval]["n"]
                per_variant[name] = {"inside": inside, "n": n}
            block[f"initial_state_{kind}"] = per_variant
        for task in FORECAST_TASKS:
            per_method = {}
            for method in PROBABILISTIC:
                inside = n = 0
                for report in reports:
                    for entry in report[task]:
                        inside += entry["interval_coverage"][method][interval]["inside"]
                        n += entry["interval_coverage"][method][interval]["n"]
                per_method[method] = {"inside": inside, "n": n}
            block[task] = per_method
        summary["coverage"][interval] = block
    return summary


def leave_one_study_out(
    series: Sequence[ThicknessSeries],
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
        "model_id": population.MODEL_ID,
        "model_version": population.MODEL_VERSION,
        "criteria": {
            "rmse_pp_max": RMSE_LIMIT_PP,
            "max_abs_pp_max": MAX_LIMIT_PP,
            "forecast_persistence_margin_pp": PERSISTENCE_MARGIN_PP,
            "unit": "percentage points of each series' preoperative CCT",
        },
        "clusters": [sorted(c) for c in clusters],
        "studies": reports,
        "summary": summarize(reports),
    }
