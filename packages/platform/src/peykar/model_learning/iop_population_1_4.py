"""IOP after glaucoma procedures, model 1.4: configurations of generic eye model 2.

Model 1.4 is developed on the 308 record studies of tranches 1-4 of dataset
``iop-glaucoma-surgery-2026-10-28`` (the training data of model 1.3 plus the 79
tranche-4 records consumed by its sealed test) and tested once on tranche 5. Model
1.2.0 (``iop_population_1_2``, candidate ``c1``; unchanged because its protocol pins it)
is the component of ``peykar eye`` and the starting point: it passed its own sealed test
on tranche 3 and held H1-H4 as the reference of the 1.3 test on tranche 4, where the
sub-procedure level of model 1.3 failed its comparison with it.

Candidates:

- ``d0`` is model 1.2.0 itself (the same configuration object and series, contexts
  ``species/procedure``); it is the reference.

``SERIES_READERS`` maps a candidate to the name of the series reader it uses when that
is not model 1.2's (none yet; the operator resolves the names); ``STACKED_CANDIDATES``
names candidates that stack the forecasts of a population candidate (none yet). Each
development round of ``experiments/eye/iop/model-1.4/development-log.md`` adds named
candidates and never changes an earlier one. Nothing here reads files or claims
clinical validity.
"""

from __future__ import annotations

from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.model_learning.eye_stacking2 import HistoryStackedCandidate
from peykar.model_learning.iop_population_1_2 import DEVELOPMENT_CANDIDATES as CANDIDATES_1_2

__all__ = [
    "DEVELOPMENT_CANDIDATES",
    "MODEL_ID",
    "REFERENCE",
    "SERIES_READERS",
    "STACKED_CANDIDATES",
]

MODEL_ID = CANDIDATES_1_2["c1"].model_id
REFERENCE = "d0"

DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # d0: the frozen model 1.2.0 (candidate c1), unchanged.
    "d0": CANDIDATES_1_2["c1"],
}

SERIES_READERS: dict[str, str] = {}

STACKED_CANDIDATES: dict[str, HistoryStackedCandidate] = {}
