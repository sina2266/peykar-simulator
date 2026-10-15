"""Durable queue worker with fenced ownership and bounded child execution."""

import logging
import math
import time

from peykar.simulation.environment import verify_environment
from peykar.simulation.execution import Executor, SubprocessExecutor
from peykar.simulation.service import SimulationService
from peykar_core.contracts import Quantity, RunManifest, RunRecord, RunStatus, SimulationResult
from peykar_core.errors import Cancelled, Conflict, InvalidRequest, OwnershipLost
from peykar_core.hashing import canonical_json, digest
from peykar_core.planning import time_grid
from peykar_core.units import convert, validate_unit

logger = logging.getLogger(__name__)


def _validate_manifest(manifest: RunManifest, run_id: str) -> None:
    model = manifest.model_snapshot
    if manifest.run_id != run_id or digest(model) != manifest.model_digest:
        raise Conflict("Run manifest model identity or digest is inconsistent")
    if (model.engine, model.engine_version) != (
        manifest.environment.engine_id,
        manifest.environment.engine_version,
    ):
        raise Conflict("Run manifest engine does not match its model snapshot")


def _validate_result(manifest: RunManifest, result: SimulationResult) -> SimulationResult:
    # Revalidate cooperative engines too: model_copy() can bypass Pydantic validators.
    try:
        result = SimulationResult.model_validate(result.model_dump(mode="json"))
    except ValueError as exc:
        raise Conflict("Engine result contract is invalid") from exc
    if result.seed != manifest.plan.seed:
        raise Conflict("Engine result seed differs from the submitted plan")
    expected = time_grid(manifest.plan)
    try:
        scale = convert(Quantity(value=1.0, unit=result.time.unit), "second")
        if len(result.time.values) != len(expected) or any(
            not math.isclose(actual * scale, planned, rel_tol=1e-12, abs_tol=0)
            for actual, planned in zip(result.time.values, expected, strict=False)
        ):
            raise Conflict("Engine result time grid differs from the submitted plan")
        outputs = {output.name: output for output in result.outputs}
        for output in result.outputs:
            validate_unit(output.unit)
        for port in manifest.model_snapshot.ports:
            if port.direction != "output" or port.shape != "scalar":
                continue
            declared_output = outputs.get(port.name)
            if declared_output is None:
                raise Conflict("Engine result is missing a declared scalar output")
            convert(Quantity(value=1.0, unit=declared_output.unit), port.unit)
    except InvalidRequest as exc:
        raise Conflict("Engine result units violate its declared contract") from exc
    return result


class Worker:
    def __init__(self, service: SimulationService, executor: Executor | None = None) -> None:
        self.service = service
        self.executor = executor or SubprocessExecutor()

    def tick(self) -> RunRecord | None:
        claim = self.service.db.claim()
        if claim is None:
            return None
        run_id, token = claim
        started = time.monotonic()
        try:
            manifest = self.service.db.manifest(run_id)
            _validate_manifest(manifest, run_id)
            engine = self.service.engines.get(
                manifest.environment.engine_id, manifest.environment.engine_version
            )
            verify_environment(manifest.environment, engine)

            def checkpoint() -> None:
                if self.service.db.heartbeat(run_id, token):
                    raise Cancelled("Cancellation requested")
                if time.monotonic() - started > manifest.plan.budget.wall_seconds:
                    raise InvalidRequest("Execution deadline exceeded")

            checkpoint()
            result = self.executor.execute(engine, manifest, checkpoint)
            result = _validate_result(manifest, result)
            checkpoint()
            checksum = self.service.artifacts.put(canonical_json(result).encode())
            return self.service.db.finish(run_id, token, RunStatus.SUCCEEDED, checksum)
        except OwnershipLost:
            # The owner may have recovered a stale lease; never overwrite its decision.
            logger.exception("Worker lost ownership", extra={"run_id": run_id})
            return self.service.db.run(run_id)
        except Cancelled:
            try:
                return self.service.db.finish(run_id, token, RunStatus.CANCELLED)
            except OwnershipLost:
                return self.service.db.run(run_id)
        except Exception as exc:
            logger.exception("Run failed", extra={"run_id": run_id})
            # Never publish engine exceptions that could contain paths, tokens or raw data.
            if isinstance(exc, InvalidRequest):
                error = "execution_constraint"
            elif isinstance(exc, Conflict):
                error = "integrity_failure"
            else:
                error = "engine_failure"
            try:
                return self.service.db.finish(run_id, token, RunStatus.FAILED, error=error)
            except OwnershipLost:
                return self.service.db.run(run_id)
