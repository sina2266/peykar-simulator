"""Explicit operator execution of one reviewed, pinned vCornea archive in Docker.

No API token, Docker socket or host data directory is mounted inside the model.
This local execution profile is not an independently reviewed hostile-code sandbox.
"""

import hashlib
import json
import math
import os
import re
import shutil
import stat
import subprocess
import tempfile
import time
import uuid
import zipfile
from pathlib import Path
from typing import Any, Literal
from xml.etree import ElementTree

from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, canonical_json, digest

ARCHIVE_SHA256 = "855245e200cc322371b133a8c9e9fc0e81310ab7f3603ebf49afc4afcaf718dc"
ARCHIVE_DOI = "10.5281/zenodo.16884414"
SOURCE_TREE_DIGEST = "af65f6f2ec17f9e1ebef0af5327a23d4b295fa3f74a272015ff88e8d94fa72a9"
ScenarioName = Literal["injury", "stem-only"]
PROJECTS = {"injury": "paper_version", "stem-only": "paper_version_STEM_ONLY"}
MAX_LOG_BYTES = 32 * 1024 * 1024
MAX_WORK_BYTES = 256 * 1024 * 1024


def extract_source(archive: Path, pins: Path, destination: Path) -> Path:
    """Publish only the 27 reviewed source files; never extract bundled Git hooks/history."""
    checksum = hashlib.sha256()
    with archive.open("rb") as stream:
        while chunk := stream.read(1024 * 1024):
            checksum.update(chunk)
    expected = json.loads(pins.read_bytes())
    files = expected.get("files", {})
    if (
        checksum.hexdigest() != ARCHIVE_SHA256
        or expected.get("doi") != ARCHIVE_DOI
        or not files
        or digest(files) != SOURCE_TREE_DIGEST
    ):
        raise Conflict("Archive or inventory differs from the reviewed source")
    if destination.exists():
        if _file_inventory(destination) != files:
            raise Conflict("Existing source destination differs; preserve it")
        return destination
    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(
        prefix=".vcornea-source-", dir=destination.parent
    ) as directory:
        staging = Path(directory) / "source"
        staging.mkdir()
        with zipfile.ZipFile(archive) as source_zip:
            for name, pinned in files.items():
                relative = Path(name)
                if relative.is_absolute() or ".." in relative.parts or ".git" in relative.parts:
                    raise InvalidRequest("Unsafe source inventory path")
                member = source_zip.getinfo("vCornea/" + name)
                if member.file_size > 8 * 1024 * 1024:
                    raise InvalidRequest("Source member exceeds the reviewed file limit")
                raw = source_zip.read(member)
                if bytes_digest(raw) != pinned:
                    raise Conflict("Source member differs from its pinned bytes")
                target = staging / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(raw)
        if _file_inventory(staging) != files:
            raise Conflict("Extracted source inventory differs")
        staging.rename(destination)
    return destination


def _file_inventory(root: Path) -> dict[str, str]:
    inventory = {}
    for path in sorted(root.rglob("*")):
        info = path.lstat()
        if stat.S_ISLNK(info.st_mode):
            raise InvalidRequest("A model workspace must not contain symlinks")
        if not stat.S_ISDIR(info.st_mode) and not stat.S_ISREG(info.st_mode):
            raise InvalidRequest("A model workspace must contain only regular files/directories")
        if path.is_file():
            if not stat.S_ISREG(info.st_mode) or info.st_size > 8 * 1024 * 1024:
                raise InvalidRequest("Model source must contain bounded regular files")
            inventory[path.relative_to(root).as_posix()] = bytes_digest(path.read_bytes())
    return inventory


def prepare_run(
    source: Path,
    pins: Path,
    destination: Path,
    *,
    scenario: ScenarioName,
    steps: int | None,
    seed: int | None,
) -> dict[str, Any]:
    """Verify the archive tree, then change only declared controls in a private copy."""
    if scenario not in PROJECTS:
        raise InvalidRequest("Unknown archived vCornea scenario")
    if steps is not None and (type(steps) is not int or not 20 <= steps <= 100_000):
        raise InvalidRequest("Steps must be between 20 and 100000 MCS")
    if seed is not None and (type(seed) is not int or not 0 <= seed <= 2**32 - 1):
        raise InvalidRequest("Seed must be an unsigned 32-bit integer")
    source = source.resolve(strict=True)
    inventory = _file_inventory(source)
    pin_document = json.loads(pins.read_bytes())
    if (
        pin_document.get("doi") != ARCHIVE_DOI
        or pin_document.get("archive", {}).get("sha256") != ARCHIVE_SHA256
        or pin_document.get("license") != "CC-BY-4.0"
        or pin_document.get("files") != inventory
        or digest(inventory) != SOURCE_TREE_DIGEST
    ):
        raise Conflict("Source tree differs from the reviewed vCornea archive inventory")
    if destination.exists():
        raise Conflict("Run destination already exists; preserve its previous evidence")
    destination.mkdir(parents=True)
    project = destination / "project"
    shutil.copytree(source / "Local" / "Project" / PROJECTS[scenario], project)
    (destination / "driver.py").write_bytes(
        Path(__file__).with_name("vcornea_driver.py").read_bytes()
    )
    controls: dict[str, int | None] = {"steps_mcs": steps, "seed": seed}
    if steps is not None:
        parameters = project / "Simulation" / "Parameters.py"
        with parameters.open("ab") as output:
            output.write(
                f"\n# Peykar execution control: bounded horizon only\nSimTime={steps}\n".encode()
            )
    if steps is not None or seed is not None:
        path = project / "Simulation" / "vCornea_v2.xml"
        tree = ElementTree.parse(path)
        potts = tree.getroot().find("Potts")
        if potts is None:
            raise InvalidRequest("Archived Potts configuration is missing")
        if steps is not None:
            element = potts.find("Steps")
            if element is None:
                raise InvalidRequest("Archived steps configuration is missing")
            # Upstream writes artifacts at SimTime+1, before stopping. Include that step.
            element.text = str(steps + 2)
        if seed is not None:
            element = potts.find("RandomSeed")
            if element is None:
                element = ElementTree.SubElement(potts, "RandomSeed")
            element.text = str(seed)
        tree.write(path, encoding="utf-8", xml_declaration=True)
    execution_inventory = _file_inventory(project)
    manifest = {
        "schema_version": "1.0.0",
        "id": destination.name,
        "archive_doi": ARCHIVE_DOI,
        "archive_digest": ARCHIVE_SHA256,
        "source_tree_digest": digest(inventory),
        "source_files": inventory,
        "execution_tree_digest": digest(execution_inventory),
        "execution_files": execution_inventory,
        "driver_digest": bytes_digest((destination / "driver.py").read_bytes()),
        "scenario": scenario,
        "controls": controls,
        "seed_scope": "Python random, NumPy and Potts RandomSeed"
        if seed is not None
        else "unknown",
        "changes": (
            "Private copy; explicit horizon/seed only; XML serialization may change formatting"
            if steps is not None or seed is not None
            else "Private byte-identical copy; no parameter or seed override"
        ),
        "scientific_evidence": "not_established_by_execution",
        "limitations": [
            "Bounded/seed-controlled integration run; published reproduction is not established.",
            "Reconstructed pinned runtime; the authors' original environment was not recovered.",
        ],
    }
    (destination / "manifest.json").write_text(canonical_json(manifest) + "\n")
    return manifest


def docker_command(
    image_id: str, directory: Path, name: str, seed: int | None, wall_seconds: float = 300
) -> tuple[str, ...]:
    """Only this private job directory is exposed to the computational container."""
    if not re.fullmatch(r"sha256:[0-9a-f]{64}", image_id):
        raise InvalidRequest("Resolve the runtime to an immutable Docker image ID")
    if not re.fullmatch(r"peykar-vcornea-[0-9a-f]{32}", name):
        raise InvalidRequest("Invalid private container name")
    if any(character in str(directory.resolve()) for character in (",", "\n", "\r")):
        raise InvalidRequest("Docker workspace paths cannot contain commas or newlines")
    command: tuple[str, ...] = (
        "docker",
        "run",
        "--rm",
        "--name",
        name,
        "--platform",
        "linux/amd64",
        "--network",
        "none",
        "--read-only",
        "--cap-drop",
        "ALL",
        "--security-opt",
        "no-new-privileges:true",
        "--pids-limit",
        "128",
        "--cpus",
        "2",
        "--memory",
        "4g",
        "--memory-swap",
        "4g",
        "--user",
        f"{os.getuid()}:{os.getgid()}" if os.getuid() else "10001:10001",
        "--tmpfs",
        "/tmp:rw,nosuid,nodev,size=128m,mode=1777",
        "--mount",
        f"type=bind,src={directory.resolve()},dst=/work",
        "--workdir",
        "/work/project",
        "--entrypoint",
        "/opt/conda/envs/cc3d/bin/python",
        image_id,
        "/work/driver.py",
    )
    if seed is not None:
        command += ("--seed", str(seed))
    command += ("--wall-seconds", str(wall_seconds))
    return command + ("--input", "/work/project/vCornea_v2.cc3d", "--output-dir", "/work/engine")


def execute_run(directory: Path, image: str, *, wall_seconds: float = 300) -> dict[str, Any]:
    if not math.isfinite(wall_seconds) or not 1 <= wall_seconds <= 86_400:
        raise InvalidRequest("Wall budget must be between 1 and 86400 seconds")
    manifest = json.loads((directory / "manifest.json").read_bytes())
    if (directory / "record.json").exists():
        raise Conflict("Run already has an execution record; create a new run for retry")
    if (
        _file_inventory(directory / "project") != manifest["execution_files"]
        or bytes_digest((directory / "driver.py").read_bytes()) != manifest["driver_digest"]
    ):
        raise Conflict("Prepared model/driver changed before execution")
    # The Docker client uses the operator's existing context, but receives no API token.
    environment = {
        key: os.environ[key] for key in ("PATH", "HOME", "DOCKER_CONFIG") if key in os.environ
    }
    image_id = subprocess.check_output(
        ("docker", "image", "inspect", "--format", "{{.Id}}", image),
        env=environment,
        timeout=15,
        text=True,
    ).strip()
    name = "peykar-vcornea-" + uuid.uuid4().hex
    command = docker_command(image_id, directory, name, manifest["controls"]["seed"], wall_seconds)
    log = directory / "engine.log"
    started = time.monotonic()
    record: dict[str, Any] = {
        "id": manifest["id"],
        "manifest_digest": digest(manifest),
        "image_id": image_id,
        "container": name,
        "wall_seconds_limit": wall_seconds,
        "status": "running",
        "command": command,
        "outputs": {},
    }
    process = None
    try:
        with log.open("xb") as output:
            process = subprocess.Popen(
                command, stdout=output, stderr=subprocess.STDOUT, env=environment
            )
            while process.poll() is None:
                if time.monotonic() - started > wall_seconds:
                    raise TimeoutError("vCornea wall deadline exceeded")
                if log.stat().st_size > MAX_LOG_BYTES:
                    raise InvalidRequest("vCornea log exceeds the bounded output limit")
                size = sum(p.stat().st_size for p in directory.rglob("*") if p.is_file())
                if size > MAX_WORK_BYTES:
                    raise InvalidRequest("vCornea workspace exceeds the bounded output limit")
                time.sleep(0.2)
        if process.returncode:
            raise InvalidRequest("vCornea runtime failed; inspect the private engine.log")
        if sum(p.stat().st_size for p in directory.rglob("*") if p.is_file()) > MAX_WORK_BYTES:
            raise InvalidRequest("vCornea workspace exceeds the bounded output limit")
        outputs = {
            p.relative_to(directory).as_posix(): bytes_digest(p.read_bytes())
            for p in sorted((directory / "project").rglob("*"))
            if p.is_file() and p.suffix in (".csv", ".parquet")
        }
        if not any(Path(p).name.startswith("cell_count_") for p in outputs):
            raise InvalidRequest("Runtime exited without a native cell-count output")
        record.update(status="succeeded", outputs=outputs)
    except BaseException as exc:
        record.update(status="cancelled" if isinstance(exc, KeyboardInterrupt) else "failed")
        record["failure"] = type(exc).__name__
        raise
    finally:
        try:
            cleanup = subprocess.run(
                ("docker", "rm", "--force", name),
                env=environment,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                timeout=15,
                check=False,
            )
            record["cleanup"] = (
                "removed_or_already_exited" if cleanup.returncode == 0 else "check_daemon"
            )
        except (OSError, subprocess.TimeoutExpired):
            record["cleanup"] = "daemon_unreachable_check_container"
        if process is not None and process.poll() is None:
            process.terminate()
            process.wait(timeout=10)
        record["elapsed_seconds"] = time.monotonic() - started
        (directory / "record.json").write_text(canonical_json(record) + "\n")
    return record
