"""Stacked forecasts from follow-up with a chosen interval scale (version 2.0.0).

``eye_stacking2`` keeps the members and the convex combination of ``eye_stacking``
1.0.0 for forecasts from follow-up, leaves initial states to the model and scales the
model's interval around the stacked point (``q' = s + c (q - m)``). This module fits the
weights and the scale on the rows of a harness report and adds the stacked forecasts to
a report:

- every forecast row with the model's point gets the prediction ``stacked`` and, when
  the model's row has an interval, the interval ``stacked`` (scaled quantiles); scores
  are recomputed with the harnesses' own scoring functions, the summary with their
  ``summarize``, plus the coverage of the stacked intervals;
- initial-state entries are left unchanged (the model's own forecasts stay primary).

The weights are fitted exactly as in version 1.0.0 (``eye_stacking_evaluation``): on the
selected targets (by default group means of human series), every forecast row of the
two forecast tasks once, only rows where every member exists. The scale is then chosen
on the same studies with those weights: over the declared grid, the factor whose pooled
coverage of the declared interval of the declared task (by default the 80% interval of
next-observation forecasts) is closest to the nominal level.
``nested_stack_report2`` is the development check: each held-out study of a
leave-one-study-out report is stacked with weights and a scale fitted without it and its
linked studies (the report's clusters). ``stack_report2`` applies a frozen stacking to a
sealed report. Nothing here reads files or claims clinical validity.
"""

from __future__ import annotations

from collections.abc import Collection, Iterable, Mapping, Sequence
from fractions import Fraction
from typing import Any

from peykar.model_learning.eye_stacking import HISTORY_MEMBERS, combine, simplex_least_squares
from peykar.model_learning.eye_stacking2 import (
    STACKING_VERSION,
    HistoryStacking,
    HistoryStackingSpec,
    scale_quantiles,
)
from peykar.validation.eye_iop_evaluation import (
    FORECAST_TASKS,
    INTERVALS,
    _interval_coverage,
    _score_rows,
    summarize,
)
from peykar.validation.eye_stacking_evaluation import (
    ADOPTION_GAIN_PP,
    ADOPTION_LOSS_PP,
    STACKED,
    WINDOWS,
    _paired_initial,
    _paired_rows,
    _rmse,
    _selected,
    _stacked_coverage,
    history_errors,
    history_members,
)
from peykar_core.errors import InvalidRequest

__all__ = [
    "EVALUATION_VERSION",
    "STACKED",
    "calibrate_scale",
    "calibration_rows",
    "development_comparison2",
    "fit_history",
    "nested_stack_report2",
    "stack_report2",
    "stack_study2",
    "summarize_stacked2",
]

EVALUATION_ID = "eye.stacking-evaluation"
EVALUATION_VERSION = "2.0.0"
CalibrationRow = tuple[float, float, float, float, float]


def calibration_rows(
    studies: Iterable[Mapping[str, Any]],
    spec: HistoryStackingSpec,
    weights: Sequence[float],
    exclude: Collection[str] = (),
) -> list[CalibrationRow]:
    """(observed, stacked point, model point, lower, upper) of the declared interval for
    every selected row of the calibration task with the model's point and interval."""
    members = spec.members
    low_q, high_q = INTERVALS[spec.calibration_level]
    rows = []
    for study in studies:
        if study["study_id"] in exclude:
            continue
        for entry in study[spec.calibration_task]:
            if not _selected(entry, members):
                continue
            for row in entry["rows"]:
                values = history_members(row, members)
                interval = row["intervals"].get(members.history_model)
                point = combine(values, weights)
                if values[0] is None or interval is None or point is None:
                    continue
                rows.append(
                    (
                        float(row["observed"]),
                        point,
                        float(values[0]),
                        float(interval[low_q]),
                        float(interval[high_q]),
                    )
                )
    return rows


def _inside(rows: Sequence[CalibrationRow], scale: float) -> int:
    return sum(
        int(s + scale * (low - m) <= o <= s + scale * (high - m)) for o, s, m, low, high in rows
    )


def calibrate_scale(rows: Sequence[CalibrationRow], spec: HistoryStackingSpec) -> float:
    """The grid factor whose pooled coverage is closest to the target (exact distances);
    ties: the factor closest to 1, then the smaller one."""
    if not rows:
        raise InvalidRequest("No calibration targets to choose the interval scale on")
    target = Fraction(spec.target_coverage)
    best: tuple[tuple[Fraction, Fraction, Fraction], Fraction] | None = None
    for scale in spec.scales():
        distance = abs(Fraction(_inside(rows, float(scale)), len(rows)) - target)
        key = (distance, abs(scale - 1), scale)
        if best is None or key < best[0]:
            best = (key, scale)
    if best is None:  # pragma: no cover - a valid grid always has a first factor
        raise InvalidRequest("Empty scale grid")
    return float(best[1])


def fit_history(
    studies: Sequence[Mapping[str, Any]],
    spec: HistoryStackingSpec,
    exclude: Collection[str] = (),
) -> HistoryStacking:
    """Weights (version 1.0.0's fit) and the interval scale on ``studies`` minus
    ``exclude``."""
    errors = history_errors(studies, spec.members, exclude)
    if not errors:
        raise InvalidRequest("No complete targets to fit stacking weights on")
    weights = simplex_least_squares(errors)
    scale = calibrate_scale(calibration_rows(studies, spec, weights, exclude), spec)
    return HistoryStacking(weights=weights, interval_scale=scale)


def _stack_forecast_entry(
    entry: Mapping[str, Any], stacking: HistoryStacking, spec: HistoryStackingSpec
) -> dict[str, Any]:
    model = spec.members.history_model
    rows = []
    for row in entry["rows"]:
        values = history_members(row, spec.members)
        predictions = dict(row["predictions"])
        intervals = dict(row["intervals"])
        point = combine(values, stacking.weights)
        if point is not None and values[0] is not None:
            predictions[STACKED] = point
            interval = row["intervals"].get(model)
            if interval is not None:
                intervals[STACKED] = scale_quantiles(
                    interval, float(values[0]), point, stacking.interval_scale
                )
        rows.append({**row, "predictions": predictions, "intervals": intervals})
    return {
        **entry,
        "rows": rows,
        "scores": _score_rows(rows) if rows else {},
        "interval_coverage": _interval_coverage(rows),
        "stacked_interval_coverage": _stacked_coverage(rows),
    }


def stack_study2(
    study: Mapping[str, Any], stacking: HistoryStacking, spec: HistoryStackingSpec
) -> dict[str, Any]:
    """One study result with stacked forecasts from follow-up added and their scores
    recomputed; initial states unchanged."""
    out = dict(study)
    for task in FORECAST_TASKS:
        out[task] = [_stack_forecast_entry(e, stacking, spec) for e in study[task]]
    out["stacking"] = stacking.describe()
    return out


def summarize_stacked2(studies: Sequence[dict[str, Any]]) -> dict[str, Any]:
    """The harnesses' summary plus the pooled coverage of the stacked forecast intervals."""
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
    return summary


def _stacking_block(spec: HistoryStackingSpec, mode: str, **extra: Any) -> dict[str, Any]:
    return {
        "evaluation_id": EVALUATION_ID,
        "evaluation_version": EVALUATION_VERSION,
        "stacking_version": STACKING_VERSION,
        "mode": mode,
        "spec": spec.describe(),
        **extra,
    }


def stack_report2(
    report: Mapping[str, Any], stacking: HistoryStacking, spec: HistoryStackingSpec
) -> dict[str, Any]:
    """A report with a frozen stacking applied to every study."""
    studies = [stack_study2(s, stacking, spec) for s in report["studies"]]
    return {
        **report,
        "studies": studies,
        "summary": summarize_stacked2(studies),
        "stacking": _stacking_block(spec, "frozen weights and scale", stacking=stacking.describe()),
    }


def _calibration_summary(
    studies: Sequence[Mapping[str, Any]], spec: HistoryStackingSpec, stacking: HistoryStacking
) -> dict[str, Any]:
    rows = calibration_rows(studies, spec, stacking.weights)
    inside = _inside(rows, stacking.interval_scale)
    moved = _inside(rows, 1.0)
    return {
        "targets": len(rows),
        "coverage_at_scale": inside / len(rows) if rows else None,
        "coverage_moved_unscaled": moved / len(rows) if rows else None,
    }


def nested_stack_report2(report: Mapping[str, Any], spec: HistoryStackingSpec) -> dict[str, Any]:
    """Each held-out study stacked with weights and a scale fitted without it and its
    linked studies."""
    linked: dict[str, set[str]] = {}
    for cluster in report.get("clusters", []):
        for member in cluster:
            linked.setdefault(member, set()).update(cluster)
    studies, by_study = [], {}
    for study in report["studies"]:
        excluded = linked.get(study["study_id"], set()) | {study["study_id"]}
        stacking = fit_history(report["studies"], spec, exclude=excluded)
        studies.append(stack_study2(study, stacking, spec))
        by_study[study["study_id"]] = stacking.describe()
    everything = fit_history(report["studies"], spec)
    return {
        **report,
        "studies": studies,
        "summary": summarize_stacked2(studies),
        "stacking": _stacking_block(
            spec,
            "nested leave-one-study-out",
            stacking_by_held_out_study=by_study,
            stacking_all_studies=everything.describe(),
            targets_all_studies={"history": len(history_errors(report["studies"], spec.members))},
            calibration_all_studies=_calibration_summary(report["studies"], spec, everything),
        ),
    }


def _coverage(
    studies: Sequence[Mapping[str, Any]], spec: HistoryStackingSpec, task: str, unscaled: bool
) -> dict[str, Any]:
    """Pooled coverage of the stacked intervals of ``task`` over the selected targets;
    ``unscaled``: the same rows with the model's interval only moved (reported)."""
    model = spec.members.history_model
    result = {}
    for interval, (low, high) in INTERVALS.items():
        inside = n = 0
        for study in studies:
            for entry in study[task]:
                if not _selected(entry, spec.members):
                    continue
                for row in entry["rows"]:
                    if STACKED not in row["intervals"]:
                        continue
                    if unscaled:
                        shift = row["predictions"][STACKED] - row["predictions"][model]
                        bounds = {q: v + shift for q, v in row["intervals"][model].items()}
                    else:
                        bounds = row["intervals"][STACKED]
                    n += 1
                    inside += int(bounds[low] <= row["observed"] <= bounds[high])
        value = inside / n if n else None
        window = WINDOWS[interval]
        result[interval] = {
            "inside": inside,
            "n": n,
            "coverage": value,
            "within_window": value is not None and window[0] <= value <= window[1],
        }
    return result


def development_comparison2(
    stacked: Mapping[str, Any], spec: HistoryStackingSpec
) -> dict[str, Any]:
    """Pooled RMSE (reported pp) of the stacked forecasts from follow-up against the
    model's primary on the selected targets, the coverage of the stacked next-observation
    intervals and the declared adoption rule (a gain of at least 0.10 pp in one forecast
    task, a loss of at most 0.05 pp in both, coverage inside both H1 windows). The
    initial state is the model's own; its RMSE is reported for completeness."""
    studies = stacked["studies"]
    members = spec.members
    tasks: dict[str, Any] = {}
    for task in FORECAST_TASKS:
        new, old = _paired_rows(studies, members, task, STACKED, members.history_model)
        both = bool(new and old)
        tasks[task] = {
            "targets": len(new),
            "stacked": _rmse(new),
            "model": _rmse(old),
        }
        tasks[task]["gain"] = tasks[task]["model"] - tasks[task]["stacked"] if both else None
    own, _ = _paired_initial(studies, members, members.initial_model, members.initial_model)
    coverage = _coverage(studies, spec, "next_observation", unscaled=False)
    gains = [v["gain"] for v in tasks.values()]
    adopted = bool(
        all(g is not None for g in gains)
        and any(g >= ADOPTION_GAIN_PP for g in gains if g is not None)
        and all(g >= -ADOPTION_LOSS_PP for g in gains if g is not None)
        and all(c["within_window"] for c in coverage.values())
    )
    return {
        "members": list(HISTORY_MEMBERS),
        "tasks": tasks,
        "initial_state_model_own": {"targets": len(own), "model": _rmse(own)},
        "coverage_next_observation": coverage,
        "reported_not_in_rule": {
            "coverage_next_observation_moved_unscaled": _coverage(
                studies, spec, "next_observation", unscaled=True
            ),
            "coverage_landmark": _coverage(studies, spec, "landmark", unscaled=False),
            "coverage_landmark_moved_unscaled": _coverage(studies, spec, "landmark", unscaled=True),
        },
        "adopted": adopted,
    }
