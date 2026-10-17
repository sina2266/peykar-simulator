"""Inspect pristine local source snapshots without importing or building their code."""

import hashlib
import json
import os
import stat
import subprocess
from pathlib import Path, PurePosixPath

from peykar.infrastructure.artifacts import ArtifactStore
from peykar.infrastructure.database import Database
from peykar_core.contracts import LibrarySourceReceipt, LibrarySourceSpec, Reference
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, canonical_json


def _relative(value: str) -> str:
    path = PurePosixPath(value)
    if path.is_absolute() or not path.parts or any(p in ("..", ".git") for p in path.parts):
        raise InvalidRequest("Source inventory requires safe relative paths")
    return path.as_posix()


class LibraryService:
    def __init__(self, db: Database, artifacts: ArtifactStore) -> None:
        self.db, self.artifacts = db, artifacts

    def inspect(self, spec: LibrarySourceSpec, root: Path) -> LibrarySourceReceipt:
        root = root.resolve(strict=True)

        def git(*arguments: str) -> bytes:
            try:
                return subprocess.check_output(
                    ("git", "--no-optional-locks", "-C", str(root), *arguments),
                    stderr=subprocess.PIPE,
                    timeout=60,
                )
            except (subprocess.SubprocessError, OSError) as exc:
                raise InvalidRequest("Cannot inspect the local Git source") from exc

        if git("rev-parse", "HEAD").decode().strip() != spec.commit:
            raise Conflict("Local source revision differs from the declared pin")
        inventory: list[dict[str, str | int]] = []
        submodules, lfs = [], []
        total = 0
        licenses = {}
        for entry in git("ls-tree", "-r", "-z", "HEAD").split(b"\0"):
            if not entry:
                continue
            metadata, name = entry.split(b"\t", 1)
            mode, kind, oid = metadata.decode().split()
            relative = _relative(name.decode("utf-8"))
            if mode == "160000":
                submodules.append(f"{relative}@{oid}")
                inventory.append({"path": relative, "mode": mode, "gitlink": oid})
                continue
            path = root / relative
            if any(parent.is_symlink() for parent in path.parents if parent != root.parent):
                raise InvalidRequest("Source parent directories cannot be symlinks")
            try:
                info = path.lstat()
                if mode == "120000":
                    if not stat.S_ISLNK(info.st_mode):
                        raise Conflict("Tracked symlink changed")
                    data = os.readlink(path).encode()
                elif stat.S_ISREG(info.st_mode) and not path.is_symlink():
                    if info.st_size > 256 * 1024 * 1024 or total + info.st_size > 4 * 1024**3:
                        raise InvalidRequest("Source inventory exceeds local inspection bounds")
                    if bool(info.st_mode & stat.S_IXUSR) != (mode == "100755"):
                        raise Conflict("Tracked executable mode changed")
                    data = path.read_bytes()
                else:
                    raise Conflict("Tracked source has a different file type")
            except FileNotFoundError as exc:
                raise Conflict("Tracked source file is missing") from exc
            transformed = False
            if kind != "blob":
                raise Conflict("Tracked source has an unsupported object type")
            if (
                hashlib.sha1(
                    b"blob " + str(len(data)).encode() + b"\0" + data, usedforsecurity=False
                ).hexdigest()
                != oid
            ):
                # Preserve actual bytes; only explicitly declared Git text EOL changes are allowed.
                attributes = git("check-attr", "-z", "text", "eol", "--", relative).split(b"\0")
                values = dict(zip(attributes[1::3], attributes[2::3], strict=True))
                text = values.get(b"text") in (b"set", b"auto") or values.get(b"eol") in (
                    b"lf",
                    b"crlf",
                )
                if not text or int(git("cat-file", "-s", oid)) > 256 * 1024 * 1024:
                    raise Conflict(f"Tracked source content changed: {relative}")
                original = git("cat-file", "blob", oid)
                if data.replace(b"\r\n", b"\n") != original.replace(b"\r\n", b"\n"):
                    raise Conflict(f"Tracked source content changed: {relative}")
                transformed = True
            total += len(data)
            checksum = bytes_digest(data)
            inventory.append(
                {"path": relative, "mode": mode, "sha256": checksum, "bytes": len(data)}
            )
            if transformed:
                inventory[-1].update(
                    {"git_blob": oid, "checkout_transform": "declared_text_crlf_only"}
                )
            if data.startswith(b"version https://git-lfs.github.com/spec/v1\n"):
                lfs.append(relative)
            if relative in spec.license_files:
                if mode == "120000" or not data.strip():
                    raise InvalidRequest("License evidence must be a nonempty regular file")
                licenses[relative] = self.artifacts.put(data)
        if set(licenses) != {_relative(path) for path in spec.license_files}:
            raise InvalidRequest("Declared license evidence is absent from the pinned source")
        receipt = LibrarySourceReceipt(
            spec=spec,
            inventory_digest=self.artifacts.put(canonical_json(inventory).encode()),
            license_digests=licenses,
            tracked_files=len(inventory),
            tracked_bytes=total,
            unresolved_submodules=tuple(submodules),
            lfs_pointers=tuple(lfs),
        )
        self.db.register("library_source", spec.id, spec.version, receipt.model_dump_json())
        return receipt

    def get(self, reference: Reference) -> LibrarySourceReceipt:
        receipt = LibrarySourceReceipt.model_validate_json(
            self.db.object("library_source", reference.id, reference.version)
        )
        self.artifacts.get(receipt.inventory_digest)
        for checksum in receipt.license_digests.values():
            self.artifacts.get(checksum)
        return receipt

    def import_receipt(
        self, receipt: LibrarySourceReceipt, inventory: bytes, licenses: dict[str, bytes]
    ) -> LibrarySourceReceipt:
        """Admit a trusted operator's inspection bundle, verifying its retained byte evidence."""
        if len(inventory) > 64 * 1024**2 or bytes_digest(inventory) != receipt.inventory_digest:
            raise Conflict("Source inventory checksum differs")
        try:
            entries = json.loads(inventory)
            names = [_relative(item["path"]) for item in entries]
            total = sum(item.get("bytes", 0) for item in entries)
        except (ValueError, KeyError, TypeError) as exc:
            raise InvalidRequest("Invalid source inventory") from exc
        if (
            len(entries) != receipt.tracked_files
            or len(set(names)) != len(names)
            or total != receipt.tracked_bytes
        ):
            raise Conflict("Source inventory differs from its receipt")
        if set(licenses) != set(receipt.spec.license_files) or set(licenses) != set(
            receipt.license_digests
        ):
            raise InvalidRequest("Provide every pinned source license file exactly once")
        for name, raw in licenses.items():
            _relative(name)
            if bytes_digest(raw) != receipt.license_digests[name]:
                raise Conflict("Source license checksum differs")
        self.artifacts.put(inventory)
        for raw in licenses.values():
            self.artifacts.put(raw)
        self.db.register(
            "library_source", receipt.spec.id, receipt.spec.version, receipt.model_dump_json()
        )
        return receipt

    def discover(self, capability: str | None = None) -> tuple[LibrarySourceReceipt, ...]:
        records = (
            self.get(Reference(id=row["id"], version=row["version"]))
            for row in self.db.objects("library_source", limit=200)
        )
        return tuple(r for r in records if capability is None or capability in r.spec.capabilities)
