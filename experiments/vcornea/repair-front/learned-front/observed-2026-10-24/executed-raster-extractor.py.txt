"""Conservative constraints from published individual-curve raster ink.

These are NOT recovered animal records, a cohort mean estimate, or a CI. At
each measured time, separated ink clusters imply at least that many different
nonnegative observations. Dividing their lower areas by the entire study's
animal count gives a deliberately weak lower bound on the treatment mean.
Unseen/overlapping curves contribute zero; no arm size or longitudinal IDs are
invented. The caller must inspect the source and freeze pixel/axis calibration.
"""

import math


def ppm_rgb(raw: bytes) -> tuple[int, int, bytes]:
    """Read a bounded uncompressed Poppler P6 raster, without executable content."""
    if len(raw) > 10_000_000:
        raise ValueError("Raster exceeds the reviewed bound")
    offset = 0
    tokens: list[bytes] = []
    while len(tokens) < 4:
        while offset < len(raw) and raw[offset] in b" \t\r\n":
            offset += 1
        if offset < len(raw) and raw[offset] == ord("#"):
            end = raw.find(b"\n", offset)
            if end < 0:
                raise ValueError("Unterminated PPM comment")
            offset = end + 1
            continue
        start = offset
        while offset < len(raw) and raw[offset] not in b" \t\r\n":
            offset += 1
        if start == offset:
            raise ValueError("Incomplete PPM header")
        tokens.append(raw[start:offset])
    if tokens[0] != b"P6" or tokens[3] != b"255":
        raise ValueError("Require an 8-bit RGB P6 raster")
    width, height = int(tokens[1]), int(tokens[2])
    if not 1 <= width <= 2000 or not 1 <= height <= 2000:
        raise ValueError("Raster dimensions outside reviewed bound")
    # Exactly one header separator; never strip whitespace-valued first pixels.
    offset += 1
    pixels = raw[offset:]
    if len(pixels) != width * height * 3:
        raise ValueError("Raster byte count differs from its dimensions")
    return width, height, pixels


def trace_mean_lower_bound(
    raw: bytes,
    *,
    column: int,
    maximum_study_animals: int,
    world_y_zero: float,
    world_y_forty: float,
    image_y_origin: float,
    image_y_scale: float,
    area_allowance_mm2: float,
) -> dict[str, object]:
    """One-sided mean constraint; unresolved curves never fabricate precision."""
    width, height, pixels = ppm_rgb(raw)
    if (
        type(column) is not int
        or not 1 <= column < width - 1
        or type(maximum_study_animals) is not int
        or not 1 <= maximum_study_animals <= 100
        or any(
            not math.isfinite(v)
            for v in (
                world_y_zero,
                world_y_forty,
                image_y_origin,
                image_y_scale,
                area_allowance_mm2,
            )
        )
        or world_y_zero <= world_y_forty
        or image_y_scale <= 0
        or not 0 < area_allowance_mm2 <= 2
    ):
        raise ValueError("Invalid frozen raster calibration")
    rows = [
        y
        for y in range(height)
        if any(
            max(pixels[(y * width + x) * 3 : (y * width + x + 1) * 3]) <= 128
            for x in range(column - 1, column + 2)
        )
    ]
    if not rows:
        raise ValueError("No curve ink at the declared measurement column")

    def area(y: int) -> float:
        return (
            (world_y_zero - image_y_origin - image_y_scale * y)
            / (world_y_zero - world_y_forty)
            * 40
        )

    intervals = sorted(
        (max(0.0, area(y) - area_allowance_mm2), area(y) + area_allowance_mm2) for y in rows
    )
    merged: list[list[float]] = []
    for lower, upper in intervals:
        if merged and lower <= merged[-1][1]:
            merged[-1][1] = max(merged[-1][1], upper)
        else:
            merged.append([lower, upper])
    if len(merged) > maximum_study_animals:
        raise ValueError("Too many separated clusters for the published total animals")
    return {
        "column": column,
        "ink_rows": rows,
        "separated_area_intervals_mm2": merged,
        "minimum_distinct_observations": len(merged),
        "maximum_study_animals": maximum_study_animals,
        "cohort_mean_lower_bound_mm2": math.fsum(i[0] for i in merged) / maximum_study_animals,
        "recovered_individual_animal_records": False,
        "estimated_cohort_mean_or_confidence_interval": False,
    }
