import json

import pytest
from fastapi.testclient import TestClient

from peykar.api.app import create_app
from peykar.model_import.libraries import LibraryService
from peykar.model_import.native_runtimes import NativeRuntimeService
from peykar_core.contracts import (
    LibrarySourceReceipt,
    LibrarySourceSpec,
    NativeRuntimeEvidence,
    Reference,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import bytes_digest


def fixture(service, tmp_path):
    notice = b"Original fixture author notice"
    inventory = json.dumps([{"path": "LICENSE", "bytes": len(notice)}]).encode()
    source = LibrarySourceReceipt(
        spec=LibrarySourceSpec(
            id="native.fixture.source",
            version="1.0.0",
            title="Synthetic fixture source",
            repository="https://github.com/example/fixture",
            commit="a" * 40,
            license="Fixture",
            license_files=("LICENSE",),
            capabilities=("test",),
            limitations=("Synthetic test source, never executed.",),
        ),
        inventory_digest=bytes_digest(inventory),
        license_digests={"LICENSE": bytes_digest(notice)},
        tracked_files=1,
        tracked_bytes=len(notice),
    )
    LibraryService(service.db, service.artifacts).import_receipt(
        source, inventory, {"LICENSE": notice}
    )
    raw = b"time,value\n0,1\n1,0.9\n"
    artifacts = {"native.csv": bytes_digest(raw)}
    execution = json.dumps(
        {
            "status": "passed",
            "profile": "fixture",
            "image_id": "sha256:" + "b" * 64,
            "artifacts": artifacts,
        }
    ).encode()
    evidence = NativeRuntimeEvidence(
        id="fixture.runtime",
        version="1.0.0",
        title="Synthetic retained execution fixture",
        engine="fixture",
        profile="fixture",
        sources=(Reference(id=source.spec.id, version="1.0.0"),),
        image_digest="b" * 64,
        execution_digest=bytes_digest(execution),
        artifact_digests=artifacts,
        limitations=("Operator assertion and synthetic numerical output, no scientific validity.",),
    )
    bundle = tmp_path / "bundle"
    bundle.mkdir()
    for value in (raw, execution):
        (bundle / bytes_digest(value)).write_bytes(value)
    return evidence, bundle


def test_native_evidence_persists_exact_artifacts_and_source_references(service, tmp_path):
    evidence, bundle = fixture(service, tmp_path)
    registry = NativeRuntimeService(service.db, service.artifacts)
    checksum = registry.import_evidence(evidence, bundle)
    assert registry.import_evidence(evidence, bundle) == checksum
    restored = NativeRuntimeService(service.db, service.artifacts)
    assert restored.get(Reference(id=evidence.id, version=evidence.version)) == evidence
    assert restored.discover("fixture") == (evidence,)
    assert restored.discover("absent") == ()
    with pytest.raises(Conflict):
        registry.import_evidence(evidence.model_copy(update={"title": "overwrite"}), bundle)


@pytest.mark.parametrize("mutation", ["checksum", "failed", "image", "source", "path"])
def test_native_evidence_rejects_mismatched_execution_atomically(service, tmp_path, mutation):
    evidence, bundle = fixture(service, tmp_path)
    if mutation == "checksum":
        (bundle / next(iter(evidence.artifact_digests.values()))).write_bytes(b"mutated")
    elif mutation == "failed":
        raw = json.loads((bundle / evidence.execution_digest).read_bytes())
        raw["status"] = "failed"
        data = json.dumps(raw).encode()
        checksum = bytes_digest(data)
        (bundle / checksum).write_bytes(data)
        evidence = evidence.model_copy(update={"execution_digest": checksum})
    elif mutation == "image":
        evidence = evidence.model_copy(update={"image_digest": "c" * 64})
    elif mutation == "source":
        evidence = evidence.model_copy(
            update={"sources": (Reference(id="unknown", version="1.0.0"),)}
        )
    else:
        evidence = evidence.model_copy(update={"artifact_digests": {"../outside": "c" * 64}})
    with pytest.raises((Conflict, InvalidRequest, NotFound)):
        NativeRuntimeService(service.db, service.artifacts).import_evidence(evidence, bundle)
    assert service.db.objects("native_runtime") == []


def test_native_evidence_checks_retained_integrity_and_read_auth(service, tmp_path):
    evidence, bundle = fixture(service, tmp_path)
    registry = NativeRuntimeService(service.db, service.artifacts)
    registry.import_evidence(evidence, bundle)
    reader = "native-evidence-reader-at-least-24-characters"
    app = create_app(
        data_dir=service.root, api_token="native-admin-at-least-24-characters", read_token=reader
    )
    with TestClient(app) as client:
        assert client.get("/v1/runtimes").status_code == 401
        headers = {"Authorization": "Bearer " + reader}
        rows = client.get("/v1/runtimes?engine=fixture", headers=headers)
        assert rows.status_code == 200 and rows.json()[0]["managed_queue_supported"] is False
        assert rows.json()[0]["scientific_validity"] == "not_established"
        assert client.get("/v1/runtimes?engine=absent", headers=headers).json() == []
        assert (
            client.get("/v1/runtimes/fixture.runtime/versions/1.0.0", headers=headers).status_code
            == 200
        )
    checksum = next(iter(evidence.artifact_digests.values()))
    service.artifacts._path(checksum).write_bytes(b"modified after registration")
    with pytest.raises(Conflict, match="integrity"):
        registry.get(Reference(id=evidence.id, version=evidence.version))
