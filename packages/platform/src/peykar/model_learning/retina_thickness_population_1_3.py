"""Central retinal thickness after treatment, model 1.3: candidates.

Model 1.3 is developed on the same 128 record studies of tranches 1 and 2 of dataset
``retina-cst-2026-10-28`` as model 1.2 (whose candidates were all rejected) and tested
once on tranche 3. The law, the series and the dose rule are those of model 1.1.0
(``retina_thickness_population_1_1``, candidate ``s5``, unchanged because its protocol
pins it).

Candidates:

- ``e0``: model 1.1.0 itself (the same configuration object and series); the reference.
- ``k1`` (round 1): ``e0`` with stacked forecasts (``eye_stacking`` 1.0.0). Forecasts
  from follow-up combine e0's primary (``population_assimilation``), persistence and the
  pooled-curve increment; initial states combine ``population_initial_state`` and the
  pooled training curve. The weights are fitted on e0's leave-one-study-out forecasts of
  human group means and then frozen; the population fit is e0's.

Each round of ``experiments/eye/retina/model-1.3/development-log.md`` adds named
candidates and never changes an earlier one. Nothing here reads files or claims
clinical validity.
"""

from __future__ import annotations

from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.model_learning.eye_stacking import StackedCandidate, StackingSpec
from peykar.model_learning.retina_thickness_population_1_1 import (
    DEVELOPMENT_CANDIDATES as CANDIDATES_1_1,
)

__all__ = ["DEVELOPMENT_CANDIDATES", "REFERENCE", "STACKED_CANDIDATES"]

REFERENCE = "e0"

DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # e0: the frozen model 1.1.0 (candidate s5), unchanged.
    "e0": CANDIDATES_1_1["s5"],
}

STACKED_CANDIDATES: dict[str, StackedCandidate] = {
    # k1 (round 1): e0 with stacked forecasts.
    "k1": StackedCandidate(
        base="e0",
        spec=StackingSpec(
            history_model="population_assimilation",
            initial_model="population_initial_state",
        ),
    ),
}
