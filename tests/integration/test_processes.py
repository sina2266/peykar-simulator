from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import pytest
from fastapi.testclient import TestClient
from pydantic import ValidationError

from peykar.api.app import create_app
from peykar.biology.processes import ProcessService
from peykar.biology.reactome_processes import read_bundle, read_json
from peykar.examples import reference_model
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    BiologicalProcessNetwork,
    MolecularStateFrameSpec,
    ProcessOwnershipSpec,
    Reference,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import bytes_digest, digest

BUNDLE = Path("biological_libraries/processes/reactome-egfr/bundle.json")
REF = Reference(id="biology.reactome-egfr-processes", version="1.0.0")
ADMIN = "process-admin-tests-at-least-24-characters"
READER = "process-reader-tests-at-least-24-characters"
AUTH = {"Authorization": "Bearer " + ADMIN}
READ = {"Authorization": "Bearer " + READER}


@pytest.fixture
def admitted(tmp_path):
    svc = SimulationService(tmp_path)
    processes = ProcessService(svc.db, svc.artifacts)
    network, raw = read_bundle(BUNDLE)
    processes.register(network, raw)
    return svc, processes, network


def test_original_reactome_multiplicity_state_and_bytes(admitted):
    svc, processes, network = admitted
    assert (len(network.sources), len(network.states), len(network.reactions)) == (19, 15, 3)
    matrix = processes.stoichiometry(REF)

    def cell(state, reaction):
        return matrix["net_coefficients"][matrix["state_ids"].index(state)][
            matrix["reaction_ids"].index(reaction)
        ]

    assert cell("state.9624419", "reaction.177922") == "-2"
    assert cell("state.113592", "reaction.177934") == "-12"
    assert cell("state.29370", "reaction.177934") == "12"
    reaction = next(r for r in network.reactions if r.id == "reaction.177934")
    assert reaction.modifiers[0].state_id == "state.9624420"
    assert reaction.modifiers[0].activity == "GO:0004713"
    assert reaction.unexpanded_relations
    phosphorylated = next(s for s in network.states if s.id == "state.179803")
    assert len([a for a in phosphorylated.attributes if a.name == "hasModifiedResidue"]) == 6
    mature = next(s for s in network.states if s.id == "state.179863")
    assert [(a.name, a.value) for a in mature.attributes] == [
        ("startCoordinate", "971"),
        ("endCoordinate", "1023"),
    ]
    assert mature.molecular_identity == "UniProt:P01133"
    ligand_set = next(s for s in network.states if s.kind == "set")
    assert len(ligand_set.alternatives) == 7 and not ligand_set.components
    assert network.approval == "unreviewed_declaration"
    for source in network.sources:
        assert bytes_digest(processes.source(REF, source.id)) == source.raw_digest
    assert ProcessService(svc.db, svc.artifacts).get(REF) == network
    assert processes.register(network) == network
    with pytest.raises(NotFound):
        processes.source(REF, "unknown")


def changed(network, mutate):
    data = network.model_dump(mode="json")
    mutate(data)
    return BiologicalProcessNetwork.model_validate(data)


@pytest.mark.parametrize(
    "mutate",
    [
        lambda d: d["states"].append(d["states"][0]),
        lambda d: d["reactions"].append(d["reactions"][0]),
        lambda d: d["sources"].append(d["sources"][0]),
        lambda d: d["states"][0].update(source_id="unknown"),
        lambda d: d["states"][0].update(species="NCBITaxon:10090"),
        lambda d: d["reactions"][0]["inputs"][0].update(state_id="unknown"),
        lambda d: d["reactions"][0]["inputs"].append(d["reactions"][0]["inputs"][0]),
        lambda d: d["reactions"][0]["inputs"][0].update(numerator=2, denominator=2),
        lambda d: d["reactions"][0].update(source_id="unknown"),
        lambda d: d["reactions"][0].update(modifiers=[{"state_id": "unknown", "role": "catalyst"}]),
        lambda d: d["states"][0].update(components=[{"state_id": d["states"][0]["id"]}]),
        lambda d: d["states"][0].update(
            kind="complex", components=[{"state_id": d["states"][0]["id"]}]
        ),
        lambda d: d["states"][0].update(kind="set", alternatives=["unknown"]),
    ],
)
def test_rejects_invalid_network(admitted, mutate):
    _, service, network = admitted
    with pytest.raises(InvalidRequest):
        service.register(changed(network, mutate))
    assert service.get(REF) == network


def test_source_and_network_corruption(admitted):
    svc, service, network = admitted
    altered = changed(network, lambda d: d.update(title="Changed immutable content"))
    with pytest.raises(Conflict):
        service.register(altered)
    source = network.sources[0]
    # Use the actual store API path layout rather than relying on a guessed layout.
    paths = list(svc.artifacts.root.rglob(source.raw_digest))
    assert len(paths) == 1
    paths[0].write_bytes(b"corrupt")
    with pytest.raises(Conflict):
        service.source(REF, source.id)


def plan(network):
    return {
        "id": "ownership.test",
        "version": "1.0.0",
        "network": REF.model_dump(mode="json"),
        "context": network.context.model_dump(mode="json"),
        "assignments": [],
        "exclusions": [
            {"reaction_id": r.id, "reason": "No applicable admitted model; intentionally excluded."}
            for r in network.reactions
        ],
    }


def test_ownership_total_partition_frozen_model_and_concurrency(admitted):
    svc, service, network = admitted
    model = reference_model().model_copy(update={"id": "owner.fixture", "context": network.context})
    svc.models.register(model)
    values = plan(network)
    values["assignments"] = [
        {
            "owner_id": "fixture.owner",
            "model_kind": "reference",
            "model": {"id": model.id, "version": model.version},
            "reaction_ids": [network.reactions[0].id],
            "rationale": "Engineering fixture; no biological mechanism claim.",
            "evidence": ["Synthetic test fixture"],
        }
    ]
    values["exclusions"] = values["exclusions"][1:]
    spec = ProcessOwnershipSpec.model_validate(values)
    with ThreadPoolExecutor(max_workers=2) as pool:
        records = list(pool.map(lambda _: service.own(spec), range(2)))
    assert records[0] == records[1] and records[0].model_digests["fixture.owner"] == digest(model)
    assert not records[0].execution_authorized
    assert service.ownership(Reference(id=spec.id, version=spec.version)) == records[0]
    values["assignments"].append({**values["assignments"][0], "owner_id": "overlap"})
    with pytest.raises(InvalidRequest, match="ownership"):
        service.own(ProcessOwnershipSpec.model_validate(values))
    values["assignments"].pop()
    values["exclusions"].pop()
    with pytest.raises(InvalidRequest, match="Every network"):
        service.own(ProcessOwnershipSpec.model_validate(values))
    values["exclusions"] = plan(network)["exclusions"][1:]
    values["context"]["species"] = "NCBITaxon:10090"
    with pytest.raises(InvalidRequest, match="context"):
        service.own(ProcessOwnershipSpec.model_validate(values))


def frame(network):
    return {
        "id": "frame.test",
        "version": "1.0.0",
        "network": REF.model_dump(mode="json"),
        "instance_id": "sample.test",
        "context": network.context.model_dump(mode="json"),
        "time": {"value": 0.0, "unit": "second"},
        "origin": "synthetic",
        "values": [
            {
                "state_id": "state.179837",
                "kind": "amount",
                "quantity": {"value": 1.0, "unit": "nanomole"},
            }
        ],
        "evidence": ["Explicit synthetic engineering fixture"],
        "limitations": ["Not an experiment or biological validation."],
    }


def test_state_frame_original_units_subset_and_immutability(admitted):
    _, service, network = admitted
    spec = MolecularStateFrameSpec.model_validate(frame(network))
    record = service.frame(spec)
    assert record.spec.values[0].quantity.unit == "nanomole"
    assert service.get_frame(Reference(id=spec.id, version=spec.version)) == record
    assert record.network_digest == digest(network)
    updated = spec.model_dump(mode="json")
    updated["values"][0]["quantity"]["value"] = 2.0
    with pytest.raises(Conflict):
        service.frame(MolecularStateFrameSpec.model_validate(updated))
    assert service.records("molecular_frame") == (record,)


@pytest.mark.parametrize(
    "mutate",
    [
        lambda d: d["values"][0]["quantity"].update(value=-1.0),
        lambda d: d["values"][0]["quantity"].update(unit="second"),
        lambda d: d["values"][0].update(state_id="unknown"),
        lambda d: d["values"][0].update(state_id="state.9624194"),
        lambda d: d["values"].append(d["values"][0]),
        lambda d: d["values"][0].update(kind="count", quantity={"value": 0.5, "unit": "count"}),
        lambda d: d["values"][0].update(kind="count", quantity={"value": 1.0, "unit": "mole"}),
        lambda d: d["values"][0].update(kind="count", quantity={"value": 1e18, "unit": "count"}),
        lambda d: d["time"].update(value=-1.0),
        lambda d: d["time"].update(unit="mole"),
        lambda d: d["context"].update(species="NCBITaxon:10090"),
    ],
)
def test_rejects_bad_state_frames(admitted, mutate):
    _, service, network = admitted
    data = frame(network)
    mutate(data)
    with pytest.raises(InvalidRequest):
        service.frame(MolecularStateFrameSpec.model_validate(data))
    assert not service.records("molecular_frame")


@pytest.mark.parametrize("raw", [b'{"a":1,"a":2}', b'{"a":NaN}', b"[]", b"\xff", b""])
def test_rejects_ambiguous_json(raw):
    with pytest.raises(InvalidRequest):
        read_json(raw)


def test_api_roles_bounds_actual_bytes_and_no_execution(admitted, tmp_path):
    svc, _, network = admitted
    app = create_app(tmp_path, ADMIN, READER)
    with TestClient(app) as client:
        base = "/v1/biology/process-networks"
        assert client.get(base).status_code == 401
        assert client.get(base, headers=READ).json() == [network.model_dump(mode="json")]
        assert (
            client.post(base, json=network.model_dump(mode="json"), headers=READ).status_code == 403
        )
        assert (
            client.post(base, json=network.model_dump(mode="json"), headers=AUTH).status_code == 201
        )
        path = base + "/" + network.id + "/versions/1.0.0"
        assert client.get(path + "/stoichiometry", headers=READ).status_code == 200
        source = network.sources[0]
        assert (
            bytes_digest(client.get(path + "/sources/" + source.id, headers=READ).content)
            == source.raw_digest
        )
        for suffix in ["?limit=51", "?limit=0", "?offset=-1"]:
            assert client.get(base + suffix, headers=READ).status_code == 422
        for suffix, data in [
            ("process-ownership", plan(network)),
            ("state-frames", frame(network)),
        ]:
            route = "/v1/biology/" + suffix
            assert client.post(route, json=data, headers=READ).status_code == 403
            assert client.post(route, json=data, headers=AUTH).status_code == 201
            assert len(client.get(route, headers=READ).json()) == 1
            assert (
                client.get(route + "/" + data["id"] + "/versions/1.0.0", headers=READ).status_code
                == 200
            )
        with svc.db.connection() as db:
            assert db.execute("select count(*) from runs").fetchone()[0] == 0
            assert db.execute("PRAGMA user_version").fetchone()[0] == 1


def test_positive_rational_coefficients_and_contract_limits(admitted):
    _, service, network = admitted
    data = network.model_dump(mode="json")
    data["id"] = "rational.fixture"
    data["reactions"][0]["inputs"][0].update(numerator=1, denominator=2)
    rational = BiologicalProcessNetwork.model_validate(data)
    service.register(rational)
    matrix = service.stoichiometry(Reference(id=rational.id, version=rational.version))
    assert any("-1/2" in row for row in matrix["net_coefficients"])
    for number in [0, -1, True, 1000001, 1.5]:
        data["reactions"][0]["inputs"][0]["numerator"] = number
        with pytest.raises(ValidationError):
            BiologicalProcessNetwork.model_validate(data)


def test_bundle_rejects_changed_bytes_revision_paths_and_missing_roots(tmp_path):
    import shutil

    original = BUNDLE.parent
    bundle = tmp_path / "bundle"
    shutil.copytree(original, bundle)
    catalog_path = bundle / "bundle.json"
    import json

    catalog = json.loads(catalog_path.read_text())
    source_path = bundle / catalog["sources"][0]["file"]
    raw = source_path.read_bytes()
    source_path.write_bytes(raw + b" ")
    with pytest.raises(InvalidRequest, match="checksum"):
        read_bundle(catalog_path)
    source_path.write_bytes(raw)
    catalog["sources"][0]["spec"]["revision"] = "wrong"
    catalog_path.write_text(json.dumps(catalog))
    with pytest.raises(InvalidRequest, match="revision"):
        read_bundle(catalog_path)
    catalog = json.loads(BUNDLE.read_text())
    catalog["sources"][0]["file"] = "../outside.json"
    catalog_path.write_text(json.dumps(catalog))
    with pytest.raises(InvalidRequest, match="filename"):
        read_bundle(catalog_path)
    catalog = json.loads(BUNDLE.read_text())
    catalog["sources"] = [
        s for s in catalog["sources"] if s["spec"]["revision"] != "R-HSA-9624419.1"
    ]
    catalog_path.write_text(json.dumps(catalog))
    with pytest.raises(InvalidRequest, match="Unresolved"):
        read_bundle(catalog_path)


def test_ownership_unknown_model_and_wrong_model_context(admitted):
    svc, service, network = admitted
    model = reference_model()
    svc.models.register(model)
    values = plan(network)
    assignment = {
        "owner_id": "owner.test",
        "model_kind": "reference",
        "model": {"id": model.id, "version": model.version},
        "reaction_ids": [network.reactions[0].id],
        "rationale": "Engineering fixture only.",
        "evidence": ["Synthetic fixture"],
    }
    values["assignments"] = [assignment]
    values["exclusions"] = values["exclusions"][1:]
    with pytest.raises(InvalidRequest, match="model context"):
        service.own(ProcessOwnershipSpec.model_validate(values))
    assignment["model"]["id"] = "missing"
    with pytest.raises(NotFound):
        service.own(ProcessOwnershipSpec.model_validate(values))
    assert not service.records("process_ownership")


def test_admission_wrong_raw_and_positive_count_and_concentration(admitted):
    _, service, network = admitted
    _, raw = read_bundle(BUNDLE)
    raw[next(iter(raw))] = b"changed"
    with pytest.raises(InvalidRequest, match="digest"):
        service.register(network, raw)
    with pytest.raises(InvalidRequest, match="Exact declared"):
        service.register(network, {})
    for kind, quantity in [
        ("count", {"value": 12.0, "unit": "count"}),
        ("concentration", {"value": 1.0, "unit": "nanomole/liter"}),
    ]:
        data = frame(network)
        data["id"] = "frame." + kind
        data["values"][0].update(kind=kind, quantity=quantity)
        stored = service.frame(MolecularStateFrameSpec.model_validate(data))
        assert stored.spec.values[0].quantity.unit == quantity["unit"]
