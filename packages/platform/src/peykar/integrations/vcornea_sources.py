"""Pinned vCornea acquisition; no archive code is imported or executed."""

import hashlib
import io
import json
import os
import re
import stat
import tempfile
import zipfile
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any
from urllib.error import URLError
from urllib.request import Request, urlopen

from peykar_core.errors import Conflict, InvalidRequest

RECORD = "16884414"
MAX_RANGE_BYTES = 16 * 1024 * 1024
MAX_TRANSFER_BYTES = 128 * 1024 * 1024
MAX_MEMBER_BYTES = 64 * 1024 * 1024
MAX_SELECTED_BYTES = 128 * 1024 * 1024
MAX_MEMBERS = 100_000


@dataclass(frozen=True)
class PinnedArchive:
    name: str
    size: int
    md5: str
    record: str = RECORD

    @property
    def url(self) -> str:
        return f"https://zenodo.org/api/records/{self.record}/files/{self.name}/content"


CODE_ARCHIVE = PinnedArchive("vCornea.zip", 228_842_524, "3be635253cd2ab8029ebddbb5e0f8ad8")
DATA_ARCHIVE = PinnedArchive("Data.zip", 6_754_563_353, "43ea77b42fb6d16ac1d1d83f69717ab9")


class HTTPRangeReader(io.RawIOBase):
    """Bounded seekable HTTP reader with strict, exact Content-Range validation."""

    def __init__(self, archive: PinnedArchive) -> None:
        self.archive = archive
        self.position = 0
        self.transferred = 0
        self._cache_start = 0
        self._cache = b""

    def readable(self) -> bool:
        return True

    def seekable(self) -> bool:
        return True

    def tell(self) -> int:
        return self.position

    def seek(self, offset: int, whence: int = os.SEEK_SET) -> int:
        bases = {os.SEEK_SET: 0, os.SEEK_CUR: self.position, os.SEEK_END: self.archive.size}
        if whence not in bases or bases[whence] + offset < 0:
            raise InvalidRequest("Invalid archive seek")
        self.position = bases[whence] + offset
        return self.position

    def read(self, size: int = -1) -> bytes:
        if size < 0:
            size = self.archive.size - self.position
        size = min(size, max(0, self.archive.size - self.position))
        if size > MAX_RANGE_BYTES:
            raise InvalidRequest("Archive read exceeds the 16 MiB range limit")
        if size == 0:
            return b""
        start = self.position
        cache_offset = start - self._cache_start
        if cache_offset >= 0 and cache_offset + size <= len(self._cache):
            payload = self._cache[cache_offset : cache_offset + size]
        else:
            count = min(max(size, 256 * 1024), self.archive.size - start)
            if self.transferred + count > MAX_TRANSFER_BYTES:
                raise InvalidRequest("Selected archive transfer exceeds the 128 MiB limit")
            end = start + count - 1
            request = Request(
                self.archive.url,
                headers={
                    "Range": f"bytes={start}-{end}",
                    "Accept-Encoding": "identity",
                    "User-Agent": "Peykar-pinned-acquisition/0.3.0",
                },
            )
            try:
                with urlopen(request, timeout=60) as response:
                    expected = f"bytes {start}-{end}/{self.archive.size}"
                    if response.status != 206 or response.headers.get("Content-Range") != expected:
                        raise InvalidRequest("Server did not honor the exact pinned byte range")
                    encoding = response.headers.get("Content-Encoding", "identity")
                    if encoding != "identity":
                        raise InvalidRequest("Encoded archive ranges are unsupported")
                    data = response.read(count + 1)
            except (URLError, TimeoutError, OSError) as exc:
                raise InvalidRequest("Pinned archive range request failed") from exc
            if len(data) != count:
                raise InvalidRequest("Archive range response has an incorrect byte count")
            self.transferred += len(data)
            self._cache_start, self._cache = start, data
            payload = data[:size]
        self.position += len(payload)
        return payload


def safe_member(name: str) -> PurePosixPath:
    path = PurePosixPath(name)
    if (
        not name
        or path.is_absolute()
        or str(path) != name
        or not path.parts
        or any(part in (".", "..") or part.casefold() == ".git" for part in path.parts)
        or "\\" in name
        or "\x00" in name
        or re.match(r"^[A-Za-z]:", name)
    ):
        raise InvalidRequest("Unsafe archive member path")
    return path


def _check_info(info: zipfile.ZipInfo) -> None:
    safe_member(info.filename)
    mode = info.external_attr >> 16
    if info.is_dir() or stat.S_ISLNK(mode) or (stat.S_IFMT(mode) not in (0, stat.S_IFREG)):
        raise InvalidRequest("Only regular archive member files may be acquired")
    if info.flag_bits & 1 or info.compress_type not in (zipfile.ZIP_STORED, zipfile.ZIP_DEFLATED):
        raise InvalidRequest("Encrypted or unsupported archive compression")
    if info.file_size > MAX_MEMBER_BYTES or info.compress_size > MAX_MEMBER_BYTES:
        raise InvalidRequest("Selected archive member exceeds the 64 MiB limit")


def _inventory(archive: zipfile.ZipFile) -> list[zipfile.ZipInfo]:
    entries = archive.infolist()
    if len(entries) > MAX_MEMBERS:
        raise InvalidRequest("Archive entry count exceeds the 100000-entry limit")
    names = [entry.filename for entry in entries]
    if len(names) != len(set(names)):
        raise InvalidRequest("Archive contains ambiguous duplicate member names")
    return entries


def _publish(root: Path, name: str, payload: bytes) -> Path:
    relative = safe_member(name)
    root.mkdir(parents=True, exist_ok=True)
    root = root.resolve()
    parent = root
    for part in relative.parts[:-1]:
        parent = parent / part
        if parent.is_symlink():
            raise InvalidRequest("Archive destination may not traverse a symlink")
        parent.mkdir(exist_ok=True)
    target = parent / relative.name
    if target.is_symlink():
        raise InvalidRequest("Archive destination may not be a symlink")
    fd, temporary = tempfile.mkstemp(dir=parent)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        try:
            os.link(temporary, target)
        except FileExistsError:
            if target.read_bytes() != payload:
                raise Conflict(
                    "Existing acquisition file differs; choose a new destination"
                ) from None
    finally:
        Path(temporary).unlink(missing_ok=True)
    return target


def list_reference_members(contains: str | None = None) -> tuple[dict[str, Any], ...]:
    with HTTPRangeReader(DATA_ARCHIVE) as remote, zipfile.ZipFile(remote) as archive:
        return tuple(
            {"path": info.filename, "bytes": info.file_size, "compressed_bytes": info.compress_size}
            for info in _inventory(archive)
            if not info.is_dir() and (contains is None or contains in info.filename)
        )


def acquire_code_archive(destination: Path) -> dict[str, Any]:
    """Download or verify the pinned code ZIP; never extract or execute its members."""
    destination.mkdir(parents=True, exist_ok=True)
    destination = destination.resolve()
    target = destination / f"vCornea-{RECORD}.zip"
    if target.is_symlink():
        raise InvalidRequest("Archive destination may not be a symlink")
    temporary: str | None = None
    try:
        if target.exists():
            source_path = target
        else:
            fd, temporary = tempfile.mkstemp(dir=destination)
            try:
                request = Request(
                    CODE_ARCHIVE.url,
                    headers={
                        "Accept-Encoding": "identity",
                        "User-Agent": "Peykar-pinned-acquisition/0.3.0",
                    },
                )
                with os.fdopen(fd, "wb") as output, urlopen(request, timeout=60) as response:
                    if response.status != 200:
                        raise InvalidRequest("Pinned code archive request was not successful")
                    size = 0
                    while chunk := response.read(1024 * 1024):
                        size += len(chunk)
                        if size > CODE_ARCHIVE.size:
                            raise InvalidRequest("Pinned code archive exceeds the declared size")
                        output.write(chunk)
                    output.flush()
                    os.fsync(output.fileno())
            except (URLError, TimeoutError, OSError) as exc:
                raise InvalidRequest("Pinned code archive download failed") from exc
            source_path = Path(temporary)
        md5, sha256 = hashlib.md5(usedforsecurity=False), hashlib.sha256()
        size = 0
        with source_path.open("rb") as stream:
            while chunk := stream.read(1024 * 1024):
                size += len(chunk)
                if size > CODE_ARCHIVE.size:
                    raise Conflict("Code archive size differs from the pinned record")
                md5.update(chunk)
                sha256.update(chunk)
        if size != CODE_ARCHIVE.size or md5.hexdigest() != CODE_ARCHIVE.md5:
            raise Conflict("Code archive does not match the pinned size and MD5")
        if temporary is not None:
            try:
                os.link(temporary, target)
            except FileExistsError as exc:
                raise Conflict(
                    "Archive destination appeared during acquisition; verify again"
                ) from exc
        report = {
            "schema_version": "1.0.0",
            "record": RECORD,
            "source_url": CODE_ARCHIVE.url,
            "license": "CC-BY-4.0 (Zenodo record declaration)",
            "archive_path": target.name,
            "bytes": size,
            "archive_md5_declared": CODE_ARCHIVE.md5,
            "archive_md5_verified": True,
            "sha256": sha256.hexdigest(),
            "extracted": False,
            "code_executed": False,
        }
        _publish(
            destination,
            f"vCornea-{RECORD}-acquisition.json",
            (json.dumps(report, indent=2) + "\n").encode(),
        )
        return report
    finally:
        if temporary is not None:
            Path(temporary).unlink(missing_ok=True)


def acquire_reference_members(destination: Path, names: tuple[str, ...]) -> dict[str, Any]:
    if not names or len(names) > 32 or len(set(names)) != len(names):
        raise InvalidRequest("Select between 1 and 32 distinct exact archive member paths")
    for name in names:
        safe_member(name)
    with HTTPRangeReader(DATA_ARCHIVE) as remote, zipfile.ZipFile(remote) as archive:
        entries = {entry.filename: entry for entry in _inventory(archive)}
        try:
            selected = [entries[name] for name in names]
        except KeyError as exc:
            raise InvalidRequest(
                "Selected reference member is absent from the pinned archive"
            ) from exc
        for info in selected:
            _check_info(info)
        if sum(info.file_size for info in selected) > MAX_SELECTED_BYTES:
            raise InvalidRequest("Selected uncompressed files exceed the 128 MiB limit")
        records = []
        for info in selected:
            try:
                with archive.open(info) as stream:
                    payload = stream.read(MAX_MEMBER_BYTES + 1)
            except (zipfile.BadZipFile, RuntimeError, EOFError) as exc:
                raise Conflict("Reference ZIP member CRC or structure verification failed") from exc
            if len(payload) != info.file_size or len(payload) > MAX_MEMBER_BYTES:
                raise Conflict("Reference ZIP member size verification failed")
            checksum = hashlib.sha256(payload).hexdigest()
            _publish(destination, info.filename, payload)
            records.append(
                {
                    "path": info.filename,
                    "bytes": len(payload),
                    "compressed_bytes": info.compress_size,
                    "zip_crc32": f"{info.CRC:08x}",
                    "crc32_verified": True,
                    "sha256": checksum,
                    "zip_header_offset": info.header_offset,
                }
            )
        report = {
            "schema_version": "1.0.0",
            "record": RECORD,
            "source_url": DATA_ARCHIVE.url,
            "license": "CC-BY-4.0 (Zenodo record declaration)",
            "archive_bytes": DATA_ARCHIVE.size,
            "archive_md5_declared": DATA_ARCHIVE.md5,
            "archive_md5_verified": False,
            "verification_scope": "selected ZIP members; not complete archive authenticity",
            "origin": "simulation",
            "role": "benchmark",
            "transfer_bytes": remote.transferred,
            "members": records,
        }
        identity = hashlib.sha256(
            json.dumps(report["members"], sort_keys=True).encode()
        ).hexdigest()
        _publish(
            destination,
            f"acquisition-{identity}.json",
            (json.dumps(report, indent=2) + "\n").encode(),
        )
        return report
