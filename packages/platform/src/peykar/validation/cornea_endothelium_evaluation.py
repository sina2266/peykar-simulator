"""Leave-one-study-out and sealed-test evaluation of endothelial density forecasters.

Every evaluated study is excluded from population fitting, noise estimation,
pooled curves and the analog library before any of its values are predicted.
Values are fractions of each series' reference ECD; errors are percentage points
(pp) of that reference. Three tasks are scored:

* ``initial_state``: from procedure context and series kind only, predict each
  control series (published group mean or single eye) at all its follow-ups;
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

from peykar.model_learning import endothelial_population as population
from peykar.model_learning.corneal_population import NoiseModel
from peykar.validation.cornea_endothelium_data import DensitySeries
from peykar.validation.cornea_population_evaluation import _coverage, _errors, _passes
from peykar_core.errors import InvalidRequest

EVALUATION_ID = "cornea.endothelial-density-evaluation"
EVALUATION_VERSION = "1.0.0"
RMSE_LIMIT_PP = 5.0
MAX_LIMIT_PP = 10.0
PERSISTENCE_MARGIN_PP = 0.5
INTERVALS = {"80": ("0.1", "0.9"), "90": ("0.05", "0.95")}
STUDENT_DF = 4.0
ANALOG_K = 5
PROBABILISTIC = (
    "population_assimilation",
    "population_assimilation_t4",
    "population_assimilation_robust",
    "flat_prior_grid",
    "population_anchored",
    "population_anchored_t4",
)
INITIAL_STATE_VARIANTS = (("population_initial_state", None), ("population_initial_state_t4", 4.0))
FORECAST_TASKS = ("next_observation", "landmark")


def _log_points(series: DensitySeries) -> tuple[list[float], list[float]]:
    months = [0.0, *series.months]
    logs = [0.0, *(math.log(max(population.MIN_FRACTION, f)) for f in series.fractions)]
    return months, logs


def log_fraction_at(series: DensitySeries, month: float) -> float | None:
    """Linear interpolation of ``ln f`` over the reference and follow-ups; ``None``
    beyond the last follow-up (no extrapolation)."""
    months, logs = _log_points(series)
    if month < 0 or month > months[-1]:
        return None
    return float(np.interp(month, months, logs))


def _library_level(
    library: Sequence[DensitySeries], context: str
) -> tuple[list[DensitySeries], str] | None:
    exact = [s for s in library if population.model_context(s) == context]
    if exact:
        return exact, context
    family = population.tissue_family(context)
    members = [
        s for s in library if population.tissue_family(population.model_context(s)) == family
    ]
    return (members, f"family:{family}") if members else None


def pooled_log_curve(
    library: Sequence[DensitySeries], context: str, months: Sequence[float]
) -> tuple[list[float], str] | None:
    """Mean ``ln f`` of training controls (per study first, then across studies).

    Uses the context's controls, or its tissue family's when the context is absent.
    A month beyond every series' last follow-up uses each series' last value.
    """
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
    library: Sequence[DensitySeries],
    *,
    context: str,
    history: Sequence[tuple[float, float]],
    target_month: float,
    k: int = ANALOG_K,
) -> dict[str, Any] | None:
    """Nearest training series (same tissue family) by ``ln f`` at the history months;
    the forecast applies their mean log change from the origin to the target."""
    observed = [(m, f) for m, f in history if m > 0]
    if not observed:
        return None
    origin, last = observed[-1]
    family = population.tissue_family(context)
    candidates = []
    for item in library:
        if population.tissue_family(population.model_context(item)) != family:
            continue
        if item.months[-1] < target_month:
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
                        (float(v) - math.log(max(population.MIN_FRACTION, f))) ** 2
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
    slope = (
        math.log(max(population.MIN_FRACTION, v2)) - math.log(max(population.MIN_FRACTION, v1))
    ) / (h2 - h1)
    return float(
        min(2.0, math.exp(math.log(max(population.MIN_FRACTION, v2)) + slope * (target - h2)))
    )


def _fit(training: Sequence[DensitySeries]) -> population.EndothelialFit | None:
    controls = [s for s in training if s.role == "control"]
    if len(controls) < 2 or len({s.study_id for s in controls}) < 2:
        return None
    return population.fit_population(controls, noise_series=training)


def _model_forecasts(
    fit: population.EndothelialFit | None,
    item: DensitySeries,
    prefix: DensitySeries,
    targets: Sequence[float],
    *,
    draws: int,
    seed: int,
) -> tuple[dict[str, list[float]], dict[str, dict[str, list[float]]], str | None]:
    predictions: dict[str, list[float]] = {}
    intervals: dict[str, dict[str, list[float]]] = {}
    context = population.model_context(item)
    noise = (
        fit.noise(item.kind, context)
        if fit is not None
        else NoiseModel(center=0.03, log_sd=population.NOISE_LOG_SD)
    )
    try:
        prior = fit.prior(context, item.kind) if fit is not None else None
    except InvalidRequest:
        prior = None
    variants: list[tuple[str, dict[str, Any] | None, float | None]] = [
        ("population_assimilation", prior, None),
        ("population_assimilation_t4", prior, STUDENT_DF),
        ("population_assimilation_robust", population.robust(prior) if prior else None, None),
        ("flat_prior_grid", population.flat_prior() if fit is not None else None, None),
    ]
    for name, variant_prior, df in variants:
        if variant_prior is None:
            continue
        result = population.predictive(
            population.posterior(variant_prior, series=prefix, noise=noise, student_df=df),
            months=list(targets),
            draws=draws,
            seed=seed,
        )
        predictions[name] = result["mean_fraction"]
        intervals[name] = result["quantiles"]
    if prior is not None:
        for name, df in (("population_anchored", None), ("population_anchored_t4", STUDENT_DF)):
            anchored = population.predictive(
                population.anchored_posterior(prior, series=prefix, noise=noise, student_df=df),
                months=list(targets),
                draws=draws,
                seed=seed,
            )
            predictions[name] = anchored["mean_fraction"]
            intervals[name] = anchored["quantiles"]
    level = fit.level_used(context) if prior is not None and fit is not None else None
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


def evaluate_study(
    *,
    study_series: Sequence[DensitySeries],
    training: Sequence[DensitySeries],
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
    fit = _fit(training)
    result: dict[str, Any] = {
        "study_id": study_id,
        "contexts": sorted({population.model_context(s) for s in study_series}),
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
        context = population.model_context(item)
        if item.role != "control":
            continue
        entry: dict[str, Any] = {
            "series_id": item.series_id,
            "context": context,
            "kind": "group_mean" if item.is_aggregate else "single_eye",
            "months": list(item.months),
            "observed": list(item.fractions),
            "predictions": {},
        }
        pooled = pooled_log_curve(training, context, item.months)
        if pooled is not None:
            entry["predictions"]["pooled_training_curve"] = [math.exp(v) for v in pooled[0]]
            entry["pooled_level_used"] = pooled[1]
        if fit is not None:
            try:
                prior = fit.prior(context, item.kind)
            except InvalidRequest as error:
                entry["population_unavailable"] = str(error)
            else:
                entry["population_level_used"] = fit.level_used(context)
                entry["population_interval_coverage"] = {}
                for name, df in INITIAL_STATE_VARIANTS:
                    prediction = population.predictive(
                        population.posterior(
                            prior,
                            series=None,
                            noise=fit.noise(item.kind, context),
                            student_df=df,
                        ),
                        months=list(item.months),
                        draws=draws,
                        seed=seed,
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
        result["initial_state"].append(entry)
    library = list(training)
    for item in study_series:
        if len(item.months) < 2:
            continue
        context = population.model_context(item)
        for task in FORECAST_TASKS:
            rows = []
            origins = range(1, len(item.months)) if task == "next_observation" else [1]
            for k in origins:
                prefix = item.prefix(item.months[k - 1])
                targets = list(item.months[k:]) if task == "landmark" else [item.months[k]]
                history = [(0.0, 1.0), *zip(prefix.months, prefix.fractions, strict=True)]
                origin, last = history[-1]
                model, intervals, level = _model_forecasts(
                    fit, item, prefix, targets, draws=draws, seed=seed
                )
                pooled = pooled_log_curve(training, context, [origin, *targets])
                for position, target in enumerate(targets):
                    observed = item.fractions[item.months.index(target)]
                    predictions: dict[str, float] = {
                        "persistence": last,
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
                        library, context=context, history=history, target_month=target
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
                    "context": context,
                    "role": item.role,
                    "kind": "group_mean" if item.is_aggregate else "single_eye",
                    "rows": rows,
                    "scores": _score_rows(rows),
                    "interval_coverage": _interval_coverage(rows),
                }
            )
    return result


def summarize(reports: Sequence[dict[str, Any]]) -> dict[str, Any]:
    """Pass counts and pooled errors per method, never replacing per-series results."""

    def stats(entries: Sequence[dict[str, Any]], *, forecast: bool) -> dict[str, Any]:
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

    initial = [e for r in reports for e in r["initial_state"]]
    summary: dict[str, Any] = {"initial_state": {}, "coverage": {}}
    for kind in ("group_mean", "single_eye"):
        members = [e for e in initial if e["kind"] == kind]
        summary["initial_state"][kind] = stats(members, forecast=False)
        seen = [e for e in members if e.get("population_level_used") == e["context"]]
        summary["initial_state"][f"{kind}_trained_context"] = stats(seen, forecast=False)
    for task in FORECAST_TASKS:
        entries = [e for r in reports for e in r[task]]
        summary[task] = stats(entries, forecast=True)
        summary[f"{task}_controls"] = stats(
            [e for e in entries if e["role"] == "control"], forecast=True
        )
    by_family: dict[str, Any] = {}
    families = sorted(
        {population.tissue_family(e["context"]) for e in initial}
        | {
            population.tissue_family(e["context"])
            for r in reports
            for t in FORECAST_TASKS
            for e in r[t]
        }
    )
    for family in families:

        def of(entries: Sequence[dict[str, Any]], family: str = family) -> list[dict[str, Any]]:
            return [e for e in entries if population.tissue_family(e["context"]) == family]

        by_family[family] = {
            "initial_state_group_mean": stats(
                of([e for e in initial if e["kind"] == "group_mean"]), forecast=False
            ),
            **{
                task: stats(of([e for r in reports for e in r[task]]), forecast=True)
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
    series: Sequence[DensitySeries],
    *,
    studies: Sequence[str] | None = None,
    clusters: Sequence[Sequence[str]] = (),
    progress: Callable[[str], None] | None = None,
    draws: int = 2000,
) -> dict[str, Any]:
    """Hold out each study; studies of one declared cluster (possibly shared
    cohorts) leave training together."""
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
        report = evaluate_study(study_series=held, training=training, draws=draws)
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
            "unit": "percentage points of each series' reference ECD",
        },
        "clusters": [sorted(c) for c in clusters],
        "studies": reports,
        "summary": summarize(reports),
    }
