import json
import os
from pathlib import Path

import pytest
from pydantic import ValidationError
from typer.testing import CliRunner

from peykar.cli import app
from peykar.model_import.external_artifacts import ExternalArtifactService
from peykar_core.contracts import ArtifactFilePin, ExternalArtifactSpec, Reference
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import bytes_digest


def inventory(directory: Path) -> ExternalArtifactSpec:
    contents = {
        "source/model.zip": b"opaque archive bytes; never extracted\x00\xff",
        "dependency.whl": b"opaque dependency bytes; never installed",
        "runtime.lock": b"python==3.12.14\nexample==1.0.0\n",
    }
    for path, payload in contents.items():
        target = directory / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(payload)
    pins = {
        path: ArtifactFilePin(path=path, digest=bytes_digest(payload))
        for path, payload in contents.items()
    }
    return ExternalArtifactSpec(
        id="external.fixture",
        version="1.0.0",
        source="https://example.invalid/operator-supplied-source",
        license="External owner license; operator review required",
        artifact=pins["source/model.zip"],
        dependencies=(pins["dependency.whl"],),
        runtime_lock=pins["runtime.lock"],
    )


def ref(spec):
    return Reference(id=spec.id, version=spec.version)


def test_offline_admission_preserves_exact_bytes_permissions_and_identity(service, tmp_path):
    directory = tmp_path / "input"
    spec = inventory(directory)
    source = directory / spec.artifact.path
    source.chmod(0o440)
    admission = ExternalArtifactService(service.db, service.artifacts)
    assert admission.admit(spec, directory) == spec
    assert admission.admit(spec, directory) == spec
    assert source.stat().st_mode & 0o777 == 0o440
    assert service.artifacts.get(spec.artifact.digest) == source.read_bytes()
    for file in directory.rglob("*"):
        if file.is_file():
            file.unlink()
    assert admission.get(ref(spec)) == spec


@pytest.mark.parametrize("file", ["source/model.zip", "dependency.whl", "runtime.lock"])
def test_every_declared_digest_is_checked_before_publication(service, tmp_path, file):
    directory = tmp_path / "input"
    spec = inventory(directory)
    (directory / file).write_bytes(b"wrong bytes")
    existing = set(service.artifacts.root.rglob("*"))
    admission = ExternalArtifactService(service.db, service.artifacts)
    with pytest.raises(InvalidRequest, match="declared digest"):
        admission.admit(spec, directory)
    with pytest.raises(NotFound):
        admission.get(ref(spec))
    assert set(service.artifacts.root.rglob("*")) == existing


def test_immutable_inventory_rejects_license_or_dependency_replacement(service, tmp_path):
    directory = tmp_path / "input"
    spec = inventory(directory)
    admission = ExternalArtifactService(service.db, service.artifacts)
    admission.admit(spec, directory)
    changed = spec.model_copy(update={"license": "another declaration"})
    with pytest.raises(Conflict, match="immutable version"):
        admission.admit(changed, directory)
    assert admission.get(ref(spec)) == spec


@pytest.mark.parametrize("file", ["artifact", "runtime_lock"])
def test_retrieval_rechecks_stored_artifact_integrity(service, tmp_path, file):
    spec = inventory(tmp_path / "input")
    admission = ExternalArtifactService(service.db, service.artifacts)
    admission.admit(spec, tmp_path / "input")
    checksum = getattr(spec, file).digest
    path = service.artifacts.root / checksum[:2] / checksum
    path.write_bytes(b"corruption")
    with pytest.raises(Conflict, match="integrity"):
        admission.get(ref(spec))


@pytest.mark.parametrize(
    "path", ["../escape", "/etc/passwd", "a/../../b", "./file", "a//b", "a\\b", ".", "a\x00b"]
)
def test_unsafe_or_nonportable_paths_are_rejected(path):
    with pytest.raises(ValidationError):
        ArtifactFilePin(path=path, digest="a" * 64)


def test_inventory_requires_distinct_files_and_runtime_pin(tmp_path):
    spec = inventory(tmp_path)
    data = spec.model_dump(mode="json")
    del data["runtime_lock"]
    with pytest.raises(ValidationError):
        ExternalArtifactSpec.model_validate(data)
    data["runtime_lock"] = data["artifact"]
    with pytest.raises(ValidationError, match="distinct"):
        ExternalArtifactSpec.model_validate(data)


@pytest.mark.parametrize("kind", ["symlink", "directory_symlink", "fifo", "missing"])
def test_import_rejects_symlinks_special_files_and_missing_files(service, tmp_path, kind):
    directory = tmp_path / "input"
    spec = inventory(directory)
    target = directory / spec.artifact.path
    target.unlink()
    if kind == "symlink":
        target.symlink_to(directory / "dependency.whl")
    elif kind == "directory_symlink":
        target.parent.rmdir()
        target.parent.symlink_to(tmp_path, target_is_directory=True)
        (tmp_path / "model.zip").write_bytes(b"outside")
    elif kind == "fifo":
        os.mkfifo(target)
    admission = ExternalArtifactService(service.db, service.artifacts)
    with pytest.raises(InvalidRequest):
        admission.admit(spec, directory)
    with pytest.raises(NotFound):
        admission.get(ref(spec))


@pytest.mark.parametrize("limit", ["MAX_FILE_BYTES", "MAX_BUNDLE_BYTES"])
def test_import_is_bounded_before_publication(service, tmp_path, monkeypatch, limit):
    directory = tmp_path / "input"
    spec = inventory(directory)
    existing = set(service.artifacts.root.rglob("*"))
    monkeypatch.setattr("peykar.model_import.external_artifacts." + limit, 1)
    with pytest.raises(InvalidRequest, match="limit"):
        ExternalArtifactService(service.db, service.artifacts).admit(spec, directory)
    assert set(service.artifacts.root.rglob("*")) == existing


def test_external_artifact_cli_import_and_verification(tmp_path, monkeypatch):
    spec = inventory(tmp_path / "input")
    descriptor = tmp_path / "spec.json"
    descriptor.write_text(spec.model_dump_json())
    monkeypatch.setenv("PEYKAR_DATA_DIR", str(tmp_path / "state"))
    runner = CliRunner()
    result = runner.invoke(
        app,
        ["external-artifacts", "import", str(descriptor), "--directory", str(tmp_path / "input")],
    )
    assert result.exit_code == 0, result.output
    report = json.loads(result.output)
    assert report["verified_files"] == 3
    assert not report["runtime_reproduced"]
    assert not report["code_executed"]
    result = runner.invoke(app, ["external-artifacts", "verify", spec.id, spec.version])
    assert result.exit_code == 0, result.output
    assert json.loads(result.output) == report
