import json
from pathlib import Path
from xml.etree import ElementTree as ET

import pytest

from peykar.validation.cornea_forecast_evidence import (
    CAPTION,
    DAY_HEADERS,
    DOI,
    GROUPS,
    HOURS,
    JATS_DOCTYPE,
    TABLE_ID,
    ijms2019_wound_aggregates,
    isolate_ijms2019_control_forecast,
    normalize_ijms2019_area_ratios,
)
from peykar_core.errors import InvalidRequest

SOURCE = Path("experiments/vcornea/repair-front/conditioned-forecast/ijms2019-original.xml")
SOURCE_SHA256 = "4c2d3b588f040797ecb5967504349e563eeb76fae9ec8fa890edbc05b76504db"


def synthetic_article():
    """Synthetic engineering fixture with an exact table layout, not real data."""
    article = ET.Element("article")
    ET.SubElement(article, "article-id", {"pub-id-type": "doi"}).text = DOI
    table_wrap = ET.SubElement(article, "table-wrap", {"id": TABLE_ID})
    ET.SubElement(table_wrap, "label").text = "Table 1"
    ET.SubElement(table_wrap, "caption").text = CAPTION
    table = ET.SubElement(table_wrap, "table")
    thead = ET.SubElement(table, "thead")
    first = ET.SubElement(thead, "tr")
    ET.SubElement(first, "th", {"rowspan": "2"}).text = "Treatment"
    ET.SubElement(first, "th", {"colspan": "7"}).text = "TIME (days)"
    second = ET.SubElement(thead, "tr")
    for label in DAY_HEADERS:
        ET.SubElement(second, "th").text = label
    body = ET.SubElement(table, "tbody")
    for group in GROUPS:
        row = ET.SubElement(body, "tr")
        ET.SubElement(row, "td").text = group
        for text in ("10 ± 1", "12 ± 2", "8 ± 2", "6 ± 2", "4 ± 2", "2 ± 2", "0"):
            ET.SubElement(row, "td").text = text
    return ET.tostring(article)


def test_actual_all28_aggregates_keep_original_cells_units_statistics_and_unknown_subjects():
    raw = SOURCE.read_bytes()
    result = ijms2019_wound_aggregates(raw)
    assert result["source_xml_sha256"] == SOURCE_SHA256
    rows = result["observations"]
    assert len(rows) == 28
    assert {(row["group"], row["hour"]) for row in rows} == {
        (group, hour) for group in GROUPS for hour in HOURS
    }
    source = ET.fromstring(raw).find(f".//table-wrap[@id='{TABLE_ID}']/table/tbody")
    assert source is not None
    for row in rows:
        cell = source[row["source_body_row_1based"] - 1][row["source_column_1based"] - 1]
        assert row["raw_text"] == " ".join("".join(cell.itertext()).split())
        fields = row["raw_text"].split(" ± ")
        assert row["mean_area_mm2"] == float(fields[0])
        assert row["reported_sd_mm2"] == (float(fields[1]) if len(fields) == 2 else None)
        assert row["unit"] == result["unit"] == "millimeter ** 2"
        assert row["statistic"] == "cohort_mean"
        assert row["animal_ids"] is None and row["sample_n"] is None
        assert row["source_cell"].endswith(
            f"tbody/tr[{row['source_body_row_1based']}]/td[{row['source_column_1based']}]"
        )
    assert result["individual_records_available"] is False
    assert result["per_arm_n_reported"] is False
    assert result["independent_biological_validation"] is False


def test_actual_conditioning_is_exactly_two_published_control_cells_without_future_data():
    split = isolate_ijms2019_control_forecast(SOURCE.read_bytes())
    conditioning = split["conditioning"]
    rows = conditioning["observations"]
    assert conditioning["role"] == "conditioning_input_only"
    assert conditioning["cutoff_hour"] == 24
    assert [(r["group"], r["hour"]) for r in rows] == [("Control", 0), ("Control", 24)]
    # The only real Control values asserted here are the already disclosed prefix.
    assert [(r["raw_text"], r["mean_area_mm2"], r["reported_sd_mm2"]) for r in rows] == [
        ("72.32 ± 7.32", 72.32, 7.32),
        ("47.02 ± 4.84", 47.02, 4.84),
    ]
    assert [(r["source_body_row_1based"], r["source_column_1based"]) for r in rows] == [
        (1, 2),
        (1, 3),
    ]
    future = split["future"]
    assert future["role"] == "reserved_future_control_evaluation_only"
    assert [(r["group"], r["hour"]) for r in future["observations"]] == [
        ("Control", hour) for hour in HOURS[2:]
    ]
    others = split["other_groups"]
    assert len(others["observations"]) == 21
    assert {r["group"] for r in others["observations"]} == set(GROUPS[1:])
    assert split["initial_state_only_prediction"] is False
    assert split["independent_biological_validation"] is False


def test_conditioning_preserves_every_field_from_the_independently_acquired_prefix():
    prefix = json.loads(SOURCE.with_name("ijms2019-control-prefix.json").read_bytes())
    conditioning = isolate_ijms2019_control_forecast(SOURCE.read_bytes())["conditioning"]
    assert len(prefix["observations"]) == len(conditioning["observations"]) == 2
    for original, admitted in zip(
        prefix["observations"], conditioning["observations"], strict=True
    ):
        assert all(admitted[key] == value for key, value in original.items())
    for key in ("study_doi", "source_xml_sha256", "unit", "role", "cutoff_hour"):
        assert conditioning[key] == prefix[key]


def test_normalization_keeps_areas_sd_and_above_baseline_ratios_without_fake_uncertainty():
    raw = synthetic_article()
    original = ijms2019_wound_aggregates(raw)
    normalized = normalize_ijms2019_area_ratios(raw)
    assert normalized["normalization"] == "ratio_of_published_cohort_means"
    assert normalized["normalized_unit"] == "dimensionless"
    assert normalized["ratio_uncertainty_available"] is False
    for before, after in zip(original["observations"], normalized["observations"], strict=True):
        assert all(after[key] == value for key, value in before.items())
        assert after["baseline_mean_area_mm2"] == 10
        assert after["mean_area_fraction"] == before["mean_area_mm2"] / 10
        assert "fraction_sd" not in after
    assert normalized["observations"][1]["mean_area_fraction"] == 1.2
    assert normalized["observations"][6]["reported_sd_mm2"] is None


@pytest.mark.parametrize("cutoff", [0, 12, 23, 25, 24.0, True, None])
def test_only_declared24h_conditioning_cutoff_is_admitted(cutoff):
    with pytest.raises(InvalidRequest, match="cutoff exactly24h"):
        isolate_ijms2019_control_forecast(synthetic_article(), cutoff_hour=cutoff)


@pytest.mark.parametrize(
    "old,new",
    [
        (DOI.encode(), b"10.3390/another"),
        (TABLE_ID.encode(), b"different-table"),
        (b"TIME (days)", b"TIME (hours)"),
        (b"Day 1.5", b"Day 1.25"),
        (b"Control", b"Different group"),
        (b"standard deviation", b"standard error"),
        (b"mm2", b"micrometer2"),
        (b"12 &#177; 2", b"NaN &#177; 2"),
        (b"12 &#177; 2", b"-12 &#177; 2"),
        (b"12 &#177; 2", b"12 &#177; -2"),
        (b"12 &#177; 2", b"12"),
        (b"<td>0</td>", b"<td>Unreported</td>"),
    ],
)
def test_changed_identity_layout_units_statistic_or_invalid_measurements_fail(old, new):
    raw = synthetic_article()
    assert old in raw
    with pytest.raises(InvalidRequest):
        ijms2019_wound_aggregates(raw.replace(old, new))


def test_duplicate_source_table_or_doi_and_missing_group_or_cell_fail_closed():
    root = ET.fromstring(synthetic_article())
    root.append(ET.fromstring(ET.tostring(root.find("table-wrap"))))
    with pytest.raises(InvalidRequest, match="identity"):
        ijms2019_wound_aggregates(ET.tostring(root))
    root = ET.fromstring(synthetic_article())
    ET.SubElement(root, "article-id", {"pub-id-type": "doi"}).text = DOI
    with pytest.raises(InvalidRequest, match="identity"):
        ijms2019_wound_aggregates(ET.tostring(root))
    for target in ("./table-wrap/table/tbody", "./table-wrap/table/tbody/tr"):
        root = ET.fromstring(synthetic_article())
        parent = root.find(target)
        assert parent is not None
        parent.remove(parent[-1])
        with pytest.raises(InvalidRequest, match="layout"):
            ijms2019_wound_aggregates(ET.tostring(root))


def test_changed_time_header_cell_span_cannot_shift_measured_source_columns():
    root = ET.fromstring(synthetic_article())
    cell = root.find("./table-wrap/table/thead/tr[2]/th")
    assert cell is not None
    cell.set("colspan", "2")
    with pytest.raises(InvalidRequest, match="layout"):
        ijms2019_wound_aggregates(ET.tostring(root))


def test_zero_baseline_cannot_be_normalized_or_split_as_a_conditioning_input():
    raw = synthetic_article().replace(b"10 &#177; 1", b"0 &#177; 1")
    assert len(ijms2019_wound_aggregates(raw)["observations"]) == 28
    with pytest.raises(InvalidRequest, match="positive baseline"):
        normalize_ijms2019_area_ratios(raw)
    with pytest.raises(InvalidRequest, match="positive baseline"):
        isolate_ijms2019_control_forecast(raw)


@pytest.mark.parametrize(
    "raw",
    [
        b"",
        b"<article>",
        b"x" * 4_000_001,
        b"\xff<article/>",
        b'<!DOCTYPE x [<!ENTITY e SYSTEM "file:///etc/passwd">]><article>&e;</article>',
        b'<!DOCTYPE article SYSTEM "https://example.com/dtd"><article/>',
        b'<article xmlns:x="http://www.w3.org/2001/XInclude"><x:include href="file"/></article>',
    ],
)
def test_unreviewed_xml_never_resolves_remote_or_local_content(raw):
    with pytest.raises(InvalidRequest, match="bounded inert XML"):
        ijms2019_wound_aggregates(raw)


def test_only_exact_known_external_jats_declaration_is_removed_without_resolution():
    raw = synthetic_article()
    assert len(ijms2019_wound_aggregates(JATS_DOCTYPE.encode() + raw)["observations"]) == 28
    for declaration in [JATS_DOCTYPE.replace("1.4", "1.3"), JATS_DOCTYPE * 2]:
        with pytest.raises(InvalidRequest, match="bounded inert XML"):
            ijms2019_wound_aggregates(declaration.encode() + raw)
