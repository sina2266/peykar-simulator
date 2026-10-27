"""Instance-based (k-nearest-trajectory) corneal wound forecaster, numpy only.

This is the declared machine-learning comparator for the population model. It
learns no parametric law: a query's observed prefix is matched against library
trajectories expressed as physical front displacement ``d(t)=r0*(1-sqrt(a(t)))``
(µm), allowing a bounded time shift to align latent phases. The forecast is the
weighted continuation of the closest analogs, anchored to the query's last
observation. Hyperparameters are fixed constants, never tuned on evaluation data.
"""

from __future__ import annotations

import math
from collections.abc import Sequence
from dataclasses import dataclass
from typing import Any

import numpy as np
from numpy.typing import NDArray

from peykar.model_learning.corneal_population import WoundSeries
from peykar_core.errors import InvalidRequest

METHOD_ID = "cornea.analog-trajectory-knn"
METHOD_VERSION = "1.0.0"
NEIGHBORS = 5
SHIFT_GRID_H: NDArray[np.float64] = np.arange(-12.0, 12.0 + 1e-9, 0.5)
FloatArray = NDArray[np.float64]


@dataclass(frozen=True)
class _Trajectory:
    series_id: str
    study_id: str
    hours: FloatArray
    displacement_um: FloatArray
    radius0_um: float
    closed_from_index: int | None

    def at(self, query: FloatArray) -> FloatArray:
        """Interpolated displacement; linear extrapolation beyond the last open point."""
        values = np.interp(query, self.hours, self.displacement_um)
        last_open = (
            len(self.hours) - 1 if self.closed_from_index is None else self.closed_from_index - 1
        )
        if last_open >= 1:
            t0, t1 = self.hours[last_open - 1], self.hours[last_open]
            d0, d1 = self.displacement_um[last_open - 1], self.displacement_um[last_open]
            slope = max(0.0, (d1 - d0) / (t1 - t0))
            beyond = query > t1
            values = np.where(beyond, d1 + slope * (query - t1), values)
        result: FloatArray = np.maximum(0.0, values)
        return result


def _trajectory(series: WoundSeries) -> _Trajectory:
    hours = np.concatenate([[0.0], np.asarray(series.hours, dtype=np.float64)])
    fractions = np.concatenate([[1.0], np.asarray(series.fractions, dtype=np.float64)])
    displacement = series.radius0_um * (1.0 - np.sqrt(fractions))
    closed = [i for i, f in enumerate(fractions) if f == 0.0]
    return _Trajectory(
        series_id=series.series_id,
        study_id=series.study_id,
        hours=hours,
        displacement_um=displacement,
        radius0_um=series.radius0_um,
        closed_from_index=closed[0] if closed else None,
    )


def displacement_at(series: WoundSeries, hours: Sequence[float]) -> FloatArray:
    """Physical front displacement (µm) of a library series at query hours."""
    return _trajectory(series).at(np.asarray(hours, dtype=np.float64))


def forecast_analog(
    library: Sequence[WoundSeries],
    *,
    prefix: WoundSeries,
    future_hours: Sequence[float],
) -> dict[str, Any]:
    """Forecast area fractions at ``future_hours`` from the closest library analogs."""
    candidates = [_trajectory(s) for s in library if s.study_id != prefix.study_id]
    if len(candidates) < NEIGHBORS:
        raise InvalidRequest("Analog forecasting needs at least five other-study trajectories")
    targets = np.asarray(future_hours, dtype=np.float64)
    if targets.size == 0 or float(targets[0]) <= prefix.hours[-1]:
        raise InvalidRequest("Analog forecasts must target hours after the observed prefix")
    query = _trajectory(prefix)
    observed_hours = query.hours[1:]
    observed = query.displacement_um[1:]
    scored = []
    for candidate in candidates:
        best = (math.inf, 0.0)
        for shift in SHIFT_GRID_H:
            shifted = observed_hours + shift
            if float(np.min(shifted)) < 0.0:
                continue
            distance = float(np.sqrt(np.mean((candidate.at(shifted) - observed) ** 2)))
            if distance < best[0]:
                best = (distance, float(shift))
        if math.isfinite(best[0]):
            scored.append((best[0], best[1], candidate))
    scored.sort(key=lambda item: (item[0], item[2].series_id))
    chosen = scored[:NEIGHBORS]
    if len(chosen) < NEIGHBORS:
        raise InvalidRequest("Too few alignable analog trajectories")
    bandwidth = max(float(np.median([c[0] for c in chosen])), 1.0)
    weights = np.array([math.exp(-0.5 * (c[0] / bandwidth) ** 2) for c in chosen])
    weights = weights / weights.sum()
    last_hour = float(observed_hours[-1])
    predictions = np.zeros(targets.size)
    for weight, (_, shift, candidate) in zip(weights, chosen, strict=True):
        anchor = float(observed[-1]) - float(candidate.at(np.array([last_hour + shift]))[0])
        displacement = candidate.at(targets + shift) + anchor
        rho = np.maximum(0.0, 1.0 - displacement / prefix.radius0_um)
        predictions += weight * rho**2
    return {
        "method_id": METHOD_ID,
        "method_version": METHOD_VERSION,
        "hours": [float(h) for h in targets],
        "area_fraction": [float(v) for v in predictions],
        "neighbors": [
            {"series_id": c[2].series_id, "distance_um": c[0], "shift_h": c[1], "weight": float(w)}
            for w, c in zip(weights, chosen, strict=True)
        ],
    }
