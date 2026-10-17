"""Replay actual offline CellML execution and retain reproducible operator bundles.

Requires the explicitly built local image and inspected pinned source. No fetching,
installation, production volume access or biological validity claim.
"""

import argparse
import csv
import json
import math
import subprocess
from pathlib import Path

from peykar.model_import.cellml import CellMLService
from peykar.model_import.libraries import LibraryService
from peykar.model_import.native_runtimes import NativeRuntimeService
from peykar.simulation.cellml_operator import execute_cellml, run_isolated_cellml
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    CellMLModelSpec,
    CellMLRunControls,
    LibrarySourceSpec,
    NativeRuntimeEvidence,
    Reference,
)
from peykar_core.errors import InvalidRequest
from peykar_core.hashing import bytes_digest

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--source", type=Path, default=ROOT / ".peykar/external/extra-sources/cellml--libcellml"
    )
    parser.add_argument("--image", default="peykar-cellml:0.7.1")
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    svc = SimulationService(output / "data")
    models = CellMLService(svc.db, svc.artifacts)
    source = LibraryService(svc.db, svc.artifacts).inspect(
        LibrarySourceSpec(
            id="libcellml",
            version="1.0.0",
            title="libCellML 0.7.1 complete source",
            repository="https://github.com/cellml/libcellml",
            commit="bc71bfa5816cd3e1e1a28430366eb5688fe9e501",
            license="Apache-2.0",
            license_files=("LICENSE",),
            capabilities=("cellml", "ode"),
            limitations=(
                "Source inspection establishes neither solver capability nor biological validity.",
            ),
        ),
        args.source,
    )
    (output / "source-receipt.json").write_text(source.model_dump_json(indent=2) + "\n")
    image_id = subprocess.check_output(
        ["docker", "image", "inspect", args.image, "--format", "{{.Id}}"], text=True, timeout=15
    ).strip()
    fixture = ROOT / "model_packages/cellml/decay/1.0.0"
    controls = CellMLRunControls.model_validate_json((fixture / "controls.json").read_bytes())
    raw = (fixture / "model.cellml").read_bytes()
    run_isolated_cellml(raw, controls, image_id, output / "bootstrap", 120)
    first = json.loads((output / "bootstrap/result.json").read_bytes())
    driver = (ROOT / "runtime_profiles/cellml/probe.py").read_bytes()
    if first["driver_digest"] != bytes_digest(driver) or first["model_digest"] != bytes_digest(raw):
        raise ValueError("The local image is stale; rebuild the reviewed profile")
    retained = {
        "driver.py": driver,
        "uv.lock": (ROOT / "runtime_profiles/cellml/uv.lock").read_bytes(),
        "bootstrap-result.json": (output / "bootstrap/result.json").read_bytes(),
        "bootstrap-series.csv": (output / "bootstrap/series.csv").read_bytes(),
    }
    artifacts = {k: bytes_digest(v) for k, v in retained.items()}
    execution = json.dumps(
        {"status": "passed", "profile": "cellml.ode", "image_id": image_id, "artifacts": artifacts},
        sort_keys=True,
    ).encode()
    bundle = output / "runtime-bundle"
    bundle.mkdir()
    for value in (*retained.values(), execution):
        (bundle / bytes_digest(value)).write_bytes(value)
    runtime = NativeRuntimeEvidence(
        id="cellml.ode.runtime",
        version="1.0.0",
        title="libCellML 0.7.1 / SciPy ODE runtime",
        engine="libcellml",
        profile="cellml.ode",
        sources=(Reference(id=source.spec.id, version=source.spec.version),),
        image_digest=image_id.removeprefix("sha256:"),
        execution_digest=bytes_digest(execution),
        artifact_digests=artifacts,
        limitations=(
            "Self-contained CellML 2 ODEs only; imports/resets/DAE/external variables unsupported.",
            "Operator-only execution; durable API queue adapter remains separate.",
            "Numerical fixture evidence is not biological validation.",
        ),
    )
    NativeRuntimeService(svc.db, svc.artifacts).import_evidence(runtime, bundle)
    (output / "runtime-receipt.json").write_text(runtime.model_dump_json(indent=2) + "\n")
    catalog = {
        "runtime": {"id": runtime.id, "version": runtime.version},
        "image_digest": runtime.image_digest,
        "driver_digest": bytes_digest(driver),
    }
    (output / "local-catalog.json").write_text(json.dumps(catalog, indent=2) + "\n")
    spec = CellMLModelSpec.model_validate_json((fixture / "spec.json").read_bytes())
    models.admit(spec, raw)
    reports = []

    def execute(spec, raw, controls, label):
        models.admit(spec, raw)
        receipt = execute_cellml(
            models,
            Reference(id=spec.id, version=spec.version),
            controls,
            output / "local-catalog.json",
            output / label,
            "cellml." + label,
        )
        with (output / label / "series.csv").open() as stream:
            rows = list(csv.DictReader(stream))
        reports.append(
            {"case": label, "receipt": receipt.model_dump(mode="json"), "samples": len(rows)}
        )
        return rows

    decay = execute(spec, raw, controls, "decay")
    error = max(abs(float(r["decay/x"]) - math.exp(-0.1 * float(r["time"]))) for r in decay)
    assert error < 1e-8
    reports[-1]["analytical_max_error"] = error
    # Upstream's original stiff engineering fixture, with its original dimensionless time.
    original = (
        ROOT
        / "vendor/dependencies/libcellml/tests/resources/generator"
        / "robertson_model_1966/model.ode.cellml"
    ).read_bytes()
    robertson = spec.model_copy(
        update={
            "id": "cellml.robertson",
            "title": "Original upstream Robertson stiff ODE fixture",
            "license": "Apache-2.0",
            "source": "https://github.com/cellml/libcellml/blob/bc71bfa5816cd3e1e1a28430366eb5688fe9e501/tests/resources/generator/robertson_model_1966/model.ode.cellml",
            "limitations": (
                "Original numerical fixture with dimensionless units; no biological claim.",
            ),
        }
    )
    stiff = CellMLRunControls(
        end=40, time_unit="dimensionless", intervals=200, method="Radau", max_step=0.2
    )
    a = execute(robertson, original, stiff, "robertson-radau")
    b = execute(
        robertson,
        original,
        stiff.model_copy(update={"method": "BDF", "max_step": 0.1}),
        "robertson-bdf",
    )
    conservation = max(
        abs(sum(float(r[k]) for k in ("main/y1", "main/y2", "main/y3")) - 1) for r in a
    )
    disagreement = max(
        abs(float(x[k]) - float(y[k]))
        for x, y in zip(a, b, strict=True)
        for k in ("main/y1", "main/y2", "main/y3")
    )
    assert conservation < 1e-10 and disagreement < 2e-7
    reports[-1].update(conservation_error=conservation, radau_bdf_max_difference=disagreement)
    scaling_path = "tests/resources/generator/cellml_unit_scaling_voi_direct/model.cellml"
    scaling = spec.model_copy(
        update={
            "id": "cellml.unit-scaling",
            "title": "Original upstream variable-of-integration scaling fixture",
            "license": "Apache-2.0",
            "source": source.spec.repository + "/blob/" + source.spec.commit + "/" + scaling_path,
            "limitations": ("Original numerical unit-scaling test; no biological claim.",),
        }
    )
    scaled = execute(
        scaling,
        (ROOT / "vendor/dependencies/libcellml" / scaling_path).read_bytes(),
        CellMLRunControls(end=10, time_unit="ms", intervals=100, max_step=0.1),
        "unit-scaling",
    )
    scaling_error = max(
        max(
            abs(float(r["main/x"]) - (3 + 0.5 * float(r["time"]) ** 2)),
            abs(float(r["main/y"]) - (5 + 0.0005 * float(r["time"]) ** 2)),
        )
        for r in scaled
    )
    assert scaling_error < 1e-10
    reports[-1]["analytical_scaling_error"] = scaling_error
    # Exercise the actual isolated driver rejection, retaining its stderr and no receipt.
    failures = []
    for label, invalid_raw, invalid_controls in (
        ("wrong-time-unit", raw, controls.model_copy(update={"time_unit": "minute"})),
        ("invalid-equation", raw.replace(b"<ci>k</ci>", b"<ci>missing</ci>"), controls),
    ):
        try:
            run_isolated_cellml(invalid_raw, invalid_controls, image_id, output / label, 120)
        except InvalidRequest:
            assert not (output / label / "result.json").exists()
            failures.append(label)
        else:
            raise AssertionError("Invalid CellML execution succeeded")
    try:
        run_isolated_cellml(
            original,
            stiff.model_copy(update={"method": "RK45", "end": 40000}),
            image_id,
            output / "timeout",
            1,
        )
    except InvalidRequest:
        assert not (output / "timeout/result.json").exists()
        failures.append("timeout")
    else:
        raise AssertionError("The stiff explicit solver unexpectedly completed in one second")
    leftovers = subprocess.check_output(
        ["docker", "ps", "-aq", "--filter", "name=peykar-cellml-"], text=True
    )
    assert not leftovers.strip(), "CellML container survived completion/failure/deadline"
    summary = {
        "status": "passed",
        "source": source.model_dump(mode="json"),
        "runtime": runtime.model_dump(mode="json"),
        "executions": reports,
        "rejected_solver_cases": failures,
        "scientific_validity": "not_established",
    }
    (output / "verification.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(
        json.dumps(
            {
                "status": "passed",
                "executions": len(reports),
                "analytical_max_error": error,
                "conservation_error": conservation,
                "radau_bdf_max_difference": disagreement,
            }
        )
    )


if __name__ == "__main__":
    main()
