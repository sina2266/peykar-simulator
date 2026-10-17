from pathlib import Path

import pytest
from fastapi.testclient import TestClient

from peykar.api.app import create_app
from peykar.composition.experiments import ExperimentService
from peykar.composition.hybrid_flows import HybridFlowService
from peykar.examples import bootstrap, reference_plan
from peykar.model_import.native_models import NativeModelService
from peykar_core.contracts import (
    ExperimentNode,
    ExperimentSpec,
    HybridFlowNode,
    HybridFlowSpec,
    NativeModelSpec,
    Reference,
)

ADMIN = "workspace-test-only-admin-at-least-24"
READER = "workspace-test-only-reader-at-least-24"
AUTH = {"Authorization": f"Bearer {ADMIN}"}
READ = {"Authorization": f"Bearer {READER}"}
ROOT = Path(__file__).resolve().parents[2]


@pytest.fixture
def client(tmp_path):
    app = create_app(tmp_path / "workspace", ADMIN, READER)
    with TestClient(app) as client:
        bootstrap(app.state.service)
        yield client


def test_session_roles_and_no_store(client):
    assert client.get("/v1/workspace/session").status_code == 401
    response = client.get("/v1/workspace/session", headers=READ)
    assert response.json() == {"role": "reader", "scope": "local_workspace"}
    assert response.headers["Cache-Control"] == "no-store"
    assert ADMIN not in response.text and READER not in response.text
    assert client.get("/v1/workspace/session", headers=AUTH).json()["role"] == "admin"
    assert client.post("/v1/models", headers=READ, json={}).status_code == 403


@pytest.mark.parametrize("kind", ["dataset", "scenario", "evaluation_set", "validation_record"])
def test_catalog_discovery_requires_auth_and_bounds(client, kind):
    url = f"/v1/workspace/catalog/{kind}"
    assert client.get(url).status_code == 401
    assert client.get(url, headers=READ).status_code == 200
    assert client.get(url + "?limit=51", headers=READ).status_code == 422
    assert client.get(url + "?offset=-1", headers=READ).status_code == 422
    assert client.get(url + "?offset=9999", headers=READ).json() == []


def test_catalog_allowlist_and_payload_integrity(client):
    assert client.get("/v1/workspace/catalog/native_derivation", headers=AUTH).status_code == 422
    response = client.get("/v1/workspace/catalog/dataset", headers=READ)
    assert response.json() and set(response.json()[0]) == {"id", "version", "digest", "title"}
    with client.app.state.service.db.connection() as db:
        db.execute("UPDATE objects SET digest=? WHERE kind='dataset'", ("a" * 64,))
    assert client.get("/v1/workspace/catalog/dataset", headers=READ).status_code == 409


@pytest.mark.parametrize("kind", ["experiments", "flows"])
def test_persisted_parent_run_discovery(client, kind):
    svc = client.app.state.service
    if kind == "experiments":
        engine = ExperimentService(svc)
        engine.freeze(
            ExperimentSpec(
                id="ui.workflow",
                version="1.0.0",
                title="UI reference DAG",
                nodes=(ExperimentNode(id="one", plan=reference_plan()),),
                limitations=("Synthetic only",),
            )
        )
        endpoint = "/v1/experiment-runs"
    else:
        engine = HybridFlowService(svc)
        engine.freeze(
            HybridFlowSpec(
                id="ui.workflow",
                version="1.0.0",
                title="UI flow",
                nodes=(HybridFlowNode(id="one", plan=reference_plan()),),
                limitations=("Synthetic only",),
            )
        )
        endpoint = "/v1/hybrid-runs"
    first = engine.submit(Reference(id="ui.workflow", version="1.0.0"), "ui-first")
    second = engine.submit(Reference(id="ui.workflow", version="1.0.0"), "ui-second")
    assert client.get(endpoint).status_code == 401
    assert client.get(endpoint + "?limit=1", headers=READ).json()[0]["id"] == second.id
    assert client.get(endpoint + "?limit=1&offset=1", headers=READ).json()[0]["id"] == first.id
    assert client.get(endpoint + "?limit=51", headers=READ).status_code == 422
    # A fresh service instance reconstructs the same saved parent and child state.
    assert client.get(endpoint + f"/{first.id}", headers=READ).json()["id"] == first.id


def test_native_model_pagination_preserves_old_default(client):
    models = NativeModelService(client.app.state.service.db, client.app.state.service.artifacts)
    plan = reference_plan()
    for name in ("native.one", "native.two"):
        models.admit(
            NativeModelSpec(
                id=name,
                version="1.0.0",
                title=name,
                format="sbml",
                context=plan.context,
                license="Synthetic fixture",
                source="Local analytic fixture",
                limitations=("Synthetic only",),
            ),
            (ROOT / "model_packages/native/transfer/1.0.0/model.sbml").read_bytes(),
        )
    assert len(client.get("/v1/native-models", headers=READ).json()) == 2
    assert (
        client.get("/v1/native-models?limit=1&offset=1", headers=READ).json()[0]["spec"]["id"]
        == "native.two"
    )
    assert client.get("/v1/native-models?limit=201", headers=READ).status_code == 422


def test_packaged_dashboard_boundary(client):
    response = client.get("/app/")
    # Builds include real production assets, never a generated test substitute.
    assert response.status_code == 200, "Run make dashboard before package/API tests"
    assert "Peykar" in response.text
    assert ADMIN not in response.text and READER not in response.text
    csp = response.headers["Content-Security-Policy"]
    assert "script-src 'self'" in csp and "unsafe-inline" not in csp
    assert "frame-ancestors 'none'" in csp
    assert response.headers["X-Content-Type-Options"] == "nosniff"
    assert response.headers["Cache-Control"] == "no-store"
    assert client.get("/app/%2e%2e/pyproject.toml").status_code == 404
    assert client.get("/app/.env").status_code == 404
    notices = client.get("/app/licenses/index.json").json()
    assert {v["name"] for v in notices} == {"react", "react-dom", "scheduler"}
    for entry in notices:
        notice = client.get("/app/licenses/" + entry["notice"])
        assert notice.status_code == 200
        assert "MIT License" in notice.text
    assert client.get("/").url.path == "/app/"
    assert client.get("/v1/models", headers=READ).status_code == 200
