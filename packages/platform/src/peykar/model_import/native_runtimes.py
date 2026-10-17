"""Admit retained operator evidence without installing or executing engine code."""

import json
from pathlib import Path, PurePosixPath

from peykar.infrastructure.artifacts import ArtifactStore
from peykar.infrastructure.database import Database
from peykar.model_import.libraries import LibraryService
from peykar_core.contracts import NativeRuntimeEvidence, Reference
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest


class NativeRuntimeService:
    def __init__(self, db: Database, artifacts: ArtifactStore) -> None:
        self.db, self.artifacts = db, artifacts

    def import_evidence(self, evidence: NativeRuntimeEvidence, bundle: Path) -> str:
        """An offline trusted-operator assertion; artifact integrity is independently checked."""
        for source in evidence.sources:
            LibraryService(self.db, self.artifacts).get(source)
        for name in evidence.artifact_digests:
            relative = PurePosixPath(name)
            if (
                relative.is_absolute()
                or not relative.parts
                or ".." in relative.parts
                or ".git" in relative.parts
            ):
                raise InvalidRequest("Native output names require safe relative paths")
        expected = {evidence.execution_digest, *evidence.artifact_digests.values()}
        values = {}
        total = 0
        for checksum in expected:
            path = bundle / checksum
            if path.is_symlink() or not path.is_file():
                raise InvalidRequest("Native evidence artifact is missing or a symlink")
            size = path.stat().st_size
            if size > 64 * 1024**2 or total + size > 256 * 1024**2:
                raise InvalidRequest("Native evidence exceeds offline admission bounds")
            raw = path.read_bytes()
            if bytes_digest(raw) != checksum:
                raise Conflict("Native evidence artifact checksum mismatch")
            total += len(raw)
            values[checksum] = raw
        try:
            execution = json.loads(values[evidence.execution_digest])
            matches = (
                execution["status"] == "passed"
                and execution["profile"] == evidence.profile
                and execution["image_id"] == "sha256:" + evidence.image_digest
                and execution["artifacts"] == evidence.artifact_digests
            )
        except (ValueError, KeyError, TypeError) as error:
            raise InvalidRequest("Invalid native execution evidence") from error
        if not matches:
            raise Conflict("Native execution evidence does not match its declaration")
        for raw in values.values():
            self.artifacts.put(raw)
        return self.db.register(
            "native_runtime", evidence.id, evidence.version, evidence.model_dump_json()
        )

    def get(self, reference: Reference) -> NativeRuntimeEvidence:
        evidence = NativeRuntimeEvidence.model_validate_json(
            self.db.object("native_runtime", reference.id, reference.version)
        )
        for source in evidence.sources:
            LibraryService(self.db, self.artifacts).get(source)
        self.artifacts.get(evidence.execution_digest)
        for checksum in set(evidence.artifact_digests.values()):
            self.artifacts.get(checksum)
        return evidence

    def discover(self, engine: str | None = None) -> tuple[NativeRuntimeEvidence, ...]:
        values = (
            self.get(Reference(id=row["id"], version=row["version"]))
            for row in self.db.objects("native_runtime", limit=200)
        )
        return tuple(value for value in values if engine is None or value.engine == engine)
