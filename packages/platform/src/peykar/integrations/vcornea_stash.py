"""Reviewed January archive snapshot, keeping its two distinct HPC mechanisms.

This is a separately identified reconstruction, not a replacement of earlier runs.
Preparation reads literal controls and copies source; it never executes the model.
"""

import json
import shutil
from pathlib import Path
from typing import Any
from xml.etree import ElementTree

from peykar.integrations.vcornea_history import (
    BATCH,
    INJURY_BATCH,
    INJURY_PROJECT,
    PROJECT,
    historical_controls,
)
from peykar.integrations.vcornea_runtime import ARCHIVE_SHA256, _file_inventory
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, canonical_json, digest

STASH_COMMIT = "c7f95d28dfa2329abb7353caab724c52f9341dee"
STASH_TREE = "cf708f1341da82cf4cc7de42173a38c3b642b95fcfffa45bf80eafddc5305e06"
STASH_INJURY_STATE = "a4da5c3c786581165ff0cf642ea017d242d7bb4d50fe3b573cd7a97627953edf"


def prepare_stash_publication(
    source: Path, pins: Path, destination: Path, recipe: str, seed: int
) -> dict[str, Any]:
    from peykar.integrations.vcornea_publication import RECIPES, repair_link_lifetime

    if recipe not in RECIPES:
        raise InvalidRequest("Unknown reviewed publication recipe")
    if type(seed) is not int or not 0 <= seed <= 2**32 - 1:
        raise InvalidRequest("Seed must be an unsigned 32-bit integer")
    source = source.resolve(strict=True)
    inventory = _file_inventory(source)
    evidence = json.loads(pins.read_bytes())
    if (
        digest(inventory) != STASH_TREE
        or evidence.get("git_commit") != STASH_COMMIT
        or evidence.get("original_archive_sha256") != ARCHIVE_SHA256
        or evidence.get("license") != "CC-BY-4.0"
        or evidence.get("files") != inventory
    ):
        raise Conflict("Source differs from the reviewed January archive snapshot")
    if destination.exists():
        raise Conflict("Preserve existing January snapshot execution evidence")
    injury = recipe != "self-organization"
    batch, project_source = (INJURY_BATCH, INJURY_PROJECT) if injury else (BATCH, PROJECT)
    controls = historical_controls((source / batch).read_bytes())
    if (
        controls["SimTime"] != (46200 if injury else 43200)
        or controls["InitBASAL_TargetVolume"] != (25 if injury else 40)
        or controls["EGF_BASAL_HalfMaxValue"] != (7 if injury else 8)
        or controls["EGF_SUPERDiffCoef"] != (20 if injury else 12)
        or controls["IsInjury"] is not False
    ):
        raise Conflict("Reviewed January HPC controls differ")
    original_project = source / project_source
    tree = ElementTree.parse(original_project / "Simulation/vCornea_v2.xml")
    potts = tree.getroot().find("Potts")
    if (
        potts is None
        or float(potts.findtext("Temperature") or "nan") != 8
        or tree.findtext("./Metadata/NumberOfProcessors") != ("1" if injury else "4")
    ):
        raise Conflict("Reviewed January HPC Potts controls differ")
    steps = potts.find("Steps")
    if steps is None or potts.find("RandomSeed") is not None:
        raise Conflict("Reviewed January horizon/seed controls differ")
    doses = {"slight-injury": 750.0, "mild-injury": 1500.0, "moderate-injury": 2500.0}
    overrides: dict[str, int | float | bool] = (
        {
            "IsInjury": True,
            "InjuryTime": 7200,
            "SimTime": 50400,
            "SLS_Concentration": doses[recipe],
        }
        if injury
        else {}
    )
    executed = {**controls, **overrides}
    destination.mkdir(parents=True)
    project = destination / "project"
    shutil.copytree(original_project, project)
    parameters = f"rep_folder={str(project.resolve())!r}\n" + "".join(
        f"{key}={value!r}\n" for key, value in executed.items()
    )
    (project / "Simulation/Parameters.py").write_text(parameters)
    steps.text = str(executed["SimTime"] + 2)  # Retain the original SimTime+1 writer.
    ElementTree.SubElement(potts, "RandomSeed").text = str(seed)
    tree.write(project / "Simulation/vCornea_v2.xml", encoding="utf-8", xml_declaration=True)
    manifest: dict[str, Any] = {
        "schema_version": "1.0.0",
        "id": destination.name,
        "archive_doi": "10.5281/zenodo.16884414",
        "archive_digest": ARCHIVE_SHA256,
        "source_git_commit": STASH_COMMIT,
        "source_tree_digest": digest(inventory),
        "source_files": inventory,
        "steppables_source_path": project_source + "/Simulation/vCornea_v2Steppables.py",
        "scenario": "injury" if injury else "stem-only",
        "controls": {"steps_mcs": executed["SimTime"], "seed": seed},
        "seed_scope": "Explicit new seed: Python random, NumPy and Potts; original seeds unknown",
        "publication_recipe": recipe,
        "publication_recipe_version": "5.0.0",
        "parameter_profile": "stash-hpc-5.0.0",
        "parameter_overrides": overrides,
        "historical_batch_controls": controls,
        "hpc_preset_evidence": {"source": batch, "sha256": inventory[batch]},
        "native_workers_requested": 1 if injury else 4,
        "initial_state": {
            "source": project_source
            + ("/Epithelium.piff" if injury else "/Epithelium_stem_only.piff"),
            "external_replacement": False,
        },
        "scientific_evidence": "not_established_by_execution",
        "limitations": [
            "January 2025 stash postdates July 2024 data; exact cohort source identity unknown.",
            "Formation and injury retain distinct mechanisms/defaults from one complete tree.",
            "Injury activation, pulse, dose and horizon are explicit reconstruction overrides.",
            "Original per-run parameters/seeds/full environment remain unknown.",
            "Execution is not independent biological validation or a passing comparison.",
        ],
    }
    initial = manifest["initial_state"]
    initial["sha256"] = inventory[initial["source"]]
    if injury and initial["sha256"] != STASH_INJURY_STATE:
        raise Conflict("January injury state differs")
    (destination / "manifest.json").write_text(canonical_json(manifest) + "\n")
    manifest = repair_link_lifetime(destination, overlay_version="2.0.0")
    # Both projects already observe ten bins: no observation-grid overlay is applied.
    guard = Path(__file__).with_name("vcornea_completion_guard.py").read_bytes()
    (destination / "completion_guard.py").write_bytes(guard)
    manifest.update(
        execution_guard_digest=bytes_digest(guard),
        execution_guard_version="1.0.0",
        changes="Private January HPC copy; original family mechanisms and declared controls",
    )
    (destination / "manifest.json").write_text(canonical_json(manifest) + "\n")
    return manifest
