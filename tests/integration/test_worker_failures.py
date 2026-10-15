from collections.abc import Callable

import pytest

from peykar.simulation.execution import InProcessExecutor
from peykar.simulation.worker import Worker
from peykar_core.contracts import (
    ModelPackage,
    ScientificPort,
    SimulationPlan,
    SimulationResult,
)
from peykar_core.errors import Cancelled, Conflict
from peykar_core.hashing import canonical_json, digest


class FailingEngine:
    id = "test.failure"
    version = "0.1.0"

    def validate(self, model: ModelPackage, plan: SimulationPlan) -> tuple[str, ...]:
        return ()

    def execute(
        self, model: ModelPackage, plan: SimulationPlan, checkpoint: Callable[[], None]
    ) -> SimulationResult:
        raise RuntimeError("private-token-and-filesystem-path")


def test_engine_failure_is_terminal_and_redacted(service, model, plan):
    service.engines.register(FailingEngine())
    service.models.register(
        model.model_copy(update={"id": "test.failing", "engine": "test.failure"})
    )
    new_plan = plan.model_copy(
        update={"model": plan.model.model_copy(update={"id": "test.failing"})}
    )
    submitted = service.submit(new_plan)
    result = Worker(service, InProcessExecutor()).tick()
    assert result.id == submitted.id
    assert result.status == "failed" and result.error == "engine_failure"
    assert "private-token" not in result.model_dump_json()


def test_corrupt_manifest_does_not_leave_job_running(service, plan):
    submitted = service.submit(plan)
    with service.db.connection() as db:
        db.execute("UPDATE runs SET manifest_digest=? WHERE id=?", ("b" * 64, submitted.id))
    result = Worker(service).tick()
    assert result.status == "failed" and result.error == "integrity_failure"


def test_wall_deadline_failure_is_explicit(service, plan):
    budget = plan.budget.model_copy(update={"wall_seconds": 1e-12})
    service.submit(plan.model_copy(update={"budget": budget}))
    result = Worker(service).tick()
    assert result.status == "failed" and result.error == "execution_constraint"


def test_completed_run_cannot_be_rewritten(service, plan):
    submitted = service.submit(plan)
    run_id, token = service.db.claim()
    from peykar_core.contracts import RunStatus

    service.db.finish(run_id, token, RunStatus.FAILED, error="test_failure")
    with pytest.raises(Conflict):
        service.db.finish(submitted.id, token, RunStatus.SUCCEEDED, "a" * 64)


@pytest.mark.parametrize("defect", ["seed", "time_grid", "time_unit", "output_unit", "alignment"])
def test_inconsistent_engine_result_is_not_published(service, plan, defect):
    class DefectiveExecutor(InProcessExecutor):
        def execute(self, engine, manifest, checkpoint):
            result = super().execute(engine, manifest, checkpoint)
            if defect == "seed":
                return result.model_copy(update={"seed": result.seed + 1})
            if defect == "time_grid":
                return result.model_copy(
                    update={
                        "time": result.time.model_copy(
                            update={"values": tuple(t * 2 for t in result.time.values)}
                        )
                    }
                )
            if defect == "time_unit":
                return result.model_copy(
                    update={"time": result.time.model_copy(update={"unit": "meter"})}
                )
            if defect == "alignment":
                return result.model_copy(update={"outputs": ()})
            return result.model_copy(
                update={"outputs": (result.outputs[0].model_copy(update={"unit": "bad-unit"}),)}
            )

    existing_artifacts = set(service.artifacts.root.glob("*/*"))
    service.submit(plan)
    record = Worker(service, DefectiveExecutor()).tick()
    assert record.status == "failed" and record.error == "integrity_failure"
    assert record.result_digest is None
    assert set(service.artifacts.root.glob("*/*")) == existing_artifacts


@pytest.mark.parametrize("defect", ["missing_output", "incompatible_unit"])
def test_result_must_match_declared_output_port(service, model, plan, defect):
    port = ScientificPort(
        name="different" if defect == "missing_output" else "concentration",
        direction="output",
        unit="second" if defect == "incompatible_unit" else "mole/liter",
        semantic_id="synthetic:concentration",
        scale="molecular",
    )
    custom = model.model_copy(update={"id": "test.ports", "ports": (port,)})
    service.models.register(custom)
    service.submit(
        plan.model_copy(update={"model": plan.model.model_copy(update={"id": custom.id})})
    )
    record = Worker(service, InProcessExecutor()).tick()
    assert record.status == "failed" and record.error == "integrity_failure"
    assert record.result_digest is None


def test_equivalent_result_time_units_are_accepted(service, plan):
    class MinutesExecutor(InProcessExecutor):
        def execute(self, engine, manifest, checkpoint):
            result = super().execute(engine, manifest, checkpoint)
            return result.model_copy(
                update={
                    "time": result.time.model_copy(
                        update={
                            "unit": "minute",
                            "values": tuple(t / 60 for t in result.time.values),
                        }
                    )
                }
            )

    service.submit(plan)
    assert Worker(service, MinutesExecutor()).tick().status == "succeeded"


@pytest.mark.parametrize("defect", ["model_digest", "engine_reference", "run_identity"])
def test_inconsistent_manifest_fails_before_execution(service, plan, defect):
    submitted = service.submit(plan)
    manifest = service.db.manifest(submitted.id)
    if defect == "model_digest":
        manifest = manifest.model_copy(update={"model_digest": "b" * 64})
    elif defect == "engine_reference":
        manifest = manifest.model_copy(
            update={
                "environment": manifest.environment.model_copy(
                    update={"engine_id": "reference.logistic"}
                )
            }
        )
    else:
        manifest = manifest.model_copy(update={"run_id": "another-run"})
    with service.db.connection() as db:
        db.execute(
            "UPDATE runs SET manifest=?,manifest_digest=? WHERE id=?",
            (canonical_json(manifest), digest(manifest), submitted.id),
        )

    record = Worker(service, InProcessExecutor()).tick()
    assert record.status == "failed" and record.error == "integrity_failure"
    assert record.result_digest is None


def test_recovery_during_cancellation_does_not_crash_worker(service, plan):
    class RecoveredExecutor:
        def execute(self, engine, manifest, checkpoint):
            service.db.cancel(manifest.run_id)
            with service.db.connection() as db:
                db.execute("UPDATE runs SET heartbeat=0 WHERE id=?", (manifest.run_id,))
            service.db.recover_stale(60)
            raise Cancelled("requested")

    service.submit(plan)
    record = Worker(service, RecoveredExecutor()).tick()
    assert record.status == "failed" and record.error == "worker_lost"
