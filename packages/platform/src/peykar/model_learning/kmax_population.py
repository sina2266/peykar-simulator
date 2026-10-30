"""Maximum keratometry after cross-linking or ring segments, model 1.0: configurations.

The model is generic eye model version 2 (``eye_recovery_population2``) on the change of
the maximum keratometry (Kmax) from its baseline value. The series of
``cornea_kmax_data`` hand the model the change as the pseudo-fraction
``exp(change / 5 D)`` and the baseline Kmax as the pseudo-reference ``exp(Kmax0 / 5 D)``.
With ``covariate_reference = exp(55 D / 5 D)`` the level covariate is centred at a
baseline Kmax of 55 D, so the law reads::

    change(t) / 5 D = ell + e r(t) + s q(t),   ell = context level + b (Kmax0 - 55 D) / 5 D

with ``r = exp(-(t - t1)/tau)``, ``q = exp(-(t - t1)/7 days)`` and ``t1 = 1 day``: Kmax
moves from its early post-treatment change towards a long-term change (negative is
flattening) that may depend on the baseline Kmax through ``b``. Scores are in tenths of
a dioptre through ``eye_additive_evaluation4`` (one reported pp is 0.1 D).

Candidates (rounds of ``experiments/eye/kmax/model-1.0/development-log.md``):

- ``k0``: contexts ``human/procedure`` (cxl_epi_off, cxl_accelerated, cxl_epi_on,
  cxl_combined, icrs, observation, other), grouping ``("context",)``, a study effect per
  procedure arm, the generic constants of the acuity and refraction models otherwise.
- ``k1``: ``k0`` without the 7-day early transient (``transient_months = None``), as
  retinal acuity 1.1 adopted: only one development series has a visit before month 1.
- ``k2``: ``k0`` with a treatment-family level: contexts ``human/<procedure>/<family>``
  and grouping ``("treatment", "context")``, so the hierarchy is species → family
  (cross-linking, ring segments, observation) → procedure; an untrained cross-linking
  protocol uses the cross-linking family instead of the species mean.
- ``k3``: ``k0`` with the support hold of ``eye_support`` (the law is held flat before
  the earliest training visit of a context).
- ``k4``: ``k0`` with persistent deviations from the law in forecasting only (fraction
  0.8 of the residual variance, correlation time 12 months), as retina model 1.0.
- ``k5``: ``k0`` with a one-month instead of a 7-day early term (``transient_months =
  1``), for the steepening in the first month after cross-linking that the monotone
  recovery term cannot follow together with the slow later flattening.
- ``k6`` (round 2): ``k1`` with forecast-only pooled noise (``forecast_noise_pooling =
  "pooled"``): forecast intervals use the pooled residual variance, likelihoods keep the
  median noise centre.
- ``k7`` (round 2): ``k1`` with pooled noise in likelihoods and forecasts
  (``noise_pooling = "pooled"``).

``FAMILY_CANDIDATES`` names the candidates whose series carry the family part
(``adapter``). Each development round adds a named candidate and never changes an
earlier one. Nothing here reads files or claims clinical validity.
"""

from __future__ import annotations

import math
from collections.abc import Callable, Mapping
from dataclasses import replace
from typing import Any

from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.model_learning.eye_support import SupportConfig
from peykar.validation.cornea_kmax_data import COVARIATE_UNIT_D, KmaxSeries, to_kmax_series

__all__ = [
    "CENTRE_D",
    "DEVELOPMENT_CANDIDATES",
    "FAMILIES",
    "FAMILY_CANDIDATES",
    "adapter",
    "family_context",
    "to_kmax_series_by_family",
]

MODEL_ID = "cornea.kmax-population"
REFERENCE = "k0"
# Baseline Kmax at which the level covariate is zero.
CENTRE_D = 55.0
FAMILIES: dict[str, str] = {
    "cxl_epi_off": "cxl",
    "cxl_accelerated": "cxl",
    "cxl_epi_on": "cxl",
    "cxl_combined": "cxl",
    # The protocol's ``other`` is first of all customized or topography-guided
    # cross-linking.
    "other": "cxl",
    "icrs": "icrs",
    "observation": "observation",
}


def _fields(version: str) -> dict[str, Any]:
    return {
        "model_id": MODEL_ID,
        "model_version": version,
        "quantity": "maximum keratometry change from the baseline Kmax (pseudo-fraction "
        "exp(change / 5 D))",
        "covariate_reference": math.exp(CENTRE_D / COVARIATE_UNIT_D),
        "covariate_unit": "pseudo-reference exp(Kmax0 / 5 D), so the covariate is "
        "(Kmax0 - 55 D) / 5 D",
        "study_effects": "per_group",
    }


def _config(version: str) -> RecoveryConfig:
    return RecoveryConfig(**_fields(version))


def _without_transient(version: str) -> RecoveryConfig:
    return replace(
        _config(version),
        transient_months=None,
        penalty_modes=(0.1, 0.1, 0.5),
        robust_sd=(0.3, 0.3, 2.0),
    )


DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # k0: one level per procedure context.
    "k0": _config("1.0.0-k0"),
    # k1: no 7-day early transient.
    "k1": _without_transient("1.0.0-k1"),
    # k2: species -> treatment family -> procedure.
    "k2": replace(_config("1.0.0-k2"), grouping=("treatment", "context")),
    # k3: the support hold before a context's earliest training visit.
    "k3": SupportConfig(**_fields("1.0.0-k3"), support_hold=True),
    # k4: persistent (Ornstein-Uhlenbeck) deviations in forecasting only.
    "k4": replace(
        _config("1.0.0-k4"),
        deviation_fraction=0.8,
        deviation_months=12.0,
        deviation_in_population_fit=False,
    ),
    # k5: a one-month early term instead of the 7-day transient.
    "k5": replace(_config("1.0.0-k5"), transient_months=1.0),
    # k6 (round 2): k1 with forecast-only pooled noise.
    "k6": replace(_without_transient("1.0.0-k6"), forecast_noise_pooling="pooled"),
    # k7 (round 2): k1 with pooled noise in likelihoods and forecasts.
    "k7": replace(_without_transient("1.0.0-k7"), noise_pooling="pooled"),
}

FAMILY_CANDIDATES: frozenset[str] = frozenset({"k2"})


def family_context(context: str) -> str:
    """``human/procedure`` → ``human/procedure/<family>``."""
    species, procedure = context.split("/")
    return f"{species}/{procedure}/{FAMILIES[procedure]}"


def to_kmax_series_by_family(record: Mapping[str, Any]) -> list[KmaxSeries]:
    """The series with the treatment family as the last context part."""
    return [replace(item, context=family_context(item.context)) for item in to_kmax_series(record)]


def adapter(name: str) -> Callable[[Mapping[str, Any]], list[KmaxSeries]]:
    """How the series of candidate ``name`` are read from a record."""
    if name in FAMILY_CANDIDATES:
        return to_kmax_series_by_family
    return to_kmax_series
