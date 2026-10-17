"""Explicit offline operator probes of locally installed scientific runtimes.

Never pulls an image or executes code on import. Each invocation freezes the
local image ID, creates a fresh private job, executes a bounded container and
retains actual outputs, failures and checksums. No Docker access is needed in
the API/worker. See runtime_profiles/README.md for scope and installation.
"""

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import uuid
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def digest(path: Path) -> str:
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def prepare(kind: str, output: Path, root: Path = ROOT) -> list[str]:
    if kind == "gears-data":
        return [
            "/opt/runtime/inspect_dataset.py",
            "--input",
            "/input/data.h5ad",
            "--output",
            "/work",
        ]
    if kind == "osp":
        return []
    if kind == "receipt":
        return ["--output", "/work"]
    if kind in ("physicell", "physiboss"):
        sample = (
            "sample_projects/template"
            if kind == "physicell"
            else "sample_projects_intracellular/boolean/physiboss_cell_lines"
        )
        source = root / "vendor/upstream/physicell" / sample / "config"
        shutil.copytree(source, output / "config")
        config = output / "config/PhysiCell_settings.xml"
        tree = ET.parse(config)
        changes = {
            "./overall/max_time": "1" if kind == "physicell" else "10",
            "./parallel/omp_num_threads": "2",
            "./save/full_data/interval": "1",
            "./save/SVG/interval": "1",
            "./save/folder": "/work/output",
        }
        for path, value in changes.items():
            element = tree.find(path)
            if element is None:
                raise ValueError(f"Original PhysiCell configuration changed: {path}")
            element.text = value
        tree.write(config)
        return ["/work/config/PhysiCell_settings.xml"]
    if kind == "chaste":
        return ["/home/chaste/build/ode/test/TestSolvingOdesTutorial"]
    if kind == "chaste-delta-notch":
        return ["/home/chaste/build/cell_based/test/TestRunningDeltaNotchSimulationsTutorial"]
    if kind == "vcell":
        source = root / "vendor/upstream/vcell/vcell-cli/src/test/resources/BioModel1.omex"
        shutil.copy2(source, output / "model.omex")
        return ["biosimulations", "-i", "/work/model.omex", "-o", "/work/output"]
    raise ValueError(f"Unsupported native profile kind: {kind}")


def verify(kind: str, engine: str, output: Path) -> dict[str, object]:
    if kind == "osp":
        receipt = json.loads((output / "osp-receipt.json").read_bytes())
        if receipt.get("engine") != engine or receipt.get("native_execution") != "passed":
            raise ValueError("OSP solver did not pass the PKML probe")
        if not (output / "native.csv").is_file() or receipt.get("samples", 0) < 2:
            raise ValueError("OSP numerical output is absent")
        return receipt
    if kind in ("receipt", "gears-data"):
        receipt = json.loads((output / "receipt.json").read_bytes())
        if receipt.get("native_execution") != "passed" or receipt.get("engine") != engine:
            raise ValueError("Native runtime did not report the expected engine/probe")
        artifacts = receipt.get("artifacts")
        if not isinstance(artifacts, dict) or not artifacts:
            raise ValueError("Native runtime did not produce evidence artifacts")
        for name, expected in artifacts.items():
            path = output / name
            if path.parent != output or path.is_symlink() or digest(path) != expected:
                raise ValueError("Native artifact checksum/path mismatch")
        return receipt
    if kind in ("physicell", "physiboss"):
        final = output / "output/final.xml"
        tree = ET.parse(final)
        if tree.getroot().tag != "MultiCellDS":
            raise ValueError("PhysiCell did not produce MultiCellDS output")
        if not (output / "output/final_cells.mat").is_file():
            raise ValueError("PhysiCell native cell state is missing")
    elif kind.startswith("chaste"):
        log = (output / "engine.log").read_text(errors="replace")
        if "OK!" not in log or not list((output / "output").rglob("*.dat")):
            raise ValueError("Chaste tutorial did not pass or retain its numerical output")
    elif kind == "vcell":
        if not list((output / "output").rglob("*.h5")):
            raise ValueError("VCell produced no numerical HDF5 output")
        # An OMEX may partially fail while still producing some datasets.
        statuses = list((output / "output").rglob("log.yml"))
        if not statuses or any(
            not re.search(r"^status: [\"\']?SUCCEEDED[\"\']?\s*$", p.read_text(), re.MULTILINE)
            or re.search(r"status: [\"\']?(?:FAILED|ABORTED|RUNNING|QUEUED)", p.read_text())
            for p in statuses
        ):
            raise ValueError("VCell archive execution failed or has no status record")
    else:
        raise ValueError("Unsupported verification kind")
    return {
        "engine": engine,
        "native_execution": "passed",
        "scientific_validity": "not_established",
    }


def run_probe(
    profile_id: str,
    output: Path,
    catalog: Path = ROOT / "runtime_profiles/catalog.json",
    input_path: Path | None = None,
) -> dict[str, object]:
    profiles = json.loads(catalog.read_bytes())["profiles"]
    profile = profiles[profile_id]
    if (profile["kind"] == "gears-data") != (input_path is not None):
        raise ValueError("Only gears-data requires an explicit --input H5AD file")
    if input_path is not None:
        input_path = input_path.resolve(strict=True)
        if not input_path.is_file():
            raise ValueError("Dataset input must be a regular file")
    image = subprocess.check_output(
        ["docker", "image", "inspect", profile["image"], "--format", "{{.Id}}"], text=True
    ).strip()
    if len(image) != 71 or not image.startswith("sha256:"):
        raise ValueError("Docker did not resolve a local immutable image ID")
    output = output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    name = "peykar-probe-" + uuid.uuid4().hex
    receipt: dict[str, object] = {
        "format": "peykar.native-execution/1",
        "profile": profile_id,
        "image_id": image,
        "image_reference": profile["image"],
        "platform": profile["platform"],
        "status": "failed",
        "scientific_validity": "not_established",
    }
    try:
        arguments = prepare(profile["kind"], output)
        command = [
            "docker",
            "run",
            "--rm",
            "--pull",
            "never",
            "--name",
            name,
            "--platform",
            profile["platform"],
            "--network",
            "none",
            "--hostname",
            "peykar-native",
            "--add-host",
            "peykar-native:127.0.0.1",
            "--read-only",
            "--cap-drop",
            "ALL",
            "--security-opt",
            "no-new-privileges:true",
            "--cpus",
            "2",
            "--memory",
            "3g",
            "--pids-limit",
            "256",
            "--user",
            f"{os.getuid()}:{os.getgid()}",
            "--tmpfs",
            "/tmp:size=512m,mode=1777",
            "--env",
            "HOME=/tmp",
            "--env",
            "CHASTE_TEST_OUTPUT=/work/output",
            "--env",
            "MAX_JAVA_MEM_MB=2048",
            "--mount",
            f"type=bind,src={output},dst=/work",
        ]
        if profile["kind"].startswith("chaste"):
            command += ["--workdir", "/home/chaste/src", "--entrypoint", "/usr/bin/env"]
        if input_path is not None:
            receipt["input_sha256"] = digest(input_path)
            command += [
                "--mount",
                f"type=bind,src={input_path},dst=/input/data.h5ad,readonly",
                "--entrypoint",
                "/opt/peykar/runtime_profiles/gears/.venv/bin/python",
            ]
        command += [image, *arguments]
        receipt["command"] = command
        with (output / "engine.log").open("wb") as log:
            process = subprocess.run(
                command,
                stdout=log,
                stderr=subprocess.STDOUT,
                timeout=profile["timeout_seconds"],
                check=False,
            )
        receipt["exit_code"] = process.returncode
        if process.returncode != 0:
            raise RuntimeError(
                f"Native engine exited {process.returncode}; see {output / 'engine.log'}"
            )
        receipt["native_evidence"] = verify(profile["kind"], profile["engine"], output)
        if input_path is not None and (
            receipt["native_evidence"].get("input_sha256") != receipt["input_sha256"]
            or digest(input_path) != receipt["input_sha256"]
        ):
            raise ValueError("Dataset input changed or decoder provenance differs")
        receipt["status"] = "passed"
        return receipt
    except Exception as error:
        receipt["error"] = f"{type(error).__name__}: {error}"
        raise
    finally:
        try:
            subprocess.run(
                ["docker", "rm", "-f", name], capture_output=True, check=False, timeout=30
            )
        except (OSError, subprocess.TimeoutExpired) as error:
            receipt["cleanup_error"] = str(error)
        receipt["artifacts"] = {
            str(p.relative_to(output)): digest(p)
            for p in sorted(output.rglob("*"))
            if p.is_file() and not p.is_symlink() and p.name != "execution.json"
        }
        (output / "execution.json").write_text(json.dumps(receipt, indent=2) + "\n")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("profile")
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--input", type=Path, help="Explicit read-only H5AD input for gears-data")
    args = parser.parse_args()
    receipt = run_probe(args.profile, args.output, input_path=args.input)
    print(
        json.dumps(
            {"profile": args.profile, "status": receipt["status"], "image_id": receipt["image_id"]}
        )
    )


if __name__ == "__main__":
    main()
