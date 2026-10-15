"""Explicit local-byte admission without fetching, extracting, installing or executing."""

import os
import stat
from pathlib import Path

from peykar.infrastructure.artifacts import ArtifactStore
from peykar.infrastructure.database import Database
from peykar_core.contracts import ArtifactFilePin, ExternalArtifactSpec, Reference
from peykar_core.errors import InvalidRequest
from peykar_core.hashing import bytes_digest

MAX_FILE_BYTES = 64 * 1024 * 1024
MAX_BUNDLE_BYTES = 256 * 1024 * 1024


def _read_file(directory: Path, pin: ArtifactFilePin) -> bytes:
    """Open relative components without following symlinks or reading special files."""
    fd = None
    try:
        fd = os.open(directory, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW)
        parts = pin.path.split("/")
        for part in parts[:-1]:
            child = os.open(part, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW, dir_fd=fd)
            os.close(fd)
            fd = child
        file_fd = os.open(parts[-1], os.O_RDONLY | os.O_NONBLOCK | os.O_NOFOLLOW, dir_fd=fd)
        with os.fdopen(file_fd, "rb") as stream:
            info = os.fstat(stream.fileno())
            if not stat.S_ISREG(info.st_mode):
                raise InvalidRequest(f"Artifact must be a regular file: {pin.path}")
            if info.st_size > MAX_FILE_BYTES:
                raise InvalidRequest(f"Artifact exceeds the 64 MiB file limit: {pin.path}")
            payload = stream.read(MAX_FILE_BYTES + 1)
            if len(payload) > MAX_FILE_BYTES:
                raise InvalidRequest(f"Artifact exceeds the 64 MiB file limit: {pin.path}")
            return payload
    except OSError as exc:
        raise InvalidRequest(
            f"Artifact cannot be read without following symlinks: {pin.path}"
        ) from exc
    finally:
        if fd is not None:
            os.close(fd)


class ExternalArtifactService:
    def __init__(self, db: Database, artifacts: ArtifactStore) -> None:
        self.db, self.artifacts = db, artifacts

    def admit(self, spec: ExternalArtifactSpec, directory: Path) -> ExternalArtifactSpec:
        try:
            directory = directory.resolve(strict=True)
        except OSError as exc:
            raise InvalidRequest("Artifact directory does not exist or is unreadable") from exc
        if not directory.is_dir():
            raise InvalidRequest("Artifact directory must be a directory")
        verified: list[bytes] = []
        total = 0
        for pin in (spec.artifact, *spec.dependencies, spec.runtime_lock):
            payload = _read_file(directory, pin)
            total += len(payload)
            if total > MAX_BUNDLE_BYTES:
                raise InvalidRequest("Artifact inventory exceeds the 256 MiB total limit")
            if bytes_digest(payload) != pin.digest:
                raise InvalidRequest(f"Artifact bytes do not match the declared digest: {pin.path}")
            verified.append(payload)
        # No content or metadata is published before every declared file is checked.
        for payload in verified:
            self.artifacts.put(payload)
        self.db.register("external_artifact", spec.id, spec.version, spec.model_dump_json())
        return self.get(Reference(id=spec.id, version=spec.version))

    def get(self, reference: Reference) -> ExternalArtifactSpec:
        spec = ExternalArtifactSpec.model_validate_json(
            self.db.object("external_artifact", reference.id, reference.version)
        )
        for pin in (spec.artifact, *spec.dependencies, spec.runtime_lock):
            self.artifacts.get(pin.digest)
        return spec
