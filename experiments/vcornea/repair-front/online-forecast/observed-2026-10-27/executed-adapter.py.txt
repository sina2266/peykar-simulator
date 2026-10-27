"""Separate original corneal series for causal sequential observation updates.

This adapter reads three reviewed source formats. It performs no fitting,
forecasting, scoring or biological approval. Callers must pin the returned source
digests in their own frozen experiment: matching a table layout is not a source
checksum certification.
"""

import math
from hashlib import sha256
from typing import Any

from peykar.validation.cornea_evidence import (
    rabbit_brush_aggregates,
    rabbit_wound_observations,
)
from peykar.validation.cornea_forecast_evidence import normalize_ijms2019_area_ratios
from peykar_core.errors import InvalidRequest

ADAPTER_ID = "cornea.online-source-series-1.0.0"


def _point(row: dict[str, Any], *, fraction: float, source_cells: list[str]) -> dict[str, Any]:
    if not math.isfinite(fraction) or fraction < 0:
        raise InvalidRequest("Corneal online measurements require finite nonnegative ratios")
    return {
        "hour": row["hour"],
        "area_fraction": fraction,
        "unit": "dimensionless",
        "measurement_origin": "observed",
        "eligible_for_conditioning": True,
        "source_cells": list(dict.fromkeys(source_cells)),
        "original": dict(row),
    }


def _rabbit2017_series(raw: bytes) -> list[dict[str, Any]]:
    rows = rabbit_wound_observations(raw)
    groups = ("Control", "0.5% SDP", "2% SDP")
    series = []
    for group, slug, dose in zip(
        groups, ("control", "sdp-0.5", "sdp-2"), (0.0, 0.5, 2.0), strict=True
    ):
        for replicate in ("1", "2", "3"):
            original = [r for r in rows if r["group"] == group and r["replicate"] == replicate]
            baseline = original[0]
            baseline_cell = baseline["source_cell"]
            series.append(
                {
                    "series_id": f"abdel-naby2017/{slug}/replicate-{replicate}",
                    "study_doi": "10.1371/journal.pone.0188154",
                    "group": group,
                    "replicate": replicate,
                    "series_nature": "individual_rabbit",
                    "normalization": "within_rabbit_area_divided_by_its_measured_baseline",
                    "source_sha256": sha256(raw).hexdigest(),
                    "source_format": "original_figure2_xlsx",
                    "baseline_radius_um": 3500.0,
                    "radius_basis": "reported_nominal_7mm_diameter_not_image_area_calibration",
                    "baseline_source_cells": [baseline_cell],
                    "baseline_raw_area": baseline["raw_area"],
                    "baseline_raw_unit": baseline["raw_unit"],
                    "context": {
                        "species": "Oryctolagus cuniculus",
                        "region": "central corneal epithelium",
                        "injury": "7mm nominal Bard-Parker blade epithelial debridement",
                        "care": "PBS vehicle plus topical moxifloxacin at6h intervals",
                        "sdp_percent_weight_per_volume": dose,
                        "raw_area_unit": "unreported_source_image_area",
                        "sample_count_note": (
                            "Three measured workbook rabbits per group; caption n4 remains "
                            "unresolved. Replicate labels1/2/3 repeat across groups."
                        ),
                        "observed_rabbits_in_group": 3,
                        "caption_group_n": 4,
                        "independent_subject_identity_beyond_workbook_label": None,
                        "initial_basement_membrane_preservation_independently_verified": False,
                    },
                    "numeric_observations": [
                        _point(
                            row,
                            fraction=row["wound_area_fraction"],
                            source_cells=[row["source_cell"], baseline_cell],
                        )
                        for row in original
                    ],
                    "categorical_observations": [],
                }
            )
    return series


def _ijms2019_series(raw: bytes) -> list[dict[str, Any]]:
    evidence = normalize_ijms2019_area_ratios(raw)
    rows = evidence["observations"]
    series = []
    for group, slug in zip(
        ("Control", "s-PRGF", "NaHA", "s-PRGF + NaHA"),
        ("control", "s-prgf", "naha", "s-prgf-naha"),
        strict=True,
    ):
        original = [row for row in rows if row["group"] == group]
        baseline = original[0]
        baseline_area = baseline["mean_area_mm2"]
        series.append(
            {
                "series_id": f"suarez-barrio2019/{slug}",
                "study_doi": evidence["study_doi"],
                "group": group,
                "replicate": None,
                "series_nature": "published_cohort_mean",
                "normalization": "ratio_of_published_cohort_means",
                "source_sha256": evidence["source_xml_sha256"],
                "source_format": "original_jats_table1",
                "baseline_radius_um": math.sqrt(baseline_area / math.pi) * 1000.0,
                "radius_basis": "equivalent_circle_of_observed_baseline_cohort_mean_mm2",
                "baseline_source_cells": [baseline["source_cell"]],
                "baseline_raw_area": baseline_area,
                "baseline_raw_unit": "millimeter ** 2",
                "context": {
                    "species": "Oryctolagus cuniculus",
                    "region": "central corneal epithelium, limbus spared",
                    "injury": "9mm nominal trephine mark and ophthalmic blade scraping",
                    "nominal_diameter_mm": 9.0,
                    "care": (
                        "All groups receive dexamethasone/chloramphenicol ointment and "
                        "diclofenac drops twice daily until epithelial closure"
                    ),
                    "sample_count_note": (
                        "33 operated eyes from17 female rabbits; contralateral operation "
                        "2–3weeks after first-eye recovery. Per-arm n and IDs unavailable."
                    ),
                    "study_rabbits": 17,
                    "study_operated_eyes": 33,
                    "per_arm_eye_n": None,
                    "per_arm_independent_rabbit_n": None,
                    "uncertainty_definition": evidence["plus_minus_definition"],
                    "reported_sd_unit": "millimeter ** 2",
                    "ratio_uncertainty_available": False,
                    "time_reporting_note": (
                        "Table1 includes half-day times despite once-daily photography "
                        "in methods; retain printed times without interpolation."
                    ),
                    "initial_basement_membrane_preservation_independently_verified": False,
                },
                "numeric_observations": [
                    _point(
                        row,
                        fraction=row["mean_area_fraction"],
                        source_cells=[row["source_cell"], baseline["source_cell"]],
                    )
                    for row in original
                ],
                "categorical_observations": [],
            }
        )
    return series


def _chung2021_series(raw: bytes) -> list[dict[str, Any]]:
    evidence = rabbit_brush_aggregates(raw)
    rows = evidence["observations"]
    series = []
    for column, group in enumerate(("Blade", "Ocu", "Amo"), start=2):
        numeric: list[dict[str, Any]] = [
            {
                "hour": 0,
                "area_fraction": 1.0,
                "unit": "dimensionless",
                "measurement_origin": "derived_normalization_baseline",
                "eligible_for_conditioning": False,
                "source_cells": [],
                "original": None,
                "derivation": (
                    "The publication divides later defect area by immediate defect area; "
                    "A0/A0=1 is a normalization identity, not a printed numeric measurement."
                ),
            }
        ]
        categorical = []
        for row_index, row in enumerate([r for r in rows if r["group"] == group], start=1):
            cell = f"tab1:tbody/tr[{row_index}]/td[{column}]"
            if row["kind"] == "categorical":
                categorical.append(
                    {
                        "hour": row["hour"],
                        "reported": row["reported"],
                        "measurement_origin": "observed_categorical",
                        "eligible_for_conditioning": False,
                        "source_cells": [cell, f"tab1:tbody/tr[5]/td[{column}]"],
                        "original": dict(row),
                    }
                )
            else:
                numeric.append(_point(row, fraction=row["mean_fraction"], source_cells=[cell]))
        series.append(
            {
                "series_id": f"chung2021/{group.lower()}",
                "study_doi": evidence["doi"],
                "group": group,
                "replicate": None,
                "series_nature": "published_cohort_aggregate",
                "normalization": "published_wound_size_ratio_to_immediate_defect_area",
                "source_sha256": evidence["source_sha256"],
                "source_format": "original_jats_table1",
                "baseline_radius_um": 3000.0,
                "radius_basis": "reported_approximately_6mm_nominal_diameter",
                "baseline_source_cells": [],
                "baseline_raw_area": None,
                "baseline_raw_unit": None,
                "context": {
                    "species": "Oryctolagus cuniculus",
                    "region": "central corneal epithelium",
                    "injury": f"approximately6mm {group} mechanical removal",
                    "care": (
                        "Blade defects washed with normal saline; "
                        "PBS/moxifloxacin equivalence unknown"
                    ),
                    "normalization_aggregation_details": (
                        "Published ratios; individual denominators and longitudinal "
                        "identities unavailable. Do not infer ratio-of-means equivalence."
                    ),
                    "sample_count_note": evidence["sample_count_note"],
                    "per_time_independent_rabbit_n": None,
                    "per_time_eye_n": None,
                    "uncertainty_definition": evidence["plus_minus_definition"],
                    "reported_plus_minus_unit": "dimensionless",
                    "categorical_detection_limit": None,
                    "initial_basement_membrane_preservation_independently_verified": False,
                },
                "numeric_observations": numeric,
                "categorical_observations": categorical,
            }
        )
    return series


def corneal_online_series(
    *, rabbit2017: bytes, ijms2019: bytes, chung2021: bytes
) -> dict[str, Any]:
    """Adapt all16 series without mixing animals, cohorts or source statistics.

    ``eligible_for_conditioning`` means a real numeric measurement supplies the
    ratio, even when division by its baseline is computed by a parser. The2021
    baseline alone is an arithmetic identity and is explicitly ineligible. A
    sequential predictor must still enforce its own causal prefix and require
    at least two eligible numeric measurements before forming a local forecast.
    Categorical healing statements cannot supply a numeric zero or a slope.
    """
    series = [
        *_rabbit2017_series(rabbit2017),
        *_ijms2019_series(ijms2019),
        *_chung2021_series(chung2021),
    ]
    if len(series) != 16 or len({s["series_id"] for s in series}) != 16:
        raise InvalidRequest("Corneal online evidence requires all16 distinct original series")
    for item in series:
        numeric = item["numeric_observations"]
        hours = [row["hour"] for row in numeric]
        if hours != sorted(set(hours)):
            raise InvalidRequest("Original corneal observations must have distinct ordered times")
        item["real_numeric_observation_count"] = sum(
            row["eligible_for_conditioning"] for row in numeric
        )
    return {
        "adapter": ADAPTER_ID,
        "series": series,
        "unit": "dimensionless",
        "time_unit": "hour",
        "source_hashes": {
            "rabbit2017": sha256(rabbit2017).hexdigest(),
            "ijms2019": sha256(ijms2019).hexdigest(),
            "chung2021": sha256(chung2021).hexdigest(),
        },
        "dataset_role": "previously_consumed_retrospective_evaluation_evidence",
        "initial_state_only_prediction": False,
        "independent_biological_validation": False,
    }
