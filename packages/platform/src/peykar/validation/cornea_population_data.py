"""Validated study records for the corneal wound-closure population dataset.

A study record is a JSON document produced by a reviewed extraction of one
publication's original source data. It keeps provenance, licence, species, wound
geometry and measurement context next to every series and source cell. This
module validates records and converts them into :class:`WoundSeries`; it never
downloads, fits or scores anything. ``structural_summary`` reports only counts
and covariates so sealed test studies can be audited without exposing outcomes.
"""

from __future__ import annotations

import math
import re
from collections.abc import Mapping, Sequence
from hashlib import sha256
from typing import Any

from peykar.model_learning.corneal_population import WoundSeries
from peykar_core.errors import InvalidRequest

SCHEMA_ID = "cornea.wound-study-record"
SCHEMA_VERSION = "1.0.0"
CONTEXTS = {"mouse", "rat", "rabbit", "pig", "dog", "human", "other"}
VALUE_KINDS = {
    "area_fraction",  # remaining open area / initial area
    "percent_remaining",  # 100 * remaining / initial
    "percent_healed",  # 100 * (1 - remaining / initial)
    "area_absolute",  # calibrated or uncalibrated area; normalized by its own hour 0
}
ROLES = {"control", "treated"}
KINDS = {"individual", "group_mean"}
BASEMENT_MEMBRANE = {"intact", "removed", "unknown"}
INJURY_FAMILIES = {
    "epithelial_debridement",  # mechanical/brush/blade/burr or alcohol/heptanol removal
    "alkali_burn",
    "keratectomy",  # stromal/Bowman removal, excimer PTK/PRK, lamellar
    "other",
}
ROLE_SPLITS = {"development", "sealed_test", "excluded"}
_ID = re.compile(r"^[a-z0-9][a-z0-9._/-]{2,120}$")


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


def validate_study(record: Mapping[str, Any]) -> dict[str, Any]:
    """Check one study record; return normalized per-series fractions and context."""
    _require(record.get("schema") == f"{SCHEMA_ID}-{SCHEMA_VERSION}", "Unknown study schema")
    study_id = record.get("study_id", "")
    _require(isinstance(study_id, str) and bool(_ID.match(study_id)), "Invalid study_id")
    _require(bool(record.get("doi")) or bool(record.get("source_url")), "Study needs DOI or URL")
    _require(record.get("context") in CONTEXTS, "Study context must be a declared species group")
    _require(record.get("split") in ROLE_SPLITS, "Study split must be declared")
    licence = record.get("license", {})
    _require(isinstance(licence, Mapping) and bool(licence.get("statement")), "Licence missing")
    files = record.get("source_files", [])
    _require(isinstance(files, list) and len(files) >= 1, "At least one source file is required")
    for item in files:
        _require(
            isinstance(item.get("sha256"), str) and len(item["sha256"]) == 64,
            "Each source file needs its SHA-256",
        )
    wound = record.get("wound", {})
    diameter = _finite(wound.get("diameter_mm"), "wound.diameter_mm")
    _require(0.3 <= diameter <= 12.0, "Wound diameter must be within 0.3..12 mm")
    _require(wound.get("basement_membrane") in BASEMENT_MEMBRANE, "Declare basement membrane")
    _require(bool(wound.get("method")), "Wound creation method is required")
    _require(wound.get("injury_family") in INJURY_FAMILIES, "Declare the injury family")
    groups = {g["group"]: g for g in record.get("groups", [])}
    _require(bool(groups), "At least one group is required")
    default_radius = diameter * 500.0
    measured = wound.get("measured_baseline_area_mm2")
    if measured is not None:
        default_radius = (
            math.sqrt(_finite(measured, "measured_baseline_area_mm2") / math.pi) * 1000.0
        )
    radii = {}
    for name, group in groups.items():
        _require(group.get("role") in ROLES, "Group role must be control or treated")
        radii[name] = default_radius
        if group.get("diameter_mm") is not None:
            override = _finite(group["diameter_mm"], "group.diameter_mm")
            _require(0.3 <= override <= 12.0, "Group wound diameter must be within 0.3..12 mm")
            radii[name] = override * 500.0
    normalized = []
    seen = set()
    for series in record.get("series", []):
        series_id = series.get("series_id", "")
        _require(series_id not in seen and bool(series_id), "Series identities must be unique")
        seen.add(series_id)
        _require(series.get("group") in groups, "Series references an undeclared group")
        _require(series.get("kind") in KINDS, "Series kind must be individual or group_mean")
        kind = series.get("value_kind")
        _require(kind in VALUE_KINDS, "Unknown value kind")
        hours = [_finite(h, "hour") for h in series.get("hours", [])]
        values = [_finite(v, "value") for v in series.get("values", [])]
        _require(len(hours) == len(values) >= 2, "Series need >=2 paired observations")
        _require(hours == sorted(set(hours)), "Series hours must be unique and increasing")
        _require(hours[0] == 0.0, "Series must start at the hour-0 normalizing observation")
        cells = series.get("source_cells", [])
        _require(len(cells) == len(values), "Each value needs its source cell reference")
        if kind == "area_fraction":
            fractions = values
        elif kind == "percent_remaining":
            fractions = [v / 100.0 for v in values]
        elif kind == "percent_healed":
            fractions = [1.0 - v / 100.0 for v in values]
        else:
            _require(values[0] > 0, "Absolute areas need a positive hour-0 area")
            fractions = [v / values[0] for v in values]
        _require(abs(fractions[0] - 1.0) <= 0.02, "Hour-0 value must normalize to one")
        _require(all(-0.02 <= f <= 3.0 for f in fractions), "Fractions must lie within 0..3")
        normalized.append(
            {
                "series_id": series_id,
                "group": series["group"],
                "role": groups[series["group"]]["role"],
                "kind": series["kind"],
                "n": series.get("n"),
                "hours": hours,
                "fractions": [max(0.0, f) for f in fractions],
                "baseline_observed": bool(series.get("baseline_observed", True)),
                "radius0_um": radii[series["group"]],
            }
        )
    _require(bool(normalized), "Study has no series")
    return {
        "study_id": study_id,
        "context": f"{record['context']}/{wound['injury_family']}",
        "species_group": record["context"],
        "split": record["split"],
        "radius0_um": default_radius,
        "series": normalized,
    }


def to_wound_series(record: Mapping[str, Any]) -> list[WoundSeries]:
    """Convert a validated record into population-model series (hour 0 excluded)."""
    checked = validate_study(record)
    result = []
    for item in checked["series"]:
        aggregate = None if item["kind"] == "individual" else int(item["n"] or 0)
        result.append(
            WoundSeries(
                series_id=f"{checked['study_id']}/{item['series_id']}",
                study_id=checked["study_id"],
                context=checked["context"],
                radius0_um=item["radius0_um"],
                hours=tuple(item["hours"][1:]),
                fractions=tuple(item["fractions"][1:]),
                aggregate_n=aggregate,
                role=item["role"],
                group=item["group"],
                baseline_observed=item["baseline_observed"],
            )
        )
    return result


def structural_summary(records: Sequence[Mapping[str, Any]]) -> dict[str, Any]:
    """Counts/covariates only, so sealed records can be audited without outcomes."""
    studies = []
    for record in records:
        checked = validate_study(record)
        studies.append(
            {
                "study_id": checked["study_id"],
                "split": checked["split"],
                "context": checked["context"],
                "radius0_um": round(checked["radius0_um"], 1),
                "groups": sorted({s["group"] for s in checked["series"]}),
                "control_series": sum(s["role"] == "control" for s in checked["series"]),
                "treated_series": sum(s["role"] == "treated" for s in checked["series"]),
                "individual_series": sum(s["kind"] == "individual" for s in checked["series"]),
                "group_mean_series": sum(s["kind"] == "group_mean" for s in checked["series"]),
                "observation_hours": sorted({h for s in checked["series"] for h in s["hours"]}),
                "record_sha256": sha256(repr(sorted(record.items())).encode()).hexdigest(),
            }
        )
    return {"schema": f"{SCHEMA_ID}-{SCHEMA_VERSION}", "studies": studies}
