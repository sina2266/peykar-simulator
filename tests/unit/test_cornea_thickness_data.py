"""Central corneal thickness records: validation, conversion and builds (synthetic)."""

import hashlib

import pytest

from peykar.validation.cornea_thickness_data import (
    ThicknessSeries,
    build_record,
    structural_summary,
    to_thickness_series,
    validate_record,
)
from peykar_core.errors import InvalidRequest

XML = b"""<article><body><table-wrap id="t3"><table>
<thead><tr><th>Time</th><th>Phaco</th><th>DMEK</th></tr></thead>
<tbody>
<tr><td>Preoperative</td><td>540 &#177; 30</td><td>680 &#177; 60</td></tr>
<tr><td>1 day</td><td>590 &#177; 40</td><td>610 &#177; 50</td></tr>
<tr><td>1 month</td><td>548 &#177; 31</td><td>530 &#177; 35</td></tr>
</tbody></table></table-wrap></body></article>"""


def base(series, **extra):
    record = {
        "schema": "cornea.central-thickness-record-1.0.0",
        "study_id": "synthetic-cct-study",
        "doi": "10.0000/cct",
        "species": "human",
        "procedure": "cataract",
        "split": "development",
        "license": {"statement": "CC0"},
        "source_files": [{"name": "a.xml", "sha256": hashlib.sha256(XML).hexdigest()}],
        "groups": [
            {"group": "phaco", "role": "control"},
            {"group": "dmek", "role": "control", "procedure": "dmek"},
        ],
        "series": series,
    }
    record.update(extra)
    return record


def series(group, kind, months, values, **extra):
    item = {
        "series_id": f"{group}/{kind}",
        "group": group,
        "kind": "group_mean",
        "n": 30,
        "value_kind": kind,
        "reference_kind": "patient_preoperative",
        "method": "ultrasound",
        "months": months,
        "values": values,
        "source_cells": ["x"] * len(values),
    }
    item.update(extra)
    return item


def test_value_kinds_normalize_to_reference_fractions():
    record = base(
        [
            series("phaco", "cct_absolute", [0, 0.0329, 1], [540, 594, 540]),
            series("dmek", "percent_change", [0, 1], [0, -20], reference_cct=680),
            series(
                "dmek",
                "cct_change",
                [0, 3],
                [0, -136],
                series_id="dmek/change",
                reference_cct=680,
                reference_cell="a.xml::t3!R2C3",
            ),
        ]
    )
    checked = validate_record(record)
    fractions = [s["fractions"] for s in checked["series"]]
    assert fractions[0] == pytest.approx([1.0, 1.1, 1.0])
    assert fractions[1] == pytest.approx([1.0, 0.8])
    assert fractions[2] == pytest.approx([1.0, 0.8])
    assert checked["contexts"] == ["human/cataract", "human/dmek"]
    converted = to_thickness_series(record)
    assert converted[0].months == (0.0329, 1.0) and converted[0].method == "ultrasound"
    assert converted[1].context == "human/dmek" and converted[1].aggregate_n == 30
    assert structural_summary([record])["studies"][0]["methods"] == ["ultrasound"]


def test_invalid_records_are_rejected():
    with pytest.raises(InvalidRequest):
        validate_record(base([series("phaco", "cct_absolute", [1, 3], [560, 545])]))
    with pytest.raises(InvalidRequest):
        validate_record(base([series("phaco", "cct_change", [0, 1], [0, 20])]))
    with pytest.raises(InvalidRequest):
        validate_record(base([series("phaco", "cct_absolute", [0, 1], [540, 545], method="?")]))
    with pytest.raises(InvalidRequest):
        validate_record(base([series("phaco", "cct_absolute", [0, 1], [250, 300])]))
    with pytest.raises(InvalidRequest):
        ThicknessSeries(
            series_id="s",
            study_id="x",
            context="human/pk",
            reference_um=550.0,
            months=(0.0,),
            fractions=(1.0,),
        )


def test_builder_reads_cited_cells_and_checks_hashes():
    spec = base(
        [
            {
                "series_id": "dmek/mean",
                "group": "dmek",
                "kind": "group_mean",
                "n": 30,
                "value_kind": "cct_absolute",
                "reference_kind": "patient_preoperative",
                "method": "oct",
                "months": [0, 0.0329, 1],
                "cells": ["a.xml::t3!R2C3", "a.xml::t3!R3C3", "a.xml::t3!R4C3"],
            }
        ]
    )
    record = build_record(spec, {"a.xml": XML})
    assert record["series"][0]["values"] == [680.0, 610.0, 530.0]
    with pytest.raises(InvalidRequest):
        build_record(spec, {"a.xml": XML + b" "})
