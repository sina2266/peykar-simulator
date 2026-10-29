"""Retinal-thickness series for model 1.2: model 1.1's series with a prior-treatment
sub-context.

The records, their validation, the series and the declared dose rule are those of
``eye_retina_data_1_1`` (unchanged because the retinal-thickness 1.1 protocol pins it).
For candidates with a prior-treatment level, a context ``species/indication/treatment``
becomes ``species/<prior>/indication|treatment``, where ``<prior>`` is the series'
declared prior treatment (``naive``, ``previously_treated``, ``mixed`` or
``unstated``). With grouping ``("treatment", "context")`` the population hierarchy is
then species → indication and treatment (the last part of the context, as before) →
prior treatment. Candidates with a device level use the series' declared OCT device
(``spectralis``, ``cirrus``, ``topcon``, ``optovue``, ``stratus``, ``other`` or
``unstated``) in the same place. Nothing here reads files.
"""

from __future__ import annotations

from collections.abc import Mapping
from dataclasses import replace
from typing import Any

from peykar.validation.eye_retina_data import RetinaSeries
from peykar.validation.eye_retina_data_1_1 import to_retina_series_1_1

__all__ = ["prior_treatment_context", "to_retina_series_1_2_device", "to_retina_series_1_2_prior"]


def prior_treatment_context(context: str, prior_treatment: str) -> str:
    """``species/indication/treatment`` → ``species/<prior>/indication|treatment``."""
    species, indication, treatment = context.split("/")
    return f"{species}/{prior_treatment}/{indication}|{treatment}"


def to_retina_series_1_2_prior(record: Mapping[str, Any]) -> list[RetinaSeries]:
    """Model 1.1's series with prior-treatment sub-contexts."""
    return [
        replace(item, context=prior_treatment_context(item.context, item.prior_treatment))
        for item in to_retina_series_1_1(record)
    ]


def to_retina_series_1_2_device(record: Mapping[str, Any]) -> list[RetinaSeries]:
    """Model 1.1's series with OCT-device sub-contexts."""
    return [
        replace(item, context=prior_treatment_context(item.context, item.device))
        for item in to_retina_series_1_1(record)
    ]
