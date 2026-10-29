"""Corrected visual acuity after keratoplasty and cross-linking, model 1.0: configurations.

The model is generic eye model version 2 (``eye_recovery_population2``) on the change
of the corrected distance acuity from its preoperative value. The series of
``cornea_acuity_data`` hand the model the change as the pseudo-fraction
``exp(change / 1 logMAR)`` and the preoperative acuity as the pseudo-reference
``exp(logMAR0 / 1 logMAR)``, so with ``covariate_reference = 1`` the law reads::

    change(t) / 1 logMAR = ell + e r(t) + s q(t),   ell = context level + b logMAR0

with ``r = exp(-(t - t1)/tau)``, ``q = exp(-(t - t1)/7 days)`` and ``t1 = 1 day``: the
acuity moves from its early postoperative value towards a long-term change that
depends on the preoperative acuity through ``b`` (``b = -1`` would mean every eye
reaches logMAR 0). Scores are in ETDRS letters through ``eye_additive_evaluation``
(one reported pp is 0.02 logMAR).

Candidates:

- ``a0``: contexts ``human/procedure`` (pk, dalk, dsaek, dmek, cxl, other), grouping
  ``("context",)``, a study effect per procedure arm, the generic constants otherwise.
- ``a1``: ``a0`` with an indication level: contexts ``human/<indication>/procedure``
  and grouping ``("treatment", "context")``, so the hierarchy is species → procedure
  → indication.
- ``a2``: ``a0`` with one hierarchy per operation family instead of one for every
  operation: contexts ``human-graft/procedure`` for keratoplasty and
  ``human-crosslinking/procedure`` for cross-linking (``other`` follows its
  indication: keratoconus → cross-linking), so each family has its own level slope
  ``b``, noise centres and population variances.
- ``a4`` (round 2): ``a0`` with persistent deviations from the law in forecasting
  only (fraction 0.8 of the residual variance, correlation time 12 months), so a
  series that is worse or better than the law at its last follow-up is expected to
  stay so.

``INDICATION_CANDIDATES`` and ``FAMILY_CANDIDATES`` name the candidates whose series
carry those contexts (``adapter``). Each development round of
``experiments/eye/acuity/model-1.0/development-log.md`` adds a named candidate and
never changes an earlier one. Nothing here reads files or claims clinical validity.
"""

from __future__ import annotations

from collections.abc import Callable, Mapping
from dataclasses import replace
from typing import Any

from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.validation.cornea_acuity_data import (
    AcuitySeries,
    to_acuity_series,
    to_acuity_series_by_indication,
)

__all__ = [
    "DEVELOPMENT_CANDIDATES",
    "FAMILY_CANDIDATES",
    "INDICATION_CANDIDATES",
    "adapter",
    "family_context",
    "operation_family",
    "to_acuity_series_by_family",
]

MODEL_ID = "cornea.acuity-population"
REFERENCE = "a0"
GRAFT_PROCEDURES = frozenset({"pk", "dalk", "dsaek", "dmek"})


def _config(version: str) -> RecoveryConfig:
    return RecoveryConfig(
        model_id=MODEL_ID,
        model_version=version,
        quantity="corrected distance visual acuity change from the preoperative logMAR "
        "(pseudo-fraction exp(change / 1 logMAR))",
        covariate_reference=1.0,
        covariate_unit="pseudo-reference exp(logMAR0 / 1 logMAR), so the covariate is logMAR0",
        study_effects="per_group",
    )


DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # a0: one level per procedure context.
    "a0": _config("1.0.0-a0"),
    # a1: procedure -> indication.
    "a1": replace(_config("1.0.0-a1"), grouping=("treatment", "context")),
    # a2: one hierarchy per operation family (keratoplasty, cross-linking).
    "a2": _config("1.0.0-a2"),
    # a4 (round 2): a0 with persistent (Ornstein-Uhlenbeck) deviations from the law in
    # forecasting only, as retina model 1.0: 80% of the residual variance, 12-month
    # correlation time; the population is fitted as in a0.
    "a4": replace(
        _config("1.0.0-a4"),
        deviation_fraction=0.8,
        deviation_months=12.0,
        deviation_in_population_fit=False,
    ),
}

INDICATION_CANDIDATES: frozenset[str] = frozenset({"a1"})
FAMILY_CANDIDATES: frozenset[str] = frozenset({"a2"})


def operation_family(procedure: str, indication: str) -> str:
    """``graft`` for keratoplasty, ``crosslinking`` for cross-linking; ``other`` follows
    its indication (keratoconus → cross-linking, anything else → graft)."""
    if procedure in GRAFT_PROCEDURES:
        return "graft"
    if procedure == "cxl":
        return "crosslinking"
    return "crosslinking" if indication == "keratoconus" else "graft"


def family_context(context: str, indication: str) -> str:
    """``human/procedure`` → ``human-<family>/procedure``."""
    species, procedure = context.split("/")
    return f"{species}-{operation_family(procedure, indication)}/{procedure}"


def to_acuity_series_by_family(record: Mapping[str, Any]) -> list[AcuitySeries]:
    """The series with one species part per operation family (separate hierarchies)."""
    return [
        replace(item, context=family_context(item.context, item.indication))
        for item in to_acuity_series(record)
    ]


def adapter(name: str) -> Callable[[Mapping[str, Any]], list[AcuitySeries]]:
    """How the series of candidate ``name`` are read from a record."""
    if name in INDICATION_CANDIDATES:
        return to_acuity_series_by_indication
    if name in FAMILY_CANDIDATES:
        return to_acuity_series_by_family
    return to_acuity_series
