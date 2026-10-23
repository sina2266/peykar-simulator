"""Real subprocess exits prove durability; synthetic fixtures prove no biology."""

import json
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

from peykar.integrations.vcornea_publication import recover_native_completion
from peykar_core.errors import Conflict
from peykar_core.hashing import bytes_digest, canonical_json, digest

GUARD = Path("packages/platform/src/peykar/integrations/vcornea_completion_guard.py")


@pytest.fixture
def workspace(tmp_path):
    root = tmp_path / "native"
    root.mkdir()
    project = root / "project"
    project.mkdir()
    (project / "Parameters.py").write_text("# Synthetic durability test only\n")
    driver = root / "driver.py"
    driver.write_text(
        "from pathlib import Path\n"
        "Path('project/cell_count_test.csv').write_text('synthetic fixture only\\n')\n"
    )
    shutil.copyfile(GUARD, root / "completion_guard.py")
    runtime = {"scope": "synthetic durability test only"}
    manifest = {
        "driver_digest": bytes_digest(driver.read_bytes()),
        "execution_guard_digest": bytes_digest(GUARD.read_bytes()),
        "execution_files": {
            "Parameters.py": bytes_digest((project / "Parameters.py").read_bytes())
        },
    }
    (root / "manifest.json").write_text(canonical_json(manifest))
    (root / "runtime-profile.json").write_text(canonical_json(runtime))
    record = {
        "status": "running",
        "outputs": {},
        "manifest_digest": digest(manifest),
        "runtime_digest": digest(runtime),
    }
    (root / "record.json").write_text(canonical_json(record))
    return root


def invoke(root):
    return subprocess.run(
        [
            sys.executable,
            str(root / "completion_guard.py"),
            "--driver",
            str(root / "driver.py"),
            "--wall-seconds",
            "20",
        ],
        cwd=root,
        capture_output=True,
        check=False,
    )


def test_actual_child_exit_and_hashes_survive_outer_record_loss(workspace):
    root = workspace
    original = (root / "record.json").read_bytes()
    result = invoke(root)
    assert result.returncode == 0
    proof = json.loads((root / "completion.json").read_bytes())
    assert proof["returncode"] == 0
    assert proof["outputs"]["project/cell_count_test.csv"] == bytes_digest(
        (root / "project/cell_count_test.csv").read_bytes()
    )
    recovered = recover_native_completion(root)
    assert recovered["status"] == "succeeded" and recovered["returncode"] == 0
    assert (root / "record-before-recovery.json").read_bytes() == original
    assert not (root / "completion.json.tmp").exists()
    with pytest.raises(Conflict, match="stranded"):
        recover_native_completion(root)


def test_no_exit_proof_or_replaced_outputs_cannot_be_promoted(workspace):
    root = workspace
    (root / "project/cell_count_test.csv").write_text("complete-looking data is not an exit code")
    with pytest.raises(FileNotFoundError):
        recover_native_completion(root)
    assert invoke(root).returncode == 0
    (root / "project/cell_count_test.csv").write_text("changed data")
    with pytest.raises(Conflict, match="output changed"):
        recover_native_completion(root)
    assert not (root / "record-before-recovery.json").exists()


def test_actual_nonzero_exit_remains_a_failure(workspace):
    root = workspace
    driver = root / "driver.py"
    driver.write_text("raise SystemExit(9)\n")
    manifest = json.loads((root / "manifest.json").read_bytes())
    manifest["driver_digest"] = bytes_digest(driver.read_bytes())
    (root / "manifest.json").write_text(canonical_json(manifest))
    record = json.loads((root / "record.json").read_bytes())
    record["manifest_digest"] = digest(manifest)
    (root / "record.json").write_text(canonical_json(record))
    assert invoke(root).returncode == 9
    recovered = recover_native_completion(root)
    assert recovered["status"] == "failed" and recovered["returncode"] == 9
