"""Corrected visual acuity after keratoplasty and cross-linking, model 1.1: configurations.

Model 1.1 is developed on the 42 tranche-1 records of dataset
``cornea-acuity-2026-10-29`` (27 development records and the 15 sealed records
consumed by the one-shot test of model 1.0.0) and tested once on tranche 2. The law,
the scale and the series are those of model 1.0 (``acuity_population``): the change of
the corrected distance acuity from its preoperative logMAR enters generic eye model
version 2 as the pseudo-fraction ``exp(change / 1 logMAR)``.

Candidates:

- ``a0``: the frozen model 1.0.0 configuration itself (contexts ``human/procedure``),
  unchanged; it is the reference every 1.1 candidate must beat.
- ``b1`` (round 1): ``a0`` in generic model 4 with correlated random effects
  (``random_correlation = "level_excess"``): the study and series levels share one
  fitted correlation between the long-term level and the transient amplitude, so the
  data decide how much of an early shortfall persists.
- ``b2`` (round 1): ``a0`` in generic model 4 with one level slope on the preoperative
  acuity for cross-linking (treatments ``cxl`` and ``other``; every ``other`` group of
  this dataset is cross-linking combined with a ring segment or an ablation for
  keratoconus) and one for keratoplasty, inside the single hierarchy of ``a0``.

Each round of ``experiments/eye/acuity/model-1.1/development-log.md`` adds named
candidates and never changes an earlier one. Nothing here reads files or claims
clinical validity.
"""

from __future__ import annotations

from collections.abc import Callable, Mapping
from typing import Any

from peykar.model_learning import acuity_population as version_1_0
from peykar.model_learning import eye_recovery_population4 as version4
from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.validation.cornea_acuity_data import AcuitySeries

__all__ = ["CROSSLINKING_SLOPE", "DEVELOPMENT_CANDIDATES", "REFERENCE", "adapter"]

REFERENCE = "a0"
CROSSLINKING_SLOPE: tuple[tuple[str, tuple[str, ...]], ...] = (("crosslinking", ("cxl", "other")),)


def _version4(version: str, **options: Any) -> version4.RecoveryConfig:
    """``a0``'s configuration in generic model 4 with ``options``."""
    a0 = version_1_0.DEVELOPMENT_CANDIDATES["a0"]
    fields = {name: getattr(a0, name) for name in a0.__dataclass_fields__}
    return version4.RecoveryConfig(**{**fields, "model_version": version, **options})


DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # a0: model 1.0.0 as frozen (the reference of every 1.1 comparison).
    "a0": version_1_0.DEVELOPMENT_CANDIDATES["a0"],
    # b1 (round 1): correlated level and transient amplitude at the study/series levels.
    "b1": _version4("1.1.0-b1", random_correlation="level_excess"),
    # b2 (round 1): one level slope for cross-linking, one for keratoplasty.
    "b2": _version4("1.1.0-b2", slope_groups=CROSSLINKING_SLOPE),
}


def adapter(name: str) -> Callable[[Mapping[str, Any]], list[AcuitySeries]]:
    """How the series of candidate ``name`` are read from a record (every 1.1
    candidate reads the series of ``a0``: contexts ``human/procedure``)."""
    if name not in DEVELOPMENT_CANDIDATES:
        raise KeyError(name)
    return version_1_0.adapter("a0")
