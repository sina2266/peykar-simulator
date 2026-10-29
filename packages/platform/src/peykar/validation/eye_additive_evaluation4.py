"""Additive-scale evaluation of the generic eye models, version 4.0.0.

Version 3.0.0 (``eye_additive_evaluation3``, unchanged because frozen protocols pin it)
with harness 6 (``eye_recovery_evaluation6``) in place of harness 2: configurations that
version 3.0.0 sends to harness 2 keep their initial-state intervals, which are rewritten
into reported units like every other value. Other configurations go to the harness
version 3.0.0 chooses. The rewriting, the criteria and the summaries are version
1.0.0's, so reports equal version 3.0.0's apart from the intervals and the evaluation
versions. Nothing here reads files.
"""

from __future__ import annotations

from collections.abc import Callable, Sequence
from typing import Any

from peykar.model_learning import eye_recovery_population2 as population
from peykar.validation import eye_additive_evaluation as version1
from peykar.validation import eye_additive_evaluation3 as version3
from peykar.validation import eye_recovery_evaluation2 as harness2
from peykar.validation import eye_recovery_evaluation6 as harness6
from peykar_core.errors import InvalidRequest

__all__ = ["AdditiveScale", "criteria", "evaluate_study", "leave_one_study_out", "summarize"]

AdditiveScale = version1.AdditiveScale
criteria = version1.criteria
summarize = version1.summarize

EVALUATION_ID = version1.EVALUATION_ID
EVALUATION_VERSION = "4.0.0"


def harness(config: population.RecoveryConfig) -> Any:
    chosen = version3._harness(config)
    return harness6 if chosen is harness2 else chosen


def rescale_study(
    result: dict[str, Any],
    scale: version1.AdditiveScale,
    baselines: dict[str, float] | None = None,
) -> dict[str, Any]:
    """Version 1.0.0's rewriting into reported units, initial-state intervals included."""
    out = version1.rescale_study(result, scale, baselines)
    for source, entry in zip(result["initial_state"], out["initial_state"], strict=True):
        intervals = source.get(harness6.INTERVAL_FIELD)
        if intervals is not None:
            entry[harness6.INTERVAL_FIELD] = {
                name: {q: [scale.report(v) for v in values] for q, values in quantiles.items()}
                for name, quantiles in intervals.items()
            }
    out["additive_evaluation_version"] = EVALUATION_VERSION
    return out


def evaluate_study(
    config: population.RecoveryConfig,
    *,
    study_series: Sequence[Any],
    training: Sequence[Any],
    scale: version1.AdditiveScale,
) -> dict[str, Any]:
    """Score one excluded study on the additive scale (training-only models)."""
    result = harness(config).evaluate_study(config, study_series=study_series, training=training)
    baselines = {s.series_id: scale.baseline(s.reference_value) for s in study_series}
    out = rescale_study(result, scale, baselines)
    if "training_support" in result:
        out["training_support"] = result["training_support"]
    return out


def leave_one_study_out(
    config: population.RecoveryConfig,
    series: Sequence[Any],
    *,
    scale: version1.AdditiveScale,
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
        report = evaluate_study(config, study_series=held, training=training, scale=scale)
        report["training_exclusions"] = sorted(excluded - {study_id})
        reports.append(report)
    return {
        "evaluation_id": EVALUATION_ID,
        "evaluation_version": EVALUATION_VERSION,
        "harness_version": harness(config).EVALUATION_VERSION,
        "model_id": config.model_id,
        "model_version": config.model_version,
        "model_configuration": config.describe(),
        "scale": scale.describe(),
        "criteria": criteria(scale),
        "clusters": [sorted(c) for c in clusters],
        "studies": reports,
        "summary": summarize(reports),
    }
