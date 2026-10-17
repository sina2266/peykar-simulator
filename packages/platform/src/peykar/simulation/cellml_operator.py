"""Explicit operator-only CellML execution. Never imported by API route handlers."""

import json
import os
import re
import subprocess
import uuid
from pathlib import Path

from peykar.model_import.cellml import CellMLService, inspect_cellml
from peykar_core.contracts import CellMLExecutionReceipt, CellMLRunControls, Reference
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest


def execute_cellml(
    service: CellMLService,
    model: Reference,
    controls: CellMLRunControls,
    catalog: Path,
    output: Path,
    id: str,
    wall_seconds: float = 120,
) -> CellMLExecutionReceipt:
    if not 1 <= wall_seconds <= 600:
        raise InvalidRequest("CellML operator deadline requires 1 to 600 seconds")
    package = service.get(model)
    raw = service.artifacts.get(package.content_digest)
    inspect_cellml(raw)
    entry = json.loads(catalog.read_bytes())
    image = entry["image_digest"]
    driver = entry["driver_digest"]
    if not all(isinstance(v, str) and re.fullmatch("[a-f0-9]{64}", v) for v in (image, driver)):
        raise InvalidRequest("CellML catalog requires exact image and driver SHA-256 identities")
    runtime = Reference.model_validate(entry["runtime"])
    from peykar.model_import.native_runtimes import NativeRuntimeService

    evidence = NativeRuntimeService(service.db, service.artifacts).get(runtime)
    if (
        evidence.engine != "libcellml"
        or evidence.image_digest != image
        or evidence.artifact_digests.get("driver.py") != driver
    ):
        raise Conflict("CellML local catalog differs from the admitted runtime/driver")
    image_id = "sha256:" + image
    actual = subprocess.check_output(
        ["docker", "image", "inspect", image_id, "--format", "{{.Id}}"], timeout=15, text=True
    ).strip()
    if actual != image_id:
        raise Conflict("CellML local image identity differs")
    output = output.resolve()
    run_isolated_cellml(raw, controls, image_id, output, wall_seconds)
    blobs = {}
    for key in ("result.json", "series.csv", "generated.py"):
        path = output / key
        if path.is_symlink() or not path.is_file() or path.stat().st_size > 32 * 1024**2:
            raise InvalidRequest("Missing, unsafe or oversized CellML output")
        blobs[key] = path.read_bytes()
    if bytes_digest((output / "model.cellml").read_bytes()) != package.content_digest:
        raise Conflict("Original CellML model changed during execution")
    receipt = CellMLExecutionReceipt(
        id=id,
        version="1.0.0",
        model=model,
        model_digest=package.content_digest,
        runtime=runtime,
        image_digest=image,
        driver_digest=driver,
        controls=controls,
        artifacts={key: bytes_digest(value) for key, value in blobs.items()},
        limitations=(
            "Explicit isolated operator execution; not a durable API queue adapter.",
            "Self-contained CellML 2 ODEs only; no resets, imports, external variables or DAE/NLA.",
            "Original model units retained; biological validity is not established.",
        ),
    )
    bundle = output / "bundle"
    bundle.mkdir()
    for key, value in blobs.items():
        (bundle / receipt.artifacts[key]).write_bytes(value)
    (output / "receipt.json").write_text(receipt.model_dump_json(indent=2) + os.linesep)
    service.import_execution(receipt, bundle)
    return receipt


def run_isolated_cellml(
    raw: bytes, controls: CellMLRunControls, image_id: str, output: Path, wall_seconds: float
) -> None:
    """Operator bootstrap/execution helper; no image pulls or data-store mounts."""
    inspect_cellml(raw)
    if not re.fullmatch(r"sha256:[a-f0-9]{64}", image_id) or not 1 <= wall_seconds <= 600:
        raise InvalidRequest("A local image digest and bounded deadline are required")
    # Output is new and contains only this model/job, never the data store or credentials.
    output = output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    output.chmod(0o777)
    (output / "model.cellml").write_bytes(raw)
    (output / "controls.json").write_text(controls.model_dump_json())
    for filename in ("model.cellml", "controls.json"):
        (output / filename).chmod(0o444)
    name = "peykar-cellml-" + uuid.uuid4().hex
    create = [
        "docker",
        "create",
        "--name",
        name,
        "--pull",
        "never",
        "--network",
        "none",
        "--read-only",
        "--user",
        "10001:10001",
        "--cap-drop",
        "ALL",
        "--security-opt",
        "no-new-privileges",
        "--memory",
        "1g",
        "--cpus",
        "1",
        "--pids-limit",
        "64",
        "--tmpfs",
        "/tmp:rw,noexec,nosuid,size=64m",
        "--mount",
        f"type=bind,src={output},dst=/work",
    ]
    for filename in ("model.cellml", "controls.json"):
        create.extend(
            ["--mount", f"type=bind,src={output / filename},dst=/work/{filename},readonly"]
        )
    create.append(image_id)
    started = False
    try:
        subprocess.run(create, check=True, capture_output=True, timeout=15)
        started = True
        with (output / "execution.log").open("wb") as log:
            subprocess.run(
                ["docker", "start", "--attach", name],
                check=True,
                stdout=log,
                stderr=subprocess.STDOUT,
                timeout=wall_seconds,
            )
    except (subprocess.SubprocessError, OSError) as error:
        raise InvalidRequest(
            "CellML solver failed or exceeded its deadline; inspect retained output"
        ) from error
    finally:
        # Also remove after an ambiguous create timeout; the random name belongs to this job.
        removed = subprocess.run(["docker", "rm", "--force", name], capture_output=True, timeout=15)
        output.chmod(0o700)
        if started and removed.returncode != 0:
            raise Conflict(
                "CellML container cleanup is uncertain; inspect Docker before another run"
            )
