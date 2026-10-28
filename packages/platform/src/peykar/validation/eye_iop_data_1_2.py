"""IOP series for model 1.2: records of schema 1.0.0 in the generic series protocol.

The records and their validation are those of ``eye_iop_data`` (unchanged because the
IOP protocols pin it). ``IopSeries`` is a ``PressureSeries`` that also exposes its
preoperative reference as ``reference_value``, the name generic eye model version 2
uses. Nothing here reads files.
"""

from __future__ import annotations

from collections.abc import Mapping
from dataclasses import dataclass, fields
from typing import Any

from peykar.validation.eye_iop_data import PressureSeries, to_pressure_series

__all__ = ["IopSeries", "to_iop_series"]


@dataclass(frozen=True)
class IopSeries(PressureSeries):
    @property
    def reference_value(self) -> float:
        return self.reference_mmhg

    def prefix(self, last_month: float) -> IopSeries:
        return _wrap(PressureSeries.prefix(self, last_month))


def _wrap(series: PressureSeries) -> IopSeries:
    return IopSeries(**{f.name: getattr(series, f.name) for f in fields(PressureSeries)})


def to_iop_series(record: Mapping[str, Any]) -> list[IopSeries]:
    return [_wrap(series) for series in to_pressure_series(record)]
