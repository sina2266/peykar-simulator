import json

import pytest
from fastapi.testclient import TestClient
from typer.testing import CliRunner

from peykar.api.app import create_app
from peykar.biology.knowledge_gaps import KnowledgeGapService
from peykar.cli import app
from peykar.examples import SYNTHETIC, reference_model
from peykar_core.contracts import KnowledgeGap, Reference, ScientificContext
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import digest

ADMIN = "knowledge-gap-admin-token-at-least-24-chars"
READER = "knowledge-gap-reader-token-at-least-24-chars"
ADMIN_HEADERS = {"Authorization": f"Bearer {ADMIN}"}
READ_HEADERS = {"Authorization": f"Bearer {READER}"}


def gap(id="gap.reference", **updates):
    payload = {
        "id": id,
        "description": "Independent biological reference evidence is unavailable for this fixture.",
        "context": SYNTHETIC,
        "affected_models": (Reference(id="reference.decay", version="0.3.0"),),
        "severity": "blocking",
    }
    return KnowledgeGap(**(payload | updates))


def test_gap_registry_persists_pinned_references_and_immutable_identity(service):
    registry = KnowledgeGapService(service.db)
    record = gap()
    assert registry.register(record) == digest(record)
    assert registry.register(record) == digest(record)
    assert KnowledgeGapService(service.db).get(record.id) == record
    with pytest.raises(Conflict):
        registry.register(gap(description="A different unknown must receive a new ID"))
    assert registry.get(record.id) == record
    assert service.models.get(record.affected_models[0]).validity.status == "unvalidated"
    assert registry.discover() == (record,)


def test_gap_registry_rejects_unknown_exact_model_version_without_writes(service):
    registry = KnowledgeGapService(service.db)
    missing = gap(affected_models=(Reference(id="reference.decay", version="99.0.0"),))
    with pytest.raises(NotFound):
        registry.register(missing)
    with pytest.raises(NotFound):
        registry.get(missing.id)
    # General missing knowledge need not invent an affected model.
    general = gap("gap.general", affected_models=())
    registry.register(general)
    assert registry.get(general.id).affected_models == ()


@pytest.mark.parametrize(
    "updates",
    [
        {"description": " \n\t"},
        {"description": "a" * 20_001},
        {"affected_models": (Reference(id="reference.decay", version="0.3.0"),) * 2},
        {"context": ScientificContext(species=" ", biological_system="unknown")},
    ],
)
def test_gap_registry_rejects_empty_or_ambiguous_declarations(service, updates):
    with pytest.raises(InvalidRequest):
        KnowledgeGapService(service.db).register(gap(**updates))


def test_filters_apply_before_pagination_and_preserve_exact_model_versions(service):
    registry = KnowledgeGapService(service.db)
    new_model = reference_model().model_copy(update={"version": "0.3.1"})
    service.models.register(new_model)
    registry.register(gap("a.unfiltered", severity="informational"))
    target_context = ScientificContext(
        species="synthetic", biological_system="local_scalar_fixture", conditions=("holdout",)
    )
    references = (Reference(id="reference.decay", version="0.3.1"),)
    first = gap("b.target", context=target_context, affected_models=references)
    second = gap("c.target", context=target_context, affected_models=references)
    registry.register(first)
    registry.register(second)
    options = dict(
        species=target_context.species,
        biological_system=target_context.biological_system,
        condition="holdout",
        severity="blocking",
        model_id="reference.decay",
        model_version="0.3.1",
        limit=1,
    )
    assert registry.discover(**options) == (first,)
    assert registry.discover(**options, offset=1) == (second,)
    assert registry.discover(**(options | {"model_version": "0.3.0"})) == ()
    assert registry.discover(condition="' OR 1=1 --") == ()
    assert len(registry.discover(model_id="reference.decay")) == 3
    with pytest.raises(InvalidRequest, match="Model version"):
        registry.discover(model_version="0.3.0")
    with pytest.raises(InvalidRequest, match="pagination"):
        registry.discover(limit=201)


def test_gap_read_and_list_detect_integrity_changes(service):
    registry = KnowledgeGapService(service.db)
    record = gap()
    registry.register(record)
    changed = gap(description="Tampered description").model_dump_json()
    with service.db.connection(immediate=True) as db:
        db.execute("UPDATE objects SET payload=? WHERE kind='knowledge_gap'", (changed,))
    with pytest.raises(Conflict, match="integrity"):
        registry.get(record.id)
    with pytest.raises(Conflict, match="integrity"):
        registry.discover()


def test_gap_api_requires_admin_write_and_authenticated_reads(service):
    with TestClient(create_app(service.root, ADMIN, READER)) as client:
        body = gap().model_dump(mode="json")
        assert client.get("/v1/knowledge-gaps").status_code == 401
        assert client.get("/v1/knowledge-gaps/gap.reference").status_code == 401
        assert client.post("/v1/knowledge-gaps", json=body).status_code == 401
        assert client.post("/v1/knowledge-gaps", json=body, headers=READ_HEADERS).status_code == 403
        created = client.post("/v1/knowledge-gaps", json=body, headers=ADMIN_HEADERS)
        assert created.status_code == 201, created.text
        assert created.headers["Location"] == "/v1/knowledge-gaps/gap.reference"
        assert created.json()["digest"] == digest(gap())
        for headers in (READ_HEADERS, ADMIN_HEADERS):
            read = client.get(created.headers["Location"], headers=headers)
            assert read.status_code == 200 and read.json() == body
        assert (
            client.post("/v1/knowledge-gaps", json=body, headers=ADMIN_HEADERS).json()
            == created.json()
        )
        body["description"] = "Changed declarations cannot overwrite a gap"
        assert (
            client.post("/v1/knowledge-gaps", json=body, headers=ADMIN_HEADERS).status_code == 409
        )
        original = client.get(created.headers["Location"], headers=READ_HEADERS)
        assert original.json() == gap().model_dump(mode="json")


def test_gap_api_exact_refs_filters_schema_and_pagination_errors(service):
    with TestClient(create_app(service.root, ADMIN, READER)) as client:
        missing = gap(affected_models=(Reference(id="absent.model", version="1.0.0"),))
        response = client.post(
            "/v1/knowledge-gaps", json=missing.model_dump(mode="json"), headers=ADMIN_HEADERS
        )
        assert response.status_code == 404
        assert client.get("/v1/knowledge-gaps", headers=READ_HEADERS).json() == []
        for record in (gap("a.informational", severity="informational"), gap("z.blocking")):
            assert (
                client.post(
                    "/v1/knowledge-gaps", json=record.model_dump(mode="json"), headers=ADMIN_HEADERS
                ).status_code
                == 201
            )
        response = client.get(
            "/v1/knowledge-gaps",
            params={"severity": "blocking", "model_id": "reference.decay", "limit": 1},
            headers=READ_HEADERS,
        )
        assert [item["id"] for item in response.json()] == ["z.blocking"]
        for params in (
            {"limit": 201},
            {"offset": -1},
            {"severity": "approved"},
            {"model_version": "latest"},
            {"model_version": "0.3.0"},
        ):
            assert (
                client.get("/v1/knowledge-gaps", params=params, headers=READ_HEADERS).status_code
                == 422
            )
        assert client.get("/v1/knowledge-gaps/unknown", headers=READ_HEADERS).status_code == 404
        body = gap("gap.invalid").model_dump(mode="json") | {"approved": True}
        assert (
            client.post("/v1/knowledge-gaps", json=body, headers=ADMIN_HEADERS).status_code == 422
        )


def test_gap_cli_register_show_and_filter(tmp_path, monkeypatch):
    monkeypatch.setenv("PEYKAR_DATA_DIR", str(tmp_path / "state"))
    runner = CliRunner()
    assert runner.invoke(app, ["init"]).exit_code == 0
    file = tmp_path / "gap.json"
    file.write_text(gap().model_dump_json())
    registered = runner.invoke(app, ["knowledge-gaps", "register", str(file)])
    assert registered.exit_code == 0, registered.output
    assert json.loads(registered.output)["digest"] == digest(gap())
    read = runner.invoke(app, ["knowledge-gaps", "show", "gap.reference"])
    assert read.exit_code == 0 and json.loads(read.output) == gap().model_dump(mode="json")
    listed = runner.invoke(
        app,
        ["knowledge-gaps", "list", "--severity", "blocking", "--model-id", "reference.decay"],
    )
    assert listed.exit_code == 0, listed.output
    assert [item["id"] for item in json.loads(listed.output)] == ["gap.reference"]
