"""Population model of an eye measurement after treatment, generic version 3.

Generic model 2 (``eye_recovery_population2``, unchanged because the retinal-thickness
1.1 and IOP 1.2 protocols pin it) with one option for the point forecast. The
posterior, the predictive distribution and its quantiles are those of version 2.

- ``point = "median"`` (version 2): the point forecast is the predictive median.
- ``point = "lognormal_mean"``: the point forecast is the mean of the lognormal
  distribution that has the predictive median ``m`` and the predictive 5% and 95%
  quantiles ``q05``, ``q95``: ``m * exp(s^2 / 2)`` with
  ``s = (ln q95 - ln q05) / (2 z_0.95)``. Scores are root mean squared errors of the
  fraction, whose minimizer is the predictive mean, not the median; the exact mixture
  mean is not used because a few components with negligible weight and very large
  variance can make it unbounded.

Nothing here reads files, registers models or claims clinical validity.
"""

from __future__ import annotations

import math
from dataclasses import dataclass
from typing import Any

from peykar.model_learning import eye_recovery_population2 as version2
from peykar_core.errors import InvalidRequest

__all__ = ["GENERIC_VERSION", "POINTS", "RecoveryConfig", "point_forecast"]

GENERIC_VERSION = "3.0.0"
POINTS = {"median", "lognormal_mean"}
Z95 = 1.6448536269514722


@dataclass(frozen=True)
class RecoveryConfig(version2.RecoveryConfig):
    """Version 2's configuration plus the point-forecast rule (not fitted)."""

    point: str = "median"

    def __post_init__(self) -> None:
        super().__post_init__()
        if self.point not in POINTS:
            raise InvalidRequest(f"The point forecast must be one of {sorted(POINTS)}")

    def describe(self) -> dict[str, Any]:
        return {**super().describe(), "generic_model_version": GENERIC_VERSION, "point": self.point}


def point_forecast(median: float, q05: float, q95: float, point: str) -> float:
    """The point forecast from the predictive median and 5%/95% quantiles."""
    if point == "median":
        return median
    if point != "lognormal_mean":
        raise InvalidRequest(f"The point forecast must be one of {sorted(POINTS)}")
    if not all(math.isfinite(v) and v > 0 for v in (median, q05, q95)):
        raise InvalidRequest("Predictive quantiles must be finite and positive")
    spread = abs(math.log(q95) - math.log(q05)) / (2.0 * Z95)
    return median * math.exp(0.5 * spread * spread)
