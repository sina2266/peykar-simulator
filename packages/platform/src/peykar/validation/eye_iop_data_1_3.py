"""IOP series for model 1.3: model 1.2's series with a declared sub-procedure context.

The records, their validation and the series are those of ``eye_iop_data_1_2``
(unchanged because the IOP 1.2 protocol pins it). Model 1.3 adds one hierarchy level:
a procedure context ``species/procedure`` becomes ``species/<sub>/procedure``, so that
with grouping ``("treatment", "context")`` the population hierarchy is species →
procedure → sub-procedure (the procedure is the last part of the context).

The sub-procedure is read from the arm's own group description and group name by the
declared keyword rules below, checked in order; only when they match nothing is the
record title used (titles of comparative studies name every arm, so they come last).
Procedures without rules, and arms that match no rule, get the sub-procedure ``all``
or ``other``. The rules were written from the training records' descriptions only.
Nothing here reads files.
"""

from __future__ import annotations

import re
from collections.abc import Mapping
from dataclasses import replace
from typing import Any

from peykar.validation.eye_iop_data_1_2 import IopSeries, to_iop_series

__all__ = ["SUB_PROCEDURE_RULES", "IopSeries", "sub_procedure", "to_iop_series_1_3"]

# (sub-procedure, pattern) per procedure, checked in order on lower-case text.
SUB_PROCEDURE_RULES: dict[str, tuple[tuple[str, str], ...]] = {
    "tube": (
        # "non-valved" contains "valved": the non-valved rule comes first.
        (
            "non_valved",
            r"non-?valved|baerveldt|molteno|\bpaul\b|clearpath|aurolab|\baadi\b|eyeplate",
        ),
        ("valved", r"ahmed|\bagv\b|valved|\bfp7\b|\bs2\b"),
    ),
    "angle_mims": (
        ("stent", r"istent|hydrus|cypass"),
        (
            "goniotomy",
            r"kahook|\bkdb\b|dual blade|goniotomy|trabectome|ab interno trabeculectomy|"
            r"\bait\b|sinskey",
        ),
        ("trabeculotomy", r"trabeculotomy|\bgatt\b|microhook|tanito|t-hook|\bomni\b"),
        ("canaloplasty", r"canaloplasty|viscodilat|viscocanalostomy|itrack|\babic\b"),
    ),
    "cyclodestruction": (
        ("micropulse", r"micropulse|\bmp-?tscpc\b|\bmp3\b|\bmp-?cpc\b|subthreshold"),
        ("endoscopic", r"endoscopic|\becp\b"),
        ("ultrasound", r"ultraso|\bucp\b|\bhifu\b|eyeop"),
        (
            "continuous",
            r"continuous|\bcw\b|cw-?tscpc|slow.?coagulation|g-?probe|diode|tscpc|"
            r"transscleral|cyclophotocoagulation|cyclocryo",
        ),
    ),
    "subconjunctival_mims": (
        ("xen", r"\bxen\b|gel stent|gel microstent"),
        ("preserflo", r"preserflo|innfocus|microshunt|micro-shunt|\bpms\b"),
    ),
}


def sub_procedure(procedure: str, group_text: str, title: str = "") -> str:
    """The declared sub-procedure of an arm (``all`` for procedures without rules)."""
    rules = SUB_PROCEDURE_RULES.get(procedure)
    if rules is None:
        return "all"
    for text in (group_text, title):
        low = text.lower()
        for name, pattern in rules:
            if re.search(pattern, low):
                return name
    return "other"


def to_iop_series_1_3(record: Mapping[str, Any]) -> list[IopSeries]:
    """Model 1.2's series with contexts ``species/<sub>/procedure``."""
    groups = {g["group"]: g for g in record["groups"]}
    specs = {f"{record['study_id']}/{s['series_id']}": s for s in record["series"]}
    title = str(record.get("title") or "")
    result = []
    for item in to_iop_series(record):
        group = groups[specs[item.series_id]["group"]]
        species, _, procedure = item.context.partition("/")
        text = f"{group.get('description') or ''} {group['group']}"
        sub = sub_procedure(procedure, text, title)
        result.append(replace(item, context=f"{species}/{sub}/{procedure}"))
    return result
