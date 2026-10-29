"""Refraction after corneal laser refractive surgery, model 1.0: configurations.

The model is generic eye model version 2 (``eye_recovery_population2``) on the change
of the spherical equivalent from its preoperative value. The series of
``cornea_refraction_data`` hand the model the change as the pseudo-fraction
``exp(change / 5 D)`` and the preoperative SE as the pseudo-reference
``exp(SE0 / 5 D)``, so with ``covariate_reference = 1`` the law reads::

    change(t) / 5 D = ell + e r(t) + s q(t),   ell = context level + b SE0 / 5 D

with ``r = exp(-(t - t1)/tau)``, ``q = exp(-(t - t1)/7 days)`` and ``t1 = 1 day``:
the SE settles from its early postoperative offset towards a long-term change that
depends on the attempted correction through ``b`` (full correction is ``b = -1``).
Scores are in dioptres through ``eye_additive_evaluation`` (1 pp = 0.1 D).

Candidates:

- ``f0``: contexts ``species/procedure`` (surface ablation, LASIK, lenticule
  extraction, other), grouping ``("context",)``, a study effect per procedure arm,
  the generic constants otherwise.
- ``f1``: ``f0`` with a refractive-error level: contexts
  ``species/<refractive error>/procedure`` and grouping ``("treatment", "context")``,
  so the hierarchy is species → procedure → myopia, hyperopia or mixed astigmatism.

``REFRACTIVE_ERROR_CANDIDATES`` names the candidates whose series carry the
refractive-error sub-context. Each development round of
``experiments/eye/refraction/model-1.0/development-log.md`` adds a named candidate
and never changes an earlier one. Nothing here reads files or claims clinical
validity.
"""

from __future__ import annotations

from dataclasses import replace

from peykar.model_learning.eye_recovery_population2 import RecoveryConfig

MODEL_ID = "cornea.refraction-population"
REFERENCE = "f0"


def _config(version: str) -> RecoveryConfig:
    return RecoveryConfig(
        model_id=MODEL_ID,
        model_version=version,
        quantity="spherical equivalent change from the preoperative SE (pseudo-fraction "
        "exp(change / 5 D))",
        covariate_reference=1.0,
        covariate_unit="pseudo-reference exp(SE0 / 5 D), so the covariate is SE0 / 5 D",
        study_effects="per_group",
    )


DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # f0: one level per procedure context.
    "f0": _config("1.0.0-f0"),
    # f1: procedure -> refractive error.
    "f1": replace(_config("1.0.0-f1"), grouping=("treatment", "context")),
}

REFRACTIVE_ERROR_CANDIDATES: frozenset[str] = frozenset({"f1"})
