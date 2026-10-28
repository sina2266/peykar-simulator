"""Validated records of central corneal thickness (CCT) after surgery.

A record (schema ``cornea.central-thickness-record-1.0.0``) keeps provenance,
licence, procedure context and per-group CCT series that start at the preoperative
reference. Values cite their source cells. This module validates records,
converts them to :class:`ThicknessSeries` (fraction of the reference CCT over
months) and builds records from cell-citing table specs. It never downloads, fits
or scores anything.
"""

from __future__ import annotations

import hashlib
import math
import re
from collections.abc import Mapping, Sequence
from dataclasses import dataclass, field
from typing import Any

from peykar.validation.cornea_table_records import cell_text, leading_number
from peykar_core.errors import InvalidRequest

SCHEMA_ID = "cornea.central-thickness-record"
SCHEMA_VERSION = "1.0.0"
SPECIES = {"human", "rabbit", "pig", "monkey", "cat", "dog", "other"}
PROCEDURES = {
    "cataract",
    "dsaek",
    "dmek",
    "pk",
    "dalk",
    "glaucoma",
    "phakic_iol",
    "vitreoretinal",
    "other",
}
REFERENCES = {"patient_preoperative"}
VALUE_KINDS = {"cct_absolute", "cct_change", "percent_change"}
METHODS = {"ultrasound", "oct", "scheimpflug", "specular", "confocal", "other", "unstated"}
ROLES = {"control", "treated"}
KINDS = {"individual", "group_mean"}
SPLITS = {"development", "sealed_test", "excluded"}
MAX_MONTHS = 600.0
MIN_CCT_UM = 300.0
MAX_CCT_UM = 1300.0
_ID = re.compile(r"^[a-z0-9][a-z0-9._/-]{2,120}$")


@dataclass(frozen=True)
class ThicknessSeries:
    """One CCT series as a fraction of its preoperative reference, over months."""

    series_id: str
    study_id: str
    context: str
    reference_um: float
    months: tuple[float, ...]
    fractions: tuple[float, ...]
    aggregate_n: int | None = None
    role: str = "control"
    group: str | None = None
    method: str = "unstated"
    metadata: Mapping[str, Any] = field(default_factory=dict)

    def __post_init__(self) -> None:
        if len(self.months) != len(self.fractions) or not self.months:
            raise InvalidRequest("Thickness series need paired months and fractions")
        if list(self.months) != sorted(set(self.months)) or self.months[0] <= 0:
            raise InvalidRequest("Follow-up months must be positive and increasing")
        if not all(math.isfinite(v) and 0.4 <= v <= 2.0 for v in self.fractions):
            raise InvalidRequest("CCT fractions must lie within 0.4..2")

    @property
    def is_aggregate(self) -> bool:
        return self.aggregate_n is not None

    @property
    def kind(self) -> str:
        return "aggregate" if self.is_aggregate else "individual"

    @property
    def effective_n(self) -> int:
        return max(1, self.aggregate_n or 1)

    def prefix(self, last_month: float) -> ThicknessSeries:
        keep = [i for i, month in enumerate(self.months) if month <= last_month]
        if not keep:
            raise InvalidRequest("A prefix needs at least one follow-up")
        return ThicknessSeries(
            series_id=self.series_id,
            study_id=self.study_id,
            context=self.context,
            reference_um=self.reference_um,
            months=tuple(self.months[i] for i in keep),
            fractions=tuple(self.fractions[i] for i in keep),
            aggregate_n=self.aggregate_n,
            role=self.role,
            group=self.group,
            method=self.method,
            metadata=self.metadata,
        )


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise InvalidRequest(message)


def _finite(value: object, name: str) -> float:
    _require(
        isinstance(value, (int, float)) and not isinstance(value, bool), f"{name} must be numeric"
    )
    number = float(value)  # type: ignore[arg-type]
    _require(math.isfinite(number), f"{name} must be finite")
    return number


def validate_record(record: Mapping[str, Any]) -> dict[str, Any]:
    """Check one record; return normalized series (fractions of the reference)."""
    _require(record.get("schema") == f"{SCHEMA_ID}-{SCHEMA_VERSION}", "Unknown record schema")
    study_id = record.get("study_id", "")
    _require(isinstance(study_id, str) and bool(_ID.match(study_id)), "Invalid study_id")
    _require(bool(record.get("doi")) or bool(record.get("source_url")), "Study needs DOI or URL")
    _require(record.get("species") in SPECIES, "Declare the species group")
    _require(record.get("procedure") in PROCEDURES, "Declare the procedure context")
    _require(record.get("split") in SPLITS, "Study split must be declared")
    licence = record.get("license", {})
    _require(isinstance(licence, Mapping) and bool(licence.get("statement")), "Licence missing")
    files = record.get("source_files", [])
    _require(isinstance(files, list) and len(files) >= 1, "At least one source file is required")
    for item in files:
        _require(
            isinstance(item.get("sha256"), str) and len(item["sha256"]) == 64,
            "Each source file needs its SHA-256",
        )
    groups = {g["group"]: g for g in record.get("groups", [])}
    _require(bool(groups), "At least one group is required")
    for group in groups.values():
        _require(group.get("role") in ROLES, "Group role must be control or treated")
        _require(
            group.get("procedure", record["procedure"]) in PROCEDURES,
            "Group procedure must be a declared context",
        )
    normalized = []
    seen = set()
    for series in record.get("series", []):
        series_id = series.get("series_id", "")
        _require(bool(series_id) and series_id not in seen, "Series identities must be unique")
        seen.add(series_id)
        _require(series.get("group") in groups, "Series references an undeclared group")
        _require(series.get("kind") in KINDS, "Series kind must be individual or group_mean")
        kind = series.get("value_kind")
        _require(kind in VALUE_KINDS, "Unknown value kind")
        _require(series.get("reference_kind") in REFERENCES, "Declare the reference kind")
        _require(series.get("method") in METHODS, "Declare the measurement method")
        months = [_finite(m, "month") for m in series.get("months", [])]
        values = [_finite(v, "value") for v in series.get("values", [])]
        _require(len(months) == len(values) >= 2, "Series need >=2 paired values")
        _require(months == sorted(set(months)), "Months must be unique and increasing")
        _require(months[0] == 0.0, "Series must start at the reference (month 0)")
        _require(months[-1] <= MAX_MONTHS, "Follow-up longer than 50 years is not accepted")
        cells = series.get("source_cells", [])
        _require(len(cells) == len(values), "Each value needs its source cell reference")
        if kind == "cct_absolute":
            _require(
                MIN_CCT_UM <= values[0] <= MAX_CCT_UM, "Reference CCT must lie within 300..1300 µm"
            )
            reference = values[0]
            fractions = [v / reference for v in values]
        elif kind == "percent_change":
            _require(abs(values[0]) <= 1e-9, "Percent change at the reference must be zero")
            reference_value = series.get("reference_cct")
            reference = (
                _finite(reference_value, "reference_cct") if reference_value is not None else 0.0
            )
            fractions = [1.0 + v / 100.0 for v in values]
        else:
            _require(abs(values[0]) <= 1e-9, "CCT change at the reference must be zero")
            reference = _finite(series.get("reference_cct"), "reference_cct")
            _require(
                MIN_CCT_UM <= reference <= MAX_CCT_UM, "Reference CCT must lie within 300..1300 µm"
            )
            _require(bool(series.get("reference_cell")), "CCT changes need a cited reference")
            fractions = [1.0 + v / reference for v in values]
        _require(all(0.4 <= f <= 2.0 for f in fractions), "Fractions must lie within 0.4..2")
        procedure = groups[series["group"]].get("procedure", record["procedure"])
        normalized.append(
            {
                "series_id": series_id,
                "group": series["group"],
                "role": groups[series["group"]]["role"],
                "context": f"{record['species']}/{procedure}",
                "kind": series["kind"],
                "n": series.get("n"),
                "months": months,
                "fractions": fractions,
                "reference_um": reference,
                "method": series["method"],
            }
        )
    _require(bool(normalized), "Record has no series")
    return {
        "study_id": study_id,
        "context": f"{record['species']}/{record['procedure']}",
        "contexts": sorted({s["context"] for s in normalized}),
        "split": record["split"],
        "series": normalized,
    }


def to_thickness_series(record: Mapping[str, Any]) -> list[ThicknessSeries]:
    checked = validate_record(record)
    result = []
    for item in checked["series"]:
        aggregate = None if item["kind"] == "individual" else int(item["n"] or 0)
        result.append(
            ThicknessSeries(
                series_id=f"{checked['study_id']}/{item['series_id']}",
                study_id=checked["study_id"],
                context=item["context"],
                reference_um=item["reference_um"],
                months=tuple(item["months"][1:]),
                fractions=tuple(item["fractions"][1:]),
                aggregate_n=aggregate,
                role=item["role"],
                group=item["group"],
                method=item["method"],
            )
        )
    return result


def structural_summary(records: Sequence[Mapping[str, Any]]) -> dict[str, Any]:
    """Counts and covariates only, so sealed records can be audited blind."""
    studies = []
    for record in records:
        checked = validate_record(record)
        studies.append(
            {
                "study_id": checked["study_id"],
                "split": checked["split"],
                "contexts": checked["contexts"],
                "groups": sorted({s["group"] for s in checked["series"]}),
                "control_series": sum(s["role"] == "control" for s in checked["series"]),
                "treated_series": sum(s["role"] == "treated" for s in checked["series"]),
                "group_mean_series": sum(s["kind"] == "group_mean" for s in checked["series"]),
                "methods": sorted({s["method"] for s in checked["series"]}),
                "follow_up_months": sorted({m for s in checked["series"] for m in s["months"]}),
            }
        )
    return {"schema": f"{SCHEMA_ID}-{SCHEMA_VERSION}", "studies": studies}


def build_record(spec: Mapping[str, Any], sources: Mapping[str, bytes]) -> dict[str, Any]:
    """Assemble and validate a record from a spec citing ``file::table!RrCc`` cells.

    A cell entry may also be ``{"definition": "definition:<where>", "value": 0}`` for
    the reference of a change series (protocol: change at time 0 is 0).
    """
    files = {item["name"]: item for item in spec["source_files"]}
    for name, item in files.items():
        if name not in sources:
            raise InvalidRequest(f"Source {name!r} was not provided")
        if hashlib.sha256(sources[name]).hexdigest() != item["sha256"]:
            raise InvalidRequest(f"Source {name!r} does not match its SHA-256")
    series_out = []
    for series in spec["series"]:
        values: list[float] = []
        cited: list[str] = []
        texts: list[str] = []
        for entry in series["cells"]:
            if isinstance(entry, Mapping):
                reference = str(entry["definition"])
                if not reference.startswith("definition:"):
                    raise InvalidRequest("Explicit values must cite a definition")
                values.append(float(entry["value"]))
                cited.append(reference)
                texts.append("")
                continue
            name, _, location = str(entry).partition("::")
            if name not in files:
                raise InvalidRequest(f"Series cites unknown source {name!r}")
            text = cell_text(sources[name], location)
            values.append(leading_number(text.replace(",", "")))
            cited.append(f"{name}::{location}")
            texts.append(text)
        item = {k: v for k, v in series.items() if k != "cells"}
        item.update(values=values, source_cells=cited, source_cell_texts=texts)
        if series.get("reference_cell"):
            name, _, location = str(series["reference_cell"]).partition("::")
            if name not in files:
                raise InvalidRequest(f"Reference cites unknown source {name!r}")
            reference_text = cell_text(sources[name], location)
            item["reference_cct"] = leading_number(reference_text.replace(",", ""))
            item["reference_cell_text"] = reference_text
        series_out.append(item)
    record = {
        "schema": f"{SCHEMA_ID}-{SCHEMA_VERSION}",
        **{k: v for k, v in spec.items() if k not in {"series", "schema"}},
        "series": series_out,
    }
    validate_record(record)
    return record
