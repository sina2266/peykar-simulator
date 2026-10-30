"""Additive-scale evaluation of the generic eye models, version 5.0.0.

Version 4.0.0 (``eye_additive_evaluation4``, unchanged because frozen protocols pin it)
with harness 7 (``eye_recovery_evaluation7``, generic eye model version 5) for
version-5 configurations; every other configuration goes to the harness version 4.0.0
chooses, so its reports equal version 4.0.0's apart from the evaluation versions. The
rewriting into reported units, the criteria and the summaries are version 1.0.0's.
Nothing here reads files.
"""

from __future__ import annotations

from collections.abc import Callable, Sequence
from typing import Any

from peykar.model_learning import eye_recovery_population2 as population
from peykar.model_learning import eye_recovery_population5 as version5
from peykar.validation import eye_additive_evaluation as version1
from peykar.validation import eye_additive_evaluation4 as version4
from peykar.validation import eye_recovery_evaluation7 as harness7
from peykar_core.errors import InvalidRequest

__all__ = ["AdditiveScale", "criteria", "evaluate_study", "leave_one_study_out", "summarize"]

AdditiveScale = version1.AdditiveScale
criteria = version1.criteria
summarize = version1.summarize
EVALUATION_ID = version1.EVALUATION_ID
EVALUATION_VERSION = "5.0.0"


def harness(config: population.RecoveryConfig) -> Any:
    if isinstance(config, version5.RecoveryConfig):
        return harness7
    return version4.harness(config)


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
    out = version4.rescale_study(result, scale, baselines)
    out["additive_evaluation_version"] = EVALUATION_VERSION
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
