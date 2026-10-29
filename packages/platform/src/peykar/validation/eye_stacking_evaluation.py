"""Stacked forecasts of the generic eye models on harness reports (version 1.0.0).

``eye_stacking`` combines a population model's forecasts with persistence and the
pooled-curve increment (forecasts from follow-up) and with the pooled training curve
(initial states). This module fits its weights on the rows of a harness report and
adds the stacked forecasts to a report:

- every forecast row gets the prediction ``stacked`` and, when the model's row has an
  interval, the interval ``stacked`` (the model's quantiles moved to the stacked point);
- every initial-state entry with a model forecast gets ``stacked_initial_state`` and,
  when the entry carries harness 6's ``initial_state_intervals``, its moved quantiles
  and their coverage;
- every score is recomputed with the harnesses' own scoring functions, the summary with
  their ``summarize``, plus the coverage of the stacked intervals.

Weights are fitted on the targets the hypotheses test (``StackingSpec``: by default
group means of human series). Forecast rows of the next-observation and landmark tasks
enter once each (a row shared by both tasks counts once), and only rows where every
member exists. ``nested_stack_report`` is the development check: each held-out study
of a leave-one-study-out report is stacked with weights fitted without it and its
linked studies (the report's clusters), so no study's own errors choose its weights.
``stack_report`` applies frozen weights to a sealed report. Nothing here reads files or
claims clinical validity.
"""

from __future__ import annotations

import math
from collections.abc import Collection, Iterable, Mapping, Sequence
from typing import Any

from peykar.model_learning.eye_stacking import (
    STACKING_ID,
    STACKING_VERSION,
    StackingSpec,
    StackingWeights,
    combine,
    shift_quantiles,
    simplex_least_squares,
)
from peykar.validation.cornea_population_evaluation import _coverage, _errors, _passes
from peykar.validation.eye_iop_evaluation import (
    FORECAST_TASKS,
    INTERVALS,
    _interval_coverage,
    _score_rows,
    summarize,
)
from peykar.validation.eye_recovery_evaluation6 import INTERVAL_FIELD
from peykar_core.errors import InvalidRequest

__all__ = [
    "EVALUATION_VERSION",
    "STACKED",
    "STACKED_INITIAL",
    "StackingSpec",
    "development_comparison",
    "fit_weights",
    "nested_stack_report",
    "stack_report",
    "stack_study",
]

EVALUATION_ID = "eye.stacking-evaluation"
EVALUATION_VERSION = "1.0.0"
STACKED = "stacked"
STACKED_INITIAL = "stacked_initial_state"
WINDOWS = {"80": (0.70, 0.90), "90": (0.80, 0.97)}
ADOPTION_GAIN_PP = 0.10
ADOPTION_LOSS_PP = 0.05


def _selected(entry: Mapping[str, Any], spec: StackingSpec) -> bool:
    return bool(entry["kind"] == spec.kind and entry["context"].startswith(spec.context_prefix))


def history_members(row: Mapping[str, Any], spec: StackingSpec) -> list[float | None]:
    """The model's point forecast, persistence and the pooled-curve increment."""
    predictions = row["predictions"]
    return [
        predictions.get(spec.history_model),
        predictions.get("persistence"),
        predictions.get("pooled_curve_ratio"),
    ]


def history_errors(
    studies: Iterable[Mapping[str, Any]], spec: StackingSpec, exclude: Collection[str] = ()
) -> list[list[float]]:
    """Member errors (forecast - observed) of the selected forecast rows, once each."""
    seen: set[tuple[Any, ...]] = set()
    errors = []
    for study in studies:
        if study["study_id"] in exclude:
            continue
        for task in FORECAST_TASKS:
            for entry in study[task]:
                if not _selected(entry, spec):
                    continue
                for row in entry["rows"]:
                    key = (
                        study["study_id"],
                        entry["series_id"],
                        row["origin_month"],
                        row["target_month"],
                    )
                    if key in seen:
                        continue
                    seen.add(key)
                    values = history_members(row, spec)
                    if any(v is None for v in values):
                        continue
                    errors.append([float(v) - row["observed"] for v in values if v is not None])
    return errors


def initial_errors(
    studies: Iterable[Mapping[str, Any]], spec: StackingSpec, exclude: Collection[str] = ()
) -> list[list[float]]:
    """Member errors of the selected initial-state targets (model, pooled curve)."""
    errors = []
    for study in studies:
        if study["study_id"] in exclude:
            continue
        for entry in study["initial_state"]:
            if not _selected(entry, spec):
                continue
            model = entry["predictions"].get(spec.initial_model)
            curve = entry["predictions"].get("pooled_training_curve")
            if model is None or curve is None:
                continue
            for m, c, o in zip(model, curve, entry["observed"], strict=True):
                errors.append([m - o, c - o])
    return errors


def fit_weights(
    studies: Sequence[Mapping[str, Any]], spec: StackingSpec, exclude: Collection[str] = ()
) -> StackingWeights:
    """Convex weights fitted on the selected targets of ``studies`` (minus ``exclude``)."""
    history = history_errors(studies, spec, exclude)
    initial = initial_errors(studies, spec, exclude)
    if not history or not initial:
        raise InvalidRequest("No complete targets to fit stacking weights on")
    return StackingWeights(
        history=simplex_least_squares(history), initial=simplex_least_squares(initial)
    )


def _stacked_coverage(rows: Sequence[Mapping[str, Any]]) -> dict[str, dict[str, int]]:
    coverage = {}
    for interval, (low, high) in INTERVALS.items():
        hits = [
            row["intervals"][STACKED][low] <= row["observed"] <= row["intervals"][STACKED][high]
            for row in rows
            if STACKED in row["intervals"]
        ]
        coverage[interval] = {"inside": int(sum(hits)), "n": len(hits)}
    return coverage


def _stack_forecast_entry(
    entry: Mapping[str, Any], weights: StackingWeights, spec: StackingSpec
) -> dict[str, Any]:
    rows = []
    for row in entry["rows"]:
        values = history_members(row, spec)
        predictions = dict(row["predictions"])
        intervals = dict(row["intervals"])
        point = combine(values, weights.history)
        if point is not None and values[0] is not None:
            predictions[STACKED] = point
            model_interval = row["intervals"].get(spec.history_model)
            if model_interval is not None:
                intervals[STACKED] = shift_quantiles(model_interval, point - float(values[0]))
        rows.append({**row, "predictions": predictions, "intervals": intervals})
    return {
        **entry,
        "rows": rows,
        "scores": _score_rows(rows) if rows else {},
        "interval_coverage": _interval_coverage(rows),
        "stacked_interval_coverage": _stacked_coverage(rows),
    }


def _stack_initial_entry(
    entry: Mapping[str, Any], weights: StackingWeights, spec: StackingSpec
) -> dict[str, Any]:
    out = dict(entry)
    predictions = dict(entry["predictions"])
    model = predictions.get(spec.initial_model)
    if model is not None:
        curve = predictions.get("pooled_training_curve")
        members = curve if curve is not None else [None] * len(model)
        stacked = []
        for m, c in zip(model, members, strict=True):
            value = combine([m, c], weights.initial)
            if value is None:  # pragma: no cover - the model forecast is always present here
                raise InvalidRequest("A stacked initial state needs the model forecast")
            stacked.append(value)
        predictions[STACKED_INITIAL] = stacked
        intervals = entry.get(INTERVAL_FIELD, {})
        if spec.initial_model in intervals:
            moved = {
                q: [
                    v + (s - m)
                    for v, s, m in zip(values, predictions[STACKED_INITIAL], model, strict=True)
                ]
                for q, values in intervals[spec.initial_model].items()
            }
            out[INTERVAL_FIELD] = {**intervals, STACKED_INITIAL: moved}
            out["population_interval_coverage"] = {
                **entry.get("population_interval_coverage", {}),
                STACKED_INITIAL: _coverage(moved, entry["observed"]),
            }
    out["predictions"] = predictions
    scores = {}
    for name, values in predictions.items():
        score = _errors(values, entry["observed"])
        score["meets_5_10_pp"] = _passes(score)
        scores[name] = score
    out["scores"] = scores
    return out


def stack_study(
    study: Mapping[str, Any], weights: StackingWeights, spec: StackingSpec
) -> dict[str, Any]:
    """One study result with stacked forecasts added and every score recomputed."""
    out = dict(study)
    out["initial_state"] = [_stack_initial_entry(e, weights, spec) for e in study["initial_state"]]
    for task in FORECAST_TASKS:
        out[task] = [_stack_forecast_entry(e, weights, spec) for e in study[task]]
    out["stacking_weights"] = weights.describe()
    return out


def summarize_stacked(studies: Sequence[dict[str, Any]]) -> dict[str, Any]:
    """The harnesses' summary plus the pooled coverage of the stacked intervals."""
    summary = summarize(studies)
    for interval in INTERVALS:
        block = summary["coverage"][interval]
        for task in FORECAST_TASKS:
            inside = n = 0
            for study in studies:
                for entry in study[task]:
                    counts = entry.get("stacked_interval_coverage", {}).get(interval)
                    if counts is not None:
                        inside += counts["inside"]
                        n += counts["n"]
            block[task][STACKED] = {"inside": inside, "n": n}
        for kind in ("group_mean", "single_eye"):
            inside = n = 0
            for study in studies:
                for entry in study["initial_state"]:
                    counts = entry.get("population_interval_coverage", {}).get(STACKED_INITIAL)
                    if entry["kind"] == kind and counts is not None:
                        inside += counts[interval]["inside"]
                        n += counts[interval]["n"]
            block[f"initial_state_{kind}"][STACKED_INITIAL] = {"inside": inside, "n": n}
    return summary


def _stacking_block(spec: StackingSpec, mode: str, **extra: Any) -> dict[str, Any]:
    return {
        "evaluation_id": EVALUATION_ID,
        "evaluation_version": EVALUATION_VERSION,
        "stacking_id": STACKING_ID,
        "stacking_version": STACKING_VERSION,
        "mode": mode,
        "spec": spec.describe(),
        **extra,
    }


def stack_report(
    report: Mapping[str, Any], weights: StackingWeights, spec: StackingSpec
) -> dict[str, Any]:
    """A report with frozen weights applied to every study."""
    studies = [stack_study(s, weights, spec) for s in report["studies"]]
    return {
        **report,
        "studies": studies,
        "summary": summarize_stacked(studies),
        "stacking": _stacking_block(spec, "frozen weights", weights=weights.describe()),
    }


def nested_stack_report(report: Mapping[str, Any], spec: StackingSpec) -> dict[str, Any]:
    """Each held-out study stacked with weights fitted without it and its linked studies."""
    linked: dict[str, set[str]] = {}
    for cluster in report.get("clusters", []):
        for member in cluster:
            linked.setdefault(member, set()).update(cluster)
    studies, by_study = [], {}
    for study in report["studies"]:
        excluded = linked.get(study["study_id"], set()) | {study["study_id"]}
        weights = fit_weights(report["studies"], spec, exclude=excluded)
        studies.append(stack_study(study, weights, spec))
        by_study[study["study_id"]] = weights.describe()
    everything = fit_weights(report["studies"], spec)
    return {
        **report,
        "studies": studies,
        "summary": summarize_stacked(studies),
        "stacking": _stacking_block(
            spec,
            "nested leave-one-study-out",
            weights_by_held_out_study=by_study,
            weights_all_studies=everything.describe(),
            targets_all_studies={
                "history": len(history_errors(report["studies"], spec)),
                "initial": len(initial_errors(report["studies"], spec)),
            },
        ),
    }


def _rmse(errors: Sequence[float]) -> float | None:
    return math.sqrt(sum(e * e for e in errors) / len(errors)) if errors else None


def _paired_rows(
    studies: Sequence[Mapping[str, Any]], spec: StackingSpec, task: str, first: str, second: str
) -> tuple[list[float], list[float]]:
    a, b = [], []
    for study in studies:
        for entry in study[task]:
            if not _selected(entry, spec):
                continue
            for row in entry["rows"]:
                p = row["predictions"]
                if first in p and second in p:
                    a.append(100 * (p[first] - row["observed"]))
                    b.append(100 * (p[second] - row["observed"]))
    return a, b


def _paired_initial(
    studies: Sequence[Mapping[str, Any]], spec: StackingSpec, first: str, second: str
) -> tuple[list[float], list[float]]:
    a, b = [], []
    for study in studies:
        for entry in study["initial_state"]:
            p = entry["predictions"]
            if not _selected(entry, spec) or first not in p or second not in p:
                continue
            for x, y, o in zip(p[first], p[second], entry["observed"], strict=True):
                a.append(100 * (x - o))
                b.append(100 * (y - o))
    return a, b


def development_comparison(stacked: Mapping[str, Any], spec: StackingSpec) -> dict[str, Any]:
    """Pooled RMSE (reported pp) of the stacked forecasts against the model's primaries
    on the selected targets, the coverage of the stacked next-observation intervals and
    the declared adoption rule (a gain of at least 0.10 pp in one task, a loss of at
    most 0.05 pp in every task, coverage inside both H1 windows)."""
    studies = stacked["studies"]
    tasks: dict[str, Any] = {}
    for task in FORECAST_TASKS:
        new, old = _paired_rows(studies, spec, task, STACKED, spec.history_model)
        tasks[task] = {"targets": len(new), "stacked": _rmse(new), "model": _rmse(old)}
    new, old = _paired_initial(studies, spec, STACKED_INITIAL, spec.initial_model)
    tasks["initial_state"] = {"targets": len(new), "stacked": _rmse(new), "model": _rmse(old)}
    for values in tasks.values():
        both = values["stacked"] is not None and values["model"] is not None
        values["gain"] = values["model"] - values["stacked"] if both else None
    coverage = {}
    for interval, (low, high) in INTERVALS.items():
        inside = n = 0
        for study in studies:
            for entry in study["next_observation"]:
                if not _selected(entry, spec):
                    continue
                for row in entry["rows"]:
                    if STACKED in row["intervals"]:
                        bounds = row["intervals"][STACKED]
                        n += 1
                        inside += int(bounds[low] <= row["observed"] <= bounds[high])
        value = inside / n if n else None
        window = WINDOWS[interval]
        coverage[interval] = {
            "inside": inside,
            "n": n,
            "coverage": value,
            "within_window": value is not None and window[0] <= value <= window[1],
        }
    gains = [v["gain"] for v in tasks.values()]
    adopted = bool(
        all(g is not None for g in gains)
        and any(g >= ADOPTION_GAIN_PP for g in gains if g is not None)
        and all(g >= -ADOPTION_LOSS_PP for g in gains if g is not None)
        and all(c["within_window"] for c in coverage.values())
    )
    return {"tasks": tasks, "coverage_next_observation": coverage, "adopted": adopted}
