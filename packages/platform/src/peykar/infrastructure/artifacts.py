"""Content-addressed immutable local artifacts with atomic publication."""

import os
import re
import tempfile
from pathlib import Path

from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import bytes_digest


class ArtifactStore:
    def __init__(self, root: Path) -> None:
        self.root = root.resolve()
        self.root.mkdir(parents=True, exist_ok=True)

    def _path(self, digest: str) -> Path:
        if not re.fullmatch(r"[a-f0-9]{64}", digest):
            raise InvalidRequest("Invalid artifact digest")
        return self.root / digest[:2] / digest

    def put(self, data: bytes) -> str:
        checksum = bytes_digest(data)
        target = self._path(checksum)
        target.parent.mkdir(parents=True, exist_ok=True)
        fd, temporary = tempfile.mkstemp(dir=target.parent)
        try:
            with os.fdopen(fd, "wb") as stream:
                stream.write(data)
                stream.flush()
                os.fsync(stream.fileno())
            try:
                os.link(temporary, target)
            except FileExistsError:
                if target.read_bytes() != data:
                    raise Conflict("Existing artifact does not match its digest") from None
        finally:
            Path(temporary).unlink(missing_ok=True)
        return checksum

    def get(self, digest: str) -> bytes:
        try:
            value = self._path(digest).read_bytes()
        except FileNotFoundError as exc:
            raise NotFound("Artifact not found") from exc
        if bytes_digest(value) != digest:
            raise Conflict("Artifact integrity verification failed")
        return value
