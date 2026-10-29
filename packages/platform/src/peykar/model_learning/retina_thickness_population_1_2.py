"""Central retinal thickness after treatment, model 1.2: configurations.

Model 1.1.0 (``retina_thickness_population_1_1``, candidate ``s5``; unchanged because
its protocol pins it) is the starting point. Its out-of-sample initial-state forecasts
on the 128 training studies of tranches 1 and 2 (its development LOSO on tranche 1
plus its consumed sealed report on tranche 2) were 3.8 pp too low on average (too much
predicted thinning), while next-observation forecasts were unbiased.

Candidates:

- ``e0`` is model 1.1.0 itself (the same configuration object and series; the dose
  rule of ``retina_thickness_population_1_1``); it is the reference.
- ``e1`` is ``s5`` on generic eye model version 3 with the point rule
  ``lognormal_mean``: every population point forecast is the mean of the lognormal
  matched to the predictive median and 5%/95% quantiles, instead of the median.
- ``e3`` is ``s5`` with a prior-treatment level: contexts
  ``species/<prior>/indication|treatment`` (``eye_retina_data_1_2``) and grouping
  ``("treatment", "context")``, so the hierarchy is species → indication and
  treatment → prior treatment (naive, previously treated, mixed or unstated). The
  out-of-sample initial-state bias of 1.1 was −3.1 pp for naive, −5.5 for previously
  treated, −8.8 for unstated and −0.1 pp for mixed groups.

- ``e4`` is ``s5`` with an OCT-device level in the same place (contexts
  ``species/<device>/indication|treatment``). In a least-squares fit of the last
  fraction at or after month 3 on the baseline covariate, indication, measure and
  device (139 training control group means), Spectralis series ended 11.0 pp (SE
  3.1) higher than Cirrus series; out of sample the initial state of 1.1 was 7.2 pp
  too low for Spectralis series against about 2 pp for the others.

``PRIOR_TREATMENT_CANDIDATES`` and ``DEVICE_CANDIDATES`` name the candidates whose
series carry those sub-contexts.

Each development round of ``experiments/eye/retina/model-1.2/development-log.md`` adds
a named candidate and never changes an earlier one. Nothing here reads files or claims
clinical validity.
"""

from __future__ import annotations

from dataclasses import replace

from peykar.model_learning import eye_recovery_population3 as version3
from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.model_learning.retina_thickness_population_1_1 import (
    DEVELOPMENT_CANDIDATES as CANDIDATES_1_1,
)

MODEL_ID = CANDIDATES_1_1["s5"].model_id
REFERENCE = "e0"
_S5 = CANDIDATES_1_1["s5"]

DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # e0: the frozen model 1.1.0 (candidate s5), unchanged.
    "e0": _S5,
    # e1: s5 with the lognormal-mean point forecast (generic version 3).
    "e1": version3.RecoveryConfig(
        **{
            **{name: getattr(_S5, name) for name in _S5.__dataclass_fields__},
            "model_version": "1.2.0-e1",
            "point": "lognormal_mean",
        }
    ),
    # e3: s5 with a prior-treatment level (species -> indication|treatment -> prior).
    "e3": replace(_S5, model_version="1.2.0-e3", grouping=("treatment", "context")),
    # e4: s5 with an OCT-device level (species -> indication|treatment -> device).
    "e4": replace(_S5, model_version="1.2.0-e4", grouping=("treatment", "context")),
}

PRIOR_TREATMENT_CANDIDATES: frozenset[str] = frozenset({"e3"})
DEVICE_CANDIDATES: frozenset[str] = frozenset({"e4"})
