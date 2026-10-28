"""Build corneal wound-study records from cited cells of article-body tables.

Extraction protocol 1.1.0 admits the JATS XML of an article as a source file and
cites a value as ``<table-wrap id>!R<r>C<c>``: ``r`` counts every ``<tr>`` of the
table in document order (thead rows, then tbody rows) and ``c`` counts the cells
of that row, both from 1, without expanding spans. This module resolves such
references, reads the **leading number** of the cited cell (the mean of
"mean ± SD" text) and assembles a ``cornea.wound-study-record-1.0.0`` document
from an explicit extraction spec. It never chooses cells, fits or scores.
"""

from __future__ import annotations

import hashlib
import re
import xml.etree.ElementTree as ET
from collections.abc import Mapping
from typing import Any

from peykar.validation.cornea_population_data import SCHEMA_ID, SCHEMA_VERSION, validate_study
from peykar_core.errors import InvalidRequest

_REFERENCE = re.compile(r"^(?P<table>[^!]+)!R(?P<row>\d+)C(?P<col>\d+)$")
_LEADING = re.compile(r"^\s*([-+]?\d+(?:[.,]\d+)?(?:[eE][-+]?\d+)?)")
MAX_XML_BYTES = 20_000_000


def _text(element: ET.Element) -> str:
    return " ".join("".join(element.itertext()).split())


def table_rows(xml: bytes, table_id: str) -> list[list[str]]:
    """All rows of one ``<table-wrap>`` as cell texts, in document order."""
    if len(xml) > MAX_XML_BYTES:
        raise InvalidRequest("Article XML is too large")
    root = ET.fromstring(xml)
    for wrap in root.iter("table-wrap"):
        if wrap.get("id") == table_id:
            return [[_text(cell) for cell in list(tr)] for tr in wrap.iter("tr")]
    raise InvalidRequest(f"Table {table_id!r} not found")


def cell_text(xml: bytes, reference: str) -> str:
    match = _REFERENCE.match(reference)
    if match is None:
        raise InvalidRequest(f"Invalid table reference {reference!r}")
    rows = table_rows(xml, match["table"])
    row, col = int(match["row"]), int(match["col"])
    if not 1 <= row <= len(rows) or not 1 <= col <= len(rows[row - 1]):
        raise InvalidRequest(f"Reference {reference!r} is outside the table")
    return rows[row - 1][col - 1]


def leading_number(text: str) -> float:
    """The first number of a cell (unicode minus accepted); no number is an error."""
    cleaned = text.replace("\u2212", "-").replace("\u2013", "-").strip()
    match = _LEADING.match(cleaned)
    if match is None:
        raise InvalidRequest(f"Cell {text!r} does not start with a number")
    return float(match.group(1).replace(",", "."))


def build_record(spec: Mapping[str, Any], sources: Mapping[str, bytes]) -> dict[str, Any]:
    """Assemble and validate a study record from a cell-citing extraction spec.

    ``sources`` maps each ``source_files[].name`` to its bytes; every SHA-256 in the
    spec must match. A series entry lists ``cells`` (``file::reference``) or, for a
    definitional baseline, ``definition:<text>`` with an explicit ``value``.
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
            if isinstance(entry, Mapping) and entry.get("transform") is not None:
                # Protocol 1.1.1: remaining percentage of a measured percent-healed cell.
                if entry["transform"] != "100-minus":
                    raise InvalidRequest("Only the declared 100-minus transform is allowed")
                name, _, reference = str(entry["cell"]).partition("::")
                if name not in files:
                    raise InvalidRequest(f"Series cites unknown source {name!r}")
                text = cell_text(sources[name], reference)
                values.append(100.0 - leading_number(text))
                cited.append(f"transform:100-minus:{name}::{reference}")
                texts.append(text)
                continue
            if isinstance(entry, Mapping):
                reference = str(entry["definition"])
                if not reference.startswith("definition:"):
                    raise InvalidRequest("Explicit values must cite a definition")
                values.append(float(entry["value"]))
                cited.append(reference)
                texts.append("")
                continue
            name, _, reference = str(entry).partition("::")
            if name not in files:
                raise InvalidRequest(f"Series cites unknown source {name!r}")
            text = cell_text(sources[name], reference)
            values.append(leading_number(text))
            cited.append(f"{name}::{reference}")
            texts.append(text)
        item = {k: v for k, v in series.items() if k != "cells"}
        item.update(values=values, source_cells=cited, source_cell_texts=texts)
        series_out.append(item)
    groups = []
    for group in spec["groups"]:
        item = dict(group)
        source = item.pop("diameter_from_area_cell", None)
        if source is not None:
            # Equal-area circle of a measured hour-0 mean area in mm^2.
            name, _, reference = str(source).partition("::")
            if name not in files:
                raise InvalidRequest(f"Group cites unknown source {name!r}")
            area = leading_number(cell_text(sources[name], reference))
            if area <= 0:
                raise InvalidRequest("A baseline area must be positive")
            item["diameter_mm"] = round(2.0 * (area / 3.141592653589793) ** 0.5, 4)
            item["diameter_basis"] = (
                f"equal-area circle of measured hour-0 area {name}::{reference}"
            )
        groups.append(item)
    record = {
        "schema": f"{SCHEMA_ID}-{SCHEMA_VERSION}",
        **{k: v for k, v in spec.items() if k not in {"series", "schema", "groups"}},
        "groups": groups,
        "series": series_out,
    }
    validate_study(record)
    return record
