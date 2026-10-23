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
INJURY_BATCH = "Epithelium/HPC/Batch_Run_vCornea_Paper_version.py"
INJURY_PROJECT = "Epithelium/HPC/Project/paper_version"
INJURY_STATE_SHA256 = "906902426fb315c670f41337a8e91aeef4d32bcea7280f0bf8af9a790509e6c4"
STATE_EVIDENCE_SHA256 = "08b485a996d7ee5a636fa950f6b64d8c1ba05f18f7d7bf3319e70573c95a2d05"


def literal_control(node: ast.expr) -> int | float | bool:
    if isinstance(node, ast.Constant) and type(node.value) in (int, float, bool):
        return cast(int | float | bool, node.value)
    if isinstance(node, ast.BinOp) and isinstance(node.op, ast.Mult):
        return literal_control(node.left) * literal_control(node.right)
    if isinstance(node, ast.BinOp) and isinstance(node.op, ast.Add):
        return literal_control(node.left) + literal_control(node.right)
    raise InvalidRequest("Historical batch control is not a reviewed numeric literal")


def historical_controls(raw: bytes, *, injury_sweep: bool = False) -> dict[str, int | float | bool]:
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
            or not value.elts
            or key.value in controls
        ):
            raise InvalidRequest("Historical HPC controls must be unique singleton literals")
        values = [literal_control(v) for v in value.elts]
        if len(values) != 1 and not (
            injury_sweep
            and key.value == "SLS_Concentration"
            and values == [1500.0, 2000.0, 2500.0, 3000.0]
        ):
            raise InvalidRequest("Only the reviewed historical chemical sweep is supported")
        controls[key.value] = values[0]
    return controls


def prepare_historical_publication(
    source: Path,
    pins: Path,
    destination: Path,
    recipe: str,
    seed: int,
    *,
    injury_state: Path | None = None,
) -> dict[str, Any]:
    from peykar.integrations.vcornea_publication import repair_link_lifetime

    injury = recipe != "self-organization"
    doses = {"slight-injury": 750.0, "mild-injury": 1500.0, "moderate-injury": 2500.0}
    if injury and (injury_state is None or recipe not in doses):
        raise InvalidRequest("Historical injury correspondence requires a separate source review")
    if not injury and injury_state is not None:
        raise InvalidRequest("Formation retains its own historical initial state")
    state = None if injury_state is None else injury_state.read_bytes()
    if state is not None and bytes_digest(state) != INJURY_STATE_SHA256:
        raise Conflict("Injury state differs from the separately evidenced archived PIFF")
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
    batch = INJURY_BATCH if injury else BATCH
    project_source = INJURY_PROJECT if injury else PROJECT
    controls = historical_controls((source / batch).read_bytes(), injury_sweep=injury)
    if (
        controls["SimTime"] != (46200 if injury else 43200)
        or controls["InitBASAL_TargetVolume"] != 40
        or controls["EGF_BASAL_HalfMaxValue"] != 8
        or controls["EGF_SUPERDiffCoef"] != 10
        or controls["IsInjury"] is not injury
    ):
        raise Conflict("Reviewed historical formation controls differ")
    destination.mkdir(parents=True)
    project = destination / "project"
    shutil.copytree(source / project_source, project)
    overrides = (
        {"SimTime": 50400, "InjuryTime": 7200, "SLS_Concentration": doses[recipe]} if injury else {}
    )
    executed_controls = {**controls, **overrides}
    parameters = f"rep_folder={str(project.resolve())!r}\n" + "".join(
        f"{key}={value!r}\n" for key, value in executed_controls.items()
    )
    (project / "Simulation/Parameters.py").write_text(parameters)
    if state is not None:
        (project / "Epithelium.piff").write_bytes(state)
    path = project / "Simulation/vCornea_v2.xml"
    tree = ElementTree.parse(path)
    potts = tree.getroot().find("Potts")
    if potts is None or float(potts.findtext("Temperature") or "nan") != 10:
        raise Conflict("Historical Potts temperature differs")
    steps = potts.find("Steps")
    if steps is None:
        raise Conflict("Historical horizon control missing")
    steps.text = str(executed_controls["SimTime"] + 2)  # Original writer at SimTime+1.
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
        "steppables_source_path": project_source + "/Simulation/vCornea_v2Steppables.py",
        "scenario": "injury" if injury else "stem-only",
        "controls": {"steps_mcs": executed_controls["SimTime"], "seed": seed},
        "seed_scope": "Explicit new seed: Python random, NumPy and Potts; original seeds unknown",
        "publication_recipe": recipe,
        "publication_recipe_version": "4.0.0" if injury else "3.0.0",
        "parameter_profile": "observed-injury-4.0.0" if injury else "historical-hpc-3.0.0",
        "parameter_overrides": overrides,
        "historical_batch_controls": controls,
        "hpc_preset_evidence": {"source": batch, "sha256": inventory[batch]},
        "scientific_evidence": "not_established_by_execution",
        "limitations": [
            "July 2024 source precedes the dated data directories; exact per-run identity unknown.",
            "Reconstructed engine environment and new seeds; no independent biological validation.",
        ],
    }
    if injury:
        manifest["state_reconstruction"] = {
            "version": "1.0.0",
            "sha256": INJURY_STATE_SHA256,
            "source": "Zenodo16884414 Local/Project/paper_version/Epithelium.piff",
            "scope": "Separately evidenced original injury state with historical HPC mechanisms",
            "not_exact_historical_tree_execution": True,
            "evidence": "experiments/vcornea/publication/source-state-correspondence-1.0.0.json",
            "evidence_sha256": STATE_EVIDENCE_SHA256,
            "limits": "Saved original MCS0 geometry differs after a sweep; original seeds unknown",
        }
        manifest["limitations"].append(
            "Derived reconstruction: observed injury PIFF, frozen7200/50400MCS and measured dose; "
            "not a claim that the historical Git tree alone generated the selected cohort."
        )
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
