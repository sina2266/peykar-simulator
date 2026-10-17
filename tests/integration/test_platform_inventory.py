import json
import subprocess

import pytest
from fastapi.testclient import TestClient

from peykar.api.app import create_app
from peykar.biology.persistent_graph import GraphService
from peykar.examples import SYNTHETIC
from peykar.model_import.libraries import LibraryService
from peykar_core.contracts import (
    BiologicalEntity,
    BiologicalGraphSpec,
    BiologicalRelationship,
    LibrarySourceSpec,
    Reference,
)
from peykar_core.errors import Conflict, InvalidRequest


def graph_spec():
    return BiologicalGraphSpec(
        id="graph.test",
        version="1.0.0",
        title="Synthetic hierarchy fixture",
        context=SYNTHETIC,
        entities=tuple(
            BiologicalEntity(id=id, label=id, kind="fixture", species="synthetic")
            for id in ("a", "b", "c")
        ),
        relationships=tuple(
            BiologicalRelationship(
                source=a,
                target=b,
                relation="part_of",
                context=SYNTHETIC,
                evidence=("Synthetic reference fixture, not biological evidence",),
            )
            for a, b in [("a", "b"), ("b", "c")]
        ),
    )


def test_persistent_graph_atomic_cycles_context_and_bounded_neighbors(service):
    registry = GraphService(service.db)
    spec = graph_spec()
    registry.register(spec)
    ref = Reference(id=spec.id, version=spec.version)
    assert GraphService(service.db).get(ref) == spec
    neighbors = registry.neighborhood(ref, "a", depth=2, max_nodes=2)
    assert len(neighbors["entities"]) == 2 and neighbors["truncated"]
    assert len(registry.neighborhood(ref, "a", depth=2)["entities"]) == 3
    with pytest.raises(Conflict):
        registry.register(spec.model_copy(update={"title": "changed"}))
    cycle = spec.relationships + (
        spec.relationships[0].model_copy(update={"source": "c", "target": "a"}),
    )
    with pytest.raises(InvalidRequest):
        registry.register(spec.model_copy(update={"id": "graph.cyclic", "relationships": cycle}))
    with pytest.raises(InvalidRequest):
        registry.register(
            spec.model_copy(
                update={
                    "id": "graph.blank",
                    "relationships": (
                        spec.relationships[0].model_copy(update={"evidence": (" ",)}),
                    ),
                }
            )
        )
    with pytest.raises(InvalidRequest):
        registry.register(
            spec.model_copy(
                update={
                    "id": "graph.species",
                    "context": SYNTHETIC.model_copy(update={"species": "other"}),
                }
            )
        )


def test_library_checks_pinned_bytes_not_only_git_head_and_retains_notices(service, tmp_path):
    source = tmp_path / "source"
    source.mkdir()

    def git(*args):
        return subprocess.check_output(["git", "-C", str(source), *args]).decode().strip()

    git("init", "--template=")
    (source / "LICENSE").write_text("Synthetic source fixture license notice\n")
    (source / "code.py").write_text('raise RuntimeError("This code must never run")\n')
    (source / "large.bin").write_text(
        "version https://git-lfs.github.com/spec/v1\noid sha256:" + "a" * 64 + "\nsize 10\n"
    )
    (source / "link").symlink_to("/private/never-read-this")
    git("add", ".")
    git(
        "-c",
        "user.name=Fixture",
        "-c",
        "user.email=fixture@example.invalid",
        "commit",
        "-m",
        "fixture",
    )
    pin = git("rev-parse", "HEAD")
    spec = LibrarySourceSpec(
        id="source.fixture",
        version="1.0.0",
        title="No-execution fixture",
        repository="https://github.com/example/fixture",
        commit=pin,
        license="Synthetic fixture",
        license_files=("LICENSE",),
        capabilities=("source_inspection",),
        limitations=("Not a real scientific library.",),
    )
    registry = LibraryService(service.db, service.artifacts)
    receipt = registry.inspect(spec, source)
    assert receipt.lfs_pointers == ("large.bin",)
    assert receipt.tracked_files == 4
    assert registry.get(Reference(id=spec.id, version=spec.version)) == receipt
    assert registry.inspect(spec, source) == receipt
    assert registry.discover("absent") == ()
    assert (
        service.artifacts.get(receipt.license_digests["LICENSE"])
        == (source / "LICENSE").read_bytes()
    )
    inventory = json.loads(service.artifacts.get(receipt.inventory_digest))
    assert next(item for item in inventory if item["path"] == "link")["bytes"] == len(
        "/private/never-read-this"
    )
    (source / "code.py").write_text("altered\n")
    with pytest.raises(Conflict):
        registry.inspect(spec, source)
    (source / "code.py").write_text('raise RuntimeError("This code must never run")\n')
    with pytest.raises(Conflict):
        registry.inspect(spec.model_copy(update={"title": "another title"}), source)
    with pytest.raises(InvalidRequest):
        registry.inspect(spec.model_copy(update={"license_files": ("../outside",)}), source)


def test_graph_and_library_api_roles_and_filtering(service):
    admin = "inventory-admin-at-least-24-characters"
    reader = "inventory-reader-at-least-24-characters"
    ah = {"Authorization": "Bearer " + admin}
    rh = {"Authorization": "Bearer " + reader}
    spec = graph_spec()
    with TestClient(create_app(service.root, admin, reader)) as client:
        assert client.get("/v1/biology/graphs").status_code == 401
        assert (
            client.post(
                "/v1/biology/graphs", json=spec.model_dump(mode="json"), headers=rh
            ).status_code
            == 403
        )
        response = client.post("/v1/biology/graphs", json=spec.model_dump(mode="json"), headers=ah)
        assert response.status_code == 201
        assert client.get(response.headers["Location"], headers=rh).json()["id"] == spec.id
        assert len(client.get("/v1/biology/graphs?species=synthetic", headers=rh).json()) == 1
        assert client.get("/v1/biology/graphs?species=other", headers=rh).json() == []
        assert (
            client.get(
                response.headers["Location"] + "/neighborhood?entity_id=a&depth=2", headers=rh
            ).status_code
            == 200
        )
        assert (
            client.get(
                response.headers["Location"] + "/neighborhood?entity_id=a&depth=5", headers=rh
            ).status_code
            == 422
        )
        assert client.get("/v1/libraries", headers=rh).json() == []
        assert client.post(
            "/v1/libraries", json={"source": "/etc/passwd"}, headers=ah
        ).status_code in (404, 405)


def test_source_receipt_import_checks_artifacts_before_registering(service, tmp_path):
    from peykar_core.contracts import LibrarySourceReceipt
    from peykar_core.hashing import bytes_digest, canonical_json

    spec = LibrarySourceSpec(
        id="source.bundle",
        version="1.0.0",
        title="Trusted operator source bundle",
        repository="https://github.com/example/fixture",
        commit="a" * 40,
        license="Fixture notice",
        license_files=("LICENSE",),
        capabilities=("source_inspection",),
        limitations=("Fixture, not a biological library.",),
    )
    raw = b"Fixture notice\n"
    inventory = canonical_json(
        [{"path": "LICENSE", "bytes": len(raw), "sha256": bytes_digest(raw)}]
    ).encode()
    receipt = LibrarySourceReceipt(
        spec=spec,
        inventory_digest=bytes_digest(inventory),
        license_digests={"LICENSE": bytes_digest(raw)},
        tracked_files=1,
        tracked_bytes=len(raw),
    )
    registry = LibraryService(service.db, service.artifacts)
    with pytest.raises(Conflict):
        registry.import_receipt(receipt, inventory, {"LICENSE": b"changed"})
    with pytest.raises(Conflict):
        registry.import_receipt(receipt, b"[]", {"LICENSE": raw})
    assert registry.import_receipt(receipt, inventory, {"LICENSE": raw}) == receipt
    assert registry.get(Reference(id=spec.id, version=spec.version)) == receipt


def test_git_declared_crlf_is_retained_but_other_modification_is_rejected(service, tmp_path):
    source = tmp_path / "eol"
    source.mkdir()

    def git(*args):
        return subprocess.check_output(["git", "-C", str(source), *args]).decode().strip()

    git("init", "--template=")
    (source / ".gitattributes").write_text("LICENSE text eol=crlf\n")
    (source / "LICENSE").write_bytes(b"Fixture\nLicense\n")
    git("add", ".")
    git(
        "-c",
        "user.name=Fixture",
        "-c",
        "user.email=fixture@example.invalid",
        "commit",
        "-m",
        "fixture",
    )
    (source / "LICENSE").write_bytes(b"Fixture\r\nLicense\r\n")
    spec = LibrarySourceSpec(
        id="source.crlf",
        version="1.0.0",
        title="Git EOL test",
        repository="https://github.com/example/fixture",
        commit=git("rev-parse", "HEAD"),
        license="Fixture",
        license_files=("LICENSE",),
        capabilities=("source_inspection",),
        limitations=("Synthetic fixture",),
    )
    registry = LibraryService(service.db, service.artifacts)
    receipt = registry.inspect(spec, source)
    inventory = json.loads(service.artifacts.get(receipt.inventory_digest))
    assert (
        next(i for i in inventory if i["path"] == "LICENSE")["checkout_transform"]
        == "declared_text_crlf_only"
    )
    assert service.artifacts.get(receipt.license_digests["LICENSE"]) == b"Fixture\r\nLicense\r\n"
    (source / "LICENSE").write_bytes(b"Changed\r\nLicense\r\n")
    with pytest.raises(Conflict):
        registry.inspect(spec, source)
