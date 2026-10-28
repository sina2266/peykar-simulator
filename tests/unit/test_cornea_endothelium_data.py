"""Endothelial-density records: validation, conversion and cell-citing builds (synthetic)."""

import hashlib

import pytest

from peykar.validation.cornea_endothelium_data import (
    DensitySeries,
    build_record,
    structural_summary,
    to_density_series,
    validate_record,
)
from peykar_core.errors import InvalidRequest

XML = b"""<article><body><table-wrap id="t2"><table>
<thead><tr><th>Time</th><th>DSAEK</th><th>DMEK</th></tr></thead>
<tbody>
<tr><td>Donor</td><td>2,650 &#177; 210</td><td>2,700 &#177; 180</td></tr>
<tr><td>6 months</td><td>1,700 &#177; 400</td><td>1,600 &#177; 420</td></tr>
<tr><td>12 months</td><td>1,550 &#177; 410</td><td>1,500 &#177; 400</td></tr>
</tbody></table></table-wrap></body></article>"""


def base(series, **extra):
    record = {
        "schema": "cornea.endothelial-density-record-1.0.0",
        "study_id": "synthetic-ecd-study",
        "doi": "10.0000/ecd",
        "species": "human",
        "procedure": "dsaek",
        "split": "development",
        "license": {"statement": "CC0"},
        "source_files": [{"name": "a.xml", "sha256": hashlib.sha256(XML).hexdigest()}],
        "groups": [
            {"group": "dsaek", "role": "control"},
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
        "n": 20,
        "value_kind": kind,
        "reference_kind": "donor_eye_bank",
        "months": months,
        "values": values,
        "source_cells": ["x"] * len(values),
    }
    item.update(extra)
    return item


def test_value_kinds_normalize_to_reference_fractions():
    record = base(
        [
            series("dsaek", "ecd_absolute", [0, 6, 12], [2500, 1750, 1500]),
            series("dmek", "percent_loss", [0, 6], [0, 30], reference_ecd=2600),
            series(
                "dmek",
                "ecd_change",
                [0, 12],
                [0, -1300],
                series_id="dmek/change",
                reference_ecd=2600,
                reference_cell="a.xml::t2!R2C3",
            ),
        ]
    )
    checked = validate_record(record)
    fractions = [s["fractions"] for s in checked["series"]]
    assert fractions[0] == pytest.approx([1.0, 0.7, 0.6])
    assert fractions[1] == pytest.approx([1.0, 0.7])
    assert fractions[2] == pytest.approx([1.0, 0.5])
    assert checked["contexts"] == ["human/dmek", "human/dsaek"]
    converted = to_density_series(record)
    assert converted[0].months == (6.0, 12.0) and converted[0].context == "human/dsaek"
    assert converted[1].context == "human/dmek" and converted[1].aggregate_n == 20
    assert structural_summary([record])["studies"][0]["control_series"] == 3


def test_invalid_records_are_rejected():
    with pytest.raises(InvalidRequest):
        validate_record(base([series("dsaek", "ecd_absolute", [6, 12], [1700, 1500])]))
    with pytest.raises(InvalidRequest):
        validate_record(base([series("dsaek", "ecd_change", [0, 6], [0, -500])]))
    with pytest.raises(InvalidRequest):
        validate_record(base([series("dsaek", "percent_loss", [0, 6], [5, 30])]))
    with pytest.raises(InvalidRequest):
        DensitySeries(
            series_id="s",
            study_id="x",
            context="human/pk",
            reference_ecd=2500.0,
            months=(0.0,),
            fractions=(1.0,),
        )


def test_builder_reads_cells_with_thousands_separators():
    spec = base(
        [
            {
                "series_id": "dmek/mean",
                "group": "dmek",
                "kind": "group_mean",
                "n": 20,
                "value_kind": "ecd_absolute",
                "reference_kind": "donor_eye_bank",
                "months": [0, 6, 12],
                "cells": ["a.xml::t2!R2C3", "a.xml::t2!R3C3", "a.xml::t2!R4C3"],
            }
        ]
    )
    record = build_record(spec, {"a.xml": XML})
    assert record["series"][0]["values"] == [2700.0, 1600.0, 1500.0]
    with pytest.raises(InvalidRequest):
        build_record(spec, {"a.xml": XML + b" "})
