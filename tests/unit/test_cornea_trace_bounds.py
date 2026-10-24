import pytest

from peykar.validation.cornea_trace_bounds import ppm_rgb, trace_mean_lower_bound


def raster(rows):
    pixels = bytearray([255] * 7 * 41 * 3)
    for y in rows:
        pixels[(y * 7 + 3) * 3 : (y * 7 + 4) * 3] = b"\0\0\0"
    return b"P6\n7 41\n255\n" + pixels


def bounds(raw):
    return trace_mean_lower_bound(
        raw,
        column=3,
        maximum_study_animals=20,
        world_y_zero=40,
        world_y_forty=0,
        image_y_origin=0,
        image_y_scale=1,
        area_allowance_mm2=0.5,
    )


def test_conservative_unknown_arm_size_and_overlaps():
    result = bounds(raster([10, 11, 30]))
    assert result["minimum_distinct_observations"] == 2
    assert result["cohort_mean_lower_bound_mm2"] == (28.5 + 9.5) / 20
    assert result["recovered_individual_animal_records"] is False


def test_empty_and_corrupt_rasters_fail():
    with pytest.raises(ValueError, match="No curve"):
        bounds(raster([]))
    with pytest.raises(ValueError, match="byte count"):
        ppm_rgb(raster([10])[:-1])


def test_whitespace_valued_first_pixel_is_preserved():
    assert ppm_rgb(b"P6\n1 1\n255\n\n\r\t")[2] == b"\n\r\t"
