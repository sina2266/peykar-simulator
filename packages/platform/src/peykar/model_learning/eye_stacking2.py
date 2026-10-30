"""Forecast stacking for the generic eye models, version 2.0.0.

Version 1.0.0 (``eye_stacking``, unchanged because frozen protocols pin it) stacked both
the forecasts from follow-up and the initial states, and moved the model's interval to
the stacked point with its width unchanged. Its two sealed tests (visual acuity 1.2,
retinal thickness 1.3) showed that the forecasts from follow-up improved, that stacking
initial states with the pooled training curve (which ignores the baseline value) did
not help, and that moved intervals covered too much. Version 2.0.0 therefore:

- stacks only forecasts from follow-up, with version 1.0.0's members and combination
  (the model's point forecast, persistence and the pooled-curve increment); initial
  states stay the model's own;
- gives a stacked forecast the model's quantiles measured from the model's point,
  scaled by one factor ``c`` and placed at the stacked point:
  ``q' = s + c (q - m)``. The factor is chosen on development forecasts from a declared
  grid so that the pooled coverage of one declared interval (by default the 80%
  interval of the next-observation forecasts) is closest to its nominal level; ties
  go to the factor closest to 1, then to the smaller one. Optionally each forecast task
  gets its own factor, chosen the same way on that task's own intervals.

Values are on the harnesses' scale (one reported pp is 0.01). Nothing here reads files
or claims clinical validity.
"""

from __future__ import annotations

import math
from collections.abc import Mapping
from dataclasses import dataclass, field
from fractions import Fraction
from typing import Any

from peykar.model_learning.eye_stacking import HISTORY_MEMBERS, STACKING_ID, StackingSpec, _check
from peykar_core.errors import InvalidRequest

__all__ = [
    "STACKING_VERSION",
    "HistoryStackedCandidate",
    "HistoryStacking",
    "HistoryStackingSpec",
    "scale_quantiles",
]

STACKING_VERSION = "2.0.0"


@dataclass(frozen=True)
class HistoryStackingSpec:
    """Which forecasts are stacked and how the interval scale is chosen.

    ``members`` names the model's forecast from follow-up (``history_model``), the
    model's own initial state that is kept unstacked (``initial_model``) and the targets
    that fit the weights and the scale (series of ``kind`` whose context starts with
    ``context_prefix``). The scale is chosen on the intervals ``calibration_level`` of
    task ``calibration_task`` so their pooled coverage is closest to
    ``target_coverage``, over the grid ``scale_grid`` = (first, last, step), written
    as decimal strings so the grid is exact. ``scale_tasks`` None: that one scale serves
    every task; a tuple of tasks (including ``calibration_task``): one scale per task,
    each chosen by the same rule on that task's own intervals."""

    members: StackingSpec
    calibration_task: str = "next_observation"
    calibration_level: str = "80"
    target_coverage: str = "0.80"
    scale_grid: tuple[str, str, str] = field(default=("0.30", "3.00", "0.01"))
    scale_tasks: tuple[str, ...] | None = None

    def __post_init__(self) -> None:
        first, last, step = (Fraction(v) for v in self.scale_grid)
        if not (0 < first <= last and step > 0) or (last - first) % step != 0:
            raise InvalidRequest("The scale grid must be positive with a whole number of steps")
        if not 0 < Fraction(self.target_coverage) < 1:
            raise InvalidRequest("The target coverage must lie between 0 and 1")
        if self.scale_tasks is not None and (
            self.calibration_task not in self.scale_tasks
            or len(set(self.scale_tasks)) != len(self.scale_tasks)
        ):
            raise InvalidRequest("Per-task scales must include the calibration task once")

    def scales(self) -> tuple[Fraction, ...]:
        """The grid of scale factors, exact."""
        first, last, step = (Fraction(v) for v in self.scale_grid)
        return tuple(first + i * step for i in range(int((last - first) / step) + 1))

    def describe(self) -> dict[str, Any]:
        out: dict[str, Any] = {
            "members": self.members.describe(),
            "initial_states": "the model's own (initial_model), not stacked",
            "calibration_task": self.calibration_task,
            "calibration_level": self.calibration_level,
            "target_coverage": self.target_coverage,
            "scale_grid": list(self.scale_grid),
            "scale_rule": "closest pooled coverage to the target; ties: closest to 1, then smaller",
        }
        if self.scale_tasks is not None:
            out["scale_tasks"] = list(self.scale_tasks)
            out["scale_per_task"] = (
                "each task's scale is chosen by the same rule on that task's own intervals"
            )
        return out


@dataclass(frozen=True)
class HistoryStackedCandidate:
    """A candidate with stacked forecasts from follow-up: its base and its spec."""

    base: str
    spec: HistoryStackingSpec

    def describe(self) -> dict[str, Any]:
        return {"base": self.base, "stacking_version": STACKING_VERSION, **self.spec.describe()}


def _positive(value: float) -> float:
    scale = float(value)
    if not (math.isfinite(scale) and scale > 0):
        raise InvalidRequest("The interval scale must be finite and positive")
    return scale


@dataclass(frozen=True)
class HistoryStacking:
    """Frozen weights of the history members and the interval scale.

    ``interval_scale`` serves every task that ``task_scales`` (pairs of task and scale)
    does not name."""

    weights: tuple[float, ...]
    interval_scale: float
    task_scales: tuple[tuple[str, float], ...] | None = None

    def __post_init__(self) -> None:
        object.__setattr__(self, "weights", _check(self.weights, HISTORY_MEMBERS))
        object.__setattr__(self, "interval_scale", _positive(self.interval_scale))
        if self.task_scales is not None:
            pairs = tuple((str(task), _positive(scale)) for task, scale in self.task_scales)
            if len({task for task, _ in pairs}) != len(pairs):
                raise InvalidRequest("Name each task's scale once")
            object.__setattr__(self, "task_scales", pairs)

    def scale_for(self, task: str) -> float:
        """The interval scale of the forecasts of ``task``."""
        return dict(self.task_scales or ()).get(task, self.interval_scale)

    def describe(self) -> dict[str, Any]:
        out: dict[str, Any] = {
            "stacking_id": STACKING_ID,
            "stacking_version": STACKING_VERSION,
            "history": dict(zip(HISTORY_MEMBERS, self.weights, strict=True)),
            "initial": "the model's own initial state (not stacked)",
            "interval_scale": self.interval_scale,
        }
        if self.task_scales is not None:
            out["task_scales"] = dict(self.task_scales)
        return out

    @classmethod
    def from_description(cls, data: Mapping[str, Any]) -> HistoryStacking:
        if data.get("stacking_id") != STACKING_ID or data.get("stacking_version") != (
            STACKING_VERSION
        ):
            raise InvalidRequest("Unknown stacking description")
        history = data["history"]
        if set(history) != set(HISTORY_MEMBERS):
            raise InvalidRequest("The stacking description names other members")
        tasks = data.get("task_scales")
        return cls(
            weights=tuple(float(history[m]) for m in HISTORY_MEMBERS),
            interval_scale=float(data["interval_scale"]),
            task_scales=None
            if tasks is None
            else tuple((str(k), float(v)) for k, v in tasks.items()),
        )


def scale_quantiles(
    quantiles: Mapping[str, float], model_point: float, stacked_point: float, scale: float
) -> dict[str, float]:
    """The model's quantiles measured from its point, scaled by ``scale`` and placed at
    the stacked point."""
    return {q: stacked_point + scale * (float(v) - model_point) for q, v in quantiles.items()}
