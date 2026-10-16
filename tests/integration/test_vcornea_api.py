import pytest
from fastapi.testclient import TestClient

from peykar.api.app import create_app
from peykar.examples import SYNTHETIC

ADMIN = "vcornea-admin-token-at-least-24-characters"
READER = "vcornea-reader-token-at-least-24-characters"
AUTH = {"Authorization": "Bearer " + ADMIN}
READ = {"Authorization": "Bearer " + READER}


@pytest.fixture
def client(tmp_path):
    with TestClient(create_app(tmp_path, ADMIN, READER)) as client:
        yield client


def request_body():
    return {
        "spec": {
            "id": "native.api-fixture",
            "version": "1.0.0",
            "title": "Synthetic native schema fixture",
            "kind": "cell_count",
            "origin": "synthetic",
            "context": SYNTHETIC.model_dump(mode="json"),
            "license": "LicenseRef-Peykar-Noncommercial-1.1.0",
            "source": "Invented fixture",
            "upstream_commit": "f5c03afb9c4c7d909494146c5db608389e957070",
            "variant": "paper_version",
            "source_tree_digest": "a" * 64,
        },
        "csv_text": "Superficial,Wing,Basal,Stem,Time\r\n10,20,30,4,0.0\r\n11,21,31,4,1.0\r\n",
    }


def test_native_api_requires_admin_and_preserves_retrievable_unvalidated_output(client):
    body = request_body()
    path = "/v1/vcornea/outputs/cell-counts"
    assert client.post(path, json=body).status_code == 401
    assert client.post(path, json=body, headers=READ).status_code == 403
    response = client.post(path, json=body, headers=AUTH)
    assert response.status_code == 201, response.text
    location = response.headers["Location"]
    assert client.get(location).status_code == 401
    output = client.get(location, headers=READ)
    assert output.status_code == 200
    data = output.json()
    assert data["seed"] is None and data["validity_status"] == "unvalidated"
    assert data["time"]["unit"] == "hour" and data["time"]["values"] == [0.0, 1.0]
    assert [series["name"] for series in data["outputs"]] == [
        "Superficial",
        "Wing",
        "Basal",
        "Stem",
    ]
    assert client.get(location + "/lineage", headers=READ).json() == response.json()
    assert client.post(path, json=body, headers=AUTH).json() == response.json()
    changed = request_body()
    changed["csv_text"] = changed["csv_text"].replace("11,21", "12,21")
    assert client.post(path, json=changed, headers=AUTH).status_code == 409
    assert client.get(location, headers=READ).json() == data


def test_native_api_rejects_invented_seed_or_experimental_origin_and_bad_schema(client):
    body = request_body()
    body["spec"]["origin"] = "experimental"
    assert (
        client.post("/v1/vcornea/outputs/cell-counts", json=body, headers=AUTH).status_code == 422
    )
    body = request_body()
    body["spec"]["seed"] = 7
    assert (
        client.post("/v1/vcornea/outputs/cell-counts", json=body, headers=AUTH).status_code == 422
    )
    body = request_body()
    body["csv_text"] = body["csv_text"].replace("Superficial", "wrong_column")
    assert (
        client.post("/v1/vcornea/outputs/cell-counts", json=body, headers=AUTH).status_code == 422
    )
    assert (
        client.get(
            "/v1/vcornea/outputs/native.api-fixture/versions/1.0.0", headers=READ
        ).status_code
        == 404
    )
