import json
from xml.etree import ElementTree

import pytest

from peykar.integrations import vcornea_runtime as runtime
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import canonical_json, digest


@pytest.fixture
def prepared_source(tmp_path, monkeypatch):
    # Tiny source fixture exercises orchestration; it does not represent vCornea biology.
    source = tmp_path / "archive"
    project = source / "Local/Project/paper_version"
    simulation = project / "Simulation"
    simulation.mkdir(parents=True)
    (project / "vCornea_v2.cc3d").write_text("<Simulation/>")
    (simulation / "Parameters.py").write_text("SimTime=7700\n")
    (simulation / "vCornea_v2.xml").write_text(
        "<CompuCell3D><Potts><Steps>100000</Steps><Temperature>8</Temperature></Potts></CompuCell3D>"
    )
    inventory = runtime._file_inventory(source)
    monkeypatch.setattr(runtime, "SOURCE_TREE_DIGEST", digest(inventory))
    pins = tmp_path / "pins.json"
    pins.write_text(
        canonical_json(
            {
                "doi": runtime.ARCHIVE_DOI,
                "archive": {"sha256": runtime.ARCHIVE_SHA256},
                "license": "CC-BY-4.0",
                "files": inventory,
            }
        )
    )
    return source, pins, project


def test_private_copy_preserves_source_and_records_declared_controls(prepared_source, tmp_path):
    source, pins, original = prepared_source
    before = runtime._file_inventory(source)
    destination = tmp_path / "run"
    manifest = runtime.prepare_run(source, pins, destination, scenario="injury", steps=20, seed=7)
    assert runtime._file_inventory(source) == before
    assert manifest["source_tree_digest"] == digest(before)
    assert manifest["controls"] == {"steps_mcs": 20, "seed": 7}
    parameters = destination / "project/Simulation/Parameters.py"
    assert parameters.read_text().endswith("SimTime=20\n")
    potts = (
        ElementTree.parse(destination / "project/Simulation/vCornea_v2.xml").getroot().find("Potts")
    )
    assert potts.findtext("Steps") == "22"
    assert potts.findtext("RandomSeed") == "7"
    assert potts.findtext("Temperature") == "8"
    assert (original / "Simulation/Parameters.py").read_text() == "SimTime=7700\n"
    with pytest.raises(Conflict, match="already exists"):
        runtime.prepare_run(source, pins, destination, scenario="injury", steps=20, seed=7)


def test_unmodified_copy_has_unknown_seed_and_preserves_all_project_bytes(
    prepared_source, tmp_path
):
    source, pins, original = prepared_source
    destination = tmp_path / "run"
    manifest = runtime.prepare_run(
        source, pins, destination, scenario="injury", steps=None, seed=None
    )
    assert manifest["seed_scope"] == "unknown"
    assert runtime._file_inventory(destination / "project") == runtime._file_inventory(original)


def test_changed_or_forged_source_pin_does_not_authorize_execution(prepared_source, tmp_path):
    source, pins, original = prepared_source
    (original / "Simulation/Parameters.py").write_text("SimTime=9000\n")
    forged = json.loads(pins.read_text())
    forged["files"] = runtime._file_inventory(source)
    pins.write_text(canonical_json(forged))
    with pytest.raises(Conflict, match="reviewed"):
        runtime.prepare_run(source, pins, tmp_path / "run", scenario="injury", steps=20, seed=7)
    assert not (tmp_path / "run").exists()


def test_symlink_source_is_rejected(prepared_source, tmp_path):
    source, pins, original = prepared_source
    (original / "link").symlink_to(pins)
    with pytest.raises(InvalidRequest, match="symlink"):
        runtime.prepare_run(source, pins, tmp_path / "run", scenario="injury", steps=20, seed=7)


@pytest.mark.parametrize(
    "steps,seed", [(19, 7), (100_001, 7), (True, 7), (20.5, 7), (20, -1), (20, True)]
)
def test_invalid_runtime_controls_are_rejected(prepared_source, tmp_path, steps, seed):
    source, pins, _ = prepared_source
    with pytest.raises(InvalidRequest):
        runtime.prepare_run(
            source, pins, tmp_path / "run", scenario="injury", steps=steps, seed=seed
        )


def test_container_receives_only_private_workspace_and_bounded_profile(tmp_path):
    command = runtime.docker_command(
        "sha256:" + "a" * 64, tmp_path, "peykar-vcornea-" + "b" * 32, 7
    )
    assert command[command.index("--network") + 1] == "none"
    assert "--read-only" in command and "no-new-privileges:true" in command
    assert command[command.index("--user") + 1] != "0:0"
    assert command.count("--mount") == 1
    assert "docker.sock" not in " ".join(command)
    assert command[command.index("--mount") + 1] == f"type=bind,src={tmp_path},dst=/work"
    with pytest.raises(InvalidRequest):
        runtime.docker_command("mutable:tag", tmp_path, "peykar-vcornea-" + "b" * 32, 7)


def test_prepared_source_tampering_fails_before_docker(prepared_source, tmp_path, monkeypatch):
    source, pins, _ = prepared_source
    destination = tmp_path / "run"
    runtime.prepare_run(source, pins, destination, scenario="injury", steps=20, seed=7)
    (destination / "driver.py").write_text("changed")
    monkeypatch.setattr(
        runtime.subprocess, "check_output", lambda *a, **k: pytest.fail("Docker called")
    )
    with pytest.raises(Conflict, match="changed"):
        runtime.execute_run(destination, "image")


def test_runtime_failure_is_recorded_and_container_cleanup_always_runs(
    prepared_source, tmp_path, monkeypatch
):
    source, pins, _ = prepared_source
    destination = tmp_path / "run"
    runtime.prepare_run(source, pins, destination, scenario="injury", steps=20, seed=7)
    monkeypatch.setenv("PEYKAR_API_TOKEN", "never-inherit-this-secret")
    monkeypatch.setattr(runtime.subprocess, "check_output", lambda *a, **k: "sha256:" + "a" * 64)
    cleanup = []

    class FailedProcess:
        returncode = 1

        def __init__(self, *args, **kwargs):
            assert "PEYKAR_API_TOKEN" not in kwargs["env"]

        def poll(self):
            return self.returncode

    class Cleaned:
        returncode = 0

    def remove(command, **kwargs):
        cleanup.append(command)
        return Cleaned()

    monkeypatch.setattr(runtime.subprocess, "Popen", FailedProcess)
    monkeypatch.setattr(runtime.subprocess, "run", remove)
    with pytest.raises(InvalidRequest, match="failed"):
        runtime.execute_run(destination, "image")
    record = json.loads((destination / "record.json").read_text())
    assert record["status"] == "failed" and not record["outputs"]
    assert cleanup[0][:3] == ("docker", "rm", "--force")
    with pytest.raises(Conflict, match="already"):
        runtime.execute_run(destination, "image")
