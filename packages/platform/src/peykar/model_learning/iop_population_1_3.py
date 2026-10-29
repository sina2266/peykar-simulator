"""IOP after glaucoma procedures, model 1.3: configurations of generic eye model 2.

Model 1.2.0 (``iop_population_1_2``, candidate ``c1``; unchanged because its protocol
pins it) is the starting point. Its out-of-sample errors on the 229 training studies
(tranches 1–3) differ by device or technique within one procedure context: after
cyclodestruction the population prior predicted too low a pressure for micropulse
laser and too high a pressure for continuous-wave, endoscopic and ultrasound
treatment; after subconjunctival MIGS too high a pressure for PreserFlo; after angle
MIGS too low a pressure in the first month for stents and goniotomy.

Candidates:

- ``d0`` is model 1.2.0 itself (the same configuration object, contexts
  ``species/procedure``); it is the reference.
- ``d1`` adds a sub-procedure level: contexts ``species/<sub>/procedure`` from the
  declared keyword rules of ``eye_iop_data_1_3`` and grouping
  ``("treatment", "context")``, so the hierarchy is species → procedure →
  sub-procedure. Everything else equals ``c1``.
- ``d2`` is ``c1`` with generic version 3's point rule ``lognormal_mean``: the
  point forecast is the mean of the lognormal matched to the predictive median and
  5%/95% quantiles instead of the median (the initial-state forecasts of 1.2 were
  1.6 pp too low on average, and RMSE is minimized by the mean). Its series are
  1.2's.

``SUB_PROCEDURE_CANDIDATES`` names the candidates whose series carry sub-procedure
contexts. Each development round of ``experiments/eye/iop/model-1.3/development-log.md``
adds a named candidate and never changes an earlier one. Nothing here reads files or
claims clinical validity.
"""

from __future__ import annotations

from dataclasses import replace

from peykar.model_learning import eye_recovery_population3 as version3
from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.model_learning.iop_population_1_2 import DEVELOPMENT_CANDIDATES as CANDIDATES_1_2

MODEL_ID = CANDIDATES_1_2["c1"].model_id
REFERENCE = "d0"

DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # d0: the frozen model 1.2.0 (candidate c1), unchanged.
    "d0": CANDIDATES_1_2["c1"],
    # d1: species -> procedure -> sub-procedure hierarchy; otherwise c1.
    "d1": replace(
        CANDIDATES_1_2["c1"], model_version="1.3.0-d1", grouping=("treatment", "context")
    ),
    # d2: c1 with the lognormal-mean point forecast (generic version 3).
    "d2": version3.RecoveryConfig(
        **{
            **{
                name: getattr(CANDIDATES_1_2["c1"], name)
                for name in CANDIDATES_1_2["c1"].__dataclass_fields__
            },
            "model_version": "1.3.0-d2",
            "point": "lognormal_mean",
        }
    ),
}

SUB_PROCEDURE_CANDIDATES: frozenset[str] = frozenset({"d1"})
