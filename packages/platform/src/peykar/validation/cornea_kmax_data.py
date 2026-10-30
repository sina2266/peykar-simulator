"""Validated records of maximum keratometry after corneal cross-linking or ring segments.

A record (schema ``cornea.kmax-record-1.0.0``) keeps provenance, licence, procedure
context and per-group series of maximum keratometry (Kmax, the steepest anterior
corneal power, in dioptres) that start at the preoperative (baseline) Kmax. Values cite
their source cells and are dioptres as printed. This module validates records, converts
them to :class:`KmaxSeries` and builds records from cell-citing table specs. It never
downloads, fits or scores anything.

Kmax changes additively, so a series is the change from the baseline Kmax,
``Kmax(t) − Kmax0`` in dioptres (negative is flattening). As for refraction
(``cornea_refraction_data``), a :class:`KmaxSeries` gives the generic eye models the
change as the pseudo-fraction ``exp(change / MODEL_UNIT_D)`` and the baseline Kmax as
the pseudo-reference ``exp(Kmax0 / COVARIATE_UNIT_D)``, so their log fraction is the
change in units of ``MODEL_UNIT_D`` and their level covariate ``ln(reference / r)`` is
``(Kmax0 − COVARIATE_UNIT_D ln r) / COVARIATE_UNIT_D``. Both maps are exact;
``peykar.validation.eye_additive_evaluation`` scores forecasts in units of
``REPORT_UNIT_D`` = 0.1 D (one reported pp).
"""

from __future__ import annotations

import hashlib
import math
import re
from collections.abc import Mapping, Sequence
from dataclasses import dataclass, field, replace
from typing import Any

from peykar.validation.cornea_refraction_data import refraction_number
from peykar.validation.cornea_table_records import cell_text
from peykar_core.errors import InvalidRequest

__all__ = [
    "KmaxSeries",
    "build_record",
    "change_from_fraction",
    "kmax_number",
    "reference_from_value",
    "structural_summary",
    "to_kmax_series",
    "validate_record",
]

SCHEMA_ID = "cornea.kmax-record"
SCHEMA_VERSION = "1.0.0"
SPECIES = {"human"}
PROCEDURES = {
    "cxl_epi_off",
    "cxl_accelerated",
    "cxl_epi_on",
    "cxl_combined",
    "icrs",
    "observation",
    "other",
}
INDICATIONS = {"keratoconus", "ectasia_after_refractive", "pellucid", "mixed", "unstated"}
AGE_GROUPS = {"pediatric", "adult", "mixed", "unstated"}
DEVICES = {"pentacam", "sirius", "galilei", "orbscan", "placido", "other", "unstated"}
VALUE_KINDS = {"kmax_absolute", "kmax_change"}
ROLES = {"control", "treated"}
KINDS = {"individual", "group_mean"}
SPLITS = {"development", "sealed_test", "excluded"}
MAX_MONTHS = 600.0
MIN_KMAX_D = 35.0
MAX_KMAX_D = 95.0
# The generic models see the change from Kmax0 in units of MODEL_UNIT_D and the level
# covariate through the pseudo-reference exp(Kmax0 / COVARIATE_UNIT_D).
MODEL_UNIT_D = 5.0
COVARIATE_UNIT_D = 5.0
# Errors are reported in tenths of a dioptre: one reported pp is 0.1 D.
REPORT_UNIT_D = 0.1
_ID = re.compile(r"^[a-z0-9][a-z0-9._/-]{2,120}$")
_LEADING_POINT = re.compile("^(\\s*[-+\u2212\u2013]?)[.](?=\\d)")


def kmax_number(text: str) -> float:
    """The leading signed number of a cell (unicode or spaced minus, decimal comma, a
    leading point without its zero), read as the refraction reader does."""
    return refraction_number(_LEADING_POINT.sub(r"\g<1>0.", text, count=1))


@dataclass(frozen=True)
class KmaxSeries:
    """One Kmax series as the change from its baseline Kmax (dioptres)."""

    series_id: str
    study_id: str
    context: str
    reference_d: float
    months: tuple[float, ...]
    changes_d: tuple[float, ...]
    aggregate_n: int | None = None
    role: str = "control"
    group: str | None = None
    indication: str = "unstated"
    age_group: str = "unstated"
    device: str = "unstated"
    metadata: Mapping[str, Any] = field(default_factory=dict)

    def __post_init__(self) -> None:
        if len(self.months) != len(self.changes_d) or not self.months:
            raise InvalidRequest("Kmax series need paired months and changes")
        if list(self.months) != sorted(set(self.months)) or self.months[0] <= 0:
            raise InvalidRequest("Follow-up months must be positive and increasing")
        if not all(math.isfinite(v) for v in (self.reference_d, *self.changes_d)):
            raise InvalidRequest("Kmax values must be finite")

    @property
    def fractions(self) -> tuple[float, ...]:
        """The change as the pseudo-fraction ``exp(change / MODEL_UNIT_D)``."""
        return tuple(math.exp(c / MODEL_UNIT_D) for c in self.changes_d)

    @property
    def reference_value(self) -> float:
        """The baseline Kmax as the pseudo-reference ``exp(Kmax0 / COVARIATE_UNIT_D)``."""
        return math.exp(self.reference_d / COVARIATE_UNIT_D)

    @property
    def is_aggregate(self) -> bool:
        return self.aggregate_n is not None

    @property
    def kind(self) -> str:
        return "aggregate" if self.is_aggregate else "individual"

    def prefix(self, last_month: float) -> KmaxSeries:
        keep = [i for i, month in enumerate(self.months) if month <= last_month]
        if not keep:
            raise InvalidRequest("A prefix needs at least one follow-up")
        return replace(
            self,
            months=tuple(self.months[i] for i in keep),
            changes_d=tuple(self.changes_d[i] for i in keep),
        )


def change_from_fraction(value: float) -> float:
    """Inverse of the pseudo-fraction: the change from Kmax0 in dioptres."""
    return MODEL_UNIT_D * math.log(value)


def reference_from_value(value: float) -> float:
    """Inverse of the pseudo-reference: the baseline Kmax in dioptres."""
    return COVARIATE_UNIT_D * math.log(value)


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


def _kmax_in_range(value: float, name: str) -> None:
    _require(MIN_KMAX_D <= value <= MAX_KMAX_D, f"{name} must lie within 35..95 D")


def _kmax_values(kind: str, values: list[float], series: Mapping[str, Any]) -> list[float]:
    """Kmax in dioptres at every time of a validated series."""
    if kind == "kmax_absolute":
        for value in values:
            _kmax_in_range(value, "Kmax")
        return list(values)
    _require(abs(values[0]) <= 1e-9, "The Kmax change at the reference must be zero")
    _require(bool(series.get("reference_cell")), "Kmax changes need a cited reference")
    reference = _finite(series.get("reference_value"), "reference_value")
    _kmax_in_range(reference, "Baseline Kmax")
    result = [reference + v for v in values]
    for value in result:
        _kmax_in_range(value, "Kmax")
    return result


def validate_record(record: Mapping[str, Any]) -> dict[str, Any]:
    """Check one record; return normalized series (changes from the baseline Kmax)."""
    _require(record.get("schema") == f"{SCHEMA_ID}-{SCHEMA_VERSION}", "Unknown record schema")
    study_id = record.get("study_id", "")
    _require(isinstance(study_id, str) and bool(_ID.match(study_id)), "Invalid study_id")
    _require(bool(record.get("doi")) or bool(record.get("source_url")), "Study needs DOI or URL")
    _require(record.get("species") in SPECIES, "Only human eyes are in scope")
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
        _require(group.get("indication") in INDICATIONS, "Declare the group's indication")
        _require(group.get("age_group") in AGE_GROUPS, "Declare the group's age group")
    normalized = []
    seen = set()
    for series in record.get("series", []):
        series_id = series.get("series_id", "")
        _require(bool(series_id) and series_id not in seen, "Series identities must be unique")
        seen.add(series_id)
        _require(series.get("group") in groups, "Series references an undeclared group")
        _require(series.get("kind") in KINDS, "Series kind must be individual or group_mean")
        _require(series.get("device") in DEVICES, "Declare the series' device")
        kind = series.get("value_kind")
        _require(kind in VALUE_KINDS, "Unknown value kind")
        months = [_finite(m, "month") for m in series.get("months", [])]
        values = [_finite(v, "value") for v in series.get("values", [])]
        _require(len(months) == len(values) >= 2, "Series need >=2 paired values")
        _require(months == sorted(set(months)), "Months must be unique and increasing")
        _require(months[0] == 0.0, "Series must start at the baseline Kmax (month 0)")
        _require(months[-1] <= MAX_MONTHS, "Follow-up longer than 50 years is not accepted")
        cells = series.get("source_cells", [])
        _require(len(cells) == len(values), "Each value needs its source cell reference")
        kmax = _kmax_values(kind, values, series)
        reference = kmax[0]
        group = groups[series["group"]]
        procedure = group.get("procedure", record["procedure"])
        normalized.append(
            {
                "series_id": series_id,
                "group": series["group"],
                "role": group["role"],
                "context": f"{record['species']}/{procedure}",
                "indication": group["indication"],
                "age_group": group["age_group"],
                "device": series["device"],
                "kind": series["kind"],
                "n": series.get("n"),
                "months": months,
                "changes_d": [v - reference for v in kmax],
                "reference_d": reference,
                "value_kind": kind,
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


def to_kmax_series(record: Mapping[str, Any]) -> list[KmaxSeries]:
    checked = validate_record(record)
    result = []
    for item in checked["series"]:
        aggregate = None if item["kind"] == "individual" else int(item["n"] or 0)
        result.append(
            KmaxSeries(
                series_id=f"{checked['study_id']}/{item['series_id']}",
                study_id=checked["study_id"],
                context=item["context"],
                reference_d=item["reference_d"],
                months=tuple(item["months"][1:]),
                changes_d=tuple(item["changes_d"][1:]),
                aggregate_n=aggregate,
                role=item["role"],
                group=item["group"],
                indication=item["indication"],
                age_group=item["age_group"],
                device=item["device"],
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
                "indications": sorted({s["indication"] for s in checked["series"]}),
                "age_groups": sorted({s["age_group"] for s in checked["series"]}),
                "devices": sorted({s["device"] for s in checked["series"]}),
                "value_kinds": sorted({s["value_kind"] for s in checked["series"]}),
                "follow_up_months": sorted({m for s in checked["series"] for m in s["months"]}),
            }
        )
    return {"schema": f"{SCHEMA_ID}-{SCHEMA_VERSION}", "studies": studies}


def _read_cell(
    entry: object, files: Mapping[str, Any], sources: Mapping[str, bytes]
) -> tuple[float, str, str]:
    """(value, citation, cell text) of a ``file::table!RrCc`` string."""
    name, _, location = str(entry).partition("::")
    if name not in files:
        raise InvalidRequest(f"Series cites unknown source {name!r}")
    text = cell_text(sources[name], location)
    return kmax_number(text), f"{name}::{location}", text


def build_record(spec: Mapping[str, Any], sources: Mapping[str, bytes]) -> dict[str, Any]:
    """Assemble and validate a record from a spec citing ``file::table!RrCc`` cells.

    A cell entry is a ``file::table!RrCc`` string or ``{"definition":
    "definition:<where>", "value": 0}`` for the reference of a change series
    (protocol: the change at time 0 is 0).
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
            if isinstance(entry, Mapping) and "definition" in entry:
                reference = str(entry["definition"])
                if not reference.startswith("definition:"):
                    raise InvalidRequest("Explicit values must cite a definition")
                values.append(float(entry["value"]))
                cited.append(reference)
                texts.append("")
                continue
            value, citation, text = _read_cell(entry, files, sources)
            values.append(value)
            cited.append(citation)
            texts.append(text)
        item = {k: v for k, v in series.items() if k != "cells"}
        item.update(values=values, source_cells=cited, source_cell_texts=texts)
        if series.get("reference_cell"):
            value, citation, text = _read_cell(series["reference_cell"], files, sources)
            item["reference_value"] = value
            item["reference_cell"] = citation
            item["reference_cell_text"] = text
        series_out.append(item)
    record = {
        "schema": f"{SCHEMA_ID}-{SCHEMA_VERSION}",
        **{k: v for k, v in spec.items() if k not in {"series", "schema"}},
        "series": series_out,
    }
    validate_record(record)
    return record
