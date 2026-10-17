import hashlib
import importlib.util
import json
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]


def script(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / "scripts" / f"{name}.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


vendor = script("vendor_sources")
verification = script("verify_vendor")
dependencies = script("fetch_source_dependencies")
native = script("probe_native_runtimes")


def git(root, *args):
    return subprocess.check_output(["git", "-C", str(root), *args], stderr=subprocess.DEVNULL)


@pytest.fixture
def source(tmp_path):
    root = tmp_path / "source"
    root.mkdir()
    git(root, "init", "--template=", ".")
    git(root, "config", "user.email", "fixture@example.test")
    git(root, "config", "user.name", "Source fixture")
    (root / "LICENSE").write_text("Original author and license\n")
    (root / "engine.py").write_text("print('original')\n")
    (root / "engine.py").chmod(0o755)
    git(root, "add", ".")
    git(root, "commit", "-m", "fixture")
    return root


def distribution(tmp_path, source):
    inventory = []
    target = tmp_path / "vendor/upstream/fixture"
    vendor.copy_tree(source, target, inventory)
    raw = json.dumps(inventory).encode()
    index = tmp_path / "vendor/inventory.json"
    index.write_bytes(raw)
    release = {
        "id": "fixture",
        "path": "vendor/upstream/fixture",
        "inventory": "vendor/inventory.json",
        "inventory_sha256": hashlib.sha256(raw).hexdigest(),
        "missing_dependencies": [],
    }
    (tmp_path / "vendor/manifest.json").write_text(json.dumps({"releases": [release]}))
    return target


def test_source_copy_retains_bytes_modes_not_git_metadata(tmp_path, source):
    target = distribution(tmp_path, source)
    assert not (target / ".git").exists()
    assert (target / "LICENSE").read_bytes() == (source / "LICENSE").read_bytes()
    assert (target / "engine.py").stat().st_mode & 0o111
    assert verification.verify(tmp_path)["files"] == 2


def test_modified_checkout_cannot_be_silently_vendored(tmp_path, source):
    (source / "engine.py").write_text("print('modified')\n")
    with pytest.raises(ValueError, match="Tracked source bytes changed"):
        vendor.copy_tree(source, tmp_path / "target", [])


def test_source_conflict_is_preserved(tmp_path, source):
    target = distribution(tmp_path, source)
    (target / "LICENSE").write_text("Existing independent notice\n")
    with pytest.raises(ValueError, match="Existing vendor file differs"):
        vendor.copy_tree(source, target, [])
    assert (target / "LICENSE").read_text() == "Existing independent notice\n"


def test_vendor_parent_symlink_cannot_redirect_copy(tmp_path, source):
    target = tmp_path / "target"
    target.symlink_to(source, target_is_directory=True)
    with pytest.raises(ValueError, match="traverses a symlink"):
        vendor.copy_tree(source, target, [])


@pytest.mark.parametrize("corruption", ["bytes", "extra", "mode"])
def test_source_verification_detects_distribution_mutations(tmp_path, source, corruption):
    target = distribution(tmp_path, source)
    if corruption == "bytes":
        (target / "LICENSE").write_text("altered")
    elif corruption == "extra":
        (target / "unexpected.py").write_text("untracked executable")
    else:
        (target / "engine.py").chmod(0o644)
    with pytest.raises(ValueError):
        verification.verify(tmp_path)


def test_unresolved_lfs_payload_cannot_pass_full_source_verification(tmp_path, source):
    (source / "pointer").write_text("version https://git-lfs.github.com/spec/v1\noid sha256:abc\n")
    git(source, "add", "pointer")
    git(source, "commit", "-m", "pointer fixture")
    distribution(tmp_path, source)
    with pytest.raises(ValueError, match="Unresolved Git LFS"):
        verification.verify(tmp_path)


def test_relative_git_dependencies_are_resolved_against_https_origin(source):
    git(source, "remote", "add", "origin", "https://github.com/owner/project.git")
    oid = git(source, "rev-parse", "HEAD").decode().strip()
    git(source, "update-index", "--add", "--cacheinfo", f"160000,{oid},thirdparty")
    (source / ".gitmodules").write_text(
        '[submodule "thirdparty"]\npath=thirdparty\nurl=../../pybind/pybind11\n'
    )
    git(source, "add", ".gitmodules")
    git(source, "commit", "-m", "dependency fixture")
    assert (
        dependencies.discover(source, "fixture")[0]["url"] == "https://github.com/pybind/pybind11"
    )


def test_native_partial_omex_failure_cannot_report_success(tmp_path):
    output = tmp_path / "output"
    output.mkdir()
    (output / "reports.h5").write_bytes(b"one successful task")
    (output / "log.yml").write_text('status: "SUCCEEDED"\ntasks:\n- status: "FAILED"\n')
    with pytest.raises(ValueError, match="archive execution failed"):
        native.verify("vcell", "VCell", tmp_path)


def test_native_artifact_mutation_is_rejected(tmp_path):
    (tmp_path / "native.csv").write_bytes(b"mutated")
    (tmp_path / "receipt.json").write_text(
        json.dumps(
            {
                "engine": "COPASI",
                "native_execution": "passed",
                "artifacts": {"native.csv": "0" * 64},
            }
        )
    )
    with pytest.raises(ValueError, match="checksum/path mismatch"):
        native.verify("receipt", "COPASI", tmp_path)


@pytest.mark.parametrize("timeout", [False, True])
def test_native_failure_retains_receipt_and_cleans_bounded_container(
    tmp_path, monkeypatch, timeout
):
    catalog = tmp_path / "catalog.json"
    catalog.write_text(
        json.dumps(
            {
                "profiles": {
                    "fixture": {
                        "engine": "Fixture",
                        "image": "locally-reviewed",
                        "platform": "linux/amd64",
                        "kind": "receipt",
                        "timeout_seconds": 1,
                    }
                }
            }
        )
    )
    calls = []
    monkeypatch.setattr(native.subprocess, "check_output", lambda *a, **k: "sha256:" + "a" * 64)

    def execute(command, **kwargs):
        calls.append(command)
        if command[1] == "run":
            assert command[command.index("--network") + 1] == "none"
            assert command[command.index("--pull") + 1] == "never"
            assert "--read-only" in command and "--user" in command
            assert kwargs["timeout"] == 1
            if timeout:
                raise subprocess.TimeoutExpired(command, 1)
            return subprocess.CompletedProcess(command, 2)
        return subprocess.CompletedProcess(command, 0)

    monkeypatch.setattr(native.subprocess, "run", execute)
    with pytest.raises(subprocess.TimeoutExpired if timeout else RuntimeError):
        native.run_probe("fixture", tmp_path / "job", catalog)
    receipt = json.loads((tmp_path / "job/execution.json").read_bytes())
    assert receipt["status"] == "failed"
    assert calls[-1][1:3] == ["rm", "-f"]
    assert calls[-1][-1] == calls[0][calls[0].index("--name") + 1]


def test_existing_native_job_is_never_overwritten(tmp_path, monkeypatch):
    job = tmp_path / "job"
    job.mkdir()
    (job / "sentinel").write_text("existing measurements")
    monkeypatch.setattr(native.subprocess, "check_output", lambda *a, **k: "sha256:" + "a" * 64)
    with pytest.raises(FileExistsError):
        native.run_probe("copasi", job)
    assert (job / "sentinel").read_text() == "existing measurements"
