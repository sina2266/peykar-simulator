"""Central retinal thickness after treatment, model 1.4: candidates.

Model 1.4 is developed on the 195 record studies of tranches 1-3 of dataset
``retina-cst-2026-10-28`` (the training data of model 1.3 plus the 67 tranche-3 records
consumed by its sealed test) and tested once on tranche 4. The law, the series and the
dose rule are those of model 1.1.0 (``retina_thickness_population_1_1``, candidate
``s5``, unchanged because its protocol pins it).

Candidates:

- ``e0``: model 1.1.0 itself (the same configuration object and series); the reference.
- ``k2`` (round 1): ``e0`` with stacked forecasts from follow-up (``eye_stacking2``
  2.0.0). Forecasts from follow-up combine e0's primary (``population_assimilation``),
  persistence and the pooled-curve increment; their interval is e0's, measured from its
  point, scaled by one factor and placed at the stacked point; the factor makes the
  pooled 80% coverage of the next-observation intervals closest to 0.80. Initial states
  stay e0's own (``population_initial_state``). The weights and the factor are fitted
  on e0's leave-one-study-out forecasts of human group means and then frozen; the
  population fit is e0's.
- ``k3`` (round 2): ``k2`` with one factor per forecast task: the next-observation
  factor is k2's, the landmark factor is chosen by the same rule on the 80% landmark
  intervals. Weights, points and next-observation intervals are k2's.

Each round of ``experiments/eye/retina/model-1.4/development-log.md`` adds named
candidates and never changes an earlier one. Nothing here reads files or claims
clinical validity.
"""

from __future__ import annotations

from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.model_learning.eye_stacking import StackingSpec
from peykar.model_learning.eye_stacking2 import HistoryStackedCandidate, HistoryStackingSpec
from peykar.model_learning.retina_thickness_population_1_1 import (
    DEVELOPMENT_CANDIDATES as CANDIDATES_1_1,
)

__all__ = ["DEVELOPMENT_CANDIDATES", "REFERENCE", "STACKED_CANDIDATES"]

REFERENCE = "e0"

DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # e0: the frozen model 1.1.0 (candidate s5), unchanged.
    "e0": CANDIDATES_1_1["s5"],
}

_MEMBERS = StackingSpec(
    history_model="population_assimilation",
    initial_model="population_initial_state",
)

STACKED_CANDIDATES: dict[str, HistoryStackedCandidate] = {
    # k2 (round 1): e0 with stacked forecasts from follow-up and a scaled interval.
    "k2": HistoryStackedCandidate(
        base="e0",
        spec=HistoryStackingSpec(
            members=_MEMBERS,
            calibration_task="next_observation",
            calibration_level="80",
            target_coverage="0.80",
            scale_grid=("0.30", "3.00", "0.01"),
        ),
    ),
    # k3 (round 2): k2 with one interval scale per forecast task.
    "k3": HistoryStackedCandidate(
        base="e0",
        spec=HistoryStackingSpec(
            members=_MEMBERS,
            calibration_task="next_observation",
            calibration_level="80",
            target_coverage="0.80",
            scale_grid=("0.30", "3.00", "0.01"),
            scale_tasks=("next_observation", "landmark"),
        ),
    ),
}
