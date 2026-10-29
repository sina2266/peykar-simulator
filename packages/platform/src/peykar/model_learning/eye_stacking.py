"""Forecast stacking for the generic eye models (version 1.0.0).

A stacked forecast is a convex combination of a population model's point forecast and
simple comparators. Its weights are non-negative, sum to one and minimize the pooled
squared error of out-of-sample forecasts (stacked regressions, Breiman 1996): they are
fitted once on the leave-one-study-out forecasts of the development studies and then
frozen. There are two sets of weights:

- ``history`` (forecasts from observed follow-up): the model's point forecast, the last
  observation (persistence) and the last observation moved by the change of the pooled
  training curve from the last visit to the target month (the harnesses'
  ``pooled_curve_ratio``);
- ``initial`` (forecasts from the baseline alone): the model's initial-state forecast
  and the pooled training curve.

A member that is missing for a target (no pooled curve when no training level matches)
drops out and the remaining weights are renormalized; without the model's forecast
there is no stacked forecast. The interval of a stacked forecast is the model's
predictive interval moved by the difference between the stacked and the model's point
forecast, with its width unchanged. Values are on the harnesses' scale (fractions of
the baseline, or the additive report scale), where one reported pp is 0.01. Nothing
here reads files or claims clinical validity.
"""

from __future__ import annotations

import itertools
import math
from collections.abc import Mapping, Sequence
from dataclasses import asdict, dataclass
from typing import Any

import numpy as np

from peykar_core.errors import InvalidRequest

__all__ = [
    "HISTORY_MEMBERS",
    "INITIAL_MEMBERS",
    "STACKING_ID",
    "STACKING_VERSION",
    "StackedCandidate",
    "StackingSpec",
    "StackingWeights",
    "combine",
    "shift_quantiles",
    "simplex_least_squares",
]

STACKING_ID = "eye.forecast-stacking"
STACKING_VERSION = "1.0.0"
HISTORY_MEMBERS = ("model", "persistence", "pooled_curve_increment")
INITIAL_MEMBERS = ("model", "pooled_training_curve")
_TOLERANCE = 1e-9


@dataclass(frozen=True)
class StackingSpec:
    """Which population forecasts are stacked (``history_model`` for forecasts from
    follow-up, ``initial_model`` for initial states) and which targets fit the weights
    (series of ``kind`` whose context starts with ``context_prefix``)."""

    history_model: str
    initial_model: str = "population_initial_state"
    context_prefix: str = "human/"
    kind: str = "group_mean"

    def describe(self) -> dict[str, Any]:
        return asdict(self)


@dataclass(frozen=True)
class StackedCandidate:
    """A stacked candidate: the population candidate it stacks and its stacking spec."""

    base: str
    spec: StackingSpec

    def describe(self) -> dict[str, Any]:
        return {"base": self.base, "spec": self.spec.describe()}


def _check(weights: Sequence[float], members: Sequence[str]) -> tuple[float, ...]:
    values = tuple(float(w) for w in weights)
    if len(values) != len(members):
        raise InvalidRequest(f"Give one weight for each of {list(members)}")
    if not all(math.isfinite(w) and w >= 0 for w in values):
        raise InvalidRequest("Stacking weights must be finite and non-negative")
    if abs(sum(values) - 1.0) > _TOLERANCE:
        raise InvalidRequest("Stacking weights must sum to one")
    return values


@dataclass(frozen=True)
class StackingWeights:
    """Frozen weights of the history members and of the initial-state members."""

    history: tuple[float, ...]
    initial: tuple[float, ...]

    def __post_init__(self) -> None:
        object.__setattr__(self, "history", _check(self.history, HISTORY_MEMBERS))
        object.__setattr__(self, "initial", _check(self.initial, INITIAL_MEMBERS))

    def describe(self) -> dict[str, Any]:
        return {
            "stacking_id": STACKING_ID,
            "stacking_version": STACKING_VERSION,
            "history": dict(zip(HISTORY_MEMBERS, self.history, strict=True)),
            "initial": dict(zip(INITIAL_MEMBERS, self.initial, strict=True)),
        }

    @classmethod
    def from_description(cls, data: Mapping[str, Any]) -> StackingWeights:
        if data.get("stacking_id") != STACKING_ID or data.get("stacking_version") != (
            STACKING_VERSION
        ):
            raise InvalidRequest("Unknown stacking description")
        history, initial = data["history"], data["initial"]
        if set(history) != set(HISTORY_MEMBERS) or set(initial) != set(INITIAL_MEMBERS):
            raise InvalidRequest("The stacking description names other members")
        return cls(
            history=tuple(float(history[m]) for m in HISTORY_MEMBERS),
            initial=tuple(float(initial[m]) for m in INITIAL_MEMBERS),
        )


def simplex_least_squares(errors: Sequence[Sequence[float]]) -> tuple[float, ...]:
    """Weights ``w >= 0``, ``sum(w) = 1`` minimizing ``sum_i (sum_k w_k e_ik)^2``.

    ``errors[i][k]`` is member ``k``'s forecast minus the observation for target ``i``.
    The minimum of this convex problem lies in the relative interior of one face of
    the simplex, where it is that face's equality-constrained minimum; every face is
    tried and the best feasible one is kept. On ties the face found first wins
    (fewer members, earlier members first), so a sole model weight is preferred."""
    matrix = np.asarray(errors, dtype=np.float64)
    if matrix.ndim != 2 or matrix.shape[0] == 0 or matrix.shape[1] == 0:
        raise InvalidRequest("Give at least one target and one member")
    if not np.all(np.isfinite(matrix)):
        raise InvalidRequest("Forecast errors must be finite")
    count = matrix.shape[1]
    gram = matrix.T @ matrix
    best: tuple[float, np.ndarray] | None = None
    for size in range(1, count + 1):
        for subset in itertools.combinations(range(count), size):
            # Optimality conditions of the face: G w + lambda 1 = 0, 1'w = 1.
            system = np.zeros((size + 1, size + 1))
            system[:size, :size] = gram[np.ix_(subset, subset)]
            system[:size, size] = 1.0
            system[size, :size] = 1.0
            right = np.zeros(size + 1)
            right[size] = 1.0
            face = np.linalg.lstsq(system, right, rcond=None)[0][:size]
            if abs(float(face.sum()) - 1.0) > 1e-8 or np.any(face < -_TOLERANCE):
                continue
            weights = np.zeros(count)
            weights[list(subset)] = np.clip(face, 0.0, None)
            weights /= weights.sum()
            value = float(weights @ gram @ weights)
            if best is None or value < best[0] * (1 - 1e-12) - 1e-15:
                best = (value, weights)
    if best is None:  # pragma: no cover - a single member always gives a feasible face
        raise InvalidRequest("No feasible stacking weights")
    return tuple(float(w) for w in best[1])


def combine(values: Sequence[float | None], weights: Sequence[float]) -> float | None:
    """Weighted mean of the available members (``values[0]`` is the model's forecast);
    the weights of missing members are dropped and the others renormalized. ``None``
    without the model forecast; the model forecast when every available weight is 0."""
    if len(values) != len(weights):
        raise InvalidRequest("Give one value for each stacking weight")
    model = values[0]
    if model is None:
        return None
    pairs = [(float(v), float(w)) for v, w in zip(values, weights, strict=True) if v is not None]
    total = sum(w for _, w in pairs)
    if not total > 0:
        return float(model)
    return sum(v * w for v, w in pairs) / total


def shift_quantiles(quantiles: Mapping[str, float], delta: float) -> dict[str, float]:
    """The model's quantiles moved by ``delta`` (the stacked minus the model's point)."""
    return {q: float(v) + delta for q, v in quantiles.items()}
