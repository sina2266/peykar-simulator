from peykar.infrastructure.artifacts import ArtifactStore
from peykar.infrastructure.database import Database
from peykar_core.contracts import DatasetLineage, DatasetSpec, Observation, Quantity, Reference
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import bytes_digest, canonical_json, digest
from peykar_core.units import convert, validate_unit


class DatasetService:
    def __init__(self, db: Database, artifacts: ArtifactStore) -> None:
        self.db, self.artifacts = db, artifacts

    def register(self, spec: DatasetSpec, observation: Observation) -> str:
        self.validate(spec, observation)
        self.artifacts.put(canonical_json(observation).encode())
        return self.db.register("dataset", spec.id, spec.version, spec.model_dump_json())

    def register_import(
        self,
        spec: DatasetSpec,
        lineage_kind: Literal["dataset_lineage", "compucell_lineage"],
        lineage_payload: str,
    ) -> None:
        """Pin one import identity across parsers, preserving historical lineage rows."""
        try:
            self.db.object("dataset", spec.id, spec.version)
        except NotFound:
            pass
        else:
            try:
                self.db.object(lineage_kind, spec.id, spec.version)
            except NotFound as exc:
                raise Conflict("Existing dataset version has a different import origin") from exc
        # A shared identity also fences concurrent imports through different parsers.
        identity = canonical_json(
            {"lineage_kind": lineage_kind, "lineage_digest": digest(json.loads(lineage_payload))}
        )
        self.db.register_many(
            (
                ("dataset", spec.id, spec.version, spec.model_dump_json()),
                (lineage_kind, spec.id, spec.version, lineage_payload),
                ("dataset_import", spec.id, spec.version, identity),
            )
        )

    @staticmethod
    def validate(spec: DatasetSpec, observation: Observation) -> None:
        if observation.dataset != Reference(id=spec.id, version=spec.version):
            raise InvalidRequest("Observation does not reference this dataset")
        t = observation.time.values
        if len(t) < 2 or len(t) != len(observation.values.values):
            raise InvalidRequest("Observation requires matching time and value series")
        if any(b <= a for a, b in zip(t, t[1:], strict=False)):
            raise InvalidRequest("Observation times must be strictly increasing")
        convert(Quantity(value=1.0, unit=observation.time.unit), "second")
        validate_unit(observation.values.unit)
        payload = canonical_json(observation).encode()
        if bytes_digest(payload) != spec.artifact_digest:
            raise InvalidRequest("Observation bytes do not match the declared artifact digest")

    def get(self, reference: Reference) -> tuple[DatasetSpec, Observation]:
        spec = DatasetSpec.model_validate_json(
            self.db.object("dataset", reference.id, reference.version)
        )
        observation = Observation.model_validate_json(self.artifacts.get(spec.artifact_digest))
        return spec, observation

    def lineage(self, reference: Reference) -> DatasetLineage:
        lineage = DatasetLineage.model_validate_json(
            self.db.object("dataset_lineage", reference.id, reference.version)
        )
        spec, _ = self.get(reference)
        if lineage.normalized_digest != spec.artifact_digest or lineage.dataset != reference:
            raise InvalidRequest("Dataset lineage does not match the observation")
        self.artifacts.get(lineage.raw_digest)
        return lineage
import json
from typing import Literal

