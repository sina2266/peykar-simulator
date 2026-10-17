import json
import math
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import pytest
from fastapi.testclient import TestClient

from peykar.api.app import create_app
from peykar.examples import SYNTHETIC
from peykar.model_import.libraries import LibraryService
from peykar.model_import.native_models import NativeModelService, inspect_xml
from peykar.model_import.native_runtimes import NativeRuntimeService
from peykar.simulation import native_agent, native_jobs
from peykar.simulation.native_agent import NativeAgent, NativeAgentClient, bounded_output
from peykar.simulation.native_jobs import NativeJobService
from peykar.simulation.native_outputs import validate_native_outputs
from peykar_core.contracts import (
    LibrarySourceReceipt,
    LibrarySourceSpec,
    NativeExecutionProfile,
    NativeModelSpec,
    NativeRunResult,
    NativeRunSpec,
    NativeRuntimeEvidence,
    Quantity,
    Reference,
)
from peykar_core.errors import Cancelled, Conflict, InvalidRequest, OwnershipLost
from peykar_core.hashing import bytes_digest, digest

ROOT = Path(__file__).resolve().parents[2]
SBML = (ROOT / "model_packages/native/transfer/1.0.0/model.sbml").read_bytes()


@pytest.fixture
def native_fixture(service, tmp_path):
    notice = b"Synthetic source author notice"
    inventory = json.dumps([{"path": "LICENSE", "bytes": len(notice)}]).encode()
    source = LibrarySourceReceipt(
        spec=LibrarySourceSpec(
            id="source.native",
            version="1.0.0",
            title="Fixture source",
            repository="https://github.com/example/fixture",
            commit="a" * 40,
            license="Synthetic",
            license_files=("LICENSE",),
            capabilities=("test",),
            limitations=("No real scientific source is executed by this fixture.",),
        ),
        inventory_digest=bytes_digest(inventory),
        license_digests={"LICENSE": bytes_digest(notice)},
        tracked_files=1,
        tracked_bytes=len(notice),
    )
    LibraryService(service.db, service.artifacts).import_receipt(
        source, inventory, {"LICENSE": notice}
    )
    bundle = tmp_path / "evidence"
    bundle.mkdir()
    raw = b"Synthetic retained native output"
    execution = json.dumps(
        {
            "status": "passed",
            "profile": "copasi",
            "image_id": "sha256:" + "b" * 64,
            "artifacts": {"fixture.csv": bytes_digest(raw)},
        }
    ).encode()
    evidence = NativeRuntimeEvidence(
        id="runtime.copasi",
        version="1.0.0",
        title="Fixture evidence",
        engine="copasi",
        profile="copasi",
        sources=(Reference(id="source.native", version="1.0.0"),),
        image_digest="b" * 64,
        execution_digest=bytes_digest(execution),
        artifact_digests={"fixture.csv": bytes_digest(raw)},
        limitations=("Synthetic native protocol fixture, not actual solver evidence.",),
    )
    for value in (raw, execution):
        (bundle / bytes_digest(value)).write_bytes(value)
    NativeRuntimeService(service.db, service.artifacts).import_evidence(evidence, bundle)
    profile = NativeExecutionProfile(
        id="copasi.sbml",
        version="1.0.0",
        kind="copasi_sbml",
        runtime=Reference(id=evidence.id, version=evidence.version),
        driver_digest="c" * 64,
        limitations=("Synthetic queue fixture.",),
    )
    models = NativeModelService(service.db, service.artifacts)
    models.register_profile(profile)
    spec = NativeModelSpec(
        id="native.fixture",
        version="1.0.0",
        title="Fixture SBML",
        format="sbml",
        context=SYNTHETIC,
        license="Fixture",
        source="Analytical reaction fixture",
        limitations=("No biological validity inferred.",),
    )
    package = models.admit(spec, SBML)
    plan = NativeRunSpec(
        model=Reference(id=spec.id, version=spec.version),
        profile=Reference(id=profile.id, version=profile.version),
        duration=Quantity(value=10.0, unit="second"),
        intervals=10,
    )
    return NativeJobService(service), models, package, profile, plan


def output(claim):
    raw = (
        "Time,A,B\n"
        + "".join(f"{t},{math.exp(-0.1 * t)},{1 - math.exp(-0.1 * t)}\n" for t in range(11))
    ).encode()
    metadata = json.dumps(
        {
            "engine": "COPASI",
            "samples": 11,
            "scientific_validity": "not_established",
            "model_units": {"time_unit": "s"},
        }
    ).encode()
    blobs = {"native.csv": raw, "quantities.json": metadata}
    result = NativeRunResult(
        run_id=claim.manifest.run_id,
        manifest_digest=digest(claim.manifest),
        image_digest=claim.manifest.runtime_snapshot.image_digest,
        driver_digest=claim.manifest.profile_snapshot.driver_digest,
        artifacts={name: bytes_digest(raw) for name, raw in blobs.items()},
        limitations=("Synthetic protocol output, not real solver execution.",),
    )
    return result, blobs


def test_native_admission_preserves_raw_units_and_immutable_versions(native_fixture):
    _, models, package, profile, _ = native_fixture
    assert models.get(Reference(id=package.spec.id, version=package.spec.version)) == package
    assert package.inspection.parameter_units == {"k": "per_second"}
    assert package.inspection.species_units == {"A": "mole", "B": "mole"}
    assert package.inspection.compartment_units == {"cell": "litre"}
    assert package.inspection.model_time_unit == "second"
    assert models.admit(package.spec, SBML) == package
    with pytest.raises(Conflict):
        models.admit(package.spec, SBML.replace(b"0.1", b"0.2"))
    assert models.profile(Reference(id=profile.id, version=profile.version)) == profile
    with pytest.raises(InvalidRequest):
        models.register_profile(profile.model_copy(update={"kind": "osp_pkml"}))


@pytest.mark.parametrize(
    "raw", [b'<!DOCTYPE a [<!ENTITY boom "x">]><a/>', b"<sbml/>", b"<a>\x00</a>", b"bad xml"]
)
def test_native_xml_rejects_unsupported_or_active_documents(raw):
    with pytest.raises(InvalidRequest):
        inspect_xml("sbml", raw)


def test_absent_sbml_units_stay_absent():
    inspection = inspect_xml("sbml", SBML.replace(b' units="per_second"', b""))
    assert inspection.parameter_units["k"] is None
    assert any("absent" in warning for warning in inspection.warnings)


def test_native_queue_idempotency_concurrent_claim_and_retained_results(native_fixture):
    jobs, _, _, profile, plan = native_fixture
    run = jobs.submit(plan, "fixed-request")
    assert jobs.submit(plan, "fixed-request").id == run.id
    with pytest.raises(Conflict):
        jobs.submit(plan.model_copy(update={"intervals": 5}), "fixed-request")
    ref = Reference(id=profile.id, version=profile.version)
    with ThreadPoolExecutor(max_workers=2) as pool:
        claims = list(pool.map(lambda _: jobs.claim((ref,)), range(2)))
    claim = next(c for c in claims if c is not None)
    assert sum(c is not None for c in claims) == 1
    result, blobs = output(claim)
    assert not jobs.heartbeat(run.id, claim.token)
    assert jobs.finish(run.id, claim.token, "succeeded", result, blobs).status == "succeeded"
    restored = NativeJobService(jobs.service)
    assert restored.result(run.id) == result
    assert [e["event"] for e in restored.events(run.id)] == ["submitted", "started", "succeeded"]
    with pytest.raises(OwnershipLost):
        jobs.finish(run.id, claim.token, "failed")


def test_native_cancel_wins_a_success_race_and_pending_cancel_skips_claim(native_fixture):
    jobs, _, _, profile, plan = native_fixture
    ref = Reference(id=profile.id, version=profile.version)
    pending = jobs.submit(plan)
    assert jobs.cancel(pending.id).status == "cancelled"
    assert jobs.claim((ref,)) is None
    run = jobs.submit(plan)
    claim = jobs.claim((ref,))
    jobs.cancel(run.id)
    assert jobs.heartbeat(run.id, claim.token)
    result, blobs = output(claim)
    completed = jobs.finish(run.id, claim.token, "succeeded", result, blobs)
    assert completed.status == "cancelled" and completed.result_digest is None
    with pytest.raises(Conflict):
        jobs.result(run.id)


def test_expired_native_agent_cannot_revive_or_publish(native_fixture, monkeypatch):
    jobs, _, _, profile, plan = native_fixture
    run = jobs.submit(plan)
    claim = jobs.claim((Reference(id=profile.id, version=profile.version),))
    clock = native_jobs.time.time()
    monkeypatch.setattr(native_jobs.time, "time", lambda: clock + 31)
    with pytest.raises(OwnershipLost):
        jobs.heartbeat(run.id, claim.token)
    with pytest.raises(OwnershipLost):
        jobs.finish(run.id, claim.token, "succeeded", *output(claim))
    assert jobs.recover_stale() == 1
    assert jobs.run(run.id).error == "agent_lost"


@pytest.mark.parametrize("mutation", ["hash", "image", "grid", "nan", "units", "shape"])
def test_native_agent_output_envelope_cannot_falsely_succeed(native_fixture, mutation):
    jobs, _, _, profile, plan = native_fixture
    run = jobs.submit(plan)
    claim = jobs.claim((Reference(id=profile.id, version=profile.version),))
    result, blobs = output(claim)
    if mutation == "image":
        result = result.model_copy(update={"image_digest": "d" * 64})
    elif mutation == "hash":
        blobs["native.csv"] += b"changed"
    else:
        if mutation == "grid":
            blobs["native.csv"] = blobs["native.csv"].replace(b"10,", b"11,")
        if mutation == "nan":
            blobs["native.csv"] = blobs["native.csv"].replace(b"0,1.0", b"0,nan")
        if mutation == "units":
            blobs["quantities.json"] = blobs["quantities.json"].replace(b'"s"', b'"min"')
        if mutation == "shape":
            blobs["native.csv"] += b"ragged\n"
        result = result.model_copy(
            update={"artifacts": {name: bytes_digest(raw) for name, raw in blobs.items()}}
        )
    with pytest.raises((InvalidRequest, Conflict)):
        jobs.finish(run.id, claim.token, "succeeded", result, blobs)
    assert jobs.run(run.id).status == "running"


def test_native_http_roles_model_bytes_job_lifecycle_and_leases(native_fixture):
    jobs, _, package, _, plan = native_fixture
    admin, reader = (
        "native-job-admin-at-least-24-characters",
        "native-job-reader-at-least-24-characters",
    )
    app = create_app(data_dir=jobs.service.root, api_token=admin, read_token=reader)
    ah, rh = {"Authorization": "Bearer " + admin}, {"Authorization": "Bearer " + reader}
    with TestClient(app) as client:
        assert client.get("/v1/native-models").status_code == 401
        body = {"spec": package.spec.model_dump(mode="json"), "xml_text": SBML.decode()}
        assert client.post("/v1/native-models", headers=rh, json=body).status_code == 403
        admitted = client.post("/v1/native-models", headers=ah, json=body)
        assert admitted.status_code == 201
        assert client.get(admitted.headers["location"] + "/content", headers=rh).content == SBML
        created = client.post(
            "/v1/native-runs",
            headers={**ah, "Idempotency-Key": "http-key"},
            json=plan.model_dump(mode="json"),
        )
        assert created.status_code == 201
        id = created.json()["id"]
        assert (
            client.post(
                "/v1/native-runs/claim",
                headers=rh,
                json={"profiles": [plan.profile.model_dump(mode="json")]},
            ).status_code
            == 403
        )
        claim = client.post(
            "/v1/native-runs/claim",
            headers=ah,
            json={"profiles": [plan.profile.model_dump(mode="json")]},
        ).json()
        assert claim["manifest"]["run_id"] == id
        assert (
            client.post(
                f"/v1/native-runs/{id}/heartbeat",
                headers={**ah, "Native-Claim-Token": "wrong"},
                json={},
            ).status_code
            == 409
        )
        assert client.post(f"/v1/native-runs/{id}/cancel", headers=rh).status_code == 403
        assert client.post(f"/v1/native-runs/{id}/cancel", headers=ah).status_code == 200
        ended = client.post(
            f"/v1/native-runs/{id}/complete",
            headers={**ah, "Native-Claim-Token": claim["token"]},
            json={"status": "cancelled"},
        )
        assert ended.status_code == 200 and ended.json()["status"] == "cancelled"
        assert client.get(f"/v1/native-runs/{id}/results", headers=rh).status_code == 409


@pytest.mark.parametrize(
    "url",
    [
        "http://example.com",
        "https://user:password@example.com",
        "file:///private",
        "https://example.com/?secret=value",
    ],
)
def test_native_agent_does_not_send_credentials_to_unsafe_urls(url):
    with pytest.raises(InvalidRequest):
        NativeAgentClient(url, "test-token-at-least-24-characters")


def test_native_output_symlink_is_rejected_before_following_host_path(tmp_path):
    secret = tmp_path / "outside"
    secret.write_bytes(b"private data")
    link = tmp_path / "native.csv"
    link.symlink_to(secret)
    with pytest.raises(OSError):
        bounded_output(link)


def test_native_additive_migration_preserves_registry_and_existing_jobs(native_fixture):
    jobs, _, _, _, plan = native_fixture
    run = jobs.submit(plan)
    with jobs.db.connection() as db:
        before = [
            tuple(row) for row in db.execute("SELECT * FROM objects ORDER BY kind,id,version")
        ]
        db.execute("DROP TABLE native_events")
    restored = NativeJobService(jobs.service)
    with jobs.db.connection() as db:
        assert db.execute("PRAGMA user_version").fetchone()[0] == 1
        assert before == [
            tuple(row) for row in db.execute("SELECT * FROM objects ORDER BY kind,id,version")
        ]
    assert restored.run(run.id) == run
    assert restored.events(run.id) == []


@pytest.mark.parametrize(
    "metadata_shape",
    ["valid", "list", "wrong_columns", "missing_unit", "fractional_id", "mixed_ids"],
)
def test_pkml_output_columns_match_reported_native_units(native_fixture, metadata_shape):
    jobs, _, _, _, plan = native_fixture
    run = jobs.submit(plan)
    manifest = jobs.manifest(run.id)
    manifest = manifest.model_copy(
        update={
            "profile_snapshot": manifest.profile_snapshot.model_copy(update={"kind": "osp_pkml"})
        }
    )
    quantities = {
        "Time": {"unit": "min", "dimension": "Time"},
        "Drug": {"unit": "µmol/l", "dimension": "Concentration (molar)"},
    }
    if metadata_shape == "list":
        quantities = []
    elif metadata_shape == "wrong_columns":
        quantities["extra"] = quantities["Drug"]
    elif metadata_shape == "missing_unit":
        quantities["Drug"]["unit"] = None
    blobs = {
        "native.csv": b"IndividualId,Time,Drug\n1,0,0\n1,1,2\n1,1,3\n1,2,1\n",
        "quantities.json": json.dumps(
            {
                "engine": "OSPSuite",
                "samples": 4,
                "quantities": quantities,
                "scientific_validity": "not_established",
            }
        ).encode(),
    }
    if metadata_shape == "fractional_id":
        blobs["native.csv"] = blobs["native.csv"].replace(b"1,0,0", b"0.5,0,0")
    elif metadata_shape == "mixed_ids":
        blobs["native.csv"] = blobs["native.csv"].replace(b"1,0,0", b"2,0,0")
    if metadata_shape == "valid":
        validate_native_outputs(manifest, blobs)
    else:
        with pytest.raises(InvalidRequest):
            validate_native_outputs(manifest, blobs)


@pytest.mark.parametrize("mode", ["succeeded", "cancelled", "timeout", "allowlist"])
def test_native_agent_protocol_supervision_and_local_admission(
    native_fixture, tmp_path, monkeypatch, mode
):
    import subprocess

    jobs, models, _, profile, plan = native_fixture
    driver = tmp_path / "driver.py"
    driver.write_text("# Explicit synthetic supervisor fixture; never executed\n")
    profile = profile.model_copy(
        update={"version": "1.0.1", "driver_digest": bytes_digest(driver.read_bytes())}
    )
    models.register_profile(profile)
    plan = plan.model_copy(
        update={
            "profile": Reference(id=profile.id, version=profile.version),
            "wall_seconds": 0.02 if mode == "timeout" else 180.0,
        }
    )
    run = jobs.submit(plan)
    catalog = tmp_path / "catalog.json"
    catalog.write_text(
        json.dumps(
            [
                {
                    "profile": {"id": profile.id, "version": profile.version},
                    "kind": profile.kind,
                    "driver_digest": profile.driver_digest,
                    "image_digest": ("d" if mode == "allowlist" else "b") * 64,
                    "driver_path": "driver.py",
                }
            ]
        )
    )
    admin = "native-agent-supervision-token-24-characters"
    app = create_app(data_dir=jobs.service.root, api_token=admin)
    calls = []

    class Proxy:
        def __init__(self, http):
            self.http = http

        def request(self, path, body=None, *, claim_token=None, binary=False):
            if mode == "cancelled" and path.endswith("/content"):
                jobs.cancel(run.id)
            headers = {"Authorization": "Bearer " + admin}
            if claim_token:
                headers["Native-Claim-Token"] = claim_token
            response = (
                self.http.get(path, headers=headers)
                if body is None
                else self.http.post(path, json=body, headers=headers)
            )
            assert response.status_code == 200, (response.status_code, response.text)
            return response.content if binary else response.json()

    class Process:
        returncode = None

        def __init__(self, command, **kwargs):
            calls.append(command)
            assert command[command.index("--network") + 1] == "none"
            assert command[command.index("--pull") + 1] == "never"
            assert "--read-only" in command and "--user" in command
            assert sum(arg.endswith(",readonly") for arg in command) == 4
            assert "PEYKAR_API_TOKEN" not in kwargs["env"]
            if mode == "succeeded":
                directory = tmp_path / "runs" / run.id
                claim = type("Claim", (), {"manifest": jobs.manifest(run.id)})()
                _, blobs = output(claim)
                for name, raw in blobs.items():
                    (directory / name).write_bytes(raw)
                (directory / "native-completion.json").write_text('{"status":"succeeded"}')
                self.returncode = 0

        def poll(self):
            return self.returncode

        def terminate(self):
            self.returncode = -15

        def wait(self, timeout):
            return self.returncode

    monkeypatch.setattr(
        native_agent.subprocess, "check_output", lambda *a, **k: "sha256:" + "b" * 64
    )
    monkeypatch.setattr(native_agent.subprocess, "Popen", Process)
    monkeypatch.setattr(
        native_agent.subprocess,
        "run",
        lambda command, **kwargs: calls.append(command) or subprocess.CompletedProcess(command, 0),
    )
    with TestClient(app) as http:
        agent = NativeAgent(Proxy(http), catalog, tmp_path / "runs")
        if mode == "succeeded":
            assert agent.run_once() == run.id
            assert jobs.result(run.id).run_id == run.id
            assert agent.run_once() is None
        else:
            expected = {"cancelled": Cancelled, "timeout": TimeoutError, "allowlist": Conflict}
            with pytest.raises(expected[mode]):
                agent.run_once()
    assert jobs.run(run.id).status == ("failed" if mode in ("allowlist", "timeout") else mode)
    if mode == "allowlist":
        assert calls == []
    else:
        assert calls[-1][1:3] == ["rm", "-f"]
