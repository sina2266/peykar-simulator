import json
import math
from concurrent.futures import ThreadPoolExecutor

import pytest
from fastapi.testclient import TestClient
from test_native_jobs import native_fixture as native_fixture
from test_native_jobs import output

from peykar.api.app import create_app
from peykar.composition.hybrid_flows import HybridFlowService
from peykar.examples import reference_model, reference_plan
from peykar.model_import.native_interfaces import (
    NativeInterfaceService,
    input_species,
    replace_initial_values,
)
from peykar.simulation.execution import InProcessExecutor
from peykar.simulation.worker import Worker
from peykar_core.contracts import (
    BatchBudget,
    HybridFlowBinding,
    HybridFlowNode,
    HybridFlowSpec,
    NativeInterfaceSpec,
    NativeModelPort,
    Reference,
    ScientificPort,
)
from peykar_core.errors import Conflict, InvalidRequest, Unsupported
from peykar_core.hashing import bytes_digest


def native_interface(package):
    return NativeInterfaceSpec(
        id="interface.native",
        version="1.0.0",
        model=Reference(id=package.spec.id, version=package.spec.version),
        ports=(
            NativeModelPort(
                port=ScientificPort(
                    name="a_out",
                    direction="output",
                    unit="millimole/liter",
                    semantic_id="synthetic.concentration",
                    scale="molecular",
                ),
                kind="native_output",
                species_id="A",
                quantity_key="A",
            ),
            NativeModelPort(
                port=ScientificPort(
                    name="a_in",
                    direction="input",
                    unit="millimole/liter",
                    semantic_id="synthetic.concentration",
                    scale="molecular",
                    ownership="write",
                ),
                kind="sbml_initial_concentration",
                species_id="A",
            ),
        ),
        evidence=("Authored analytical synthetic A to B fixture",),
        limitations=("Engineering verification only",),
    )


def setup_flow(native_fixture):
    jobs, _, package, _, plan = native_fixture
    service = jobs.service
    interface = native_interface(package)
    NativeInterfaceService(service).register(interface)
    model = reference_model().model_copy(
        update={
            "id": "flow.reference",
            "version": "1.0.0",
            "ports": (
                ScientificPort(
                    name="concentration",
                    direction="output",
                    unit="mole/liter",
                    semantic_id="synthetic.concentration",
                    scale="molecular",
                ),
                ScientificPort(
                    name="initial_in",
                    direction="input",
                    unit="mole/liter",
                    semantic_id="synthetic.concentration",
                    scale="molecular",
                    ownership="write",
                ),
            ),
        }
    )
    service.models.register(model)
    refplan = reference_plan().model_copy(
        update={"model": Reference(id=model.id, version=model.version)}
    )
    spec = HybridFlowSpec(
        id="flow.mixed",
        version="1.0.0",
        title="Synthetic mixed chain",
        nodes=(
            HybridFlowNode(
                id="first",
                plan=plan,
                native_interface=Reference(id=interface.id, version=interface.version),
            ),
            HybridFlowNode(id="second", plan=refplan),
            HybridFlowNode(
                id="third",
                plan=plan,
                native_interface=Reference(id=interface.id, version=interface.version),
            ),
        ),
        bindings=(
            HybridFlowBinding(
                source_node="first",
                source_port="a_out",
                target_node="second",
                target_port="initial_in",
                target_parameter="initial",
                evidence=("Synthetic analytical serial intervals",),
            ),
            HybridFlowBinding(
                source_node="second",
                source_port="concentration",
                target_node="third",
                target_port="a_in",
                evidence=("Explicit concentration transfer",),
            ),
        ),
        limitations=("No biological or continuous feedback validation",),
    )
    return HybridFlowService(service), spec, interface, package, plan


def finish_native(jobs, plan):
    claim = jobs.claim((plan.profile,))
    assert claim
    result, blobs = output(claim)
    metadata = json.loads(blobs["quantities.json"])
    metadata["species"] = [{"name": n, "sbml_id": n, "unit": "mole/liter"} for n in ("A", "B")]
    blobs["quantities.json"] = json.dumps(metadata).encode()
    result = result.model_copy(
        update={"artifacts": {n: bytes_digest(raw) for n, raw in blobs.items()}}
    )
    jobs.finish(claim.manifest.run_id, claim.token, "succeeded", result, blobs)
    return claim


def test_mixed_chain_transfers_units_and_admits_exact_derived_sbml_atomically(native_fixture):
    flows, spec, interface, parent, plan = setup_flow(native_fixture)
    raw = flows.artifacts.get(parent.content_digest)
    frozen = flows.freeze(spec)
    assert flows.freeze(spec) == frozen
    run = flows.submit(Reference(id=spec.id, version=spec.version), "same")
    assert flows.submit(run.flow, "same").id == run.id
    flows.advance(run.id)
    finish_native(flows.native, plan)
    worker = Worker(flows.service, InProcessExecutor())
    worker.tick()
    record = flows.record(run.id)
    assert [n.status for n in record.nodes] == ["succeeded", "succeeded", "pending"]
    reference_child = flows.db.manifest(record.nodes[1].run_id)
    assert reference_child.plan.parameters[0].quantity.value == pytest.approx(math.exp(-1))
    flows = HybridFlowService(flows.service)
    flows.advance(run.id)
    target = flows.record(run.id).nodes[2]
    child = flows.native.manifest(target.run_id)
    assert child.plan.model != plan.model
    derived_raw = flows.artifacts.get(child.model_snapshot.content_digest)
    # Only the exact numeric span changes; licenses, namespaces and source bytes survive.
    root, species = input_species(derived_raw, "A")
    assert float(species.get("initialConcentration")) == pytest.approx(math.exp(-2))
    assert flows.artifacts.get(parent.content_digest) == raw
    assert child.model_snapshot.spec.license == parent.spec.license
    receipt = json.loads(flows.db.object("native_derivation", target.derivation.id, "1.0.0"))
    assert receipt["parent"]["content_digest"] == parent.content_digest
    assert receipt["transfers"][0]["target_quantity"]["unit"] == "millimole/liter"
    assert receipt["transfers"][0]["target_quantity"]["value"] == pytest.approx(1000 * math.exp(-2))
    finish_native(flows.native, plan)
    flows.advance(run.id)
    assert flows.record(run.id).status == "succeeded"
    assert len(flows.results(run.id)["nodes"]) == 3
    assert [e["event"] for e in flows.events(run.id)].count("node_submitted") == 3


def test_byte_preserving_transform_handles_bom_comments_prefixes_and_multiple_values(
    native_fixture,
):
    jobs, _, parent, _, _ = native_fixture
    raw = jobs.artifacts.get(parent.content_digest)
    raw = b"\xef\xbb\xbf" + raw.replace(
        b"<listOfSpecies>", b"<!-- Attribution and units stay exact -->\r\n<listOfSpecies>"
    )
    changed = replace_initial_values(raw, {"A": ".25", "B": ".5"})
    expected = raw.replace(b'initialConcentration="1"', b'initialConcentration=".25"').replace(
        b'initialConcentration="0"', b'initialConcentration=".5"'
    )
    assert changed == expected
    assert replace_initial_values(raw, {}) == raw


@pytest.mark.parametrize(
    "mutation", ["amount", "unit", "dimension", "assigned", "event", "constant", "only_substance"]
)
def test_unsupported_or_overridden_native_initial_state_rejected(native_fixture, mutation):
    jobs, models, parent, _, _ = native_fixture
    raw = jobs.artifacts.get(parent.content_digest)
    if mutation == "amount":
        raw = raw.replace(b'initialConcentration="1"', b'initialAmount="1"')
    if mutation == "unit":
        raw = raw.replace(b'substanceUnits="mole"', b'substanceUnits="item"')
    if mutation == "dimension":
        raw = raw.replace(b'spatialDimensions="3"', b'spatialDimensions="2"')
    if mutation == "assigned":
        raw = raw.replace(
            b"</model>",
            b'<listOfInitialAssignments><initialAssignment symbol="A"/>'
            b"</listOfInitialAssignments></model>",
        )
    if mutation == "event":
        raw = raw.replace(
            b"</model>",
            b'<listOfEvents><event><listOfEventAssignments><eventAssignment variable="A"/>'
            b"</listOfEventAssignments></event></listOfEvents></model>",
        )
    if mutation == "constant":
        raw = raw.replace(
            b'boundaryCondition="false" constant="false"',
            b'boundaryCondition="false" constant="true"',
        )
    if mutation == "only_substance":
        raw = raw.replace(b'hasOnlySubstanceUnits="false"', b'hasOnlySubstanceUnits="true"')
    with pytest.raises(InvalidRequest):
        input_species(raw, "A")


@pytest.mark.parametrize(
    "mutation",
    [
        "cycle",
        "context",
        "semantics",
        "unknown_port",
        "unknown_node",
        "duplicate_writer",
        "native_parameter",
        "parameter",
        "budget",
        "duplicate_node",
        "missing_interface",
    ],
)
def test_freeze_rejects_unsafe_or_unsupported_graphs(native_fixture, mutation):
    flows, spec, interface, _, _ = setup_flow(native_fixture)
    first, second, third = spec.nodes
    binding = spec.bindings[0]
    if mutation == "cycle":
        spec = spec.model_copy(
            update={
                "bindings": spec.bindings
                + (
                    HybridFlowBinding(
                        source_node="third",
                        source_port="a_out",
                        target_node="first",
                        target_port="a_in",
                        evidence=("Synthetic cycle",),
                    ),
                )
            }
        )
    if mutation == "context":
        second = second.model_copy(
            update={
                "plan": second.plan.model_copy(
                    update={"context": second.plan.context.model_copy(update={"species": "other"})}
                )
            }
        )
        spec = spec.model_copy(update={"nodes": (first, second, third)})
    if mutation == "semantics":
        bad = interface.model_copy(
            update={
                "id": "interface.bad",
                "ports": (
                    interface.ports[0].model_copy(
                        update={
                            "port": interface.ports[0].port.model_copy(
                                update={"semantic_id": "other"}
                            )
                        }
                    ),
                    interface.ports[1],
                ),
            }
        )
        flows.interfaces.register(bad)
        first = first.model_copy(
            update={"native_interface": Reference(id=bad.id, version=bad.version)}
        )
        spec = spec.model_copy(update={"nodes": (first, second, third)})
    if mutation == "unknown_port":
        spec = spec.model_copy(
            update={"bindings": (binding.model_copy(update={"source_port": "missing"}),)}
        )
    if mutation == "unknown_node":
        spec = spec.model_copy(
            update={"bindings": (binding.model_copy(update={"source_node": "missing"}),)}
        )
    if mutation == "duplicate_writer":
        spec = spec.model_copy(update={"bindings": spec.bindings + (binding,)})
    if mutation == "native_parameter":
        spec = spec.model_copy(
            update={"bindings": (spec.bindings[1].model_copy(update={"target_parameter": "k"}),)}
        )
    if mutation == "parameter":
        spec = spec.model_copy(
            update={"bindings": (binding.model_copy(update={"target_parameter": "missing"}),)}
        )
    if mutation == "budget":
        spec = spec.model_copy(update={"budget": BatchBudget(max_jobs=1)})
    if mutation == "duplicate_node":
        spec = spec.model_copy(update={"nodes": (first, first)})
    if mutation == "missing_interface":
        spec = spec.model_copy(
            update={"nodes": (first.model_copy(update={"native_interface": None}), second)}
        )
    with pytest.raises((InvalidRequest, Unsupported)):
        flows.freeze(spec)
    assert not flows.db.objects("hybrid_flow")


def test_concurrent_dispatch_cancel_and_late_success_fencing(native_fixture):
    flows, spec, _, _, plan = setup_flow(native_fixture)
    flows.freeze(spec)
    run = flows.submit(Reference(id=spec.id, version=spec.version))
    with ThreadPoolExecutor(max_workers=4) as pool:
        list(pool.map(lambda _: HybridFlowService(flows.service).advance(run.id), range(8)))
    assert len(flows.native.runs()) == 1
    claim = flows.native.claim((plan.profile,))
    flows.cancel(run.id)
    flows.native.finish(claim.manifest.run_id, claim.token, "succeeded", *output(claim))
    flows.advance(run.id)
    record = flows.record(run.id)
    assert record.status == "cancelled" and [n.status for n in record.nodes] == [
        "cancelled",
        "skipped",
        "skipped",
    ]
    assert flows.cancel(run.id) == record


def test_derived_registry_child_and_association_rollback_together(native_fixture, monkeypatch):
    flows, spec, _, _, plan = setup_flow(native_fixture)
    # Native to native makes the second dispatch register a derived model/receipt.
    spec = spec.model_copy(
        update={
            "nodes": (spec.nodes[0], spec.nodes[2]),
            "bindings": (
                HybridFlowBinding(
                    source_node="first",
                    source_port="a_out",
                    target_node="third",
                    target_port="a_in",
                    evidence=("Synthetic concentration",),
                ),
            ),
        }
    )
    flows.freeze(spec)
    run = flows.submit(Reference(id=spec.id, version=spec.version))
    flows.advance(run.id)
    finish_native(flows.native, plan)
    original = flows._event

    def fail(db, id, event, detail=None):
        if event == "node_submitted":
            raise Conflict("Injected post-enqueue rollback")
        original(db, id, event, detail)

    monkeypatch.setattr(flows, "_event", fail)
    with pytest.raises(Conflict):
        flows.advance(run.id)
    assert len(flows.native.runs()) == 1 and not flows.db.objects("native_derivation")
    assert flows.record(run.id).nodes[1].run_id is None


def test_hybrid_http_roles_idempotency_and_transfer_routes(native_fixture):
    flows, spec, interface, _, _ = setup_flow(native_fixture)
    admin = "hybrid-admin-token-at-least-24-characters"
    reader = "hybrid-reader-token-at-least-24-characters"
    ah = {"Authorization": "Bearer " + admin}
    rh = {"Authorization": "Bearer " + reader}
    with TestClient(
        create_app(data_dir=flows.service.root, api_token=admin, read_token=reader)
    ) as c:
        assert c.get("/v1/hybrid-flows").status_code == 401
        assert (
            c.post(
                "/v1/native-interfaces", json=interface.model_dump(mode="json"), headers=rh
            ).status_code
            == 403
        )
        assert (
            c.post(
                "/v1/native-interfaces", json=interface.model_dump(mode="json"), headers=ah
            ).status_code
            == 201
        )
        frozen = c.post("/v1/hybrid-flows", json=spec.model_dump(mode="json"), headers=ah)
        assert frozen.status_code == 201
        assert c.get(frozen.headers["location"], headers=rh).json() == frozen.json()
        ref = {"id": spec.id, "version": spec.version}
        run = c.post("/v1/hybrid-runs", json=ref, headers={**ah, "Idempotency-Key": "same"})
        assert run.status_code == 202
        assert (
            c.post("/v1/hybrid-runs", json=ref, headers={**ah, "Idempotency-Key": "same"}).json()[
                "id"
            ]
            == run.json()["id"]
        )
        url = run.headers["location"]
        assert c.get(url + "/nodes/third/transfers", headers=rh).status_code == 404
        assert c.post(url + "/cancel", headers=rh).status_code == 403
        assert c.post(url + "/cancel", headers=ah).json()["status"] == "cancelled"
        assert c.get(url + "/results", headers=rh).status_code == 409


def test_corrupted_source_manifest_fails_flow_without_downstream_jobs(native_fixture):
    flows, spec, _, _, plan = setup_flow(native_fixture)
    flows.freeze(spec)
    run = flows.submit(Reference(id=spec.id, version=spec.version))
    flows.advance(run.id)
    finish_native(flows.native, plan)
    with flows.db.connection() as db:
        db.execute(
            "UPDATE hybrid_nodes SET manifest_digest=? WHERE hybrid_run_id=? AND node_id=?",
            ("a" * 64, run.id, "first"),
        )
    flows.advance_pending()
    assert flows.record(run.id).status == "failed"
    assert len(flows.native.runs()) == 1
    assert [n.status for n in flows.record(run.id).nodes][1:] == ["skipped", "skipped"]


def test_failed_native_source_skips_downstream_and_preserves_original_model(native_fixture):
    flows, spec, _, parent, plan = setup_flow(native_fixture)
    flows.freeze(spec)
    run = flows.submit(Reference(id=spec.id, version=spec.version))
    flows.advance(run.id)
    claim = flows.native.claim((plan.profile,))
    flows.native.finish(claim.manifest.run_id, claim.token, "failed")
    flows.advance(run.id)
    assert flows.record(run.id).error == "node_failed"
    assert not flows.db.objects("native_derivation")
    assert bytes_digest(flows.artifacts.get(parent.content_digest)) == parent.content_digest


def test_fanout_capacity_and_parent_failure_cancel_other_active_children(native_fixture):
    flows, spec, _, _, plan = setup_flow(native_fixture)
    a, b, c = spec.nodes
    spec = spec.model_copy(
        update={
            "bindings": (
                spec.bindings[0],
                spec.bindings[1].model_copy(
                    update={"source_node": "first", "source_port": "a_out"}
                ),
            ),
            "budget": BatchBudget(max_in_flight=1),
        }
    )
    flows.freeze(spec)
    run = flows.submit(Reference(id=spec.id, version=spec.version))
    flows.advance(run.id)
    finish_native(flows.native, plan)
    flows.advance(run.id)
    record = flows.record(run.id)
    assert sum(n.status in ("queued", "running") for n in record.nodes) == 1
    assert record.nodes[2].status == "pending"
    flows.cancel(run.id)
    assert flows.record(run.id).status == "cancelled"
