"""Validated records of corrected visual acuity after treatment of macular disease.

A record (schema ``eye.retina-acuity-record-1.0.0``) keeps provenance, licence,
indication, treatment context and per-group series of best-corrected visual acuity
that start at the pretreatment acuity. Values cite their source cells and are logMAR
or ETDRS letter scores as printed. The vocabularies of indication, treatment, regimen
and prior treatment are those of the retinal-thickness records
(``eye_retina_data``), so the two components describe the same eyes alike; the units,
the letter conversion ``logMAR = (85 − letters) / 50`` and the cell readers are those
of the corneal acuity records (``cornea_acuity_data``). This module validates records,
converts them to :class:`RetinaAcuitySeries` and builds records from cell-citing table
specs. It never downloads, fits or scores anything.

As for corneal acuity, a series is the change from the pretreatment acuity in logMAR
(negative is better), handed to the generic eye models as the pseudo-fraction
``exp(change / 1 logMAR)`` with the pseudo-reference ``exp(logMAR0 / 1 logMAR)``;
``peykar.validation.eye_additive_evaluation`` scores forecasts in ETDRS letters.
Contexts are ``human/<indication>/<treatment>``.
"""

from __future__ import annotations

import hashlib
import math
import re
from collections.abc import Mapping, Sequence
from dataclasses import dataclass
from typing import Any

from peykar.validation.cornea_acuity_data import (
    COVARIATE_UNIT_LOGMAR,
    MODEL_UNIT_LOGMAR,
    REPORT_UNIT_LOGMAR,
    AcuitySeries,
    acuity_number,
    letters_to_logmar,
)
from peykar.validation.cornea_table_records import cell_text
from peykar.validation.eye_retina_data import INDICATIONS, PRIOR_TREATMENT, REGIMENS, TREATMENTS
from peykar_core.errors import InvalidRequest

__all__ = [
    "COVARIATE_UNIT_LOGMAR",
    "MODEL_UNIT_LOGMAR",
    "REPORT_UNIT_LOGMAR",
    "RetinaAcuitySeries",
    "build_record",
    "context_of",
    "structural_summary",
    "to_retina_acuity_series",
    "validate_record",
]

SCHEMA_ID = "eye.retina-acuity-record"
SCHEMA_VERSION = "1.0.0"
SPECIES = {"human"}
VALUE_KINDS = {"logmar_absolute", "letters_absolute", "logmar_change", "letters_change"}
ROLES = {"control", "treated"}
KINDS = {"individual", "group_mean"}
SPLITS = {"development", "sealed_test", "excluded"}
MISSING_DATA = {"observed", "locf", "unstated"}
MAX_MONTHS = 600.0
MIN_LOGMAR = -0.5
MAX_LOGMAR = 4.0
MIN_LETTERS = 0.0
MAX_LETTERS = 100.0
_ID = re.compile(r"^[a-z0-9][a-z0-9._/-]{2,120}$")


@dataclass(frozen=True)
class RetinaAcuitySeries(AcuitySeries):
    """One corrected-acuity series after treatment of macular disease, as the change
    from its pretreatment logMAR (``indication`` as in the record, plus the treatment,
    regimen and prior treatment of the series)."""

    treatment: str = "other"
    regimen: str = "unstated"
    prior_treatment: str = "unstated"


def context_of(species: str, indication: str, treatment: str) -> str:
    return f"{species}/{indication}/{treatment}"


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


def _logmar_in_range(value: float, name: str) -> None:
    _require(MIN_LOGMAR <= value <= MAX_LOGMAR, f"{name} must lie within -0.5..4.0 logMAR")


def _letters_in_range(value: float, name: str) -> None:
    _require(MIN_LETTERS <= value <= MAX_LETTERS, f"{name} must lie within 0..100 letters")


def _logmar_values(kind: str, values: list[float], series: Mapping[str, Any]) -> list[float]:
    """The acuity in logMAR at every time of a validated series."""
    if kind == "logmar_absolute":
        for value in values:
            _logmar_in_range(value, "Visual acuity")
        return list(values)
    if kind == "letters_absolute":
        for value in values:
            _letters_in_range(value, "Letter score")
        return [letters_to_logmar(v) for v in values]
    _require(abs(values[0]) <= 1e-9, "The acuity change at the reference must be zero")
    _require(bool(series.get("reference_cell")), "Acuity changes need a cited reference")
    reference = _finite(series.get("reference_value"), "reference_value")
    if kind == "logmar_change":
        _logmar_in_range(reference, "Pretreatment visual acuity")
        result = [reference + v for v in values]
        for value in result:
            _logmar_in_range(value, "Visual acuity")
        return result
    _letters_in_range(reference, "Pretreatment letter score")
    letters = [reference + v for v in values]
    for value in letters:
        _letters_in_range(value, "Letter score")
    return [letters_to_logmar(v) for v in letters]


def validate_record(record: Mapping[str, Any]) -> dict[str, Any]:
    """Check one record; return normalized series (changes from the pretreatment logMAR)."""
    _require(record.get("schema") == f"{SCHEMA_ID}-{SCHEMA_VERSION}", "Unknown record schema")
    study_id = record.get("study_id", "")
    _require(isinstance(study_id, str) and bool(_ID.match(study_id)), "Invalid study_id")
    _require(bool(record.get("doi")) or bool(record.get("source_url")), "Study needs DOI or URL")
    _require(record.get("species") in SPECIES, "Only human eyes are in scope")
    _require(record.get("indication") in INDICATIONS, "Declare the indication")
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
    for group in groups.values():
        _require(group.get("role") in ROLES, "Group role must be control or treated")
        _require(
            group.get("indication", record["indication"]) in INDICATIONS,
            "Group indication must be declared",
        )
        _require(
            group.get("treatment", record["treatment"]) in TREATMENTS,
            "Group treatment must be a declared context",
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
        _require(series.get("regimen") in REGIMENS, "Declare the treatment regimen")
        _require(series.get("prior_treatment") in PRIOR_TREATMENT, "Declare prior treatment")
        _require(series.get("missing_data") in MISSING_DATA, "Declare how missing visits count")
        months = [_finite(m, "month") for m in series.get("months", [])]
        values = [_finite(v, "value") for v in series.get("values", [])]
        _require(len(months) == len(values) >= 2, "Series need >=2 paired values")
        _require(months == sorted(set(months)), "Months must be unique and increasing")
        _require(months[0] == 0.0, "Series must start at the pretreatment acuity (month 0)")
        _require(months[-1] <= MAX_MONTHS, "Follow-up longer than 50 years is not accepted")
        cells = series.get("source_cells", [])
        _require(len(cells) == len(values), "Each value needs its source cell reference")
        logmar = _logmar_values(kind, values, series)
        reference = logmar[0]
        group = groups[series["group"]]
        indication = group.get("indication", record["indication"])
        treatment = group.get("treatment", record["treatment"])
        normalized.append(
            {
                "series_id": series_id,
                "group": series["group"],
                "role": group["role"],
                "context": context_of(record["species"], indication, treatment),
                "indication": indication,
                "treatment": treatment,
                "regimen": series["regimen"],
                "prior_treatment": series["prior_treatment"],
                "missing_data": series["missing_data"],
                "kind": series["kind"],
                "n": series.get("n"),
                "months": months,
                "changes_logmar": [v - reference for v in logmar],
                "reference_logmar": reference,
                "value_kind": kind,
            }
        )
    _require(bool(normalized), "Record has no series")
    return {
        "study_id": study_id,
        "context": context_of(record["species"], record["indication"], record["treatment"]),
        "contexts": sorted({s["context"] for s in normalized}),
        "split": record["split"],
        "series": normalized,
    }


def to_retina_acuity_series(record: Mapping[str, Any]) -> list[RetinaAcuitySeries]:
    checked = validate_record(record)
    result = []
    for item in checked["series"]:
        aggregate = None if item["kind"] == "individual" else int(item["n"] or 0)
        result.append(
            RetinaAcuitySeries(
                series_id=f"{checked['study_id']}/{item['series_id']}",
                study_id=checked["study_id"],
                context=item["context"],
                reference_logmar=item["reference_logmar"],
                months=tuple(item["months"][1:]),
                changes_logmar=tuple(item["changes_logmar"][1:]),
                aggregate_n=aggregate,
                role=item["role"],
                group=item["group"],
                indication=item["indication"],
                treatment=item["treatment"],
                regimen=item["regimen"],
                prior_treatment=item["prior_treatment"],
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
                "regimens": sorted({s["regimen"] for s in series}),
                "prior_treatment": sorted({s["prior_treatment"] for s in series}),
                "missing_data": sorted({s["missing_data"] for s in series}),
                "value_kinds": sorted({s["value_kind"] for s in series}),
                "follow_up_months": sorted({m for s in series for m in s["months"]}),
            }
        )
    return {"schema": f"{SCHEMA_ID}-{SCHEMA_VERSION}", "studies": studies}


def _read_cell(
    entry: object, files: Mapping[str, Any], sources: Mapping[str, bytes]
) -> tuple[float, str, str]:
    """(value, citation, cell text) of a ``file::table!RrCc`` string or a
    ``{"cell": ..., "take": "parenthesized"}`` mapping."""
    take = "leading"
    if isinstance(entry, Mapping):
        take = str(entry.get("take", "leading"))
        entry = entry["cell"]
    name, _, location = str(entry).partition("::")
    if name not in files:
        raise InvalidRequest(f"Series cites unknown source {name!r}")
    text = cell_text(sources[name], location)
    citation = f"{name}::{location}" + ("" if take == "leading" else f"#{take}")
    return acuity_number(text, take), citation, text


def build_record(spec: Mapping[str, Any], sources: Mapping[str, bytes]) -> dict[str, Any]:
    """Assemble and validate a record from a spec citing ``file::table!RrCc`` cells.

    A cell entry is a ``file::table!RrCc`` string, a mapping ``{"cell": <that string>,
    "take": "parenthesized"}`` for a value printed in parentheses, or
    ``{"definition": "definition:<where>", "value": 0}`` for the reference of a change
    series (protocol: the change at time 0 is 0).
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
