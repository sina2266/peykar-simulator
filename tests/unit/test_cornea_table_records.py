"""Cell-citing record builder for article-body tables (synthetic JATS only)."""

import hashlib
import math

import pytest

from peykar.validation.cornea_table_records import (
    build_record,
    cell_text,
    leading_number,
    table_rows,
)
from peykar_core.errors import InvalidRequest

XML = b"""<article><body><table-wrap id="t1"><table>
<thead><tr><th>Time</th><th>Vehicle</th><th>Drug</th></tr></thead>
<tbody>
<tr><td>0 h</td><td>12.0 &#177; 1.0</td><td>4.0 &#177; 0.5</td></tr>
<tr><td>24 h</td><td>6.0 &#177; 1.0</td><td>&#8722;1.0</td></tr>
<tr><td>48 h</td><td>0.0</td><td>n.d.</td></tr>
</tbody></table></table-wrap></body></article>"""


def spec(series):
    return {
        "study_id": "synthetic-table-study",
        "doi": "10.0000/synthetic",
        "context": "rat",
        "split": "development",
        "license": {
            "statement": "CC0",
            "url": "https://creativecommons.org/publicdomain/zero/1.0/",
        },
        "source_files": [{"name": "a.xml", "sha256": hashlib.sha256(XML).hexdigest()}],
        "wound": {
            "diameter_mm": 3.0,
            "basement_membrane": "unknown",
            "method": "burr",
            "injury_family": "epithelial_debridement",
        },
        "groups": [
            {"group": "vehicle", "role": "control", "diameter_from_area_cell": "a.xml::t1!R2C2"},
            {"group": "drug", "role": "treated"},
        ],
        "series": series,
    }


def test_rows_are_counted_across_thead_and_tbody():
    rows = table_rows(XML, "t1")
    assert rows[0] == ["Time", "Vehicle", "Drug"]
    assert cell_text(XML, "t1!R3C2") == "6.0 \u00b1 1.0"
    assert leading_number("\u22121.0") == -1.0
    with pytest.raises(InvalidRequest):
        leading_number("n.d.")
    with pytest.raises(InvalidRequest):
        cell_text(XML, "t1!R9C1")


def test_build_record_cites_cells_definitions_and_transforms():
    series = [
        {
            "series_id": "vehicle/mean",
            "group": "vehicle",
            "kind": "group_mean",
            "n": 5,
            "value_kind": "area_absolute",
            "hours": [0.0, 24.0, 48.0],
            "cells": ["a.xml::t1!R2C2", "a.xml::t1!R3C2", "a.xml::t1!R4C2"],
        },
        {
            "series_id": "drug/remaining",
            "group": "drug",
            "kind": "group_mean",
            "n": 5,
            "value_kind": "area_absolute",
            "hours": [0.0, 24.0],
            "cells": [
                {"transform": "100-minus", "cell": "a.xml::t1!R2C3"},
                {"transform": "100-minus", "cell": "a.xml::t1!R3C3"},
            ],
        },
        {
            "series_id": "drug/definitional",
            "group": "drug",
            "kind": "group_mean",
            "n": 5,
            "value_kind": "percent_remaining",
            "hours": [0.0, 24.0],
            "cells": [{"definition": "definition:caption", "value": 100.0}, "a.xml::t1!R3C2"],
        },
    ]
    record = build_record(spec(series), {"a.xml": XML})
    first, second, third = record["series"]
    assert first["values"] == [12.0, 6.0, 0.0]
    assert second["values"] == [96.0, 101.0]
    assert second["source_cells"][0] == "transform:100-minus:a.xml::t1!R2C3"
    assert third["source_cells"][0] == "definition:caption"
    vehicle = record["groups"][0]
    assert vehicle["diameter_mm"] == pytest.approx(2 * math.sqrt(12.0 / math.pi), abs=1e-4)


def test_build_record_rejects_changed_sources_and_unknown_transforms():
    series = [
        {
            "series_id": "vehicle/mean",
            "group": "vehicle",
            "kind": "group_mean",
            "n": 5,
            "value_kind": "area_absolute",
            "hours": [0.0, 24.0],
            "cells": ["a.xml::t1!R2C2", "a.xml::t1!R3C2"],
        }
    ]
    with pytest.raises(InvalidRequest):
        build_record(spec(series), {"a.xml": XML + b" "})
    series[0]["cells"][1] = {"transform": "log", "cell": "a.xml::t1!R3C2"}
    with pytest.raises(InvalidRequest):
        build_record(spec(series), {"a.xml": XML})
