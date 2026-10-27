import json
import math
from hashlib import sha256
from pathlib import Path
from xml.etree import ElementTree as ET

import pytest

from peykar.validation.cornea_evidence import rabbit_brush_aggregates, rabbit_wound_observations
from peykar.validation.cornea_forecast_evidence import normalize_ijms2019_area_ratios
from peykar.validation.cornea_online_evidence import ADAPTER_ID, corneal_online_series
from peykar_core.errors import InvalidRequest

SOURCES = {
    "rabbit2017": Path("experiments/data/cornea-evidence-20261024/raw/rabbit-figure2.xlsx"),
    "ijms2019": Path("experiments/vcornea/repair-front/conditioned-forecast/ijms2019-original.xml"),
    "chung2021": Path("experiments/vcornea/repair-front/chung2021-manuscript.xml"),
}


@pytest.fixture
def raw_sources():
    return {key: path.read_bytes() for key, path in SOURCES.items()}


def test_all16_original_series_stay_distinct_and_exact_source_digests_are_reported(raw_sources):
    result = corneal_online_series(**raw_sources)
    series = result["series"]
    assert result["adapter"] == ADAPTER_ID
    assert len(series) == len({s["series_id"] for s in series}) == 16
    assert result["source_hashes"] == {
        key: sha256(raw).hexdigest() for key, raw in raw_sources.items()
    }
    assert len([s for s in series if s["series_nature"] == "individual_rabbit"]) == 9
    assert len([s for s in series if s["series_nature"] == "published_cohort_mean"]) == 4
    assert len([s for s in series if s["series_nature"] == "published_cohort_aggregate"]) == 3
    assert result["dataset_role"] == "previously_consumed_retrospective_evaluation_evidence"
    assert result["initial_state_only_prediction"] is False
    assert result["independent_biological_validation"] is False
    assert json.loads(json.dumps(result, allow_nan=False)) == result


def test_2017_every_actual_animal_cell_and_baseline_denominator_is_retained(raw_sources):
    expected = rabbit_wound_observations(raw_sources["rabbit2017"])
    series = corneal_online_series(**raw_sources)["series"][:9]
    assert {(s["group"], s["replicate"]) for s in series} == {
        (group, replicate)
        for group in ("Control", "0.5% SDP", "2% SDP")
        for replicate in ("1", "2", "3")
    }
    for item in series:
        original = [
            row
            for row in expected
            if (row["group"], row["replicate"]) == (item["group"], item["replicate"])
        ]
        points = item["numeric_observations"]
        assert [p["original"] for p in points] == original
        assert len(points) == item["real_numeric_observation_count"] == 10
        assert [p["hour"] for p in points] == list(range(0, 55, 6))
        for point, row in zip(points, original, strict=True):
            assert point["area_fraction"] == row["raw_area"] / original[0]["raw_area"]
            assert point["source_cells"] == list(
                dict.fromkeys([row["source_cell"], original[0]["source_cell"]])
            )
            assert point["eligible_for_conditioning"] is True
            assert point["measurement_origin"] == "observed"
            assert row["raw_unit"] == "unreported_source_image_area"
        assert item["baseline_radius_um"] == 3500
        assert "nominal" in item["radius_basis"]
        assert item["baseline_raw_unit"] == "unreported_source_image_area"
        assert item["context"]["observed_rabbits_in_group"] == 3
        assert item["context"]["caption_group_n"] == 4
        assert item["categorical_observations"] == []


def test_2019_aggregate_ratios_preserve_sd_units_covariance_limits_and_eye_hierarchy(raw_sources):
    expected = normalize_ijms2019_area_ratios(raw_sources["ijms2019"])
    series = corneal_online_series(**raw_sources)["series"][9:13]
    for item in series:
        original = [r for r in expected["observations"] if r["group"] == item["group"]]
        assert item["replicate"] is None
        assert item["normalization"] == "ratio_of_published_cohort_means"
        assert item["real_numeric_observation_count"] == 7
        assert (
            item["baseline_radius_um"] == math.sqrt(original[0]["mean_area_mm2"] / math.pi) * 1000
        )
        assert item["baseline_radius_um"] != 4500
        assert "observed_baseline_cohort_mean" in item["radius_basis"]
        for point, row in zip(item["numeric_observations"], original, strict=True):
            assert point["original"] == row
            assert point["area_fraction"] == row["mean_area_mm2"] / original[0]["mean_area_mm2"]
            assert row["animal_ids"] is None and row["sample_n"] is None
            assert point["source_cells"] == list(
                dict.fromkeys([row["source_cell"], original[0]["source_cell"]])
            )
            assert point["eligible_for_conditioning"] is True
            assert "fraction_sd" not in point and "confidence_interval" not in point
        context = item["context"]
        assert context["reported_sd_unit"] == "millimeter ** 2"
        assert context["ratio_uncertainty_available"] is False
        assert context["study_rabbits"] == 17 and context["study_operated_eyes"] == 33
        assert context["per_arm_eye_n"] is None and context["per_arm_independent_rabbit_n"] is None
        assert "dexamethasone" in context["care"]
        assert "half-day" in context["time_reporting_note"]
    prgf = next(s for s in series if s["group"] == "s-PRGF")
    assert prgf["numeric_observations"][-1]["original"]["reported_sd_mm2"] is None


def test_2021_retains_all_numeric_and_split_categorical_cells_without_zero_or_clipping(raw_sources):
    original = rabbit_brush_aggregates(raw_sources["chung2021"])
    root = ET.fromstring(raw_sources["chung2021"])
    table_rows = root.findall(".//table-wrap[@id='tab1']/table/tbody/tr")
    series = corneal_online_series(**raw_sources)["series"][13:]
    actual_numeric = []
    actual_categorical = []
    for column, item in enumerate(series, start=2):
        assert item["baseline_radius_um"] == 3000
        assert "nominal" in item["radius_basis"]
        assert item["baseline_raw_area"] is None
        assert item["baseline_source_cells"] == []
        baseline = item["numeric_observations"][0]
        assert baseline["hour"] == 0 and baseline["area_fraction"] == 1
        assert baseline["measurement_origin"] == "derived_normalization_baseline"
        assert baseline["eligible_for_conditioning"] is False
        assert baseline["original"] is None and baseline["source_cells"] == []
        for row_index, point in enumerate(item["numeric_observations"][1:]):
            raw = point["original"]
            actual_numeric.append(raw)
            assert point["area_fraction"] == raw["mean_fraction"]
            assert point["source_cells"] == [f"tab1:tbody/tr[{row_index + 1}]/td[{column}]"]
            assert raw["reported"] == "".join(table_rows[row_index][column - 1].itertext()).strip()
            assert point["eligible_for_conditioning"] is True
        for point in item["categorical_observations"]:
            actual_categorical.append(point["original"])
            assert point["hour"] == 50 and point["reported"] == "Completely Healed"
            assert point["eligible_for_conditioning"] is False
            assert "area_fraction" not in point
            assert point["source_cells"] == [
                f"tab1:tbody/tr[4]/td[{column}]",
                f"tab1:tbody/tr[5]/td[{column}]",
            ]
        context = item["context"]
        assert context["per_time_independent_rabbit_n"] is None
        assert context["per_time_eye_n"] is None
        assert "unresolved" in context["uncertainty_definition"]
        assert context["reported_plus_minus_unit"] == "dimensionless"
        assert context["categorical_detection_limit"] is None

    def key(row):
        return row["group"], row["hour"]

    assert sorted(actual_numeric, key=key) == sorted(
        [row for row in original["observations"] if row["kind"] == "numeric_aggregate"], key=key
    )
    assert sorted(actual_categorical, key=key) == sorted(
        [row for row in original["observations"] if row["kind"] == "categorical"], key=key
    )
    ocu = next(s for s in series if s["group"] == "Ocu")
    assert ocu["numeric_observations"][1]["area_fraction"] == 1.02
    assert [s["real_numeric_observation_count"] for s in series] == [4, 3, 3]


def test_a_derived_baseline_cannot_supply_a_second_measurement_for_early_conditioning(raw_sources):
    series = corneal_online_series(**raw_sources)["series"]
    for item in series:
        eligible = [p for p in item["numeric_observations"] if p["eligible_for_conditioning"]]
        earliest_two_real = eligible[1]["hour"]
        if item["series_id"].startswith("abdel-naby2017/"):
            assert earliest_two_real == 6
        elif item["series_id"].startswith("suarez-barrio2019/"):
            assert earliest_two_real == 24
        else:
            assert earliest_two_real == 18
            assert len([p for p in eligible if p["hour"] <= 4]) == 1
            assert len([p for p in item["numeric_observations"] if p["hour"] <= 4]) == 2


@pytest.mark.parametrize("source", list(SOURCES))
def test_missing_or_wrong_source_does_not_return_partial_series(raw_sources, source):
    raw_sources[source] = b""
    with pytest.raises(InvalidRequest):
        corneal_online_series(**raw_sources)


def test_swapped_publication_sources_are_not_silently_mislabelled(raw_sources):
    raw_sources["ijms2019"], raw_sources["chung2021"] = (
        raw_sources["chung2021"],
        raw_sources["ijms2019"],
    )
    with pytest.raises(InvalidRequest):
        corneal_online_series(**raw_sources)
