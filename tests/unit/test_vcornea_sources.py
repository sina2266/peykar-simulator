import hashlib
import io
import json
import struct
import zipfile

import pytest

from peykar.integrations import vcornea_sources as sources
from peykar_core.errors import Conflict, InvalidRequest


class Response(io.BytesIO):
    def __init__(self, data, start, end, size, status=206):
        super().__init__(data[start : end + 1])
        self.status = status
        self.headers = {"Content-Range": f"bytes {start}-{end}/{size}"}


def archive_bytes(*, zip64=False, corrupt=False):
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w", compression=zipfile.ZIP_STORED) as archive:
        archive.writestr("Data/Mild/0/count.csv", "Time,Basal\n0,10\n1,11\n")
        archive.writestr("Data/Moderate/0/count.csv", "Time,Basal\n0,10\n1,9\n")
    data = buffer.getvalue()
    if corrupt:
        data = data.replace(b"0,10", b"0,99", 1)
    if zip64:
        end = data[-22:]
        values = struct.unpack("<4s4H2LH", end)
        offset = len(data) - 22
        record = struct.pack(
            "<4sQ2H2L4Q", b"PK\x06\x06", 44, 45, 45, 0, 0, 2, 2, values[5], values[6]
        )
        locator = struct.pack("<4sLQL", b"PK\x06\x07", 0, offset, 1)
        end = struct.pack("<4s4H2LH", b"PK\x05\x06", 0, 0, 65535, 65535, 0xFFFFFFFF, 0xFFFFFFFF, 0)
        data = data[:-22] + record + locator + end
    return data


def mock_server(monkeypatch, data, *, status=206, range_mismatch=False):
    requests = []

    def open_request(request, timeout):
        assert timeout <= 60
        start, end = map(int, request.get_header("Range").removeprefix("bytes=").split("-"))
        requests.append((start, end))
        response = Response(data, start, end, len(data), status)
        if range_mismatch:
            response.headers["Content-Range"] = "bytes 0-0/1"
        return response

    monkeypatch.setattr(
        sources, "DATA_ARCHIVE", sources.PinnedArchive("Data.zip", len(data), "a" * 32)
    )
    monkeypatch.setattr(sources, "urlopen", open_request)
    return requests


@pytest.mark.parametrize("zip64", [False, True])
def test_range_zip_acquires_exact_members_with_integrity_and_manifest(tmp_path, monkeypatch, zip64):
    data = archive_bytes(zip64=zip64)
    requests = mock_server(monkeypatch, data)
    listing = sources.list_reference_members("Mild")
    assert listing[0]["path"] == "Data/Mild/0/count.csv"
    report = sources.acquire_reference_members(tmp_path, (listing[0]["path"],))
    assert (tmp_path / listing[0]["path"]).read_text() == "Time,Basal\n0,10\n1,11\n"
    assert report["members"][0]["crc32_verified"]
    assert not report["archive_md5_verified"]
    assert report["origin"] == "simulation"
    assert report["transfer_bytes"] < 4 * len(data)
    assert json.loads(next(tmp_path.glob("acquisition-*.json")).read_text()) == report
    assert all(0 <= start <= end < len(data) for start, end in requests)
    assert sources.acquire_reference_members(tmp_path, (listing[0]["path"],)) == report


@pytest.mark.parametrize("status,mismatch", [(200, False), (206, True)])
def test_ignored_or_incorrect_ranges_fail_closed(monkeypatch, status, mismatch):
    data = archive_bytes()
    mock_server(monkeypatch, data, status=status, range_mismatch=mismatch)
    with pytest.raises(InvalidRequest, match="exact pinned byte range"):
        sources.list_reference_members()


@pytest.mark.parametrize(
    "path",
    [
        "../escape",
        "/absolute",
        "a/../escape",
        "./file",
        "a//b",
        "a\\b",
        ".git/hooks/x",
        "a/.GIT/config",
        "C:/escape",
    ],
)
def test_unsafe_member_paths_rejected_before_network(tmp_path, monkeypatch, path):
    monkeypatch.setattr(sources, "urlopen", lambda *args, **kwargs: pytest.fail("network used"))
    with pytest.raises(InvalidRequest, match="Unsafe"):
        sources.acquire_reference_members(tmp_path, (path,))


def test_bad_crc_does_not_publish_member(tmp_path, monkeypatch):
    mock_server(monkeypatch, archive_bytes(corrupt=True))
    with pytest.raises(Conflict, match="CRC"):
        sources.acquire_reference_members(tmp_path, ("Data/Mild/0/count.csv",))
    assert list(tmp_path.iterdir()) == []


def test_existing_files_and_symlink_destination_are_preserved(tmp_path, monkeypatch):
    mock_server(monkeypatch, archive_bytes())
    target = tmp_path / "Data/Mild/0/count.csv"
    target.parent.mkdir(parents=True)
    target.write_bytes(b"existing user data")
    with pytest.raises(Conflict, match="Existing"):
        sources.acquire_reference_members(tmp_path, ("Data/Mild/0/count.csv",))
    assert target.read_bytes() == b"existing user data"
    target.unlink()
    target.symlink_to(tmp_path / "outside")
    with pytest.raises(InvalidRequest, match="symlink"):
        sources.acquire_reference_members(tmp_path, ("Data/Mild/0/count.csv",))


def test_bounds_and_missing_member_fail_before_publication(tmp_path, monkeypatch):
    mock_server(monkeypatch, archive_bytes())
    with pytest.raises(InvalidRequest, match="absent"):
        sources.acquire_reference_members(tmp_path, ("Data/missing.csv",))
    monkeypatch.setattr(sources, "MAX_MEMBER_BYTES", 1)
    with pytest.raises(InvalidRequest, match="64 MiB"):
        sources.acquire_reference_members(tmp_path, ("Data/Mild/0/count.csv",))
    assert list(tmp_path.iterdir()) == []


def test_symlink_zip_entry_is_rejected(monkeypatch, tmp_path):
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w") as archive:
        info = zipfile.ZipInfo("Data/link.csv")
        info.external_attr = 0o120777 << 16
        archive.writestr(info, "/etc/passwd")
    mock_server(monkeypatch, buffer.getvalue())
    with pytest.raises(InvalidRequest, match="regular"):
        sources.acquire_reference_members(tmp_path, ("Data/link.csv",))


def test_range_read_limit_is_checked_before_network(monkeypatch):
    monkeypatch.setattr(sources, "urlopen", lambda *args, **kwargs: pytest.fail("network used"))
    reader = sources.HTTPRangeReader(sources.DATA_ARCHIVE)
    with pytest.raises(InvalidRequest, match="range limit"):
        reader.read(sources.MAX_RANGE_BYTES + 1)


def test_pinned_code_archive_download_is_verified_and_idempotent(tmp_path, monkeypatch):
    data = archive_bytes()
    pin = sources.PinnedArchive("vCornea.zip", len(data), hashlib.md5(data).hexdigest())
    monkeypatch.setattr(sources, "CODE_ARCHIVE", pin)
    calls = []

    def fetch(request, timeout):
        calls.append(request.full_url)
        response = io.BytesIO(data)
        response.status = 200
        return response

    monkeypatch.setattr(sources, "urlopen", fetch)
    report = sources.acquire_code_archive(tmp_path)
    assert report["archive_md5_verified"]
    assert report["sha256"] == hashlib.sha256(data).hexdigest()
    assert not report["extracted"] and not report["code_executed"]
    assert sources.acquire_code_archive(tmp_path) == report
    assert len(calls) == 1


def test_pinned_code_archive_hash_mismatch_preserves_existing_data(tmp_path, monkeypatch):
    data = archive_bytes()
    monkeypatch.setattr(
        sources, "CODE_ARCHIVE", sources.PinnedArchive("vCornea.zip", len(data), "f" * 32)
    )
    target = tmp_path / f"vCornea-{sources.RECORD}.zip"
    target.write_bytes(data)
    with pytest.raises(Conflict, match="pinned size and MD5"):
        sources.acquire_code_archive(tmp_path)
    assert target.read_bytes() == data
    assert list(tmp_path.iterdir()) == [target]
