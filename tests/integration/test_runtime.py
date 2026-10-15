import math
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import pytest

from peykar.examples import reference_model, reference_plan
from peykar.simulation.service import SimulationService
from peykar.simulation.worker import Worker
from peykar_core.contracts import Intervention, Quantity, RunStatus
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import digest


def test_decay_matches_independent_analytical_solution(service, plan):
    submitted = service.submit(plan)
    record = Worker(service).tick()
    assert record.status == RunStatus.SUCCEEDED
    result = service.result(submitted.id)
    assert result.origin == "simulation"
    assert result.validity_status == "unvalidated"
    assert result.outputs[0].values == pytest.approx(
        [math.exp(-0.1 * t) for t in result.time.values], abs=1e-14
    )
    assert [e["event"] for e in service.db.events(submitted.id)] == [
        "submitted",
        "started",
        "succeeded",
    ]


def test_two_engine_structures_and_seed_reproduction(service):
    checksums = []
    for seed in (42, 42, 43):
        record = service.submit(reference_plan("logistic", seed))
        Worker(service).tick()
        result = service.result(record.id)
        checksums.append(digest(result))
        assert all(0 <= v <= 200 and v.is_integer() for v in result.outputs[0].values)
    assert checksums[0] == checksums[1]
    assert checksums[0] != checksums[2]


def test_unit_equivalent_plan_same_values(service, plan):
    short = plan.model_copy(update={"duration": Quantity(value=1 / 6, unit="minute")})
    outputs = []
    for item in (plan, short):
        record = service.submit(item)
        Worker(service).tick()
        outputs.append(service.result(record.id))
    assert outputs[0] == outputs[1]


def test_piecewise_rate_intervention(service, plan):
    intervention = Intervention(
        at=Quantity(value=5, unit="s"), parameter="rate", quantity=Quantity(value=0.2, unit="1/s")
    )
    submitted = service.submit(plan.model_copy(update={"interventions": (intervention,)}))
    Worker(service).tick()
    assert service.result(submitted.id).outputs[0].values[-1] == pytest.approx(math.exp(-1.5))


@pytest.mark.parametrize("at", [-1, 0.5, 11])
def test_misaligned_interventions_rejected(service, plan, at):
    event = Intervention(
        at=Quantity(value=at, unit="s"), parameter="rate", quantity=Quantity(value=0.2, unit="1/s")
    )
    with pytest.raises(InvalidRequest):
        service.submit(plan.model_copy(update={"interventions": (event,)}))


def test_unsupported_state_intervention_rejected(service, plan):
    event = Intervention(
        at=Quantity(value=2, unit="s"),
        parameter="initial",
        quantity=Quantity(value=2, unit="mole/liter"),
    )
    with pytest.raises(InvalidRequest):
        service.submit(plan.model_copy(update={"interventions": (event,)}))


def test_idempotent_submission_and_conflict(service, plan):
    a = service.submit(plan, "same-request")
    assert service.submit(plan, "same-request").id == a.id
    with pytest.raises(Conflict):
        service.submit(plan.model_copy(update={"seed": 43}), "same-request")


def test_immutable_models(service, model):
    assert service.models.register(model) == digest(model)
    with pytest.raises(Conflict):
        service.models.register(model.model_copy(update={"title": "Changed"}))


def test_run_survives_service_restart(service, plan):
    a = service.submit(plan)
    restarted = SimulationService(service.root)
    assert restarted.db.run(a.id) == a
    assert Worker(restarted).tick().id == a.id
    assert restarted.db.manifest(a.id).model_snapshot == reference_model()


def test_cancel_queued_is_idempotent(service, plan):
    a = service.submit(plan)
    assert service.db.cancel(a.id).status == "cancelled"
    assert service.db.cancel(a.id).status == "cancelled"
    assert Worker(service).tick() is None


def test_running_cancel_wins_result_race(service, plan):
    a = service.submit(plan)
    id, token = service.db.claim()
    assert id == a.id
    service.db.cancel(id)
    assert service.db.heartbeat(id, token)
    record = service.db.finish(id, token, RunStatus.SUCCEEDED, "a" * 64)
    assert record.status == "cancelled" and record.result_digest is None


def test_stale_worker_is_fenced(service, plan):
    a = service.submit(plan)
    id, token = service.db.claim()
    with service.db.connection() as db:
        db.execute("UPDATE runs SET heartbeat=0 WHERE id=?", (id,))
    assert service.db.recover_stale(60) == 1
    assert service.db.run(a.id).error == "worker_lost"
    with pytest.raises(Conflict):
        service.db.finish(id, token, RunStatus.SUCCEEDED, "a" * 64)


def test_parallel_claims_are_unique(service, plan):
    ids = {service.submit(plan).id for _ in range(12)}
    with ThreadPoolExecutor(max_workers=6) as pool:
        claimed = list(pool.map(lambda _: service.db.claim(), range(18)))
    actual = [item[0] for item in claimed if item]
    assert len(actual) == 12 and set(actual) == ids


def test_result_before_completion_rejected(service, plan):
    with pytest.raises(Conflict):
        service.result(service.submit(plan).id)


def test_artifact_corruption_detected(service):
    checksum = service.artifacts.put(b"original")
    assert service.artifacts.put(b"original") == checksum
    path = service.artifacts.root / checksum[:2] / checksum
    path.write_bytes(b"tampered")
    with pytest.raises(Conflict):
        service.artifacts.get(checksum)


@pytest.mark.parametrize("value", ["../escape", "/tmp/file", "a" * 63, "G" * 64])
def test_artifact_traversal_blocked(service, value):
    with pytest.raises(InvalidRequest):
        service.artifacts.get(value)


def test_manifest_tampering_detected(service, plan):
    a = service.submit(plan)
    with service.db.connection() as db:
        db.execute("UPDATE runs SET manifest_digest=? WHERE id=?", ("a" * 64, a.id))
    with pytest.raises(Conflict):
        service.db.manifest(a.id)


def test_future_database_schema_rejected(tmp_path: Path):
    svc = SimulationService(tmp_path)
    with svc.db.connection() as db:
        db.execute("PRAGMA user_version=900")
    with pytest.raises(Conflict):
        SimulationService(tmp_path)
