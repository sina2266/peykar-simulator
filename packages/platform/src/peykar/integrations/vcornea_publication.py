"""Reviewed full-horizon vCornea recipes and explicit macOS operator execution.

Original model files and historical Docker run meanings remain unchanged.
"""

import json
import math
import os
import re
import signal
import subprocess
import time
from contextlib import suppress
from pathlib import Path
from typing import Any

from peykar.integrations.vcornea_runtime import (
    MAX_LOG_BYTES,
    MAX_WORK_BYTES,
    PROJECTS,
    ScenarioName,
    _file_inventory,
    prepare_run,
)
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, canonical_json, digest

RECIPES: dict[str, tuple[ScenarioName, int, dict[str, int | float]]] = {
    "self-organization": ("stem-only", 43200, {}),
    "slight-injury": ("injury", 50400, {"InjuryTime": 7200, "SLS_Concentration": 750.0}),
    "mild-injury": ("injury", 50400, {"InjuryTime": 7200, "SLS_Concentration": 1500.0}),
    "moderate-injury": ("injury", 50400, {"InjuryTime": 7200, "SLS_Concentration": 2500.0}),
}
PARAMETER_PROFILES = (
    "local-1.0.0",
    "hpc-2.0.0",
    "historical-hpc-3.0.0",
    "observed-injury-4.0.0",
    "stash-hpc-5.0.0",
    "observed-injury-serial-6.0.0",
    "stash-formation-serial-7.0.0",
)


def write_operator_json(path: Path, value: dict[str, Any]) -> None:
    temporary = path.with_name(path.name + ".tmp")
    with temporary.open("w") as stream:
        stream.write(canonical_json(value) + "\n")
        stream.flush()
        os.fsync(stream.fileno())
    temporary.replace(path)


def recover_native_completion(directory: Path) -> dict[str, Any]:
    """Adopt an actual durable child-exit proof, never infer success from output files."""
    root = directory.resolve(strict=True)
    record_path = root / "record.json"
    original = record_path.read_bytes()
    record: dict[str, Any] = json.loads(original)
    if record["status"] != "running":
        raise Conflict("Recovery requires a stranded running record")
    manifest = json.loads((root / "manifest.json").read_bytes())
    runtime = json.loads((root / "runtime-profile.json").read_bytes())
    proof = json.loads((root / "completion.json").read_bytes())
    if (
        not manifest.get("execution_guard_digest")
        or bytes_digest((root / "completion_guard.py").read_bytes())
        != manifest["execution_guard_digest"]
        or record["manifest_digest"] != digest(manifest)
        or proof.get("scope") != "observed_native_child_exit"
        or proof["manifest_digest"] != digest(manifest)
        or proof["runtime_digest"] != digest(runtime)
        or record["runtime_digest"] != digest(runtime)
    ):
        raise Conflict("Durable completion identity differs")
    for relative, checksum in manifest["execution_files"].items():
        if bytes_digest((root / "project" / relative).read_bytes()) != checksum:
            raise Conflict("Executed source changed")
    if bytes_digest((root / "driver.py").read_bytes()) != manifest["driver_digest"]:
        raise Conflict("Executed driver changed")
    for relative, checksum in proof["outputs"].items():
        path = (root / relative).resolve(strict=True)
        if not path.is_relative_to(root) or bytes_digest(path.read_bytes()) != checksum:
            raise Conflict("Durable output changed or escaped the workspace")
    if proof["returncode"] == 0 and not any(
        Path(name).name.startswith("cell_count_") for name in proof["outputs"]
    ):
        raise Conflict("Durable completion has no native count output")
    # Retain the exact interrupted record before the explicit operator reconciliation.
    with (root / "record-before-recovery.json").open("xb") as stream:
        stream.write(original)
    record.update(
        status="succeeded" if proof["returncode"] == 0 else "failed",
        returncode=proof["returncode"],
        outputs=proof["outputs"],
        elapsed_seconds=proof["elapsed_seconds"],
        completion_digest=bytes_digest((root / "completion.json").read_bytes()),
        reconciliation="actual_child_exit_proof_after_outer_supervisor_loss",
    )
    write_operator_json(record_path, record)
    return record


def publication_overrides(recipe: str, parameter_profile: str) -> dict[str, int | float]:
    if recipe not in RECIPES or parameter_profile not in PARAMETER_PROFILES:
        raise InvalidRequest("Unknown reviewed publication recipe or parameter profile")
    overrides = dict(RECIPES[recipe][2])
    if parameter_profile == "hpc-2.0.0" and recipe == "self-organization":
        # Exact values in the pinned HPC script named STEM_ONLY_proper_thickness_6months.
        # These differ from Local Parameters.py AND the final article's S8/S9 tables.
        overrides.update(
            InitBASAL_TargetVolume=40.0, EGF_BASAL_HalfMaxValue=8.0, EGF_SUPERDiffCoef=12.0
        )
    return overrides


def prepare_publication(
    source: Path,
    pins: Path,
    destination: Path,
    recipe: str,
    seed: int,
    *,
    parameter_profile: str = "local-1.0.0",
    injury_state: Path | None = None,
) -> dict[str, Any]:
    if parameter_profile in ("stash-hpc-5.0.0", "stash-formation-serial-7.0.0"):
        from peykar.integrations.vcornea_stash import prepare_stash_publication

        if injury_state is not None:
            raise InvalidRequest("January snapshot retains its own original initial state")
        return prepare_stash_publication(
            source,
            pins,
            destination,
            recipe,
            seed,
            serial_formation=parameter_profile == "stash-formation-serial-7.0.0",
        )
    if parameter_profile in (
        "historical-hpc-3.0.0",
        "observed-injury-4.0.0",
        "observed-injury-serial-6.0.0",
    ):
        from peykar.integrations.vcornea_history import prepare_historical_publication

        if parameter_profile in ("observed-injury-4.0.0", "observed-injury-serial-6.0.0"):
            if injury_state is None or recipe == "self-organization":
                raise InvalidRequest("Observed injury requires the explicit evidenced initial PIFF")
        elif injury_state is not None:
            raise InvalidRequest("Historical formation profile does not accept another state")
        return prepare_historical_publication(
            source,
            pins,
            destination,
            recipe,
            seed,
            injury_state=injury_state,
            serial_injury=parameter_profile == "observed-injury-serial-6.0.0",
        )
    if injury_state is not None:
        raise InvalidRequest("This source profile retains its original initial state")
    overrides = publication_overrides(recipe, parameter_profile)
    scenario, steps, _ = RECIPES[recipe]
    manifest = prepare_run(source, pins, destination, scenario=scenario, steps=steps, seed=seed)
    if overrides:
        path = destination / "project/Simulation/Parameters.py"
        with path.open("ab") as stream:
            stream.write(b"\n# Peykar: restore documented publication injury controls\n")
            for key, value in overrides.items():
                stream.write(f"{key}={value!r}\n".encode())
    manifest.update(
        publication_recipe=recipe,
        publication_recipe_version="1.0.0",
        parameter_profile=parameter_profile,
        parameter_overrides=overrides,
        publication_controls_evidence=(
            "Original Parameters.py documents injury at7200 and total50400; "
            "archived plot_manager.py selects self-organization Comb0 with43200 horizon. "
            "Injury severities are original README declarations."
        ),
        hpc_preset_evidence=(
            {
                "source": "HPC/Batch_Run_vCornea_Paper_version_STEM_ONLY_noVTK.py",
                "sha256": manifest["source_files"][
                    "HPC/Batch_Run_vCornea_Paper_version_STEM_ONLY_noVTK.py"
                ],
                "scope": "Published batch defaults; historical per-run parameters remain unknown",
            }
            if parameter_profile == "hpc-2.0.0" and recipe == "self-organization"
            else None
        ),
        execution_files=_file_inventory(destination / "project"),
        changes="Private copy: selected seed, full horizon and documented injury controls",
        limitations=[
            "Archive run parameters/seeds were not published; selected new seeds are explicit.",
            "Original plots and numerical comparison are required; preparation proves no match.",
            "Reproduction does not establish independent biological validation.",
        ],
    )
    manifest["execution_tree_digest"] = digest(manifest["execution_files"])
    (destination / "manifest.json").write_text(canonical_json(manifest) + "\n")
    manifest = repair_link_lifetime(destination, overlay_version="2.0.0")
    guard = Path(__file__).with_name("vcornea_completion_guard.py").read_bytes()
    (destination / "completion_guard.py").write_bytes(guard)
    manifest["execution_guard_digest"] = bytes_digest(guard)
    manifest["execution_guard_version"] = "1.0.0"
    (destination / "manifest.json").write_text(canonical_json(manifest) + "\n")
    return manifest


def repair_link_lifetime(directory: Path, *, overlay_version: str = "1.0.0") -> dict[str, Any]:
    """Version a native lifetime repair in a private, previously verified source copy."""
    if (directory / "record.json").exists():
        raise Conflict("Preserve executed source; use a new run for the lifetime repair")
    if overlay_version not in ("1.0.0", "2.0.0"):
        raise InvalidRequest("Unknown reviewed native lifetime overlay")
    manifest: dict[str, Any] = json.loads((directory / "manifest.json").read_bytes())
    path = directory / "project/Simulation/vCornea_v2Steppables.py"
    raw = path.read_bytes()
    source_path = manifest.get(
        "steppables_source_path",
        "Local/Project/" + PROJECTS[manifest["scenario"]] + "/Simulation/vCornea_v2Steppables.py",
    )
    if bytes_digest(raw) != manifest["source_files"][source_path]:
        raise Conflict("Lifetime overlay requires pristine verified upstream steppables")
    pattern = (
        rb"(        # update the forces if so defined[ \t]*\r?\n)"
        rb"            if self.AutoAdjustLinks:"
    )
    newline = b"\r\n" if b"\r\n" in raw else b"\n"
    replacement = (
        b"            # Peykar repair: reacquire a live link after possible deletion."
        + newline
        + b"            existing_link_with_closestWALL = self.get_fpp_link_by_cells("
        + b"cell, closest_neighbor_WALL)"
        + newline
        + b"            if existing_link_with_closestWALL and self.AutoAdjustLinks:"
    )
    modified, count = re.subn(pattern, lambda match: match[1] + replacement, raw)
    if count != 1:
        raise Conflict("Expected single reviewed deleted-link dereference site")
    if overlay_version == "2.0.0":
        # Deleting a membrane cell invalidates the live PixelTracker iterator.
        # A second next()/delete_cell() on that container accesses freed native data.
        pattern = (
            rb"(if self.SLS\[pixel.pixel.x, pixel.pixel.y, pixel.pixel.z\] > 0.1:[ \t]*\r?\n)"
            rb"([ \t]+)self.delete_cell\(cell\)[ \t]*\r?\n"
        )
        modified, count = re.subn(
            pattern,
            lambda m: (
                m[1]
                + m[2]
                + b"self.delete_cell(cell)"
                + newline
                + m[2]
                + b"break  # Peykar: deleted cell invalidates its pixel iterator"
                + newline
            ),
            modified,
        )
        if count != 1:
            raise Conflict("Expected single reviewed deleted-membrane iterator site")
    path.write_bytes(modified)
    manifest["compatibility_overlay"] = {
        "id": "vcornea.fpp-lifetime",
        "version": overlay_version,
        "upstream_digest": bytes_digest(raw),
        "patched_digest": bytes_digest(modified),
        "reason": (
            "Reacquire deleted links; version2 also stops a deleted membrane's pixel iterator"
        ),
        "scientific_scope": "New repaired source version; cohort agreement needs separate checks",
    }
    if "publication_recipe" in manifest:
        driver = Path(__file__).with_name("vcornea_publication_driver.py").read_bytes()
        (directory / "driver.py").write_bytes(driver)
        manifest["driver_digest"] = bytes_digest(driver)
        manifest["observation_scope"] = (
            "Read-only cell snapshots/progress; no RNG draws or physical state changes"
        )
    manifest["execution_files"] = _file_inventory(directory / "project")
    manifest["execution_tree_digest"] = digest(manifest["execution_files"])
    (directory / "manifest.json").write_text(canonical_json(manifest) + "\n")
    return manifest


def native_command(directory: Path, prefix: Path, wall_seconds: float) -> tuple[str, ...]:
    if not math.isfinite(wall_seconds) or not 1 <= wall_seconds <= 86400:
        raise InvalidRequest("Wall budget must be between1 and86400 seconds")
    root = directory.resolve(strict=True)
    manifest = json.loads((root / "manifest.json").read_bytes())
    temporary = root / "tmp"
    temporary.mkdir(exist_ok=True)
    profile = (
        "(version 1) (allow default) (deny network*) (deny file-write*) "
        f"(allow file-write* (subpath {json.dumps(str(root))})) "
        '(allow file-write* (literal "/dev/null"))'
    )
    entry = (
        (str(root / "completion_guard.py"), "--driver", str(root / "driver.py"))
        if manifest.get("execution_guard_digest")
        else (str(root / "driver.py"),)
    )
    command: tuple[str, ...] = (
        "/usr/bin/sandbox-exec",
        "-p",
        profile,
        str(prefix.resolve() / "bin/python"),
        *entry,
        "--wall-seconds",
        str(wall_seconds),
    )
    seed = manifest["controls"]["seed"]
    if seed is not None:
        command += ("--seed", str(seed))
    return command + (
        "--input",
        str(root / "project/vCornea_v2.cc3d"),
        "--output-dir",
        str(root / "engine"),
    )


def failed_native_details(
    root: Path, manifest: dict[str, Any], runtime: dict[str, Any], wrapper_returncode: int
) -> dict[str, Any]:
    """Retain a verified failed child's actual signal and partial hashes, never a result."""
    path = root / "completion.json"
    if not manifest.get("execution_guard_digest") or not path.exists():
        return {}
    proof = json.loads(path.read_bytes())
    code = proof.get("returncode")
    if (
        type(code) is not int
        or code == 0
        or (code if code > 0 else 128 - code) != wrapper_returncode
        or proof.get("scope") != "observed_native_child_exit"
        or proof.get("manifest_digest") != digest(manifest)
        or proof.get("runtime_digest") != digest(runtime)
        or bytes_digest((root / "completion_guard.py").read_bytes())
        != manifest["execution_guard_digest"]
    ):
        raise Conflict("Failed native completion proof disagrees")
    for relative, checksum in proof["outputs"].items():
        artifact = (root / relative).resolve(strict=True)
        if (
            not artifact.is_relative_to(root.resolve())
            or bytes_digest(artifact.read_bytes()) != checksum
        ):
            raise Conflict("Failed native artifact changed or escaped the workspace")
    return {
        "native_returncode": code,
        "native_signal": signal.Signals(-code).name if code < 0 else None,
        "completion_digest": bytes_digest(path.read_bytes()),
        "partial_outputs": proof["outputs"],
        "partial_output_scope": "failed_child_evidence_only_not_a_completed_result",
    }


def execute_native(
    directory: Path, prefix: Path, profile: Path, *, wall_seconds: float = 86400
) -> dict[str, Any]:
    """Run trusted pinned code with a kernel deadline, denied network and bounded writes."""
    root = directory.resolve(strict=True)
    manifest = json.loads((root / "manifest.json").read_bytes())
    runtime = json.loads(profile.read_bytes())
    if runtime["platform"] != "macos-arm64" or runtime["cc3d_version"] not in ("4.6.0", "4.7.0"):
        raise InvalidRequest("Native runtime identity differs")
    binary = (prefix / runtime.get("python_binary", "bin/python3.12")).resolve(strict=True)
    if not binary.is_relative_to(prefix.resolve(strict=True)):
        raise InvalidRequest("Native Python runtime path escapes its prefix")
    if bytes_digest(binary.read_bytes()) != runtime["python_digest"]:
        raise Conflict("Native Python runtime changed")
    for relative, checksum in runtime.get("installed_cc3d_hashes", {}).items():
        if bytes_digest((prefix / relative).read_bytes()) != checksum:
            raise Conflict("Installed CC3D runtime file changed")
    if (root / "record.json").exists():
        raise Conflict("Run already has a record; preserve it and choose a new run")
    if (
        _file_inventory(root / "project") != manifest["execution_files"]
        or bytes_digest((root / "driver.py").read_bytes()) != manifest["driver_digest"]
    ):
        raise Conflict("Prepared source or driver changed")
    if (
        manifest.get("execution_guard_digest")
        and bytes_digest((root / "completion_guard.py").read_bytes())
        != manifest["execution_guard_digest"]
    ):
        raise Conflict("Native completion guard changed")
    (root / "runtime-profile.json").write_text(canonical_json(runtime) + "\n")
    command = native_command(root, prefix, wall_seconds)
    environment = {
        "PATH": str(prefix.resolve() / "bin") + ":/usr/bin:/bin",
        "HOME": str(root / "tmp"),
        "TMPDIR": str(root / "tmp"),
        "QT_QPA_PLATFORM": "offscreen",
        "MPLBACKEND": "Agg",
        "MPLCONFIGDIR": str(root / "tmp/matplotlib"),
        "PYTHONDONTWRITEBYTECODE": "1",
        "PYTHONUNBUFFERED": "1",
        "OMP_NUM_THREADS": "1",
        "OPENBLAS_NUM_THREADS": "1",
        "MKL_NUM_THREADS": "1",
    }
    log = root / "engine.log"
    started = time.monotonic()
    record: dict[str, Any] = {
        "id": manifest["id"],
        "manifest_digest": digest(manifest),
        "runtime_kind": "reviewed_macos_native",
        "runtime_digest": digest(runtime),
        "status": "running",
        "wall_seconds_limit": wall_seconds,
        "network": "denied_by_seatbelt",
        "writes": "private_job_only",
        "outputs": {},
        "command": command,
        "limitations": [
            "Trusted reviewed scientific runtime; no hostile-code or clinical validity claim.",
            "No hard native address-space memory bound is claimed.",
        ],
    }
    process = None
    awake = None
    try:
        with log.open("xb") as output:
            process = subprocess.Popen(
                command,
                cwd=root / "project",
                stdout=output,
                stderr=subprocess.STDOUT,
                env=environment,
                start_new_session=True,
            )
            record["pid"] = process.pid
            write_operator_json(root / "record.json", record)
            awake = subprocess.Popen(
                ("/usr/bin/caffeinate", "-i", "-w", str(process.pid)),
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            while process.poll() is None:
                if time.monotonic() - started > wall_seconds:
                    raise TimeoutError("Native vCornea deadline exceeded")
                if log.stat().st_size > MAX_LOG_BYTES:
                    raise InvalidRequest("Native log exceeds32 MiB")
                if sum(p.stat().st_size for p in root.rglob("*") if p.is_file()) > MAX_WORK_BYTES:
                    raise InvalidRequest("Native workspace exceeds256 MiB")
                time.sleep(1)
        record["returncode"] = process.returncode
        if process.returncode:
            record.update(failed_native_details(root, manifest, runtime, process.returncode))
            raise InvalidRequest("Native vCornea failed; inspect retained engine.log")
        outputs = {
            p.relative_to(root).as_posix(): bytes_digest(p.read_bytes())
            for p in sorted((root / "project").rglob("*"))
            if p.is_file() and p.suffix in (".csv", ".parquet")
        }
        if not any(Path(p).name.startswith("cell_count_") for p in outputs):
            raise InvalidRequest("Native run ended without count output")
        for observation in sorted(root.glob("cells-*.csv")):
            outputs[observation.name] = bytes_digest(observation.read_bytes())
        for observation in sorted(root.glob("lattice-*.npz")):
            outputs[observation.name] = bytes_digest(observation.read_bytes())
        if (root / "progress.jsonl").exists():
            outputs["progress.jsonl"] = bytes_digest((root / "progress.jsonl").read_bytes())
        record.update(status="succeeded", outputs=outputs)
        if manifest.get("execution_guard_digest"):
            proof = json.loads((root / "completion.json").read_bytes())
            if (
                proof["returncode"] != 0
                or proof["manifest_digest"] != digest(manifest)
                or proof["runtime_digest"] != digest(runtime)
                or proof["outputs"] != outputs
            ):
                raise Conflict("Durable native completion proof disagrees")
            record["completion_digest"] = bytes_digest((root / "completion.json").read_bytes())
    except BaseException as error:
        record.update(
            status="cancelled" if isinstance(error, KeyboardInterrupt) else "failed",
            failure=type(error).__name__,
        )
        raise
    finally:
        if process is not None and process.poll() is None:
            with suppress(ProcessLookupError):
                os.killpg(process.pid, signal.SIGTERM)
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                with suppress(ProcessLookupError):
                    os.killpg(process.pid, signal.SIGKILL)
                process.wait(timeout=10)
        if awake is not None and awake.poll() is None:
            awake.terminate()
            awake.wait(timeout=10)
        record["elapsed_seconds"] = time.monotonic() - started
        write_operator_json(root / "record.json", record)
    return record
