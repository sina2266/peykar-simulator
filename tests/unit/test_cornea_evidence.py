import io
import math
import zipfile
from pathlib import Path

import pytest

from peykar.validation.cornea_evidence import (
    context_disagreements,
    epithelial_column_geometry,
    human_oct_summary,
    rabbit_wound_observations,
)
from peykar_core.errors import InvalidRequest

DATA = Path("experiments/data/cornea-evidence-20261024/raw")


def test_actual_rabbit_samples_keep_subjects_time_units_and_source_cells():
    rows = rabbit_wound_observations((DATA / "rabbit-figure2.xlsx").read_bytes())
    assert len(rows) == 90
    controls = [r for r in rows if r["group"] == "Control"]
    assert len(controls) == 30
    assert {r["replicate"] for r in controls} == {"1", "2", "3"}
    six = next(r for r in controls if r["replicate"] == "1" and r["hour"] == 6)
    assert six["raw_area"] == 1504552
    assert six["source_cell"] == "C7"
    assert six["wound_area_fraction"] == pytest.approx(1504552 / 1700653)
    assert six["closure_fraction"] == pytest.approx(1 - 1504552 / 1700653)
    assert six["raw_unit"] == "unreported_source_image_area"
    assert {r["hour"] for r in rows} == set(range(0, 55, 6))
    # The caption's n4 never turns into an invented fourth measured rabbit.
    assert all(r["replicate"] != "4" for r in rows)


def test_raw_formula_is_rejected_instead_of_using_cached_measurement():
    raw = (DATA / "rabbit-figure2.xlsx").read_bytes()
    output = io.BytesIO()
    with zipfile.ZipFile(io.BytesIO(raw)) as source, zipfile.ZipFile(output, "w") as target:
        for entry in source.infolist():
            value = source.read(entry)
            if entry.filename == "xl/worksheets/sheet1.xml":
                value = value.replace(b"<v>1700653</v>", b"<f>NOW()</f><v>1700653</v>")
            target.writestr(entry, value)
    with pytest.raises(InvalidRequest, match="formula"):
        rabbit_wound_observations(output.getvalue())


def test_actual_human_control_is_not_disease_group_or_fake_individual_data():
    result = human_oct_summary((DATA / "buffault2020-manuscript.xml").read_bytes())
    assert result["independent_subjects"] == 22
    assert result["group"] == "Normal"
    assert result["observations"]["Central"] == {"mean": 52.5, "sample_sd": 3.6}
    assert len(result["observations"]) == 7
    assert result["individual_observations_available"] is False


def test_clinical_xml_never_resolves_unreviewed_entities():
    raw = b'<!DOCTYPE x [<!ENTITY y SYSTEM "file:///etc/passwd">]><article>&y;</article>'
    with pytest.raises(InvalidRequest, match="inert XML"):
        human_oct_summary(raw)


def test_geometry_is_thickness_not_absolute_y_and_does_not_fill_holes():
    a = epithelial_column_geometry([[7, 6, 3, 4, 5, 10]], micrometres_per_site=2)
    b = epithelial_column_geometry([[7, 7, 7, 7, 6, 3, 4, 5, 10]], micrometres_per_site=2)
    assert a["mean_vertical_span_um"] == b["mean_vertical_span_um"] == 6
    assert a["clinical_observation_equivalence"] is False
    holes = epithelial_column_geometry(
        [[7, 3, 10, 5, 10], [7, 6, 10, 10, 10]], micrometres_per_site=2
    )
    assert holes["columns"][0]["vertical_span_um"] == 6
    assert holes["columns"][0]["occupied_height_um"] == 4
    assert holes["columns"][0]["internal_non_epithelial_sites"] == 1
    assert holes["columns"][1]["vertical_span_um"] is None
    assert holes["missing_columns"] == 1
    assert holes["columns_with_internal_gaps"] == 1


@pytest.mark.parametrize("scale", [0, -2, math.nan, math.inf, True, 1e308])
def test_unusable_spatial_scale_never_generates_a_measurement(scale):
    with pytest.raises(InvalidRequest, match="spatial scale"):
        epithelial_column_geometry([[3, 4, 5]], micrometres_per_site=scale)


@pytest.mark.parametrize("lattice", [[], [[]], [[3], [4, 5]], [[3.0]], [[True]], [[99]]])
def test_unusable_lattice_is_rejected(lattice):
    with pytest.raises(InvalidRequest):
        epithelial_column_geometry(lattice, micrometres_per_site=2)


def test_context_unknowns_and_whole_cornea_endpoints_are_not_auto_matches():
    context = {
        key: "known"
        for key in [
            "species",
            "tissue",
            "condition",
            "exposure",
            "endpoint",
            "measurement",
            "region",
        ]
    }
    assert context_disagreements(context, context) == ()
    assert context_disagreements({**context, "region": None}, context) == ("region",)
    assert context_disagreements(
        {**context, "region": "unknown"}, {**context, "region": "unknown"}
    ) == ("region",)
    assert context_disagreements(context, {**context, "endpoint": "whole cornea thickness"}) == (
        "endpoint",
    )
