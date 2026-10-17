"""Execute one admitted SBML time course inside the pinned COPASI container."""

import hashlib
import json
import math
import sys
from pathlib import Path

import basico

sys.path.insert(0, "/opt/runtime")
from copasi_compat import prepare_output_flags


def main():
    prepare_output_flags()
    controls = json.loads(Path("/work/controls.json").read_bytes())
    model = basico.load_model_from_string(Path("/work/model.xml").read_text(encoding="utf-8-sig"))
    if model is None:
        raise ValueError("COPASI could not import the SBML model")
    units = basico.get_model_units(model=model)
    scales = {
        "ns": 1e-9,
        "us": 1e-6,
        "µs": 1e-6,
        "ms": 1e-3,
        "s": 1,
        "min": 60,
        "h": 3600,
        "d": 86400,
    }
    if units["time_unit"] not in scales:
        raise ValueError("COPASI time unit is unsupported by this reviewed driver")
    duration = controls["duration_seconds"] / scales[units["time_unit"]]
    frame = basico.run_time_course(
        duration=duration, intervals=controls["intervals"], start_time=0, model=model
    )
    if len(frame) != controls["intervals"] + 1:
        raise ValueError("Native solver returned a different sample grid")
    times = [float(t) for t in frame.index]
    if not all(math.isfinite(t) for t in times) or any(
        b <= a for a, b in zip(times, times[1:], strict=False)
    ):
        raise ValueError("Native solver returned a nonfinite/nonmonotone time grid")
    if not all(
        math.isfinite(float(v)) for row in frame.itertuples(index=False, name=None) for v in row
    ):
        raise ValueError("Native solver returned nonfinite outputs")
    frame.to_csv("/work/native.csv", index_label="Time")
    species = basico.get_species(model=model).reset_index()
    metadata = {
        "engine": "COPASI",
        "model_units": units,
        "samples": len(frame),
        "species": species[
            [
                c
                for c in ("name", "display_name", "unit", "compartment", "sbml_id")
                if c in species.columns
            ]
        ].to_dict(orient="records"),
        "columns": list(frame.columns),
        "scientific_validity": "not_established",
        "limitations": [
            "Native deterministic integration; scientific applicability remains unvalidated.",
            "Original SBML, compartment/species/parameter units and runtime identity are retained.",
        ],
    }
    Path("/work/quantities.json").write_text(json.dumps(metadata, indent=2) + "\n")
    artifacts = {
        name: hashlib.sha256(Path("/work", name).read_bytes()).hexdigest()
        for name in ("native.csv", "quantities.json")
    }
    Path("/work/native-completion.json").write_text(
        json.dumps({"status": "succeeded", "artifacts": artifacts}, indent=2) + "\n"
    )


if __name__ == "__main__":
    main()
