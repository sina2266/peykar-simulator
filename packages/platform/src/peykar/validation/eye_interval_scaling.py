"""Model-only interval scaling for the generic eye models (version 1.0.0).

Stacking 2.0.0 (``eye_stacking2`` and ``eye_stacking_evaluation2``, unchanged because
frozen protocols pin them) combines the model's forecast from follow-up with
persistence and the pooled-curve increment and scales the model's interval around the
combined point, ``q' = s + c (q - m)``. This module keeps the model's own point forecast
(its weight is fixed at one and the other members' at zero) and only scales its
intervals, ``q' = m + c (q - m)``. The factor ``c`` is chosen by stacking 2.0.0's rule on
development forecasts: over the declared grid, the factor whose pooled coverage of the
declared interval of the declared task is closest to its nominal level; ties go to the
factor closest to one, then to the smaller one. Optionally each forecast task gets its
own factor (``HistoryStackingSpec.scale_tasks``).

The width of a population model's intervals comes from its noise model, which is
estimated from the training studies' residuals around their own fitted laws; how wide
the intervals must be for a study the model has not seen is measured here directly, on
leave-one-study-out forecasts.

- ``fit_scale`` chooses the factor (or one per task) on studies minus exclusions;
- ``nested_scale_report`` (development) scales each held-out study of a
  leave-one-study-out report with a factor fitted without it and its linked studies
  (the report's clusters);
- ``scale_report`` applies a frozen factor to a sealed report.

The scaled forecasts are named ``stacked`` as in stacking 2.0.0, so the harnesses'
scores, summaries and coverages apply unchanged; their points equal the model's. Initial
states are left unchanged. Nothing here reads files or claims clinical validity.
"""

from __future__ import annotations

from collections.abc import Collection, Mapping, Sequence
from typing import Any

from peykar.model_learning.eye_stacking2 import HistoryStacking, HistoryStackingSpec
from peykar.validation.eye_stacking_evaluation import history_errors
from peykar.validation.eye_stacking_evaluation2 import (
    calibrate_scale,
    calibration_rows,
    stack_study2,
    summarize_stacked2,
)
from peykar_core.errors import InvalidRequest

__all__ = [
    "EVALUATION_VERSION",
    "MODEL_ONLY",
    "fit_scale",
    "nested_scale_report",
    "scale_report",
]

EVALUATION_ID = "eye.interval-scaling"
EVALUATION_VERSION = "1.0.0"
# Weights of (model, persistence, pooled-curve increment): the model's point only.
MODEL_ONLY = (1.0, 0.0, 0.0)


def fit_scale(
    studies: Sequence[Mapping[str, Any]],
    spec: HistoryStackingSpec,
    exclude: Collection[str] = (),
) -> HistoryStacking:
    """The interval scale (or one scale per task of ``spec.scale_tasks``) chosen on
    ``studies`` minus ``exclude``, with the model's point as the forecast."""
    scale = calibrate_scale(calibration_rows(studies, spec, MODEL_ONLY, exclude), spec)
    if spec.scale_tasks is None:
        return HistoryStacking(weights=MODEL_ONLY, interval_scale=scale)
    scales = []
    for task in spec.scale_tasks:
        if task == spec.calibration_task:
            scales.append((task, scale))
        else:
            rows = calibration_rows(studies, spec, MODEL_ONLY, exclude, task)
            scales.append((task, calibrate_scale(rows, spec)))
    return HistoryStacking(weights=MODEL_ONLY, interval_scale=scale, task_scales=tuple(scales))


def _block(spec: HistoryStackingSpec, mode: str, **extra: Any) -> dict[str, Any]:
    return {
        "evaluation_id": EVALUATION_ID,
        "evaluation_version": EVALUATION_VERSION,
        "mode": mode,
        "weights": "the model's point forecast only (weights 1, 0, 0 of model, persistence "
        "and pooled-curve increment)",
        "spec": spec.describe(),
        **extra,
    }


def _check_model_only(stacking: HistoryStacking) -> None:
    if tuple(stacking.weights) != MODEL_ONLY:
        raise InvalidRequest("Interval scaling keeps the model's point (weights 1, 0, 0)")


def scale_report(
    report: Mapping[str, Any], stacking: HistoryStacking, spec: HistoryStackingSpec
) -> dict[str, Any]:
    """A report with a frozen interval scale applied to every study."""
    _check_model_only(stacking)
    studies = [stack_study2(s, stacking, spec) for s in report["studies"]]
    return {
        **report,
        "studies": studies,
        "summary": summarize_stacked2(studies),
        "interval_scaling": _block(spec, "frozen scale", scaling=stacking.describe()),
    }


def nested_scale_report(report: Mapping[str, Any], spec: HistoryStackingSpec) -> dict[str, Any]:
    """Each held-out study scaled with a factor fitted without it and its linked studies;
    the factor fitted on every study is reported for freezing."""
    linked: dict[str, set[str]] = {}
    for cluster in report.get("clusters", []):
        for member in cluster:
            linked.setdefault(member, set()).update(cluster)
    studies, by_study = [], {}
    for study in report["studies"]:
        excluded = linked.get(study["study_id"], set()) | {study["study_id"]}
        stacking = fit_scale(report["studies"], spec, exclude=excluded)
        studies.append(stack_study2(study, stacking, spec))
        by_study[study["study_id"]] = stacking.describe()
    everything = fit_scale(report["studies"], spec)
    targets = len(calibration_rows(report["studies"], spec, MODEL_ONLY))
    return {
        **report,
        "studies": studies,
        "summary": summarize_stacked2(studies),
        "interval_scaling": _block(
            spec,
            "nested leave-one-study-out",
            scaling_by_held_out_study=by_study,
            scaling_all_studies=everything.describe(),
            calibration_targets_all_studies=targets,
            history_targets_all_studies=len(history_errors(report["studies"], spec.members)),
        ),
    }
