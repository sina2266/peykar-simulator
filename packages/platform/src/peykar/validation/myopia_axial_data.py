"""Validated records of ocular axial length in childhood myopia, treated or not.

A record (schema ``eye.myopia-axial-record-1.0.0``) keeps provenance, licence, treatment
context and per-group series of ocular axial length (AL, millimetres) that start at the
baseline (enrolment) value. Values cite their source cells and are millimetres as
printed. This module validates records, converts them to :class:`AxialSeries` and builds
records from cell-citing table specs. It never downloads, fits or scores anything.

Axial length changes additively, so a series is the change from the baseline axial
length, ``AL(t) - AL0`` in millimetres (positive is elongation). As for refraction and
Kmax, an :class:`AxialSeries` gives the generic eye models the change as the
pseudo-fraction ``exp(change / MODEL_UNIT_MM)`` and the baseline axial length as the
pseudo-reference ``exp(AL0 / COVARIATE_UNIT_MM)``, so their log fraction is the change in
units of ``MODEL_UNIT_MM`` and their level covariate ``ln(reference / r)`` is
``(AL0 - COVARIATE_UNIT_MM ln r) / COVARIATE_UNIT_MM``. Both maps are exact;
``peykar.validation.eye_additive_evaluation`` scores forecasts in units of
``REPORT_UNIT_MM`` = 0.01 mm (one reported pp).
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
    "AxialSeries",
    "COVARIATE_UNIT_MM",
    "MODEL_UNIT_MM",
    "REPORT_UNIT_MM",
    "axial_number",
    "build_record",
    "change_from_fraction",
    "reference_from_value",
    "structural_summary",
    "to_axial_series",
    "validate_record",
]

SCHEMA_ID = "eye.myopia-axial-record"
SCHEMA_VERSION = "1.0.0"
SPECIES = {"human"}
TREATMENTS = {
    "single_vision",
    "atropine",
    "orthokeratology",
    "defocus_spectacles",
    "multifocal_contacts",
    "red_light",
    "combination",
    "outdoor",
    "other",
}
REGIONS = {
    "east_asia",
    "south_asia",
    "southeast_asia",
    "europe",
    "north_america",
    "latin_america",
    "middle_east",
    "africa",
    "oceania",
    "mixed",
    "unstated",
}
DEVICES = {"iolmaster", "lenstar", "aladdin", "myopia_master", "ultrasound", "other", "unstated"}
VALUE_KINDS = {"axial_absolute", "axial_change"}
ROLES = {"control", "treated"}
KINDS = {"individual", "group_mean"}
SPLITS = {"development", "sealed_test", "excluded"}
MAX_MONTHS = 240.0
MIN_AXIAL_MM = 19.0
MAX_AXIAL_MM = 32.0
MIN_AGE_YEARS = 3.0
MAX_AGE_YEARS = 18.0
# The generic models see the change from AL0 in units of MODEL_UNIT_MM and the level
# covariate through the pseudo-reference exp(AL0 / COVARIATE_UNIT_MM).
MODEL_UNIT_MM = 1.0
COVARIATE_UNIT_MM = 1.0
# Errors are reported in hundredths of a millimetre: one reported pp is 0.01 mm.
REPORT_UNIT_MM = 0.01
_ID = re.compile(r"^[a-z0-9][a-z0-9._/-]{2,120}$")
_LEADING_POINT = re.compile("^(\\s*[-+\u2212\u2013]?)[.](?=\\d)")


def axial_number(text: str) -> float:
    """The leading signed number of a cell (unicode or spaced minus, decimal comma, a
    leading point without its zero), read as the refraction reader does."""
    return refraction_number(_LEADING_POINT.sub(r"\g<1>0.", text, count=1))


@dataclass(frozen=True)
class AxialSeries:
    """One axial-length series as the change from its baseline axial length (mm)."""

    series_id: str
    study_id: str
    context: str
    reference_mm: float
    months: tuple[float, ...]
    changes_mm: tuple[float, ...]
    aggregate_n: int | None = None
    role: str = "control"
    group: str | None = None
    region: str = "unstated"
    device: str = "unstated"
    mean_age_years: float | None = None
    atropine_percent: float | None = None
    metadata: Mapping[str, Any] = field(default_factory=dict)

    def __post_init__(self) -> None:
        if len(self.months) != len(self.changes_mm) or not self.months:
            raise InvalidRequest("Axial-length series need paired months and changes")
        if list(self.months) != sorted(set(self.months)) or self.months[0] <= 0:
            raise InvalidRequest("Follow-up months must be positive and increasing")
        if not all(math.isfinite(v) for v in (self.reference_mm, *self.changes_mm)):
            raise InvalidRequest("Axial-length values must be finite")

    @property
    def fractions(self) -> tuple[float, ...]:
        """The change as the pseudo-fraction ``exp(change / MODEL_UNIT_MM)``."""
        return tuple(math.exp(c / MODEL_UNIT_MM) for c in self.changes_mm)

    @property
    def reference_value(self) -> float:
        """The baseline axial length as the pseudo-reference ``exp(AL0 / COVARIATE_UNIT_MM)``."""
        return math.exp(self.reference_mm / COVARIATE_UNIT_MM)

    @property
    def is_aggregate(self) -> bool:
        return self.aggregate_n is not None

    @property
    def kind(self) -> str:
        return "aggregate" if self.is_aggregate else "individual"

    def prefix(self, last_month: float) -> AxialSeries:
        keep = [i for i, month in enumerate(self.months) if month <= last_month]
        if not keep:
            raise InvalidRequest("A prefix needs at least one follow-up")
        return replace(
            self,
            months=tuple(self.months[i] for i in keep),
            changes_mm=tuple(self.changes_mm[i] for i in keep),
        )


def change_from_fraction(value: float) -> float:
    """Inverse of the pseudo-fraction: the change from AL0 in millimetres."""
    return MODEL_UNIT_MM * math.log(value)


def reference_from_value(value: float) -> float:
    """Inverse of the pseudo-reference: the baseline axial length in millimetres."""
    return COVARIATE_UNIT_MM * math.log(value)


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


def _axial_in_range(value: float, name: str) -> None:
    _require(MIN_AXIAL_MM <= value <= MAX_AXIAL_MM, f"{name} must lie within 19..32 mm")


def _axial_values(kind: str, values: list[float], series: Mapping[str, Any]) -> list[float]:
    """Axial length in millimetres at every time of a validated series."""
    if kind == "axial_absolute":
        for value in values:
            _axial_in_range(value, "Axial length")
        return list(values)
    _require(abs(values[0]) <= 1e-9, "The axial-length change at the reference must be zero")
    _require(bool(series.get("reference_cell")), "Axial-length changes need a cited reference")
    reference = _finite(series.get("reference_value"), "reference_value")
    _axial_in_range(reference, "Baseline axial length")
    result = [reference + v for v in values]
    for value in result:
        _axial_in_range(value, "Axial length")
    return result


def _group_checks(group: Mapping[str, Any], record: Mapping[str, Any]) -> str:
    _require(group.get("role") in ROLES, "Group role must be control or treated")
    treatment = group.get("treatment", record["treatment"])
    _require(treatment in TREATMENTS, "Group treatment must be a declared context")
    _require(group.get("region") in REGIONS, "Declare the group's region")
    age = group.get("mean_age_years")
    if age is not None:
        number = _finite(age, "mean_age_years")
        _require(MIN_AGE_YEARS <= number <= MAX_AGE_YEARS, "Mean age must lie within 3..18 years")
    dose = group.get("atropine_percent")
    if treatment == "atropine":
        _require(dose is not None, "Atropine groups need their concentration")
    if dose is not None:
        number = _finite(dose, "atropine_percent")
        _require(0.0 < number <= 1.0, "Atropine concentration must lie in (0, 1] percent")
    return str(treatment)


def validate_record(record: Mapping[str, Any]) -> dict[str, Any]:
    """Check one record; return normalized series (changes from the baseline AL)."""
    _require(record.get("schema") == f"{SCHEMA_ID}-{SCHEMA_VERSION}", "Unknown record schema")
    study_id = record.get("study_id", "")
    _require(isinstance(study_id, str) and bool(_ID.match(study_id)), "Invalid study_id")
    _require(bool(record.get("doi")) or bool(record.get("source_url")), "Study needs DOI or URL")
    _require(record.get("species") in SPECIES, "Only human eyes are in scope")
    _require(record.get("treatment") in TREATMENTS, "Declare the treatment context")
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
    treatments = {name: _group_checks(group, record) for name, group in groups.items()}
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
        _require(months[0] == 0.0, "Series must start at the baseline axial length (month 0)")
        _require(months[-1] <= MAX_MONTHS, "Follow-up longer than 20 years is not accepted")
        cells = series.get("source_cells", [])
        _require(len(cells) == len(values), "Each value needs its source cell reference")
        axial = _axial_values(kind, values, series)
        reference = axial[0]
        group = groups[series["group"]]
        normalized.append(
            {
                "series_id": series_id,
                "group": series["group"],
                "role": group["role"],
                "context": f"{record['species']}/{treatments[series['group']]}",
                "region": group["region"],
                "mean_age_years": group.get("mean_age_years"),
                "atropine_percent": group.get("atropine_percent"),
                "device": series["device"],
                "kind": series["kind"],
                "n": series.get("n"),
                "months": months,
                "changes_mm": [v - reference for v in axial],
                "reference_mm": reference,
                "value_kind": kind,
            }
        )
    _require(bool(normalized), "Record has no series")
    return {
        "study_id": study_id,
        "context": f"{record['species']}/{record['treatment']}",
        "contexts": sorted({s["context"] for s in normalized}),
        "split": record["split"],
        "series": normalized,
    }


def to_axial_series(record: Mapping[str, Any]) -> list[AxialSeries]:
    checked = validate_record(record)
    result = []
    for item in checked["series"]:
        aggregate = None if item["kind"] == "individual" else int(item["n"] or 0)
        result.append(
            AxialSeries(
                series_id=f"{checked['study_id']}/{item['series_id']}",
                study_id=checked["study_id"],
                context=item["context"],
                reference_mm=item["reference_mm"],
                months=tuple(item["months"][1:]),
                changes_mm=tuple(item["changes_mm"][1:]),
                aggregate_n=aggregate,
                role=item["role"],
                group=item["group"],
                region=item["region"],
                device=item["device"],
                mean_age_years=item["mean_age_years"],
                atropine_percent=item["atropine_percent"],
            )
        )
    return result


def structural_summary(records: Sequence[Mapping[str, Any]]) -> dict[str, Any]:
    """Counts and covariates only, so sealed records can be audited blind."""
    studies = []
    for record in records:
        checked = validate_record(record)
        series = checked["series"]
        studies.append(
            {
                "study_id": checked["study_id"],
                "split": checked["split"],
                "contexts": checked["contexts"],
                "groups": sorted({s["group"] for s in series}),
                "control_series": sum(s["role"] == "control" for s in series),
                "treated_series": sum(s["role"] == "treated" for s in series),
                "group_mean_series": sum(s["kind"] == "group_mean" for s in series),
                "regions": sorted({s["region"] for s in series}),
                "devices": sorted({s["device"] for s in series}),
                "value_kinds": sorted({s["value_kind"] for s in series}),
                "atropine_percents": sorted(
                    {s["atropine_percent"] for s in series if s["atropine_percent"] is not None}
                ),
                "mean_age_reported": sum(s["mean_age_years"] is not None for s in series),
                "follow_up_months": sorted({m for s in series for m in s["months"]}),
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
    return axial_number(text), f"{name}::{location}", text


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
