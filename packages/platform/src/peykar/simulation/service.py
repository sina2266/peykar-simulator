"""Application service shared by CLI, API and Scientific Instrument Adapter."""

import uuid
from pathlib import Path

from peykar.infrastructure.artifacts import ArtifactStore
from peykar.infrastructure.database import Database, now
from peykar.model_registry.catalog import EngineCatalog
from peykar.model_registry.registry import ModelRegistry
from peykar.simulation.environment import capture_environment
from peykar_core.contracts import (
    ModelPackage,
    RunManifest,
    RunRecord,
    Scenario,
    SimulationPlan,
    SimulationResult,
)
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import digest
from peykar_core.planning import time_grid, validate_plan


class SimulationService:
    def __init__(self, root: Path, catalog: EngineCatalog | None = None) -> None:
        self.root = root.resolve()
        self.db = Database(self.root / "peykar.sqlite3")
        self.artifacts = ArtifactStore(self.root / "artifacts")
        self.models = ModelRegistry(self.db)
        self.engines = catalog or EngineCatalog()

    def validate(self, plan: SimulationPlan) -> ModelPackage:
        model = self.models.get(plan.model)
        validate_plan(model, plan)
        engine = self.engines.get(model.engine, model.engine_version)
        errors = engine.validate(model, plan)
        if errors:
            raise InvalidRequest("; ".join(errors))
        return model

    def estimate(self, plan: SimulationPlan) -> dict[str, int | float | str]:
        self.validate(plan)
        return {
            "samples": len(time_grid(plan)),
            "wall_seconds_limit": plan.budget.wall_seconds,
            "cost": "unknown",
            "basis": "sample count only; not a billing quote",
        }

    def submit(self, plan: SimulationPlan, idempotency_key: str | None = None) -> RunRecord:
        if idempotency_key is not None and not 1 <= len(idempotency_key) <= 128:
            raise InvalidRequest("Idempotency key must contain 1 to 128 characters")
        model = self.validate(plan)
        engine = self.engines.get(model.engine, model.engine_version)
        environment = capture_environment(engine)
        manifest = RunManifest(
            run_id=str(uuid.uuid4()),
            created_at=now(),
            plan=plan,
            model_snapshot=model,
            model_digest=digest(model),
            dependency_snapshots=self.models.dependencies(model),
            environment=environment,
        )
        return self.db.submit(manifest, idempotency_key)

    def result(self, run_id: str) -> SimulationResult:
        run = self.db.run(run_id)
        if run.status != "succeeded" or run.result_digest is None:
            raise Conflict("Successful results are not available for this run")
        return SimulationResult.model_validate_json(self.artifacts.get(run.result_digest))

    def register_scenario(self, scenario: Scenario) -> str:
        self.validate(scenario.plan)
        if scenario.parent:
            if scenario.parent.id == scenario.id and scenario.parent.version == scenario.version:
                raise InvalidRequest("A scenario cannot be its own parent")
            self.db.object("scenario", scenario.parent.id, scenario.parent.version)
        return self.db.register(
            "scenario", scenario.id, scenario.version, scenario.model_dump_json()
        )
