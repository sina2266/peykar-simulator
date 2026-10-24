"""Offline corneal observation operators; no execution or biological approval."""

import io
import math
import statistics
import zipfile
from collections.abc import Mapping, Sequence
from hashlib import sha256
from typing import Any
from xml.etree import ElementTree as ET

from peykar_core.errors import InvalidRequest

SHEET_NS = {"s": "http://schemas.openxmlformats.org/spreadsheetml/2006/main"}
JATS_DOCTYPE = (
    '<!DOCTYPE article PUBLIC "-//NLM//DTD JATS (Z39.96) Journal Publishing DTD '
    'v1.1d3 20150301//EN" '
    '"http://jats.nlm.nih.gov/publishing/1.1d3/JATS-journalpublishing1.dtd">'
)


def _xml(raw: bytes, *, publisher_jats: bool = False) -> ET.Element:
    if not raw or len(raw) > 4_000_000:
        raise InvalidRequest("Corneal evidence XML exceeds its bounded input size")
    try:
        text = raw.decode("utf-8-sig")
        if publisher_jats:
            # Only this literal external declaration is removed, without resolving it.
            text = text.replace(JATS_DOCTYPE, "", 1)
        if any(token in text.upper() for token in ["<!DOCTYPE", "<!ENTITY", "\x00"]):
            raise ValueError("Unreviewed DTD/entities")
        root = ET.fromstring(text)
        if any(e.tag.startswith("{http://www.w3.org/2001/XInclude}") for e in root.iter()):
            raise ValueError("External includes")
        return root
    except (UnicodeError, ValueError, ET.ParseError) as error:
        raise InvalidRequest("Corneal evidence requires bounded inert XML") from error


def rabbit_wound_observations(raw: bytes) -> list[dict[str, Any]]:
    """Read actual Fig2 measured cells from Abdel-Naby2017, not cached formulas.

    Area units are unreported in the workbook. Only within-rabbit ratios are
    dimensionless; no pixel-to-mm conversion or missing fourth rabbit is invented.
    """
    if not raw or len(raw) > 2_000_000:
        raise InvalidRequest("Rabbit wound workbook must be bounded")
    try:
        with zipfile.ZipFile(io.BytesIO(raw)) as archive:
            members = archive.infolist()
            names = [m.filename for m in members]
            if (
                len(names) != len(set(names))
                or len(names) > 100
                or sum(m.file_size for m in members) > 8_000_000
                or any(m.file_size > 4_000_000 for m in members)
            ):
                raise ValueError("Unexpected workbook inventory")
            strings = [
                "".join(node.itertext())
                for node in _xml(archive.read("xl/sharedStrings.xml")).findall("s:si", SHEET_NS)
            ]
            sheet = _xml(archive.read("xl/worksheets/sheet1.xml"))
    except (ValueError, KeyError, zipfile.BadZipFile) as error:
        raise InvalidRequest("Expected the original bounded Fig2 XLSX workbook") from error
    cells = {}
    for node in sheet.findall(".//s:c", SHEET_NS):
        reference = node.get("r")
        if reference in cells:
            raise InvalidRequest("Duplicate source worksheet cell")
        cells[reference] = node

    def value(reference: str) -> str:
        node = cells.get(reference)
        if node is None or node.find("s:f", SHEET_NS) is not None:
            raise InvalidRequest("Missing measured worksheet cell or formula in raw input")
        child = node.find("s:v", SHEET_NS)
        if child is None or child.text is None:
            raise InvalidRequest("Missing measured worksheet value")
        try:
            return strings[int(child.text)] if node.get("t") == "s" else child.text
        except (ValueError, IndexError) as error:
            raise InvalidRequest("Invalid worksheet shared-string reference") from error

    columns = "BCDEFGHIJK"
    if value("A1") != "Rabbit Wound Healing Data" or [
        value(f"{column}3") for column in columns
    ] != [str(t) for t in range(0, 55, 6)]:
        raise InvalidRequest("The original Fig2 identity/time layout differs")
    observations = []
    for header, first, group in [(6, 7, "Control"), (13, 14, "0.5% SDP"), (20, 21, "2% SDP")]:
        if value(f"A{header}") != group:
            raise InvalidRequest("Original measured group layout differs")
        for offset in range(3):
            row = first + offset
            if value(f"A{row}") != str(offset + 1):
                raise InvalidRequest("Original rabbit identifier differs")
            try:
                areas = [float(value(f"{column}{row}")) for column in columns]
            except ValueError as error:
                raise InvalidRequest("Measured rabbit area must be numeric") from error
            if not all(math.isfinite(a) and a >= 0 for a in areas) or areas[0] <= 0:
                raise InvalidRequest("Measured areas need a finite positive baseline")
            for hour, column, area in zip(range(0, 55, 6), columns, areas, strict=True):
                observations.append(
                    {
                        "group": group,
                        "replicate": str(offset + 1),
                        "hour": hour,
                        "raw_area": area,
                        "raw_unit": "unreported_source_image_area",
                        "wound_area_fraction": area / areas[0],
                        "closure_fraction": 1 - area / areas[0],
                        "source_cell": f"{column}{row}",
                    }
                )
    return observations


def human_oct_summary(raw: bytes) -> dict[str, Any]:
    """Recover the published NORMAL aggregates, never synthetic individual eyes."""
    root = _xml(raw, publisher_jats=True)
    if root.findtext(".//article-id[@pub-id-type='doi']") != "10.1371/journal.pone.0239124":
        raise InvalidRequest("Expected the original Buffault2020 article")

    def rows(identifier: str) -> list[list[str]]:
        table = root.find(f".//table-wrap[@id='{identifier}']")
        if table is None:
            raise InvalidRequest("Original clinical table is missing")
        return [
            ["".join(cell.itertext()).strip() for cell in row] for row in table.findall(".//tr")
        ]

    demographics = rows("pone.0239124.t001")
    measurements = rows("pone.0239124.t002")
    try:
        normal_demo = demographics[0].index("Normal")
        normal_measure = measurements[0].index("Normal")
        n = int(
            next(row for row in demographics if row[0] == "No. of eyes (subjects)")[normal_demo]
        )
        values = {}
        for row in measurements[1:]:
            mean, sd = [float(v) for v in row[normal_measure].replace(" ", "").split("±")]
            if not all(math.isfinite(v) and v >= 0 for v in [mean, sd]) or row[0] in values:
                raise ValueError("Invalid clinical aggregate")
            values[row[0]] = {"mean": mean, "sample_sd": sd}
        if n < 2 or set(values) != {
            "Central",
            "Inf",
            "Max",
            "Irregularity",
            "Inf−sup",
            "Min",
            "Sup",
        }:
            raise ValueError("Original normal clinical endpoints differ")
    except (ValueError, IndexError, StopIteration) as error:
        raise InvalidRequest("Original normal group/header/aggregate layout differs") from error
    return {
        "doi": "10.1371/journal.pone.0239124",
        "source_sha256": sha256(raw).hexdigest(),
        "group": "Normal",
        "independent_subjects": n,
        "unit": "micrometer",
        "observations": values,
        "individual_observations_available": False,
    }


def epithelial_column_geometry(
    lattice: Sequence[Sequence[int]], *, micrometres_per_site: float
) -> dict[str, Any]:
    """Vertical 2D epithelial span and occupancy; independent of absolute y origin.

    Missing columns stay null, internal holes are explicit, and the nominal scale
    is supplied by an operator. This is not calibrated OCT-normal thickness.
    """
    if (
        not lattice
        or not lattice[0]
        or len(lattice) * len(lattice[0]) > 1_000_000
        or type(micrometres_per_site) not in (int, float)
        or not math.isfinite(micrometres_per_site)
        or micrometres_per_site <= 0
        or not math.isfinite(micrometres_per_site * len(lattice[0]))
    ):
        raise InvalidRequest("Geometry needs a bounded lattice and positive finite spatial scale")
    ny = len(lattice[0])
    columns: list[dict[str, Any]] = []
    spans = []
    for x, column in enumerate(lattice):
        if len(column) != ny or any(type(k) is not int or not 0 <= k <= 10 for k in column):
            raise InvalidRequest("Geometry needs rectangular integer original cell-type fields")
        positions = [y for y, kind in enumerate(column) if kind in (1, 3, 4, 5)]
        span = positions[-1] - positions[0] + 1 if positions else None
        span_um = None if span is None else span * micrometres_per_site
        if span_um is not None:
            spans.append(span_um)
        columns.append(
            {
                "x_lattice": x,
                "vertical_span_um": span_um,
                "occupied_height_um": len(positions) * micrometres_per_site,
                "internal_non_epithelial_sites": 0 if span is None else span - len(positions),
            }
        )
    return {
        "operator": "cornea.vertical-epithelial-span-1.0.0",
        "unit": "micrometer",
        "micrometres_per_site": micrometres_per_site,
        "columns": columns,
        "occupied_columns": len(spans),
        "missing_columns": len(columns) - len(spans),
        "mean_vertical_span_um": statistics.mean(spans) if spans else None,
        "spatial_population_sd_um": statistics.pstdev(spans) if spans else None,
        "columns_with_internal_gaps": sum(c["internal_non_epithelial_sites"] > 0 for c in columns),
        "clinical_observation_equivalence": False,
    }


def context_disagreements(
    model: Mapping[str, str | None], data: Mapping[str, str | None]
) -> tuple[str, ...]:
    """Fail closed on unknown or mismatched biological/measurement declarations.

    Matching declarations alone do not verify provenance, independence or validity.
    """
    fields = ("species", "tissue", "condition", "exposure", "endpoint", "measurement", "region")
    unknown = {"", "unknown", "unreported", "unmapped", "not_reported"}
    return tuple(
        field
        for field in fields
        if (model.get(field) or "").strip().lower() in unknown
        or model.get(field) != data.get(field)
    )
