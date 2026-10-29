"""Additive-scale evaluation of the generic eye models (version 1.0.0).

Some eye outcomes change additively, for example the spherical equivalent after
refractive surgery. Their series hand the generic models the pseudo-fraction
``f = exp(change / model_unit)`` (see ``cornea_refraction_data``), so the models' log
fraction is the change in model units and every comparator of the harness becomes
its additive analogue: persistence keeps the last change, the log-linear
extrapolations are linear in the change, the pooled-curve ratio adds the pooled
curve's change and the analogs match changes.

The version-2 and version-3 harnesses (unchanged) score fractions as
``100 (f_hat - f)``, percentage points of the baseline. This module runs them as they
are and then rewrites every observation, point forecast and interval of a study
result as ``g = 1 + change / (100 report_unit)`` and the reported reference as the
baseline value itself, and recomputes every score with the harnesses' own scoring
functions. One "pp" of the rewritten report is one ``report_unit``: for refraction
with ``report_unit = 0.1 D`` the per-series criteria (RMSE <= 5, max <= 10 pp,
forecasts at least 0.5 pp below persistence) read 0.5 D, 1.0 D and 0.05 D. Interval
coverage is unchanged because the map is monotone. Nothing here reads files.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Sequence
from dataclasses import asdict, dataclass
from typing import Any

from peykar.model_learning import eye_recovery_population2 as population
from peykar.model_learning import eye_recovery_population3 as version3
from peykar.validation import eye_recovery_evaluation2 as harness2
from peykar.validation import eye_recovery_evaluation3 as harness3
from peykar.validation.cornea_population_evaluation import _errors, _passes
from peykar.validation.eye_iop_evaluation import (
    MAX_LIMIT_PP,
    PERSISTENCE_MARGIN_PP,
    RMSE_LIMIT_PP,
    _interval_coverage,
    _score_rows,
    summarize,
)
from peykar_core.errors import InvalidRequest

__all__ = ["AdditiveScale", "criteria", "evaluate_study", "leave_one_study_out", "summarize"]

EVALUATION_ID = "eye.additive-recovery-evaluation"
EVALUATION_VERSION = "1.0.0"


@dataclass(frozen=True)
class AdditiveScale:
    """How pseudo-fractions map to the outcome: change = model_unit ln f, baseline =
    covariate_unit ln(reference); one reported pp is one report_unit."""

    quantity: str
    unit: str
    model_unit: float
    covariate_unit: float
    report_unit: float

    def __post_init__(self) -> None:
        if not all(v > 0 for v in (self.model_unit, self.covariate_unit, self.report_unit)):
            raise InvalidRequest("Additive scale units must be positive")

    def change(self, fraction: float) -> float:
        return self.model_unit * math.log(fraction)

    def report(self, fraction: float) -> float:
        """The reported value ``1 + change / (100 report_unit)``."""
        return 1.0 + self.change(fraction) / (100.0 * self.report_unit)

    def baseline(self, reference: float) -> float:
        return self.covariate_unit * math.log(reference)

    def describe(self) -> dict[str, Any]:
        return asdict(self)


def _harness(config: population.RecoveryConfig) -> Any:
    return harness3 if isinstance(config, version3.RecoveryConfig) else harness2


def criteria(scale: AdditiveScale) -> dict[str, Any]:
    return {
        "rmse_pp_max": RMSE_LIMIT_PP,
        "max_abs_pp_max": MAX_LIMIT_PP,
        "forecast_persistence_margin_pp": PERSISTENCE_MARGIN_PP,
        "unit": f"1 pp = {scale.report_unit:g} {scale.unit} of {scale.quantity} change from "
        "baseline",
        "rmse_max": f"{RMSE_LIMIT_PP * scale.report_unit:g} {scale.unit}",
        "max_abs_max": f"{MAX_LIMIT_PP * scale.report_unit:g} {scale.unit}",
        "forecast_persistence_margin": f"{PERSISTENCE_MARGIN_PP * scale.report_unit:g} "
        f"{scale.unit}",
    }


def _initial(entry: dict[str, Any], scale: AdditiveScale) -> dict[str, Any]:
    out = dict(entry)
    out["reference_value"] = scale.baseline(entry["reference_value"])
    out["observed"] = [scale.report(v) for v in entry["observed"]]
    out["predictions"] = {
        name: [scale.report(v) for v in values] for name, values in entry["predictions"].items()
    }
    out["scores"] = {}
    for name, values in out["predictions"].items():
        score = _errors(values, out["observed"])
        score["meets_5_10_pp"] = _passes(score)
        out["scores"][name] = score
    return out


def _forecast(entry: dict[str, Any], scale: AdditiveScale) -> dict[str, Any]:
    rows = []
    for row in entry["rows"]:
        rows.append(
            {
                **row,
                "observed": scale.report(row["observed"]),
                "predictions": {
                    name: scale.report(value) for name, value in row["predictions"].items()
                },
                "intervals": {
                    name: {q: scale.report(v) for q, v in quantiles.items()}
                    for name, quantiles in row["intervals"].items()
                },
            }
        )
    return {
        **entry,
        "rows": rows,
        "scores": _score_rows(rows),
        "interval_coverage": _interval_coverage(rows),
    }


def rescale_study(result: dict[str, Any], scale: AdditiveScale) -> dict[str, Any]:
    """A harness study result in reported units, every score recomputed."""
    out = dict(result)
    out["initial_state"] = [_initial(e, scale) for e in result["initial_state"]]
    for task in ("next_observation", "landmark"):
        out[task] = [_forecast(e, scale) for e in result[task]]
    out["additive_evaluation_version"] = EVALUATION_VERSION
    return out


def evaluate_study(
    config: population.RecoveryConfig,
    *,
    study_series: Sequence[Any],
    training: Sequence[Any],
    scale: AdditiveScale,
) -> dict[str, Any]:
    """Score one excluded study on the additive scale (training-only models)."""
    result = _harness(config).evaluate_study(config, study_series=study_series, training=training)
    return rescale_study(result, scale)


def leave_one_study_out(
    config: population.RecoveryConfig,
    series: Sequence[Any],
    *,
    scale: AdditiveScale,
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
        "harness_version": _harness(config).EVALUATION_VERSION,
        "model_id": config.model_id,
        "model_version": config.model_version,
        "model_configuration": config.describe(),
        "scale": scale.describe(),
        "criteria": criteria(scale),
        "clusters": [sorted(c) for c in clusters],
        "studies": reports,
        "summary": summarize(reports),
    }
