"""Corrected visual acuity after treatment of macular disease, model 1.1: configurations.

Model 1.0.0 (``retina_acuity_population``, candidate ``v3``; unchanged because its
protocol pins it) failed its sealed test. After unsealing, the failure traced to the
law's early transient ``s q(t)`` (a 7-day time constant), which no training visit
before day 30 constrained, read through the day-1 visit of one sealed study whose
contexts had no training study; a dexamethasone implant's waning gain was also missed.
Version 1.1 declares one change per cause, all in generic eye model version 2
(``eye_recovery_population2``):

- ``w0``: model 1.0.0 itself (the frozen ``v3`` configuration object), the reference.
- ``w1``: ``w0`` without the early transient (``transient_months = None``), so the law
  is ``change(t) / 1 logMAR = ell + e r(t)`` with ``r = exp(-(t - 1 day)/tau)``: the
  early course is the extrapolation of the recovery term that the visits from month 1
  on identify. The penalty modes and robust SDs drop the transient's entry.
- ``w2``: ``w0`` with ``treatment_fallback``: a context without training series takes
  the pooled level of the trained contexts of its treatment before the species level.
- ``w3``: ``w0`` with wear-off of one dexamethasone implant: the declared dose rule of
  retinal-thickness model 1.1 (``retina_thickness_population_1_1.effect_duration_months``,
  unchanged; duration 3.5 months, logistic width 0.5 months) and 80% of the treatment
  effect lost once the implant has worn off, the retinal-thickness 1.1.0 setting.

Each round of ``experiments/eye/retina-acuity/model-1.1/development-log.md`` adds a
named candidate and never changes an earlier one. Nothing here reads files or claims
clinical validity.
"""

from __future__ import annotations

from collections.abc import Callable, Mapping
from dataclasses import replace
from typing import Any

from peykar.model_learning import retina_acuity_population as version_1_0
from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.model_learning.retina_thickness_population_1_1 import (
    WEAR_OFF_WIDTH_MONTHS,
    effect_duration_months,
)
from peykar.validation.eye_retina_acuity_data import (
    RetinaAcuitySeries,
    to_retina_acuity_series,
)

__all__ = [
    "DEVELOPMENT_CANDIDATES",
    "REFERENCE",
    "adapter",
    "to_retina_acuity_series_dose",
]

REFERENCE = "w0"
_V3 = version_1_0.DEVELOPMENT_CANDIDATES["v3"]


def to_retina_acuity_series_dose(record: Mapping[str, Any]) -> list[RetinaAcuitySeries]:
    """The model 1.0 series with each group's declared single-dose effect duration."""
    groups = {group["group"]: group for group in record["groups"]}
    result = []
    for item in to_retina_acuity_series(record):
        group = groups[item.group] if item.group is not None else {}
        duration = effect_duration_months(item.treatment, item.regimen, group.get("agent"))
        result.append(replace(item, metadata={**item.metadata, "effect_duration_months": duration}))
    return result


DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # w0: model 1.0.0 (v3), unchanged.
    "w0": _V3,
    # w1: no early transient; the law is ell + e r(t).
    "w1": replace(
        _V3,
        model_version="1.1.0-w1",
        transient_months=None,
        penalty_modes=(0.1, 0.1, 0.5),
        robust_sd=(0.3, 0.3, 2.0),
    ),
    # w2: untrained contexts borrow the trained contexts of their treatment.
    "w2": replace(_V3, model_version="1.1.0-w2", treatment_fallback=True),
    # w3: one dexamethasone implant loses 80% of its effect as it wears off.
    "w3": replace(
        _V3,
        model_version="1.1.0-w3",
        wear_off_loss=0.8,
        wear_off_width_months=WEAR_OFF_WIDTH_MONTHS,
    ),
}

_ADAPTERS: dict[str, Callable[[Mapping[str, Any]], list[RetinaAcuitySeries]]] = {
    "w3": to_retina_acuity_series_dose,
}


def adapter(name: str) -> Callable[[Mapping[str, Any]], list[RetinaAcuitySeries]]:
    """How the series of candidate ``name`` are read from a record."""
    if name not in DEVELOPMENT_CANDIDATES:
        raise KeyError(name)
    return _ADAPTERS.get(name, to_retina_acuity_series)
