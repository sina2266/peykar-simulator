"""Training time support of a population law and the support hold (generic, optional).

A population law is identified only where its training visits lie. Two sealed tests
(retinal acuity 1.0 and 1.1) failed on visits before the first training visit of a
context: an early term that no data constrained was read as real. The support hold
removes that extrapolation without changing any fit:

* the support of a context is the earliest visit month of the training control series
  of that context (the series that fit the population); a context without training
  controls uses the earliest visit of its species' controls;
* the law is held flat before that month, ``f(t) = f(max(t, t_support))``, both in the
  likelihood of a series' own visits and in every forecast; an early visit therefore
  counts as an observation of the curve at the first supported month.

Training controls never have a visit before the support of their own context, so the
population fit is the same with or without the hold; only forecasts and initial states
that involve earlier months change. ``SupportConfig`` is a version-2 configuration
with ``support_hold``; with ``support_hold = False`` it behaves exactly as version 2.
"""

from __future__ import annotations

from collections.abc import Mapping, Sequence
from dataclasses import dataclass
from typing import Any

from peykar.model_learning import eye_recovery_population2 as version2
from peykar_core.errors import InvalidRequest

__all__ = ["SupportConfig", "hold_month", "support_months"]

SUPPORT_RULE = (
    "law held flat before the earliest visit month of the training control series of the "
    "series' context (species' controls for an untrained context): f(t) = f(max(t, t_s)), "
    "in likelihoods and forecasts"
)


@dataclass(frozen=True)
class SupportConfig(version2.RecoveryConfig):
    """Generic eye model 2 configuration with the optional support hold."""

    support_hold: bool = True

    def __post_init__(self) -> None:
        super().__post_init__()
        if self.support_hold and (self.deviation_fraction > 0 or self.early_noise is not None):
            # Held early visits share a month; correlated deviations and early noise
            # are defined on distinct months.
            raise InvalidRequest("The support hold needs no deviations and no early noise")

    def describe(self) -> dict[str, Any]:
        return {
            **super().describe(),
            "support_hold": self.support_hold,
            "support_rule": SUPPORT_RULE if self.support_hold else None,
        }


def _species(context: str) -> str:
    return context.split("/", 1)[0]


def support_months(training: Sequence[Any]) -> dict[str, Any]:
    """Earliest visit month of the training controls that fit the population, per
    context and per species (the controls ``fit_population`` uses: role control and a
    positive baseline)."""
    contexts: dict[str, float] = {}
    species: dict[str, float] = {}
    for item in training:
        if item.role != "control" or not item.reference_value > 0 or not item.months:
            continue
        first = float(min(item.months))
        contexts[item.context] = min(contexts.get(item.context, first), first)
        key = _species(item.context)
        species[key] = min(species.get(key, first), first)
    return {"contexts": contexts, "species": species}


def hold_month(support: Mapping[str, Any], context: str) -> float | None:
    """The month before which the law is held for a series of ``context`` (``None``:
    no training control of its species)."""
    own = support["contexts"].get(context)
    if own is not None:
        return float(own)
    value = support["species"].get(_species(context))
    return None if value is None else float(value)
