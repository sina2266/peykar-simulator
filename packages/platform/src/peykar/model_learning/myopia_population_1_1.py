"""Axial length in childhood myopia, treated or not, model 1.1: configurations.

Model 1.1 is developed on the 33 tranche-1 records of dataset ``myopia-axial-2026-10-30``
(the 22 development records and the 11 sealed records consumed by the one-shot test of
model 1.0.0) and tested once on the tranche-2 records, which were reserved before
triage. Model 1.0.0 is candidate ``m4`` of ``myopia_population`` (frozen by its
protocol, so it is imported here and never changed): generic eye model 5 with the
zero-start law ``change(t) / 1 mm = ell (1 - r(t))``, ``r = exp(-(t - 1 day)/tau)``,
``ell = context level + b (AL0 - 24.5 mm) / 1 mm``, the hierarchy species → treatment
family → treatment, a study effect per family and forecast-only pooled noise.

Candidates (rounds of ``experiments/eye/myopia/model-1.1/development-log.md``):

- ``n0`` (round 0): model 1.0.0 unchanged apart from its version string, refitted on
  the 33 training records; the reference of round 1.
- ``n1`` (round 1): ``n0`` with group-mean noise not scaled by group size
  (``max_noise_exponent = 0``): the residual of a group mean around the law is taken as
  a visit-level error that does not shrink with the number of children.
- ``n2`` (round 1): ``n0`` with the zero-start law written in its initial rate
  (``start = "zero_rate"``, ``R = 12`` months), ``change(t) / 1 mm = v (tau / 12)(1 -
  r(t))``, so the hierarchy, the covariate and the anchored variants act on the
  elongation per year at the start (the law of 1.0 candidate ``m3``).
- ``n3`` (round 1): ``n0`` with persistent deviations in forecasting only: 0.8 of each
  series' residual variance follows an Ornstein-Uhlenbeck process with a 12-month
  correlation time, so a forecast carries the current deviation of its series forward;
  the population is fitted under working independence.

``REFERENCES`` names frozen earlier versions that a sealed run may score in the same run
with the same training (``"1.0.0"``: ``m4`` itself). Every candidate and reference reads
its series with the treatment family as the last context part. Each development round
adds a named candidate and never changes an earlier one. Nothing here reads files or
claims clinical validity.
"""

from __future__ import annotations

from collections.abc import Callable, Mapping
from dataclasses import replace
from typing import Any

from peykar.model_learning import eye_recovery_population5 as version5
from peykar.model_learning import myopia_population as version1
from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.validation.myopia_axial_data import AxialSeries

__all__ = [
    "DEVELOPMENT_CANDIDATES",
    "MODEL_ID",
    "REFERENCES",
    "adapter",
]

MODEL_ID = version1.MODEL_ID


def _version1() -> version5.RecoveryConfig:
    config = version1.DEVELOPMENT_CANDIDATES["m4"]
    if not isinstance(config, version5.RecoveryConfig):
        raise TypeError("Model 1.0.0 must be a generic eye model 5 configuration")
    return config


VERSION1 = _version1()

REFERENCES: dict[str, RecoveryConfig] = {
    # Model 1.0.0 as frozen by its protocol (protocol-1.0.0.json, candidate m4).
    "1.0.0": VERSION1,
}

DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # n0 (round 0): model 1.0.0 refitted on the 33 training records.
    "n0": replace(VERSION1, model_version="1.1.0-n0"),
    # n1 (round 1): group-mean noise not scaled by group size.
    "n1": replace(VERSION1, model_version="1.1.0-n1", max_noise_exponent=0.0),
    # n2 (round 1): the zero-start law in its initial rate per year.
    "n2": replace(VERSION1, model_version="1.1.0-n2", start="zero_rate"),
    # n3 (round 1): forecast-only persistent deviations (0.8, 12 months).
    "n3": replace(
        VERSION1,
        model_version="1.1.0-n3",
        deviation_fraction=0.8,
        deviation_months=12.0,
        deviation_in_population_fit=False,
    ),
}


def adapter(name: str) -> Callable[[Mapping[str, Any]], list[AxialSeries]]:
    """How the series of candidate or reference ``name`` are read from a record."""
    if name not in DEVELOPMENT_CANDIDATES and name not in REFERENCES:
        raise KeyError(name)
    return version1.to_axial_series_by_family
