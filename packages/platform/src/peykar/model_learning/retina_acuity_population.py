"""Corrected visual acuity after treatment of macular disease, model 1.0: configurations.

The model is generic eye model version 2 (``eye_recovery_population2``) on the change
of the best-corrected visual acuity from its pretreatment value, exactly as the corneal
acuity component (``acuity_population``): the series of ``eye_retina_acuity_data`` hand
the model the change as the pseudo-fraction ``exp(change / 1 logMAR)`` and the
pretreatment acuity as the pseudo-reference ``exp(logMAR0 / 1 logMAR)``, so with
``covariate_reference = 1`` the law reads::

    change(t) / 1 logMAR = ell + e r(t) + s q(t),   ell = context level + b logMAR0

with ``r = exp(-(t - t1)/tau)``, ``q = exp(-(t - t1)/7 days)`` and ``t1 = 1 day``.
Contexts are ``human/<indication>/<treatment>`` (indication and treatment vocabularies
of the retinal-thickness component). Scores are in ETDRS letters through
``eye_additive_evaluation`` (one reported pp is 0.02 logMAR).

Candidates:

- ``v0``: the corneal acuity model 1.0.0 settings (one level per context, grouping
  ``("context",)``, a study effect per context arm); a context without training
  studies falls back to the species level.
- ``v1``: ``v0`` with contexts nested in their treatment (grouping
  ``("treatment", "context")``), so a new indication of a known treatment borrows that
  treatment's level, amplitude and time constant.

Each development round of ``experiments/eye/retina-acuity/model-1.0/development-log.md``
adds a named candidate and never changes an earlier one. Nothing here reads files or
claims clinical validity.
"""

from __future__ import annotations

from collections.abc import Callable, Mapping
from dataclasses import replace
from typing import Any

from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.validation.eye_retina_acuity_data import (
    RetinaAcuitySeries,
    to_retina_acuity_series,
)

__all__ = ["DEVELOPMENT_CANDIDATES", "MODEL_ID", "REFERENCE", "adapter"]

MODEL_ID = "eye.retina-acuity-population"
REFERENCE = "v0"


def _config(version: str) -> RecoveryConfig:
    return RecoveryConfig(
        model_id=MODEL_ID,
        model_version=version,
        quantity="best-corrected visual acuity change from the pretreatment logMAR "
        "(pseudo-fraction exp(change / 1 logMAR))",
        covariate_reference=1.0,
        covariate_unit="pseudo-reference exp(logMAR0 / 1 logMAR), so the covariate is logMAR0",
        study_effects="per_group",
    )


DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # v0: one level per context (the corneal acuity 1.0.0 settings).
    "v0": _config("1.0.0-v0"),
    # v1: contexts nested in their treatment.
    "v1": replace(_config("1.0.0-v1"), grouping=("treatment", "context")),
}


def adapter(name: str) -> Callable[[Mapping[str, Any]], list[RetinaAcuitySeries]]:
    """How the series of candidate ``name`` are read from a record."""
    if name not in DEVELOPMENT_CANDIDATES:
        raise KeyError(name)
    return to_retina_acuity_series
