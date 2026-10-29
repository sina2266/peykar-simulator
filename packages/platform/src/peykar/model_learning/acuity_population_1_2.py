"""Corrected visual acuity after keratoplasty and cross-linking, model 1.2: candidates.

Model 1.2 is developed on the same 42 tranche-1 records as model 1.1 (dataset
``cornea-acuity-2026-10-29``: 27 development records and the 15 sealed records consumed
by the one-shot test of model 1.0.0) and tested once on tranche 2. The law, the scale
and the series are those of model 1.0 (``acuity_population``).

Candidates:

- ``a0``: the frozen model 1.0.0 configuration itself, unchanged; it is the reference.
- ``k1`` (round 1): ``a0`` with stacked forecasts (``eye_stacking`` 1.0.0). Forecasts
  from follow-up combine a0's primary (``population_assimilation_robust``), persistence
  and the pooled-curve increment; initial states combine ``population_initial_state``
  and the pooled training curve. The weights are fitted on a0's leave-one-study-out
  forecasts of human group means and then frozen; the population fit is a0's.

Each round of ``experiments/eye/acuity/model-1.2/development-log.md`` adds named
candidates and never changes an earlier one. Nothing here reads files or claims
clinical validity.
"""

from __future__ import annotations

from collections.abc import Callable, Mapping
from typing import Any

from peykar.model_learning import acuity_population as version_1_0
from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.model_learning.eye_stacking import StackedCandidate, StackingSpec
from peykar.validation.cornea_acuity_data import AcuitySeries

__all__ = ["DEVELOPMENT_CANDIDATES", "REFERENCE", "STACKED_CANDIDATES", "adapter"]

REFERENCE = "a0"

DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # a0: model 1.0.0 as frozen (the reference of every 1.2 comparison).
    "a0": version_1_0.DEVELOPMENT_CANDIDATES["a0"],
}

STACKED_CANDIDATES: dict[str, StackedCandidate] = {
    # k1 (round 1): a0 with stacked forecasts.
    "k1": StackedCandidate(
        base="a0",
        spec=StackingSpec(
            history_model="population_assimilation_robust",
            initial_model="population_initial_state",
        ),
    ),
}


def adapter(name: str) -> Callable[[Mapping[str, Any]], list[AcuitySeries]]:
    """How the series of candidate ``name`` are read (every candidate reads a0's)."""
    if name not in DEVELOPMENT_CANDIDATES and name not in STACKED_CANDIDATES:
        raise KeyError(name)
    return version_1_0.adapter("a0")
