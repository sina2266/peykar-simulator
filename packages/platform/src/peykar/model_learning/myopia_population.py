"""Axial length in childhood myopia, treated or not, model 1.0: configurations.

The model is generic eye model version 2 (``eye_recovery_population2``) on the change of
the ocular axial length (AL) from its baseline value. The series of
``myopia_axial_data`` hand the model the change as the pseudo-fraction
``exp(change / 1 mm)`` and the baseline axial length as the pseudo-reference
``exp(AL0 / 1 mm)``. With ``covariate_reference = exp(24.5)`` the level covariate is
centred at a baseline axial length of 24.5 mm, so the law reads::

    change(t) / 1 mm = ell + e r(t),   ell = context level + b (AL0 - 24.5 mm) / 1 mm

with ``r = exp(-(t - t1)/tau)`` and ``t1 = 1 day``: the eye elongates from its baseline
towards a long-term change along one exponential, so with a long ``tau`` the growth is
nearly linear over a few years and slows later. Scores are in hundredths of a millimetre
through ``eye_additive_evaluation4`` (one reported pp is 0.01 mm).

The base candidate carries the two lessons of the Kmax model 1.0 development, decided
before any myopia run: no 7-day early transient (no development series has a visit
before month 1, so the term would be read from no data) and forecast-only pooled noise
(the median of the studies' residual SDs gave too narrow intervals when most studies
have few residual degrees of freedom).

Candidates (rounds of ``experiments/eye/myopia/model-1.0/development-log.md``):

- ``m0``: contexts ``human/treatment`` (single_vision, atropine, orthokeratology,
  defocus_spectacles, multifocal_contacts, red_light, combination, outdoor, other),
  grouping ``("context",)``, a study effect per treatment arm, no early transient,
  forecast-only pooled noise, the generic constants otherwise.
- ``m1``: ``m0`` with a treatment-family level: contexts ``human/<treatment>/<family>``
  and grouping ``("treatment", "context")``, so the hierarchy is species → family
  (uncorrected or single-vision control, optical defocus, atropine, light) → treatment;
  an untrained optical treatment uses the optical family instead of the species mean.
- ``m2``: ``m0`` with the median noise centre in forecasts too (no forecast pooling).

``FAMILY_CANDIDATES`` names the candidates whose series carry the family part
(``adapter``). Each development round adds a named candidate and never changes an
earlier one. Nothing here reads files or claims clinical validity.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Mapping
from dataclasses import replace
from typing import Any

from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.validation.myopia_axial_data import COVARIATE_UNIT_MM, AxialSeries, to_axial_series

__all__ = [
    "CENTRE_MM",
    "DEVELOPMENT_CANDIDATES",
    "FAMILIES",
    "FAMILY_CANDIDATES",
    "adapter",
    "family_context",
    "to_axial_series_by_family",
]

MODEL_ID = "eye.myopia-axial-population"
REFERENCE = "m0"
# Baseline axial length at which the level covariate is zero.
CENTRE_MM = 24.5
FAMILIES: dict[str, str] = {
    "single_vision": "control",
    # Uncorrected eyes, observational cohorts and drugs that are not a listed treatment.
    "other": "control",
    "orthokeratology": "optical",
    "defocus_spectacles": "optical",
    "multifocal_contacts": "optical",
    "atropine": "atropine",
    # Combinations in this dataset are atropine with an optical treatment.
    "combination": "atropine",
    "red_light": "light",
    "outdoor": "light",
}


def _fields(version: str) -> dict[str, Any]:
    return {
        "model_id": MODEL_ID,
        "model_version": version,
        "quantity": "axial length change from the baseline axial length (pseudo-fraction "
        "exp(change / 1 mm))",
        "covariate_reference": math.exp(CENTRE_MM / COVARIATE_UNIT_MM),
        "covariate_unit": "pseudo-reference exp(AL0 / 1 mm), so the covariate is "
        "(AL0 - 24.5 mm) / 1 mm",
        "study_effects": "per_group",
        "transient_months": None,
        "penalty_modes": (0.1, 0.1, 0.5),
        "robust_sd": (0.3, 0.3, 2.0),
    }


def _config(version: str) -> RecoveryConfig:
    return RecoveryConfig(**_fields(version), forecast_noise_pooling="pooled")


DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # m0: one level per treatment context, no transient, forecast-only pooled noise.
    "m0": _config("1.0.0-m0"),
    # m1: species -> treatment family -> treatment.
    "m1": replace(_config("1.0.0-m1"), grouping=("treatment", "context")),
    # m2: m0 with the median noise centre in forecasts too.
    "m2": replace(_config("1.0.0-m2"), forecast_noise_pooling=None),
}

FAMILY_CANDIDATES: frozenset[str] = frozenset({"m1"})


def family_context(context: str) -> str:
    """``human/treatment`` → ``human/treatment/<family>``."""
    species, treatment = context.split("/")
    return f"{species}/{treatment}/{FAMILIES[treatment]}"


def to_axial_series_by_family(record: Mapping[str, Any]) -> list[AxialSeries]:
    """The series with the treatment family as the last context part."""
    return [replace(item, context=family_context(item.context)) for item in to_axial_series(record)]


def adapter(name: str) -> Callable[[Mapping[str, Any]], list[AxialSeries]]:
    """How the series of candidate ``name`` are read from a record."""
    if name in FAMILY_CANDIDATES:
        return to_axial_series_by_family
    return to_axial_series
