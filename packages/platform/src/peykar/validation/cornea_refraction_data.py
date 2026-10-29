"""Validated records of refraction after corneal laser refractive surgery.

A record (schema ``cornea.refraction-record-1.0.0``) keeps provenance, licence,
procedure context and per-group series of the spherical equivalent (SE, dioptres)
that start at the preoperative refraction. Values cite their source cells. This
module validates records, converts them to :class:`RefractionSeries` and builds
records from cell-citing table specs. It never downloads, fits or scores anything.

Refraction changes additively, so a series is the change from the preoperative SE,
``SE(t) − SE0`` in dioptres. The generic eye models work on the logarithm of a
fraction; a :class:`RefractionSeries` therefore exposes the change as the
pseudo-fraction ``exp((SE − SE0) / MODEL_UNIT_D)`` and the preoperative SE as the
pseudo-reference ``exp(SE0 / COVARIATE_UNIT_D)``, so that the models' log fraction
is the change in units of ``MODEL_UNIT_D`` and their level covariate
``ln(reference / 1)`` is ``SE0 / COVARIATE_UNIT_D``. Both maps are exact and
invertible; ``peykar.validation.eye_additive_evaluation`` turns every forecast back
into dioptres before it is scored.
"""

from __future__ import annotations

import hashlib
import math
import re
from collections.abc import Mapping, Sequence
from dataclasses import dataclass, field
from typing import Any

from peykar.validation.cornea_table_records import cell_text
from peykar.validation.eye_retina_data import thickness_number
from peykar_core.errors import InvalidRequest

SCHEMA_ID = "cornea.refraction-record"
SCHEMA_VERSION = "1.0.0"
SPECIES = {"human", "rabbit", "other"}
PROCEDURES = {"surface_ablation", "lasik", "lenticule_extraction", "other"}
REFRACTIVE_ERRORS = {"myopia", "hyperopia", "mixed_astigmatism"}
TARGETS = {"plano", "unstated"}
VALUE_KINDS = {"se_absolute", "se_change"}
METHODS = {"manifest", "cycloplegic", "objective", "unstated"}
ROLES = {"control", "treated"}
KINDS = {"individual", "group_mean"}
SPLITS = {"development", "sealed_test", "excluded"}
MAX_MONTHS = 600.0
MIN_SE_D = -25.0
MAX_SE_D = 15.0
# The generic models see the change from SE0 in units of MODEL_UNIT_D and the level
# covariate SE0 / COVARIATE_UNIT_D (see the module docstring).
MODEL_UNIT_D = 5.0
COVARIATE_UNIT_D = 5.0
_ID = re.compile(r"^[a-z0-9][a-z0-9._/-]{2,120}$")


def refraction_number(text: str) -> float:
    """Leading signed number of a cell (unicode minus, decimal comma, change sign)."""
    return thickness_number(text)


@dataclass(frozen=True)
class RefractionSeries:
    """One SE series as the change from its preoperative SE (dioptres), over months."""

    series_id: str
    study_id: str
    context: str
    reference_d: float
    months: tuple[float, ...]
    changes_d: tuple[float, ...]
    aggregate_n: int | None = None
    role: str = "control"
    group: str | None = None
    method: str = "unstated"
    refractive_error: str = "myopia"
    metadata: Mapping[str, Any] = field(default_factory=dict)

    def __post_init__(self) -> None:
        if len(self.months) != len(self.changes_d) or not self.months:
            raise InvalidRequest("Refraction series need paired months and changes")
        if list(self.months) != sorted(set(self.months)) or self.months[0] <= 0:
            raise InvalidRequest("Follow-up months must be positive and increasing")
        if not all(math.isfinite(v) for v in (self.reference_d, *self.changes_d)):
            raise InvalidRequest("Refraction values must be finite")

    @property
    def fractions(self) -> tuple[float, ...]:
        """The change as the pseudo-fraction ``exp(change / MODEL_UNIT_D)``."""
        return tuple(math.exp(c / MODEL_UNIT_D) for c in self.changes_d)

    @property
    def reference_value(self) -> float:
        """The preoperative SE as the pseudo-reference ``exp(SE0 / COVARIATE_UNIT_D)``."""
        return math.exp(self.reference_d / COVARIATE_UNIT_D)

    @property
    def is_aggregate(self) -> bool:
        return self.aggregate_n is not None

    @property
    def kind(self) -> str:
        return "aggregate" if self.is_aggregate else "individual"

    def prefix(self, last_month: float) -> RefractionSeries:
        keep = [i for i, month in enumerate(self.months) if month <= last_month]
        if not keep:
            raise InvalidRequest("A prefix needs at least one follow-up")
        return RefractionSeries(
            series_id=self.series_id,
            study_id=self.study_id,
            context=self.context,
            reference_d=self.reference_d,
            months=tuple(self.months[i] for i in keep),
            changes_d=tuple(self.changes_d[i] for i in keep),
            aggregate_n=self.aggregate_n,
            role=self.role,
            group=self.group,
            method=self.method,
            refractive_error=self.refractive_error,
            metadata=self.metadata,
        )


def change_from_fraction(value: float) -> float:
    """Inverse of the pseudo-fraction: the change from SE0 in dioptres."""
    return MODEL_UNIT_D * math.log(value)


def reference_from_value(value: float) -> float:
    """Inverse of the pseudo-reference: the preoperative SE in dioptres."""
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


def _se_in_range(value: float, name: str) -> None:
    _require(MIN_SE_D <= value <= MAX_SE_D, f"{name} must lie within -25..+15 D")


def validate_record(record: Mapping[str, Any]) -> dict[str, Any]:
    """Check one record; return normalized series (changes from the preoperative SE)."""
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
        _require(
            group.get("refractive_error") in REFRACTIVE_ERRORS,
            "Declare the group's refractive error (myopia, hyperopia or mixed_astigmatism)",
        )
        _require(group.get("target") in TARGETS, "Declare the target refraction (plano)")
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
        _require(series.get("method") in METHODS, "Declare the refraction method")
        months = [_finite(m, "month") for m in series.get("months", [])]
        values = [_finite(v, "value") for v in series.get("values", [])]
        _require(len(months) == len(values) >= 2, "Series need >=2 paired values")
        _require(months == sorted(set(months)), "Months must be unique and increasing")
        _require(months[0] == 0.0, "Series must start at the preoperative refraction (month 0)")
        _require(months[-1] <= MAX_MONTHS, "Follow-up longer than 50 years is not accepted")
        cells = series.get("source_cells", [])
        _require(len(cells) == len(values), "Each value needs its source cell reference")
        if kind == "se_absolute":
            for value in values:
                _se_in_range(value, "Spherical equivalent")
            reference = values[0]
            changes = [v - reference for v in values]
        else:
            _require(abs(values[0]) <= 1e-9, "The SE change at the reference must be zero")
            reference = _finite(series.get("reference_se"), "reference_se")
            _se_in_range(reference, "Preoperative spherical equivalent")
            _require(bool(series.get("reference_cell")), "SE changes need a cited reference")
            changes = list(values)
            for change in changes:
                _se_in_range(reference + change, "Spherical equivalent")
        group = groups[series["group"]]
        procedure = group.get("procedure", record["procedure"])
        normalized.append(
            {
                "series_id": series_id,
                "group": series["group"],
                "role": group["role"],
                "context": f"{record['species']}/{procedure}",
                "refractive_error": group["refractive_error"],
                "kind": series["kind"],
                "n": series.get("n"),
                "months": months,
                "changes_d": changes,
                "reference_d": reference,
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


def to_refraction_series(record: Mapping[str, Any]) -> list[RefractionSeries]:
    checked = validate_record(record)
    result = []
    for item in checked["series"]:
        aggregate = None if item["kind"] == "individual" else int(item["n"] or 0)
        result.append(
            RefractionSeries(
                series_id=f"{checked['study_id']}/{item['series_id']}",
                study_id=checked["study_id"],
                context=item["context"],
                reference_d=item["reference_d"],
                months=tuple(item["months"][1:]),
                changes_d=tuple(item["changes_d"][1:]),
                aggregate_n=aggregate,
                role=item["role"],
                group=item["group"],
                method=item["method"],
                refractive_error=item["refractive_error"],
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
                "refractive_errors": sorted({s["refractive_error"] for s in checked["series"]}),
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
            values.append(refraction_number(text))
            cited.append(f"{name}::{location}")
            texts.append(text)
        item = {k: v for k, v in series.items() if k != "cells"}
        item.update(values=values, source_cells=cited, source_cell_texts=texts)
        if series.get("reference_cell"):
            name, _, location = str(series["reference_cell"]).partition("::")
            if name not in files:
                raise InvalidRequest(f"Reference cites unknown source {name!r}")
            reference_text = cell_text(sources[name], location)
            item["reference_se"] = refraction_number(reference_text)
            item["reference_cell_text"] = reference_text
        series_out.append(item)
    record = {
        "schema": f"{SCHEMA_ID}-{SCHEMA_VERSION}",
        **{k: v for k, v in spec.items() if k not in {"series", "schema"}},
        "series": series_out,
    }
    validate_record(record)
    return record
