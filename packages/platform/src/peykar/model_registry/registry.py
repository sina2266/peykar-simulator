"""Immutable packages, explicit exact dependencies and species-aware capabilities."""

from peykar.infrastructure.database import Database
from peykar_core.contracts import ModelPackage, Reference
from peykar_core.errors import InvalidRequest
from peykar_core.units import validate_unit


class ModelRegistry:
    def __init__(self, db: Database) -> None:
        self.db = db

    def get(self, reference: Reference) -> ModelPackage:
        return ModelPackage.model_validate_json(
            self.db.object("model", reference.id, reference.version)
        )

    def dependencies(self, model: ModelPackage) -> tuple[ModelPackage, ...]:
        visiting = {(model.id, model.version)}
        done: dict[tuple[str, str], ModelPackage] = {}

        def visit(ref: Reference) -> None:
            key = (ref.id, ref.version)
            if key in visiting:
                raise InvalidRequest("Model dependency cycle")
            if key in done:
                return
            visiting.add(key)
            dependency = self.get(ref)
            for child in dependency.dependencies:
                visit(child)
            visiting.remove(key)
            done[key] = dependency

        for ref in model.dependencies:
            visit(ref)
        return tuple(done.values())

    def register(self, model: ModelPackage) -> str:
        for param in model.parameters:
            validate_unit(param.default.unit)
        for port in model.ports:
            validate_unit(port.unit)
        self.dependencies(model)
        return self.db.register("model", model.id, model.version, model.model_dump_json())

    def discover(
        self,
        capability: str | None = None,
        species: str | None = None,
        limit: int = 100,
        offset: int = 0,
    ) -> tuple[ModelPackage, ...]:
        return tuple(
            ModelPackage.model_validate_json(row["payload"])
            for row in self.db.objects("model", limit, offset, capability, species)
        )
