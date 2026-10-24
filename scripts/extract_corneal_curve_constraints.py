"""Explicit read-only extraction from the retained Fischak2017 PBS figure."""

import argparse
import json
from pathlib import Path

from peykar.validation.cornea_trace_bounds import ppm_rgb, trace_mean_lower_bound
from peykar_core.hashing import bytes_digest, canonical_json

BASE = Path("experiments/vcornea/repair-front/learned-front/fischak2017")
PROTOCOL = BASE / "extraction-protocol-1.0.0.json"
PROTOCOL_SHA256 = "f4b2b58247a7f2b430295c64c801095cf327a1d513af1c4d84edf8b8d7eeb7f2"
SOURCE = Path("packages/platform/src/peykar/validation/cornea_trace_bounds.py")


def extract() -> dict:
    if bytes_digest(PROTOCOL.read_bytes()) != PROTOCOL_SHA256:
        raise ValueError("Frozen extraction protocol changed")
    protocol = json.loads(PROTOCOL.read_bytes())
    if bytes_digest(SOURCE.read_bytes()) != protocol["extractor_sha256"]:
        raise ValueError("Frozen raster extractor changed")
    for name, digest in protocol["source_files"].items():
        if bytes_digest((BASE / name).read_bytes()) != digest:
            raise ValueError(f"Published/derived source changed: {name}")
    raw = (BASE / "fluorescein-pbs.ppm").read_bytes()
    if ppm_rgb(raw)[:2] != (protocol["width"], protocol["height"]):
        raise ValueError("Unexpected source figure dimensions")
    keys = (
        "maximum_study_animals",
        "world_y_zero",
        "world_y_forty",
        "image_y_origin",
        "image_y_scale",
        "area_allowance_mm2",
    )
    rows = []
    for hour, column in protocol["times_columns"]:
        row = trace_mean_lower_bound(raw, column=column, **{k: protocol[k] for k in keys})
        row["hour"] = hour
        row["area_ratio_lower_bound"] = (
            row["cohort_mean_lower_bound_mm2"] / protocol["baseline_fluorescein_mean_mm2"]
        )
        rows.append(row)
    return {
        "protocol_sha256": PROTOCOL_SHA256,
        "extractor_sha256": bytes_digest(SOURCE.read_bytes()),
        "operator_sha256": bytes_digest(Path(__file__).read_bytes()),
        "observations": rows,
        "scope": protocol["class"],
        "limitations": protocol["limitations"],
        "independent_biological_validation": False,
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    result = extract()
    with args.output.open("x") as stream:
        stream.write(canonical_json(result))
    print([(r["hour"], r["area_ratio_lower_bound"]) for r in result["observations"]])
