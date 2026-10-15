from pathlib import Path

import pytest
from fastapi.testclient import TestClient

from peykar.api.app import create_app
from peykar.examples import analytical_dataset, bootstrap, reference_plan
from peykar.simulation.worker import Worker

ADMIN = "test-admin-credential-at-least-24-characters"
READER = "test-readonly-credential-at-least-24-characters"
AUTH = {"Authorization": f"Bearer {ADMIN}"}


@pytest.fixture
def client(tmp_path: Path):
    app = create_app(tmp_path / "api", ADMIN, READER)
    with TestClient(app) as value:
        bootstrap(app.state.service)
        yield value


def test_authentication_and_authorization(client):
    assert client.get("/healthz").status_code == 200
    assert client.get("/v1/models").status_code == 401
    assert client.get("/v1/models", headers=AUTH).status_code == 200
    read = {"Authorization": f"Bearer {READER}"}
    assert client.get("/v1/models", headers=read).status_code == 200
    assert (
        client.post(
            "/v1/simulations", json=reference_plan().model_dump(mode="json"), headers=read
        ).status_code
        == 403
    )


def test_api_lifecycle(client):
    plan = reference_plan().model_dump(mode="json")
    response = client.post(
        "/v1/simulations", json=plan, headers={**AUTH, "Idempotency-Key": "api-test"}
    )
    assert response.status_code == 202, response.text
    id = response.json()["id"]
    assert response.headers["Location"] == f"/v1/simulations/{id}"
    assert response.headers["Retry-After"] == "1"
    assert (
        client.post(
            "/v1/simulations", json=plan, headers={**AUTH, "Idempotency-Key": "api-test"}
        ).json()["id"]
        == id
    )
    assert client.get(f"/v1/simulations/{id}/results", headers=AUTH).status_code == 409
    Worker(client.app.state.service).tick()
    result = client.get(f"/v1/simulations/{id}/results", headers=AUTH)
    assert result.status_code == 200
    assert result.json()["origin"] == "simulation"
    repeated = client.post(
        "/v1/simulations", json=plan, headers={**AUTH, "Idempotency-Key": "api-test"}
    )
    assert repeated.headers["Location"] == f"/v1/simulations/{id}"
    assert "Retry-After" not in repeated.headers
    assert client.get(f"/v1/simulations/{id}/manifest", headers=AUTH).json()["plan"] == plan
    assert len(client.get(f"/v1/simulations/{id}/events", headers=AUTH).json()) == 3
    assert (
        client.get(f"/v1/simulations/{id}/provenance", headers=AUTH).json()["@type"]
        == "prov:Activity"
    )
    assert client.post(f"/v1/simulations/{id}/cancel", headers=AUTH).status_code == 409


def test_validation_is_not_scientific_promotion(client):
    response = client.post(
        "/v1/simulations", json=reference_plan().model_dump(mode="json"), headers=AUTH
    )
    id = response.json()["id"]
    Worker(client.app.state.service).tick()
    spec, obs = analytical_dataset()
    report = client.post(
        "/v1/validation/runs",
        headers=AUTH,
        json={
            "run_id": id,
            "dataset": obs.dataset.model_dump(mode="json"),
            "tolerance": {"value": 1e-12, "unit": "mole/liter"},
        },
    )
    assert report.status_code == 201, report.text
    assert report.json()["passed"]
    assert report.json()["evidence_class"] == "synthetic_verification"


def test_missing_run_and_invalid_body(client):
    assert client.get("/v1/simulations/missing", headers=AUTH).status_code == 404
    assert client.post("/v1/simulations", json={"typo": True}, headers=AUTH).status_code == 422
    assert client.get("/v1/models?limit=500", headers=AUTH).status_code == 422
    assert client.get("/v1/models/example/versions/latest", headers=AUTH).status_code == 422


def test_model_filtering_precedes_pagination(client):
    svc = client.app.state.service
    from peykar.examples import reference_model

    svc.models.register(
        reference_model().model_copy(
            update={"id": "z.filtered", "capabilities": ("unique_capability",)}
        )
    )
    response = client.get("/v1/models?capability=unique_capability&limit=1", headers=AUTH)
    assert response.status_code == 200
    assert [m["id"] for m in response.json()] == ["z.filtered"]


def test_request_body_limit(client):
    response = client.post("/v1/simulations", content=b"x" * 2_000_001, headers=AUTH)
    assert response.status_code == 413


def test_chunked_body_limit(client):
    def chunks():
        yield b"x" * 1_000_001
        yield b"x" * 1_000_001

    assert client.post("/v1/simulations", content=chunks(), headers=AUTH).status_code == 413


def test_fail_closed_without_credentials(tmp_path: Path, monkeypatch):
    monkeypatch.delenv("PEYKAR_API_TOKEN", raising=False)
    with pytest.raises(RuntimeError):
        create_app(tmp_path)
    with pytest.raises(RuntimeError):
        create_app(tmp_path, ADMIN, ADMIN)


def test_openapi_references_resolve(client):
    import jsonschema

    document = client.get("/openapi.json").json()
    assert "/v1/simulations" in document["paths"]
    assert "HTTPBearer" in document["components"]["securitySchemes"]
    # The contract export is JSON Schema 2020-12, checked separately as well.
    from peykar_core.contracts import SimulationPlan

    jsonschema.Draft202012Validator.check_schema(SimulationPlan.model_json_schema())
