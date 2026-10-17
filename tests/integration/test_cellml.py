"""CellML contract, authorization, exact-byte and operator-output release gates."""

import csv
import io
import json
import math
import subprocess
from pathlib import Path

import pytest
from fastapi.testclient import TestClient
from pydantic import ValidationError

from peykar.api.app import create_app
from peykar.infrastructure.artifacts import ArtifactStore
from peykar.infrastructure.database import Database
from peykar.model_import.cellml import CellMLService, inspect_cellml
from peykar.model_import.libraries import LibraryService
from peykar.model_import.native_runtimes import NativeRuntimeService
from peykar.simulation.cellml_operator import run_isolated_cellml
from peykar_core.contracts import (
    CellMLExecutionReceipt,
    CellMLModelSpec,
    CellMLRunControls,
    LibrarySourceReceipt,
    LibrarySourceSpec,
    NativeRuntimeEvidence,
    Reference,
)
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = ROOT / "model_packages/cellml/decay/1.0.0"
RAW = (FIXTURE / "model.cellml").read_bytes()
SPEC = CellMLModelSpec.model_validate_json((FIXTURE / "spec.json").read_bytes())
CONTROLS = CellMLRunControls.model_validate_json((FIXTURE / "controls.json").read_bytes())
REF = Reference(id=SPEC.id, version=SPEC.version)
ADMIN, READER = "cellml-admin-test-at-least-24", "cellml-reader-test-at-least-24"
AUTH = {"Authorization": "Bearer " + ADMIN}
READ = {"Authorization": "Bearer " + READER}


@pytest.fixture
def service(tmp_path):
    return CellMLService(
        Database(tmp_path / "state.sqlite3"), ArtifactStore(tmp_path / "artifacts")
    )


def test_admission_preserves_original_bytes_units_and_immutable_identity(service):
    original = b"\xef\xbb\xbf" + RAW.replace(b"\n", b"\r\n")
    admitted = service.admit(SPEC, original)
    assert service.artifacts.get(admitted.content_digest) == original
    assert admitted.inspection.units["per_second"] == ({"units": "second", "exponent": "-1"},)
    assert [v.units for v in admitted.inspection.variables] == [
        "second",
        "dimensionless",
        "per_second",
    ]
    assert service.admit(SPEC, original) == admitted
    assert service.get(REF) == admitted
    assert service.discover(1, 1) == ()
    with pytest.raises(Conflict):
        service.admit(SPEC, RAW.replace(b'initial_value="1"', b'initial_value="2"'))
    path = service.artifacts._path(admitted.content_digest)
    path.write_bytes(b"corrupt")
    with pytest.raises(Conflict):
        service.get(REF)


@pytest.mark.parametrize(
    "raw",
    [
        b"",
        b"\xff",
        b"\0",
        b"<model",
        b"<model/>",
        RAW.replace(b"cellml/2.0#", b"cellml/1.1#"),
        RAW.replace(b"<model", b'<!DOCTYPE model [<!ENTITY x SYSTEM "file:///etc/passwd">]><model'),
        RAW.replace(b"</model>", b'<import xmlns="http://www.cellml.org/cellml/2.0#"/></model>'),
        RAW.replace(
            b"</model>",
            b'<xi:include xmlns:xi="http://www.w3.org/2001/XInclude" href="https://example.com"/></model>',
        ),
        RAW.replace(b'units="per_second"', b'units="missing"'),
        RAW.replace(b'name="x"', b'name="t"'),
        RAW.replace(b'name="per_second"', b'name="second"'),
        RAW.replace(b'name="decay"', b'name="bad-name"'),
        b" " * (10 * 1024**2 + 1),
    ],
)
def test_rejects_malformed_unsafe_or_unsupported_admission(raw):
    with pytest.raises(InvalidRequest):
        inspect_cellml(raw)


@pytest.mark.parametrize(
    "change",
    [
        {"end": 0},
        {"end": float("inf")},
        {"start": float("nan")},
        {"max_step": 21},
        {"max_step": 0},
        {"intervals": True},
        {"relative_tolerance": 1e-14},
        {"method": "Euler"},
    ],
)
def test_solver_controls_have_finite_physical_grid_and_tolerance_bounds(change):
    with pytest.raises(ValidationError):
        CellMLRunControls.model_validate({**CONTROLS.model_dump(), **change})


def retained_runtime(service, tmp_path):
    inventory, license_raw, driver = (
        b'[{"path":"LICENSE","bytes":7}]',
        b"license",
        b"reviewed-driver",
    )
    spec = LibrarySourceSpec(
        id="libcellml",
        version="1.0.0",
        title="CellML source",
        repository="https://github.com/cellml/libcellml",
        commit="a" * 40,
        license="Apache-2.0",
        license_files=("LICENSE",),
        capabilities=("cellml",),
        limitations=("Test fixture",),
    )
    source = LibrarySourceReceipt(
        spec=spec,
        inventory_digest=bytes_digest(inventory),
        license_digests={"LICENSE": bytes_digest(license_raw)},
        tracked_files=1,
        tracked_bytes=7,
    )
    LibraryService(service.db, service.artifacts).import_receipt(
        source, inventory, {"LICENSE": license_raw}
    )
    bundle = tmp_path / "runtime-bundle"
    bundle.mkdir()
    execution = json.dumps(
        {
            "status": "passed",
            "profile": "cellml.ode",
            "image_id": "sha256:" + "b" * 64,
            "artifacts": {"driver.py": bytes_digest(driver)},
        }
    ).encode()
    for raw in (execution, driver):
        (bundle / bytes_digest(raw)).write_bytes(raw)
    evidence = NativeRuntimeEvidence(
        id="cellml.runtime",
        version="1.0.0",
        title="Test runtime",
        engine="libcellml",
        profile="cellml.ode",
        sources=(Reference(id=spec.id, version=spec.version),),
        image_digest="b" * 64,
        execution_digest=bytes_digest(execution),
        artifact_digests={"driver.py": bytes_digest(driver)},
        limitations=("Synthetic only",),
    )
    NativeRuntimeService(service.db, service.artifacts).import_evidence(evidence, bundle)
    return evidence, driver


def execution_bundle(service, tmp_path):
    package = service.admit(SPEC, RAW)
    runtime, driver = retained_runtime(service, tmp_path)
    result = {
        "status": "succeeded",
        "model_digest": package.content_digest,
        "driver_digest": bytes_digest(driver),
        "controls": CONTROLS.model_dump(mode="json"),
        "voi": {"name": "t", "units": "second", "component": "decay"},
        "columns": [{"name": "x", "units": "dimensionless", "component": "decay", "kind": "state"}],
    }
    csvfile = io.StringIO()
    writer = csv.writer(csvfile)
    writer.writerow(["time", "decay/x"])
    for i in range(101):
        t = i * 0.2
        writer.writerow([t, math.exp(-0.1 * t)])
    blobs = {
        "result.json": json.dumps(result).encode(),
        "series.csv": csvfile.getvalue().encode(),
        "generated.py": b"# retained generated code, never executed at import\n",
    }
    receipt = CellMLExecutionReceipt(
        id="cellml.test-run",
        version="1.0.0",
        model=REF,
        model_digest=package.content_digest,
        runtime=Reference(id=runtime.id, version=runtime.version),
        image_digest=runtime.image_digest,
        driver_digest=bytes_digest(driver),
        controls=CONTROLS,
        artifacts={k: bytes_digest(v) for k, v in blobs.items()},
        limitations=("Synthetic only",),
    )
    bundle = tmp_path / "result-bundle"
    bundle.mkdir()
    for k, raw in blobs.items():
        (bundle / receipt.artifacts[k]).write_bytes(raw)
    return receipt, bundle, blobs


def test_output_retention_checks_lineage_grid_hashes_and_reconstructs(service, tmp_path):
    receipt, bundle, blobs = execution_bundle(service, tmp_path)
    checksum = service.import_execution(receipt, bundle)
    assert (
        len(checksum) == 64
        and service.execution(Reference(id=receipt.id, version=receipt.version)) == receipt
    )
    assert service.executions() == (receipt,)
    assert service.import_execution(receipt, bundle) == checksum
    corrupt = {**blobs, "series.csv": blobs["series.csv"].replace(b"1.0", b"nan", 1)}
    with pytest.raises(InvalidRequest):
        service.validate_result(receipt, corrupt)
    result = json.loads(blobs["result.json"])
    result["columns"][0]["units"] = "mole"
    raw = json.dumps(result).encode()
    (bundle / bytes_digest(raw)).write_bytes(raw)
    altered = receipt.model_copy(
        update={"artifacts": {**receipt.artifacts, "result.json": bytes_digest(raw)}}
    )
    with pytest.raises(Conflict, match="units"):
        service.import_execution(altered, bundle)
    with pytest.raises(Conflict, match="identity"):
        service.import_execution(receipt.model_copy(update={"image_digest": "c" * 64}), bundle)
    (bundle / receipt.artifacts["series.csv"]).write_bytes(b"corrupt")
    with pytest.raises(Conflict):
        service.import_execution(receipt, bundle)


def test_cellml_http_is_authenticated_and_never_executes_or_imports_results(tmp_path):
    with TestClient(create_app(tmp_path / "api", ADMIN, READER)) as client:
        assert client.get("/v1/cellml-models").status_code == 401
        body = {"spec": SPEC.model_dump(mode="json"), "xml_text": RAW.decode()}
        assert client.post("/v1/cellml-models", headers=READ, json=body).status_code == 403
        response = client.post("/v1/cellml-models", headers=AUTH, json=body)
        assert response.status_code == 201
        path = response.headers["Location"]
        assert client.get(path, headers=READ).json()["content_digest"] == bytes_digest(RAW)
        assert client.get(path + "/content", headers=READ).content == RAW
        assert client.get("/v1/cellml-models?limit=51", headers=READ).status_code == 422
        assert client.get("/v1/cellml-models?offset=-1", headers=READ).status_code == 422
        assert client.get("/v1/cellml-executions", headers=READ).json() == []
        assert client.post("/v1/cellml-executions", headers=AUTH, json={}).status_code == 405
        svc = client.app.state.service
        receipt, bundle, _ = execution_bundle(CellMLService(svc.db, svc.artifacts), tmp_path)
        CellMLService(svc.db, svc.artifacts).import_execution(receipt, bundle)
        url = "/v1/cellml-executions/cellml.test-run/versions/1.0.0"
        assert client.get(url, headers=READ).json()["managed_queue_supported"] is False
        assert client.get(url + "/artifacts/series.csv", headers=READ).status_code == 200
        assert client.get(url + "/artifacts/missing", headers=READ).status_code == 404


def test_operator_deadline_cleans_up_the_acknowledged_container(monkeypatch, tmp_path):
    calls = []

    def run(args, **kwargs):
        calls.append(args)
        if args[1] == "start":
            raise subprocess.TimeoutExpired(args, 1)
        return subprocess.CompletedProcess(args, 0)

    monkeypatch.setattr(subprocess, "run", run)
    output = tmp_path / "timeout"
    with pytest.raises(InvalidRequest, match="deadline"):
        run_isolated_cellml(RAW, CONTROLS, "sha256:" + "b" * 64, output, 1)
    assert [c[1] for c in calls] == ["create", "start", "rm"]
    assert calls[0][calls[0].index("--network") + 1] == "none"
    assert calls[0][calls[0].index("--pull") + 1] == "never"
    assert output.stat().st_mode & 0o777 == 0o700
    assert not (output / "result.json").exists()
