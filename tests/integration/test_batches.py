import json
import math
import statistics
from concurrent.futures import ThreadPoolExecutor

import pytest
from fastapi.testclient import TestClient
from test_native_jobs import native_fixture as native_fixture
from test_native_jobs import output

from peykar.api.app import create_app
from peykar.composition.batch_results import report
from peykar.composition.batches import BatchService
from peykar.examples import reference_plan
from peykar.simulation.execution import InProcessExecutor
from peykar.simulation.worker import Worker
from peykar_core.contracts import (
    BatchBudget,
    BatchCase,
    BatchSpec,
    ParameterSweep,
    Quantity,
    Reference,
)
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, digest


def spec(plan, **kwargs):
    return BatchSpec(
        id="batch.fixture",
        version="1.0.0",
        title="Synthetic cohort",
        cases=(BatchCase(id="decay", plan=plan),),
        **kwargs,
    )


def registered(service, value):
    batches = BatchService(service)
    batches.freeze(value)
    return batches, batches.submit(Reference(id=value.id, version=value.version))


def drain(batches, id):
    worker = Worker(batches.service, InProcessExecutor())
    for _ in range(260):
        worker.tick()
        if batches.record(id).status in ("succeeded", "failed", "cancelled"):
            return batches.record(id)
    raise AssertionError("Cohort did not finish")


def test_cartesian_sweep_replicates_physical_units_seed_allocation_and_lineage(service, plan):
    value = spec(
        plan,
        replicates=3,
        base_seed=100,
        sweeps=(
            ParameterSweep(
                case_id="decay",
                parameter="rate",
                values=(Quantity(value=6, unit="1/minute"), Quantity(value=0.2, unit="1/second")),
            ),
            ParameterSweep(
                case_id="decay",
                parameter="initial",
                values=(Quantity(value=1, unit="mole/liter"), Quantity(value=2, unit="mole/liter")),
            ),
        ),
    )
    batches, run = registered(service, value)
    frozen = batches.get(run.batch)
    assert len(frozen.members) == 12 and frozen.sample_upper_bound == 132
    assert frozen.total_wall_seconds_limit == 720
    assert [m.manifest.plan.seed for m in frozen.members] == list(range(100, 112))
    assert batches.freeze(value) == frozen
    with pytest.raises(Conflict):
        batches.freeze(value.model_copy(update={"base_seed": 200}))
    assert batches.submit(run.batch, "same").id == batches.submit(run.batch, "same").id
    with pytest.raises(Conflict):
        report(batches, run.id)
    assert drain(batches, run.id).status == "succeeded"
    summary = report(BatchService(service), run.id)
    assert len(summary.groups) == 4 and all(g.complete for g in summary.groups)
    for group in summary.groups:
        selected = {p.name: p.quantity.value for p in group.parameters}
        rate = 0.1 if selected["rate"] == 6 else 0.2
        endpoint = group.endpoints[0]
        assert endpoint.mean == pytest.approx(selected["initial"] * math.exp(-rate * 10))
        assert endpoint.sample_standard_deviation == 0 and endpoint.samples == 3
    assert all(m.manifest_digest and m.result_digest for m in summary.members)
    assert report(batches, run.id) == summary
    assert [e["event"] for e in batches.events(run.id)].count("member_submitted") == 12


def test_stochastic_summary_matches_independently_computed_replicates(service):
    batches, run = registered(
        service, spec(reference_plan("logistic"), replicates=5, base_seed=701)
    )
    drain(batches, run.id)
    summary = report(batches, run.id)
    actual = [
        service.result(m.run_id).outputs[0].values[-1] for m in batches.record(run.id).members
    ]
    endpoint = summary.groups[0].endpoints[0]
    assert len(set(actual)) > 1
    assert endpoint.mean == pytest.approx(statistics.mean(actual))
    assert endpoint.sample_standard_deviation == pytest.approx(statistics.stdev(actual))
    assert summary.uncertainty_scope == "within_case_terminal_sample_variability"


@pytest.mark.parametrize(
    "constraint",
    [
        "jobs",
        "samples",
        "wall",
        "seed",
        "unknown_case",
        "duplicate_axis",
        "unit",
        "same_physical_value",
        "unknown_parameter",
        "duplicate_case",
        "range",
        "snapshot_bytes",
    ],
)
def test_batch_constraints_reject_before_admission_or_queue_writes(
    service, plan, monkeypatch, constraint
):
    from peykar.composition import batches as module

    value = spec(plan, replicates=2)
    sweep = ParameterSweep(
        case_id="decay",
        parameter="rate",
        values=(Quantity(value=0.1, unit="1/second"), Quantity(value=0.2, unit="1/second")),
    )
    if constraint == "jobs":
        value = value.model_copy(update={"budget": BatchBudget(max_jobs=1)})
    if constraint == "samples":
        value = value.model_copy(update={"budget": BatchBudget(max_total_samples=2)})
    if constraint == "wall":
        value = value.model_copy(update={"budget": BatchBudget(max_total_wall_seconds=1)})
    if constraint == "seed":
        value = value.model_copy(update={"base_seed": 2**32 - 1})
    if constraint == "unknown_case":
        value = value.model_copy(
            update={"sweeps": (sweep.model_copy(update={"case_id": "missing"}),)}
        )
    if constraint == "duplicate_axis":
        value = value.model_copy(update={"sweeps": (sweep, sweep)})
    if constraint == "unit":
        value = value.model_copy(
            update={
                "sweeps": (
                    sweep.model_copy(
                        update={
                            "values": (
                                Quantity(value=1, unit="mole"),
                                Quantity(value=2, unit="mole"),
                            )
                        }
                    ),
                )
            }
        )
    if constraint == "same_physical_value":
        value = value.model_copy(
            update={
                "sweeps": (
                    sweep.model_copy(
                        update={
                            "values": (
                                Quantity(value=0.1, unit="1/second"),
                                Quantity(value=6, unit="1/minute"),
                            )
                        }
                    ),
                )
            }
        )
    if constraint == "unknown_parameter":
        value = value.model_copy(
            update={"sweeps": (sweep.model_copy(update={"parameter": "missing"}),)}
        )
    if constraint == "duplicate_case":
        value = value.model_copy(update={"cases": value.cases * 2})
    if constraint == "range":
        value = value.model_copy(
            update={
                "sweeps": (
                    sweep.model_copy(
                        update={
                            "values": (
                                Quantity(value=-1, unit="1/second"),
                                Quantity(value=1, unit="1/second"),
                            )
                        }
                    ),
                )
            }
        )
    if constraint == "snapshot_bytes":
        monkeypatch.setattr(module, "MAX_FROZEN_BYTES", 100)
    batches = BatchService(service)
    with pytest.raises(InvalidRequest):
        batches.freeze(value)
    with service.db.connection() as db:
        assert db.execute("SELECT count(*) FROM runs").fetchone()[0] == 0
        assert db.execute("SELECT count(*) FROM objects WHERE kind='batch'").fetchone()[0] == 0


def test_atomic_dispatch_rolls_back_child_association_and_events(service, plan, monkeypatch):
    batches, run = registered(service, spec(plan))
    original = batches._event

    def fail(db, id, event, detail=None):
        if event == "member_submitted":
            raise Conflict("Injected rollback")
        return original(db, id, event, detail)

    monkeypatch.setattr(batches, "_event", fail)
    with pytest.raises(Conflict):
        batches.advance(run.id)
    with service.db.connection() as db:
        assert db.execute("SELECT count(*) FROM runs").fetchone()[0] == 0
    assert batches.record(run.id).members[0].run_id is None
    assert [e["event"] for e in batches.events(run.id)] == ["submitted"]


def test_concurrent_dispatch_capacity_cancellation_and_restart(service, plan):
    batches, run = registered(
        service, spec(plan, replicates=8, budget=BatchBudget(max_in_flight=2))
    )
    with ThreadPoolExecutor(max_workers=4) as pool:
        list(pool.map(lambda _: BatchService(service).advance(run.id), range(8)))
    assert sum(m.status == "queued" for m in batches.record(run.id).members) == 2
    with ThreadPoolExecutor(max_workers=2) as pool:
        tasks = [pool.submit(batches.advance, run.id), pool.submit(batches.cancel, run.id)]
        [t.result() for t in tasks]
    done = BatchService(service).record(run.id)
    assert done.status == "cancelled" and sum(m.status == "skipped" for m in done.members) == 6
    assert batches.cancel(run.id) == done
    summary = report(batches, run.id)
    assert not summary.groups[0].complete and summary.groups[0].endpoints == ()


@pytest.mark.parametrize("policy", ["continue", "fail_fast"])
def test_failure_policy_preserves_successes_and_discloses_excluded_members(service, plan, policy):
    batches, run = registered(
        service,
        spec(plan, replicates=3, failure_policy=policy, budget=BatchBudget(max_in_flight=1)),
    )
    Worker(service, InProcessExecutor()).tick()
    batches.advance(run.id)
    second = batches.record(run.id).members[1]
    service.db.cancel(second.run_id)
    done = drain(batches, run.id)
    assert done.status == "failed" and done.error == "member_failed"
    summary = report(batches, run.id)
    group = summary.groups[0]
    assert not group.complete and len(group.excluded_member_ids) == (
        1 if policy == "continue" else 2
    )
    assert group.endpoints[0].samples == (2 if policy == "continue" else 1)
    assert (group.endpoints[0].sample_standard_deviation is None) == (policy == "fail_fast")


def test_running_cancellation_fences_late_worker_success(service, plan):
    batches, run = registered(
        service, spec(plan, replicates=2, budget=BatchBudget(max_in_flight=1))
    )
    batches.advance(run.id)
    id, token = service.db.claim()
    assert batches.cancel(run.id).status == "running"
    service.db.finish(id, token, "succeeded", "a" * 64)
    batches.advance(run.id)
    assert batches.record(run.id).status == "cancelled"
    assert service.db.run(id).result_digest is None


@pytest.mark.parametrize("quantity_metadata", [True, False])
def test_mixed_native_reference_batch_and_native_capability_rejection(
    native_fixture, quantity_metadata
):
    jobs, _, _, _, native_plan = native_fixture
    value = spec(reference_plan()).model_copy(
        update={
            "cases": (
                BatchCase(id="decay", plan=reference_plan()),
                BatchCase(id="native", plan=native_plan),
            )
        }
    )
    batches, run = registered(jobs.service, value)
    Worker(jobs.service, InProcessExecutor()).tick()
    claim = jobs.claim((native_plan.profile,))
    assert claim
    result, blobs = output(claim)
    if quantity_metadata:
        metadata = json.loads(blobs["quantities.json"])
        metadata["species"] = [{"name": n, "unit": "mole/liter"} for n in ("A", "B")]
        blobs["quantities.json"] = json.dumps(metadata).encode()
        result = result.model_copy(
            update={"artifacts": {n: bytes_digest(raw) for n, raw in blobs.items()}}
        )
    jobs.finish(claim.manifest.run_id, claim.token, "succeeded", result, blobs)
    batches.advance(run.id)
    assert batches.record(run.id).status == "succeeded"
    # The native protocol fixture has no species metadata; failed reporting cannot
    # invent its physical units. Real driver metadata is verified separately.
    if quantity_metadata:
        summary = report(batches, run.id)
        assert len(summary.groups) == 2
        assert {e.output for e in summary.groups[1].endpoints} == {"A", "B"}
        assert all(e.sample_standard_deviation is None for g in summary.groups for e in g.endpoints)
    else:
        with pytest.raises(Conflict):
            report(batches, run.id)
    with pytest.raises(InvalidRequest):
        batches.freeze(value.model_copy(update={"version": "1.0.1", "replicates": 2}))
    with pytest.raises(InvalidRequest):
        batches.freeze(
            value.model_copy(
                update={
                    "version": "1.0.1",
                    "sweeps": (
                        ParameterSweep(
                            case_id="native",
                            parameter="rate",
                            values=(
                                Quantity(value=0.1, unit="1/second"),
                                Quantity(value=0.2, unit="1/second"),
                            ),
                        ),
                    ),
                }
            )
        )


def test_report_detects_child_manifest_corruption_and_missing_members(service, plan):
    batches, run = registered(service, spec(plan))
    drain(batches, run.id)
    id = batches.record(run.id).members[0].run_id
    manifest = service.db.manifest(id)
    modified = manifest.model_copy(update={"plan": manifest.plan.model_copy(update={"seed": 77})})
    with service.db.connection() as db:
        db.execute(
            "UPDATE runs SET manifest=?,manifest_digest=? WHERE id=?",
            (modified.model_dump_json(), digest(modified), id),
        )
    with pytest.raises(Conflict):
        report(batches, run.id)
    with service.db.connection() as db:
        db.execute("DELETE FROM batch_members WHERE batch_run_id=?", (run.id,))
    with pytest.raises(Conflict):
        report(batches, run.id)


def test_batch_api_roles_idempotency_lifecycle_report_and_pagination(service, plan):
    admin = "batch-admin-token-at-least-24-characters"
    reader = "batch-read-token-at-least-24-characters"
    ah = {"Authorization": "Bearer " + admin}
    rh = {"Authorization": "Bearer " + reader}
    with TestClient(
        create_app(data_dir=service.root, api_token=admin, read_token=reader)
    ) as client:
        assert client.get("/v1/batches").status_code == 401
        value = spec(plan).model_dump(mode="json")
        assert client.post("/v1/batches", headers=rh, json=value).status_code == 403
        frozen = client.post("/v1/batches", headers=ah, json=value)
        assert frozen.status_code == 201
        assert client.get(frozen.headers["location"], headers=rh).json() == frozen.json()
        ref = {"id": value["id"], "version": value["version"]}
        response = client.post(
            "/v1/batch-runs", headers={**ah, "Idempotency-Key": "http-batch"}, json=ref
        )
        assert response.status_code == 202
        again = client.post(
            "/v1/batch-runs", headers={**ah, "Idempotency-Key": "http-batch"}, json=ref
        )
        assert again.json()["id"] == response.json()["id"]
        prefix = response.headers["location"]
        id = response.json()["id"]
        assert client.post(prefix + "/cancel", headers=rh).status_code == 403
        assert client.get(prefix + "/results", headers=rh).status_code == 409
        drain(BatchService(service), id)
        result = client.get(prefix + "/results", headers=rh)
        assert result.status_code == 200
        assert result.headers["etag"] == '"' + digest(result.json()) + '"'
        assert client.get("/v1/batch-runs?limit=0", headers=rh).status_code == 422
        assert len(client.get("/v1/batch-runs?limit=1", headers=rh).json()) == 1
        assert client.post(prefix + "/cancel", headers=ah).status_code == 409


def test_native_dispatch_rollback_and_cancel_running_agent(native_fixture, monkeypatch):
    jobs, _, _, _, plan = native_fixture
    value = BatchSpec(
        id="batch.native",
        version="1.0.0",
        title="Native protocol",
        cases=(BatchCase(id="native", plan=plan),),
    )
    batches, run = registered(jobs.service, value)
    original = batches._event

    def fail(db, id, event, detail=None):
        if event == "member_submitted":
            raise Conflict("Injected failure")
        original(db, id, event, detail)

    monkeypatch.setattr(batches, "_event", fail)
    with pytest.raises(Conflict):
        batches.advance(run.id)
    assert jobs.runs() == ()
    monkeypatch.setattr(batches, "_event", original)
    batches.advance(run.id)
    claim = jobs.claim((plan.profile,))
    assert batches.cancel(run.id).status == "running"
    jobs.finish(claim.manifest.run_id, claim.token, "succeeded", *output(claim))
    batches.advance(run.id)
    assert batches.record(run.id).status == "cancelled"
    assert jobs.run(claim.manifest.run_id).result_digest is None


def test_batch_migration_preserves_existing_queues_and_spec_idempotency_conflicts(service, plan):
    ordinary = service.submit(plan)
    before = service.db.run(ordinary.id)
    batches, run = registered(service, spec(plan))
    assert service.db.run(ordinary.id) == before
    with service.db.connection() as db:
        assert db.execute("PRAGMA user_version").fetchone()[0] == 1
    assert (
        batches.submit(run.batch, "idempotency").id == batches.submit(run.batch, "idempotency").id
    )
    other = spec(plan).model_copy(update={"id": "batch.other"})
    batches.freeze(other)
    with pytest.raises(Conflict):
        batches.submit(Reference(id=other.id, version=other.version), "idempotency")


def test_same_frozen_cohort_replay_uses_identical_seeds_and_stochastic_outputs(service):
    batches, first = registered(service, spec(reference_plan("logistic"), replicates=3))
    drain(batches, first.id)
    second = batches.submit(first.batch)
    drain(batches, second.id)
    a, b = report(batches, first.id), report(batches, second.id)
    assert a.groups == b.groups
    assert [m.result_digest for m in a.members] == [m.result_digest for m in b.members]
    assert {m.run_id for m in a.members}.isdisjoint({m.run_id for m in b.members})
