"""IOP series for model 1.4: model 1.2's series with a sub-procedure context only for
selected procedures.

The records, their validation and the series are those of ``eye_iop_data_1_2``; the
sub-procedure of an arm is read by the frozen keyword rules of ``eye_iop_data_1_3``
(both unchanged because frozen protocols pin them). A procedure context
``species/procedure`` becomes ``species/<sub>/procedure`` when the procedure is in the
selected set and ``species/all/procedure`` otherwise, so that with grouping
``("treatment", "context")`` the hierarchy is species → procedure → sub-procedure and an
unselected procedure has one sub-procedure node, as procedures without rules had in
model 1.3. Nothing here reads files.
"""

from __future__ import annotations

from collections.abc import Collection, Mapping
from dataclasses import replace
from typing import Any

from peykar.validation.eye_iop_data_1_2 import IopSeries, to_iop_series
from peykar.validation.eye_iop_data_1_3 import SUB_PROCEDURE_RULES, sub_procedure
from peykar_core.errors import InvalidRequest

__all__ = ["IopSeries", "to_iop_series_1_4"]


def to_iop_series_1_4(record: Mapping[str, Any], selected: Collection[str]) -> list[IopSeries]:
    """Model 1.2's series with contexts ``species/<sub>/procedure`` for the selected
    procedures and ``species/all/procedure`` for the others."""
    unknown = set(selected) - set(SUB_PROCEDURE_RULES)
    if unknown:
        raise InvalidRequest(f"No sub-procedure rules for {sorted(unknown)}")
    groups = {g["group"]: g for g in record["groups"]}
    specs = {f"{record['study_id']}/{s['series_id']}": s for s in record["series"]}
    title = str(record.get("title") or "")
    result = []
    for item in to_iop_series(record):
        species, _, procedure = item.context.partition("/")
        if procedure in selected:
            group = groups[specs[item.series_id]["group"]]
            text = f"{group.get('description') or ''} {group['group']}"
            sub = sub_procedure(procedure, text, title)
        else:
            sub = "all"
        result.append(replace(item, context=f"{species}/{sub}/{procedure}"))
    return result
