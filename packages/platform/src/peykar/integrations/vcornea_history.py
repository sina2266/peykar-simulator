"""Separately versioned July 2024 HPC source recovered from the licensed archive.

Preparation reads a pinned AST, never imports or runs the upstream batch launcher.
Only an explicit offline operator may subsequently execute the private project.
"""

import ast
import json
import shutil
from pathlib import Path
from typing import Any, cast
from xml.etree import ElementTree

from peykar.integrations.vcornea_runtime import ARCHIVE_SHA256, _file_inventory
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, canonical_json, digest

HISTORICAL_COMMIT = "b6123adf992e5b48442c0ebedd34ba7cd3162ef2"
HISTORICAL_TREE = "124adb716da552fc110ca8cc9050aa08c80f69301780f56d45d95d30fd1c4c33"
BATCH = "Epithelium/HPC/Batch_Run_vCornea_Paper_version_STEM_ONLY_noVTK.py"
PROJECT = "Epithelium/HPC/Project/paper_version_STEM_ONLY"


def literal_control(node: ast.expr) -> int | float | bool:
    if isinstance(node, ast.Constant) and type(node.value) in (int, float, bool):
        return cast(int | float | bool, node.value)
    if isinstance(node, ast.BinOp) and isinstance(node.op, ast.Mult):
        return literal_control(node.left) * literal_control(node.right)
    raise InvalidRequest("Historical batch control is not a reviewed numeric literal")


def historical_controls(raw: bytes) -> dict[str, int | float | bool]:
    trees = [
        n.value
        for n in ast.parse(raw).body
        if isinstance(n, ast.Assign)
        and any(isinstance(t, ast.Name) and t.id == "var_dict" for t in n.targets)
    ]
    if len(trees) != 1 or not isinstance(trees[0], ast.Dict):
        raise InvalidRequest("Expected one historical HPC control dictionary")
    controls = {}
    for key, value in zip(trees[0].keys, trees[0].values, strict=True):
        if (
            not isinstance(key, ast.Constant)
            or not isinstance(key.value, str)
            or not key.value.isidentifier()
            or not isinstance(value, ast.List)
            or len(value.elts) != 1
            or key.value in controls
        ):
            raise InvalidRequest("Historical HPC controls must be unique singleton literals")
        controls[key.value] = literal_control(value.elts[0])
    return controls


def prepare_historical_publication(
    source: Path, pins: Path, destination: Path, recipe: str, seed: int
) -> dict[str, Any]:
    from peykar.integrations.vcornea_publication import repair_link_lifetime

    if recipe != "self-organization":
        raise InvalidRequest("Historical injury correspondence requires a separate source review")
    if type(seed) is not int or not 0 <= seed <= 2**32 - 1:
        raise InvalidRequest("Seed must be an unsigned 32-bit integer")
    source = source.resolve(strict=True)
    inventory = _file_inventory(source)
    evidence = json.loads(pins.read_bytes())
    if (
        digest(inventory) != HISTORICAL_TREE
        or evidence.get("git_commit") != HISTORICAL_COMMIT
        or evidence.get("original_archive_sha256") != ARCHIVE_SHA256
        or evidence.get("license") != "CC-BY-4.0"
        or evidence.get("files") != inventory
    ):
        raise Conflict("Source differs from the reviewed historical archive objects")
    if destination.exists():
        raise Conflict("Preserve existing historical execution evidence")
    controls = historical_controls((source / BATCH).read_bytes())
    if (
        controls["SimTime"] != 43200
        or controls["InitBASAL_TargetVolume"] != 40
        or controls["EGF_BASAL_HalfMaxValue"] != 8
        or controls["EGF_SUPERDiffCoef"] != 10
        or controls["IsInjury"] is not False
    ):
        raise Conflict("Reviewed historical formation controls differ")
    destination.mkdir(parents=True)
    project = destination / "project"
    shutil.copytree(source / PROJECT, project)
    parameters = f"rep_folder={str(project.resolve())!r}\n" + "".join(
        f"{key}={value!r}\n" for key, value in controls.items()
    )
    (project / "Simulation/Parameters.py").write_text(parameters)
    path = project / "Simulation/vCornea_v2.xml"
    tree = ElementTree.parse(path)
    potts = tree.getroot().find("Potts")
    if potts is None or float(potts.findtext("Temperature") or "nan") != 10:
        raise Conflict("Historical Potts temperature differs")
    steps = potts.find("Steps")
    if steps is None:
        raise Conflict("Historical horizon control missing")
    steps.text = "43202"  # Original writer executes at SimTime+1.
    ElementTree.SubElement(potts, "RandomSeed").text = str(seed)
    tree.write(path, encoding="utf-8", xml_declaration=True)
    manifest: dict[str, Any] = {
        "schema_version": "1.0.0",
        "id": destination.name,
        "archive_doi": "10.5281/zenodo.16884414",
        "archive_digest": ARCHIVE_SHA256,
        "source_git_commit": HISTORICAL_COMMIT,
        "source_tree_digest": digest(inventory),
        "source_files": inventory,
        "steppables_source_path": PROJECT + "/Simulation/vCornea_v2Steppables.py",
        "scenario": "stem-only",
        "controls": {"steps_mcs": 43200, "seed": seed},
        "seed_scope": "Explicit new seed: Python random, NumPy and Potts; original seeds unknown",
        "publication_recipe": recipe,
        "publication_recipe_version": "3.0.0",
        "parameter_profile": "historical-hpc-3.0.0",
        "parameter_overrides": {},
        "historical_batch_controls": controls,
        "hpc_preset_evidence": {"source": BATCH, "sha256": inventory[BATCH]},
        "scientific_evidence": "not_established_by_execution",
        "limitations": [
            "July 2024 source precedes the dated data directories; exact per-run identity unknown.",
            "Reconstructed engine environment and new seeds; no independent biological validation.",
        ],
    }
    (destination / "manifest.json").write_text(canonical_json(manifest) + "\n")
    manifest = repair_link_lifetime(destination, overlay_version="2.0.0")
    # Observational only: retain the original height formula, on the frozen ten-bin grid.
    # No mechanics, random draws, state updates or formation parameters are changed.
    path = project / "Simulation/vCornea_v2Steppables.py"
    raw = path.read_bytes()
    if raw.count(b"self.bins_number = 5") != 1:
        raise Conflict("Historical observation grid differs")
    path.write_bytes(raw.replace(b"self.bins_number = 5", b"self.bins_number = 10"))
    manifest["observation_overlay"] = {
        "version": "1.0.0",
        "change": "Five to ten spatial output bins; original hourly height formula retained",
        "before_digest": bytes_digest(raw),
        "after_digest": bytes_digest(path.read_bytes()),
        "acceptance": "Existing frozen protocol unchanged; no new fitting or relaxed tolerances",
    }
    guard = Path(__file__).with_name("vcornea_completion_guard.py").read_bytes()
    (destination / "completion_guard.py").write_bytes(guard)
    manifest.update(
        execution_guard_digest=bytes_digest(guard),
        execution_guard_version="1.0.0",
        execution_files=_file_inventory(project),
        changes="Private historical HPC copy; original batch controls, seed and observations",
    )
    manifest["execution_tree_digest"] = digest(manifest["execution_files"])
    (destination / "manifest.json").write_text(canonical_json(manifest) + "\n")
    return manifest
