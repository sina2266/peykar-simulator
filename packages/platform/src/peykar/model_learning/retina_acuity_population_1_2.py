"""Corrected visual acuity after treatment of macular disease, model 1.2: configurations.

Model 1.1.0 (``retina_acuity_population_1_1``, candidate ``w1``; unchanged because its
protocol pins it) failed its sealed test. After unsealing: without the 7-day transient,
the recovery term took its place (fitted time constants of 7.5-13 days), so its size in
the first weeks was again set by no data before each context's first training visit,
and anchored forecasts carried the error of early visits forward. Version 1.2 is
developed on the 53 record studies of tranches 1-2 and declares, in generic eye model
version 2 with the support hold of ``eye_support``:

- ``x0``: model 1.1.0 itself (the frozen ``w1`` configuration object), the reference.
- ``x1``: ``x0`` with the support hold: the law is held flat before the earliest visit of
  the training controls of the series' context (``eye_support.SUPPORT_RULE``). The fit is
  unchanged; early visits enter the likelihood, and early months are forecast, at the
  first supported month.
- ``x2``: ``x1`` with ``treatment_fallback``: a context without training series takes the
  pooled level of the trained contexts of its treatment before the species level.
- ``x3``: ``x1`` with wear-off of one dexamethasone implant, the rule of 1.1's ``w3``
  (retinal-thickness 1.1 dose rule, duration 3.5 months, width 0.5 months, 80% lost).

Each round of ``experiments/eye/retina-acuity/model-1.2/development-log.md`` adds a
named candidate and never changes an earlier one. Nothing here reads files or claims
clinical validity.
"""

from __future__ import annotations

from collections.abc import Callable, Mapping
from dataclasses import fields, replace
from typing import Any

from peykar.model_learning import retina_acuity_population_1_1 as version_1_1
from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.model_learning.eye_support import SupportConfig
from peykar.model_learning.retina_thickness_population_1_1 import WEAR_OFF_WIDTH_MONTHS
from peykar.validation.eye_retina_acuity_data import (
    RetinaAcuitySeries,
    to_retina_acuity_series,
)

__all__ = ["DEVELOPMENT_CANDIDATES", "REFERENCE", "adapter"]

REFERENCE = "x0"
_W1 = version_1_1.DEVELOPMENT_CANDIDATES["w1"]
_HELD = SupportConfig(**{f.name: getattr(_W1, f.name) for f in fields(_W1)}, support_hold=True)

DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # x0: model 1.1.0 (w1), unchanged.
    "x0": _W1,
    # x1: the support hold.
    "x1": replace(_HELD, model_version="1.2.0-x1"),
    # x2: the support hold and the treatment fallback.
    "x2": replace(_HELD, model_version="1.2.0-x2", treatment_fallback=True),
    # x3: the support hold and dexamethasone implant wear-off.
    "x3": replace(
        _HELD,
        model_version="1.2.0-x3",
        wear_off_loss=0.8,
        wear_off_width_months=WEAR_OFF_WIDTH_MONTHS,
    ),
}

_ADAPTERS: dict[str, Callable[[Mapping[str, Any]], list[RetinaAcuitySeries]]] = {
    "x3": version_1_1.to_retina_acuity_series_dose,
}


def adapter(name: str) -> Callable[[Mapping[str, Any]], list[RetinaAcuitySeries]]:
    """How the series of candidate ``name`` are read from a record."""
    if name not in DEVELOPMENT_CANDIDATES:
        raise KeyError(name)
    return _ADAPTERS.get(name, to_retina_acuity_series)
