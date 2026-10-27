"""Inert IJMS2019 wound aggregates and an explicit post-injury forecast split.

These are published cohort means, not individual animals. Conditioning at24h
uses an observed post-injury state; later comparisons cannot establish an
initial-state-only prediction or grant independent biological validity.
"""

import math
import re
from hashlib import sha256
from typing import Any
from xml.etree import ElementTree as ET

from peykar_core.errors import InvalidRequest

DOI = "10.3390/ijms20071655"
TABLE_ID = "ijms-20-01655-t001"
GROUPS = ("Control", "s-PRGF", "NaHA", "s-PRGF + NaHA")
HOURS = (0, 24, 36, 48, 60, 72, 84)
DAY_HEADERS = ("Day 0", "Day 1", "Day 1.5", "Day 2", "Day 2.5", "Day 3", "Day 3.5")
JATS_DOCTYPE = (
    '<!DOCTYPE article\n  PUBLIC "-//NLM//DTD JATS (Z39.96) Journal Archiving and '
    'Interchange DTD with MathML3 v1.4 20241031//EN" "JATS-archivearticle1-4-mathml3.dtd">'
)
CAPTION = (
    "In vivo experiment assessing the progression of epithelial wound healing in rabbit "
    "eyes treated with s-PRGF, s-PRGF and NaHA, NaHA, and PBS as the control. "
    "The results are expressed as mean wound area ± standard deviation in mm2."
)


def _text(node: ET.Element) -> str:
    return " ".join("".join(node.itertext()).split())


def _xml(raw: bytes) -> ET.Element:
    if not raw or len(raw) > 4_000_000:
        raise InvalidRequest("IJMS2019 evidence requires bounded inert XML")
    try:
        text = raw.decode("utf-8-sig")
        # Remove only the retained publisher declaration; never resolve its DTD.
        text = text.replace(JATS_DOCTYPE, "", 1)
        if any(token in text.upper() for token in ("<!DOCTYPE", "<!ENTITY", "\x00")):
            raise ValueError("Unreviewed DTD/entities")
        root = ET.fromstring(text)
        if root.tag != "article" or any(
            node.tag.startswith("{http://www.w3.org/2001/XInclude}") for node in root.iter()
        ):
            raise ValueError("Expected an article without external includes")
        return root
    except (UnicodeError, ValueError, ET.ParseError) as error:
        raise InvalidRequest("IJMS2019 evidence requires bounded inert XML") from error


def ijms2019_wound_aggregates(raw: bytes) -> dict[str, Any]:
    """Read all28 Table1 cells with exact source coordinates and original units.

    The caption explicitly defines ± as standard deviation. A printed lone zero
    has no reported SD, so its SD remains None. Neither per-arm sample sizes nor
    animal IDs are recovered from this aggregate table.
    """
    root = _xml(raw)
    identifiers = root.findall(".//article-id[@pub-id-type='doi']")
    tables = root.findall(f".//table-wrap[@id='{TABLE_ID}']")
    if len(identifiers) != 1 or identifiers[0].text != DOI or len(tables) != 1:
        raise InvalidRequest("Expected the exact IJMS2019 DOI and Table1 identity")
    table = tables[0]
    caption = table.find("caption")
    if table.findtext("label") != "Table 1" or caption is None or _text(caption) != CAPTION:
        raise InvalidRequest("IJMS2019 Table1 caption/statistic/area units differ")
    headers = table.findall("./table/thead/tr")
    rows = table.findall("./table/tbody/tr")
    if (
        len(headers) != 2
        or len(headers[0]) != 2
        or tuple(_text(cell) for cell in headers[0]) != ("Treatment", "TIME (days)")
        or headers[0][0].get("rowspan") != "2"
        or headers[0][0].get("colspan", "1") != "1"
        or headers[0][1].get("colspan") != "7"
        or headers[0][1].get("rowspan", "1") != "1"
        or tuple(_text(cell) for cell in headers[1]) != DAY_HEADERS
        or any(cell.get("rowspan", "1") != "1" for cell in headers[1])
        or any(cell.get("colspan", "1") != "1" for cell in headers[1])
        or any(cell.tag != "th" for row in headers for cell in row)
        or len(rows) != 4
        or any(len(row) != 8 or any(cell.tag != "td" for cell in row) for row in rows)
        or tuple(_text(row[0]) for row in rows) != GROUPS
        or any(cell.get("rowspan", "1") != "1" for row in rows for cell in row)
        or any(cell.get("colspan", "1") != "1" for row in rows for cell in row)
    ):
        raise InvalidRequest("Original IJMS2019 treatment/time/cell layout differs")
    observations = []
    for row_index, (group, row) in enumerate(zip(GROUPS, rows, strict=True), start=1):
        for column, (hour, cell) in enumerate(zip(HOURS, list(row)[1:], strict=True), start=2):
            text = _text(cell)
            match = re.fullmatch(r"(\d+(?:\.\d+)?) ± (\d+(?:\.\d+)?)", text)
            if match is not None:
                mean, sd = float(match[1]), float(match[2])
            elif text == "0":
                mean, sd = 0.0, None
            else:
                raise InvalidRequest("IJMS2019 requires original numeric means and reported SD")
            if not math.isfinite(mean) or (sd is not None and not math.isfinite(sd)):
                raise InvalidRequest("IJMS2019 means and SD must be finite and nonnegative")
            observations.append(
                {
                    "source_table": TABLE_ID,
                    "source_body_row_1based": row_index,
                    "source_column_1based": column,
                    "source_cell": f"{TABLE_ID}:tbody/tr[{row_index}]/td[{column}]",
                    "group": group,
                    "hour": hour,
                    "raw_text": text,
                    "mean_area_mm2": mean,
                    "reported_sd_mm2": sd,
                    "unit": "millimeter ** 2",
                    "statistic": "cohort_mean",
                    "animal_ids": None,
                    "sample_n": None,
                }
            )
    return {
        "study_doi": DOI,
        "source_xml_sha256": sha256(raw).hexdigest(),
        "source_table": TABLE_ID,
        "unit": "millimeter ** 2",
        "observation": "calibrated fluorescein photographic projected wound area",
        "plus_minus_definition": "standard deviation, explicitly stated in Table1 caption",
        "individual_records_available": False,
        "per_arm_n_reported": False,
        "observations": observations,
        "independent_biological_validation": False,
    }


def normalize_ijms2019_area_ratios(raw: bytes) -> dict[str, Any]:
    """Add ratio-of-published-means values; retain every original area and SD.

    No individual normalization or uncertainty propagation can be inferred from
    aggregate means/SD without sample identities and longitudinal covariance.
    Above-baseline measurements remain above1 rather than being clipped.
    """
    evidence = ijms2019_wound_aggregates(raw)
    baselines = {
        row["group"]: row["mean_area_mm2"] for row in evidence["observations"] if row["hour"] == 0
    }
    if set(baselines) != set(GROUPS) or any(value <= 0 for value in baselines.values()):
        raise InvalidRequest("Each IJMS2019 group needs its own positive baseline mean")
    observations = [
        {
            **row,
            "baseline_mean_area_mm2": baselines[row["group"]],
            "mean_area_fraction": row["mean_area_mm2"] / baselines[row["group"]],
        }
        for row in evidence["observations"]
    ]
    if any(not math.isfinite(row["mean_area_fraction"]) for row in observations):
        raise InvalidRequest("IJMS2019 mean-area ratios must remain finite")
    return {
        **evidence,
        "normalization": "ratio_of_published_cohort_means",
        "normalized_unit": "dimensionless",
        "ratio_uncertainty_available": False,
        "observations": observations,
    }


def isolate_ijms2019_control_forecast(raw: bytes, *, cutoff_hour: int = 24) -> dict[str, Any]:
    """Separate exactly Control0/24h from its five reserved later measurements.

    Other treatment groups remain a separate retained object, never conditioning
    or Control validation. This function returns data only and executes no model.
    """
    if type(cutoff_hour) is not int or cutoff_hour != 24:
        raise InvalidRequest("The reviewed IJMS2019 forecast requires cutoff exactly24h")
    evidence = normalize_ijms2019_area_ratios(raw)
    metadata = {key: value for key, value in evidence.items() if key != "observations"}
    rows = evidence["observations"]
    return {
        "class": "postinjury_conditioned_forecast_data_not_initial_state_validation",
        "conditioning_cutoff_hour": cutoff_hour,
        "conditioning": {
            **metadata,
            "role": "conditioning_input_only",
            "cutoff_hour": cutoff_hour,
            "observations": [
                row for row in rows if row["group"] == "Control" and row["hour"] <= cutoff_hour
            ],
        },
        "future": {
            **metadata,
            "role": "reserved_future_control_evaluation_only",
            "cutoff_hour": cutoff_hour,
            "observations": [
                row for row in rows if row["group"] == "Control" and row["hour"] > cutoff_hour
            ],
        },
        "other_groups": {
            **metadata,
            "role": "retained_treatment_observations_not_control_forecast",
            "observations": [row for row in rows if row["group"] != "Control"],
        },
        "initial_state_only_prediction": False,
        "independent_biological_validation": False,
    }
