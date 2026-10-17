import math
from concurrent.futures import ThreadPoolExecutor

import pytest
from fastapi.testclient import TestClient

from peykar.api.app import create_app
from peykar.composition.experiments import ExperimentService
from peykar.examples import reference_model, reference_plan
from peykar.simulation.execution import InProcessExecutor
from peykar.simulation.worker import Worker
from peykar_core.contracts import (
    EndpointBinding,
    ExperimentNode,
    ExperimentSpec,
    Quantity,
    Reference,
    ScientificPort,
)
from peykar_core.errors import Conflict, InvalidRequest, Unsupported


def fixture_spec(service):
    original = reference_model()
    ports = tuple(
        ScientificPort(
            name=name,
            direction=direction,
            unit="mole/liter",
            semantic_id="synthetic.concentration",
            scale="molecular",
        )
        for name, direction in [("concentration", "output"), ("initial_input", "input")]
    )
    model = original.model_copy(update={"id": "workflow.decay", "version": "1.0.0", "ports": ports})
    service.models.register(model)
    plan = reference_plan().model_copy(
        update={"model": Reference(id=model.id, version=model.version)}
    )
    return ExperimentSpec(
        id="workflow.chain",
        version="1.0.0",
        title="Two sequential decay stages",
        nodes=tuple(ExperimentNode(id=id, plan=plan) for id in ("first", "second")),
        bindings=(
            EndpointBinding(
                source_node="first",
                source_port="concentration",
                target_node="second",
                target_port="initial_input",
                target_parameter="initial",
                evidence=("Analytical synthetic exp(-k*t) serial interval verification",),
            ),
        ),
        limitations=(
            "Synthetic sequential transfer; no biological or feedback-coupling validation.",
        ),
    )


def test_durable_workflow_executes_real_transfer_and_preserves_lineage(service):
    workflow = ExperimentService(service)
    spec = fixture_spec(service)
    frozen = workflow.freeze(spec)
    assert frozen.order == ("first", "second")
    assert workflow.freeze(spec) == frozen
    run = workflow.submit(Reference(id=spec.id, version=spec.version), "same-work")
    assert workflow.submit(run.experiment, "same-work").id == run.id
    with pytest.raises(Conflict):
        workflow.results(run.id)
    worker = Worker(service, InProcessExecutor())
    worker.tick()
    intermediate = workflow.record(run.id)
    assert intermediate.nodes[0].status == "succeeded"
    assert intermediate.nodes[1].status == "pending"
    # A fresh service recovers orchestration from persistent state.
    workflow = ExperimentService(service)
    worker.tick()
    worker.tick()
    done = workflow.record(run.id)
    assert done.status == "succeeded"
    target = next(n for n in done.nodes if n.node_id == "second")
    manifest = service.db.manifest(target.run_id)
    assert manifest.plan.parameters[0].quantity.value == pytest.approx(math.exp(-1))
    assert service.result(target.run_id).outputs[0].values[-1] == pytest.approx(math.exp(-2))
    results = workflow.results(run.id)
    assert results["scientific_validity"] == "not_established_by_orchestration"
    assert len(results["nodes"]) == 2
    assert [e["event"] for e in workflow.events(run.id)].count("node_submitted") == 2
    with pytest.raises(Conflict):
        workflow.cancel(run.id)


def test_concurrent_dispatch_and_cancel_cannot_create_unassociated_children(service):
    workflow = ExperimentService(service)
    spec = fixture_spec(service)
    workflow.freeze(spec)
    run = workflow.submit(Reference(id=spec.id, version=spec.version))
    with ThreadPoolExecutor(max_workers=6) as pool:
        list(pool.map(lambda _: workflow.advance(run.id), range(12)))
    with service.db.connection() as db:
        assert db.execute("SELECT count(*) FROM runs").fetchone()[0] == 1
        assert (
            db.execute("SELECT count(*) FROM experiment_nodes WHERE run_id IS NOT NULL").fetchone()[
                0
            ]
            == 1
        )
    with ThreadPoolExecutor(max_workers=2) as pool:
        tasks = [pool.submit(workflow.cancel, run.id), pool.submit(workflow.advance, run.id)]
        [task.result() for task in tasks]
    record = workflow.record(run.id)
    assert record.status == "cancelled"
    assert [n.status for n in record.nodes] == ["cancelled", "skipped"]
    assert workflow.cancel(run.id) == record


def test_dispatch_rolls_back_child_and_link_together(service, monkeypatch):
    workflow = ExperimentService(service)
    spec = fixture_spec(service)
    workflow.freeze(spec)
    run = workflow.submit(Reference(id=spec.id, version=spec.version))
    original = service.db.enqueue

    def fail(db, manifest, key):
        original(db, manifest, key)
        raise RuntimeError("Crash between enqueue and association")

    monkeypatch.setattr(service.db, "enqueue", fail)
    with pytest.raises(RuntimeError):
        workflow.advance(run.id)
    with service.db.connection() as db:
        assert db.execute("SELECT count(*) FROM runs").fetchone()[0] == 0
    assert all(n.run_id is None for n in workflow.record(run.id).nodes)


def test_failed_node_stops_downstream_but_not_unrelated_queue(service):
    workflow = ExperimentService(service)
    spec = fixture_spec(service)
    workflow.freeze(spec)
    run = workflow.submit(Reference(id=spec.id, version=spec.version))
    workflow.advance(run.id)
    child = workflow.record(run.id).nodes[0].run_id
    service.db.cancel(child)
    workflow.advance(run.id)
    assert workflow.record(run.id).status == "failed"
    assert workflow.record(run.id).nodes[1].status == "skipped"
    standalone = service.submit(reference_plan())
    Worker(service, InProcessExecutor()).tick()
    assert service.db.run(standalone.id).status == "succeeded"


@pytest.mark.parametrize(
    "variant",
    ["cycle", "double-writer", "parameter-override", "context", "units", "semantic", "field"],
)
def test_incompatible_workflow_is_rejected_before_dispatch(service, variant):
    workflow = ExperimentService(service)
    spec = fixture_spec(service)
    binding = spec.bindings[0]
    if variant == "cycle":
        spec = spec.model_copy(
            update={
                "bindings": spec.bindings
                + (binding.model_copy(update={"source_node": "second", "target_node": "first"}),)
            }
        )
    elif variant == "double-writer":
        spec = spec.model_copy(update={"bindings": spec.bindings * 2})
    else:
        nodes = list(spec.nodes)
        if variant == "parameter-override":
            from peykar_core.contracts import ParameterValue

            nodes[1] = nodes[1].model_copy(
                update={
                    "plan": nodes[1].plan.model_copy(
                        update={
                            "parameters": (
                                ParameterValue(
                                    name="initial", quantity=Quantity(value=1.0, unit="mole/liter")
                                ),
                            )
                        }
                    )
                }
            )
        elif variant == "context":
            nodes[1] = nodes[1].model_copy(
                update={
                    "plan": nodes[1].plan.model_copy(
                        update={
                            "context": nodes[1].plan.context.model_copy(update={"species": "other"})
                        }
                    )
                }
            )
        else:
            model = service.models.get(nodes[1].plan.model)
            ports = list(model.ports)
            update = (
                {"unit": "second"}
                if variant == "units"
                else {"semantic_id": "unmapped"}
                if variant == "semantic"
                else {"shape": "field"}
            )
            ports[1] = ports[1].model_copy(update=update)
            model = model.model_copy(update={"id": "workflow.bad", "ports": tuple(ports)})
            service.models.register(model)
            nodes[1] = nodes[1].model_copy(
                update={
                    "plan": nodes[1].plan.model_copy(
                        update={"model": Reference(id=model.id, version=model.version)}
                    )
                }
            )
        spec = spec.model_copy(update={"nodes": tuple(nodes)})
    with pytest.raises((InvalidRequest, Unsupported)):
        workflow.freeze(spec)
    with service.db.connection() as db:
        assert db.execute("SELECT count(*) FROM experiment_runs").fetchone()[0] == 0


def test_shared_api_auth_registration_submission_and_results(service):
    spec = fixture_spec(service)
    admin = "platform-admin-at-least-24-characters"
    reader = "platform-reader-at-least-24-characters"
    ah = {"Authorization": "Bearer " + admin}
    rh = {"Authorization": "Bearer " + reader}
    with TestClient(create_app(service.root, admin, reader)) as client:
        assert client.get("/v1/libraries").status_code == 401
        assert (
            client.post(
                "/v1/experiments", json=spec.model_dump(mode="json"), headers=rh
            ).status_code
            == 403
        )
        registered = client.post("/v1/experiments", json=spec.model_dump(mode="json"), headers=ah)
        assert registered.status_code == 201
        assert client.get(registered.headers["Location"], headers=rh).status_code == 200
        response = client.post(
            "/v1/experiment-runs",
            json={"id": spec.id, "version": spec.version},
            headers=ah | {"Idempotency-Key": "api-chain"},
        )
        assert response.status_code == 202
        run = response.json()
        assert client.get(response.headers["Location"] + "/results", headers=rh).status_code == 409
        worker = Worker(service, InProcessExecutor())
        for _ in range(3):
            worker.tick()
        assert client.get(response.headers["Location"], headers=rh).json()["status"] == "succeeded"
        assert client.get(response.headers["Location"] + "/results", headers=rh).status_code == 200
        assert client.get(response.headers["Location"] + "/events", headers=rh).status_code == 200
        repeat = client.post(
            "/v1/experiment-runs",
            json={"id": spec.id, "version": spec.version},
            headers=ah | {"Idempotency-Key": "api-chain"},
        )
        assert repeat.json()["id"] == run["id"]
