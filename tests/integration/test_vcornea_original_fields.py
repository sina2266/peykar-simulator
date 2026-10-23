"""An intact archive member can still contain a truncated original VTK field."""

import importlib.util
from pathlib import Path

import pytest

spec = importlib.util.spec_from_file_location(
    "cornea_original_spatial",
    Path("runtime_profiles/vcornea-publication/inspect_original_spatial.py"),
)
original = importlib.util.module_from_spec(spec)
spec.loader.exec_module(original)


def vtk_text(fields, declared=None):
    header = (
        "# vtk DataFile Version 5.1\noriginal observation\nASCII\n"
        "DATASET STRUCTURED_POINTS\nDIMENSIONS 2 2 1\nSPACING 1 1 1\n"
        "ORIGIN 0 0 0\nPOINT_DATA 4\n"
        f"FIELD FieldData {len(fields) if declared is None else declared}\n"
    )
    return (header + "\n".join(f"{name} 1 4 double\n{data}" for name, data in fields)).encode()


def read(raw, required=("CellType", "EGF")):
    return original.read_ascii_fields(raw, required, (2, 2, 1))


def test_partial_optional_field_retains_actual_required_values_and_explicit_gap():
    # The observed archive fault: complete earlier fields, partial Pressure, then EOF.
    raw = vtk_text([("CellType", "1 3 4 5"), ("EGF", "0 .1 .2 .3"), ("Pressure", "12 8")], 4)
    arrays, quality = read(raw)
    assert arrays == {"CellType": [1, 3, 4, 5], "EGF": [0, 0.1, 0.2, 0.3]}
    assert quality["required_fields_complete"]
    assert not quality["all_declared_fields_complete"]
    assert quality["undiscovered_field_count"] == 1
    pressure = quality["fields"][-1]
    assert pressure["expected_values"] == 4 and pressure["observed_values"] == 2
    assert not pressure["complete"]
    assert "Pressure" not in arrays
    with pytest.raises(ValueError, match="Required.*incomplete"):
        read(raw, ["Pressure"])


def test_whole_missing_fields_are_not_invented_from_the_declaration():
    raw = vtk_text([("CellType", "1 3 4 5")], 3)
    arrays, quality = read(raw, ["CellType"])
    assert arrays["CellType"] == [1, 3, 4, 5]
    assert quality["undiscovered_field_count"] == 2
    with pytest.raises(ValueError, match="fields are missing"):
        read(raw)


def test_complete_original_layout_is_read_without_a_native_allocator():
    arrays, quality = read(vtk_text([("CellType", "1 3 4 5"), ("EGF", "0 1e-2 .2 .3")]))
    assert arrays["EGF"] == [0, 0.01, 0.2, 0.3]
    assert quality["all_declared_fields_complete"]
    assert quality["undiscovered_field_count"] == 0


@pytest.mark.parametrize("nonfinite", ["nan", "inf", "-inf"])
def test_nonfinite_values_never_count_as_a_complete_field(nonfinite):
    raw = vtk_text([("CellType", "1 3 4 5"), ("EGF", f"0 {nonfinite} .2 .3")])
    with pytest.raises(ValueError, match="Required.*nonfinite"):
        read(raw)
    _, quality = read(raw, ["CellType"])
    assert not quality["all_declared_fields_complete"]
    assert not quality["fields"][-1]["finite"]


def test_bad_numeric_counts_duplicate_names_and_trailing_data_are_rejected():
    for fields, message in [
        ([("CellType", "1 3"), ("EGF", "0 .1 .2 .3")], "Nonnumeric"),
        ([("CellType", "1 3 4 5"), ("CellType", "1 3 4 5")], "duplicate"),
    ]:
        with pytest.raises(ValueError, match=message):
            read(vtk_text(fields))
    with pytest.raises(ValueError, match="trailing"):
        read(vtk_text([("CellType", "1 3 4 5 6")]), ["CellType"])


def test_other_geometry_binary_or_mismatched_tuple_counts_are_rejected():
    raw = vtk_text([("CellType", "1 3 4 5"), ("EGF", "0 .1 .2 .3")])
    for modified, message in [
        (raw.replace(b"DIMENSIONS 2 2 1", b"DIMENSIONS 4 4 1"), "dimensions"),
        (raw.replace(b"POINT_DATA 4", b"POINT_DATA 16"), "point count"),
        (raw.replace(b"ASCII", b"BINARY"), "layout"),
        (raw.replace(b"EGF 1 4 double", b"EGF 1 3 double"), "scalar field"),
    ]:
        with pytest.raises(ValueError, match=message):
            read(modified)
