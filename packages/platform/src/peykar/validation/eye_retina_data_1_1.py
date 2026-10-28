"""Retina thickness series for model 1.1: records of schema 1.0.0 with the dose rule.

The records and their validation are those of ``eye_retina_data`` (unchanged because
retinal-thickness protocol 1.0.0 pins it). Each series additionally carries, in its
``metadata``, the effect duration that model 1.1's declared dose rule assigns to its
group (``effect_duration_months``; ``None`` for no single-dose wear-off). Nothing here
reads files.
"""

from __future__ import annotations

from collections.abc import Mapping
from dataclasses import replace
from typing import Any

from peykar.model_learning.retina_thickness_population_1_1 import effect_duration_months
from peykar.validation.eye_retina_data import RetinaSeries, to_retina_series

__all__ = ["to_retina_series_1_1"]


def to_retina_series_1_1(record: Mapping[str, Any]) -> list[RetinaSeries]:
    groups = {group["group"]: group for group in record["groups"]}
    result = []
    for item in to_retina_series(record):
        group = groups[item.group]
        treatment = group.get("treatment", record["treatment"])
        duration = effect_duration_months(treatment, item.regimen, group.get("agent"))
        result.append(replace(item, metadata={**item.metadata, "effect_duration_months": duration}))
    return result
