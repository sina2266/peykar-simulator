"""Axial length in childhood myopia, treated or not, model 1.2: configurations.

Model 1.2 is developed on the 59 consumed records of dataset ``myopia-axial-2026-10-30``
(the 33 tranche-1 records and the 26 tranche-2 records consumed by the one-shot test of
model 1.1) and tested once on screen-2 tranche 1, which was reserved before triage.
Earlier versions are imported and never changed: model 1.0.0 is candidate ``m4`` of
``myopia_population`` and model 1.1.0 is candidate ``n3`` of ``myopia_population_1_1``
(1.0.0 with forecast-only persistent deviations).

Candidates (rounds of ``experiments/eye/myopia/model-1.2/development-log.md``):

- ``p0`` (round 0): model 1.1.0 unchanged apart from its version string, refitted on the
  59 records; the reference of round 1.
- ``p0m`` (round 0): model 1.0.0 unchanged apart from its version string, refitted on
  the 59 records.

``SCALED_CANDIDATES`` names candidates that keep a population candidate's point
forecasts and scale its intervals by a factor chosen on development forecasts
(``peykar.validation.eye_interval_scaling``). ``REFERENCES`` names frozen earlier versions
that a sealed run may score in the same run with the same training. Every candidate and
reference reads its series with the treatment family as the last context part. Each
development round adds a named candidate and never changes an earlier one. Nothing here
reads files or claims clinical validity.
"""

from __future__ import annotations

from collections.abc import Callable, Mapping
from dataclasses import dataclass, replace
from typing import Any

from peykar.model_learning import myopia_population as version1
from peykar.model_learning import myopia_population_1_1 as version1_1
from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.model_learning.eye_stacking2 import HistoryStackingSpec
from peykar.validation.myopia_axial_data import AxialSeries

__all__ = [
    "DEVELOPMENT_CANDIDATES",
    "MODEL_ID",
    "REFERENCES",
    "SCALED_CANDIDATES",
    "ScaledCandidate",
    "adapter",
]

MODEL_ID = version1.MODEL_ID
VERSION1_0 = version1_1.VERSION1
VERSION1_1 = version1_1.DEVELOPMENT_CANDIDATES["n3"]

REFERENCES: dict[str, RecoveryConfig] = {
    # Model 1.0.0 as frozen by its protocol (candidate m4).
    "1.0.0": VERSION1_0,
    # Model 1.1.0 as frozen by its protocol (candidate n3).
    "1.1.0": VERSION1_1,
}

DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # p0 (round 0): model 1.1.0 refitted on the 59 consumed records.
    "p0": replace(VERSION1_1, model_version="1.2.0-p0"),
    # p0m (round 0): model 1.0.0 refitted on the 59 consumed records.
    "p0m": replace(VERSION1_0, model_version="1.2.0-p0m"),
}


@dataclass(frozen=True)
class ScaledCandidate:
    """A population candidate's point forecasts with intervals scaled by a factor chosen
    on development forecasts by ``spec`` (model-only interval scaling 1.0.0)."""

    base: str
    spec: HistoryStackingSpec

    def describe(self) -> dict[str, Any]:
        return {
            "base": self.base,
            "interval_scaling": "eye_interval_scaling 1.0.0 (weights 1, 0, 0)",
            **self.spec.describe(),
        }


SCALED_CANDIDATES: dict[str, ScaledCandidate] = {}


def adapter(name: str) -> Callable[[Mapping[str, Any]], list[AxialSeries]]:
    """How the series of candidate or reference ``name`` are read from a record."""
    if name in SCALED_CANDIDATES:
        name = SCALED_CANDIDATES[name].base
    if name not in DEVELOPMENT_CANDIDATES and name not in REFERENCES:
        raise KeyError(name)
    return version1.to_axial_series_by_family
