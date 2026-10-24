"""Keep the original family's different mechanisms, controls and source identity."""

import ast
import json
from pathlib import Path
from xml.etree import ElementTree

import pytest

from peykar.integrations.vcornea_history import BATCH, INJURY_BATCH, INJURY_PROJECT, PROJECT
from peykar.integrations.vcornea_publication import prepare_publication
from peykar.integrations.vcornea_runtime import _file_inventory
from peykar.integrations.vcornea_stash import STASH_COMMIT, STASH_INJURY_STATE, STASH_TREE
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, digest

SOURCE = Path("vendor/models/vcornea-history-c7f95d28")
PINS = Path("engine_adapters/compucell3d/vcornea-history-c7f95d28.json")


@pytest.mark.parametrize(
    "recipe,dose",
    [
        ("self-organization", None),
        ("slight-injury", 750),
        ("mild-injury", 1500),
        ("moderate-injury", 2500),
    ],
)
def test_snapshot_keeps_family_specific_source_and_control_semantics(tmp_path, recipe, dose):
    root = tmp_path / "run"
    before = _file_inventory(SOURCE)
    manifest = prepare_publication(
        SOURCE, PINS, root, recipe, 11, parameter_profile="stash-hpc-5.0.0"
    )
    injury = dose is not None
    family = INJURY_PROJECT if injury else PROJECT
    assert digest(before) == STASH_TREE
    assert manifest["source_git_commit"] == STASH_COMMIT
    assert manifest["source_files"] == before == _file_inventory(SOURCE)
    assert manifest["publication_recipe_version"] == "5.0.0"
    assert manifest["hpc_preset_evidence"]["source"] == (INJURY_BATCH if injury else BATCH)
    controls = manifest["historical_batch_controls"]
    assert controls["IsInjury"] is False  # The supplied injury batch did not activate exposure.
    assert controls["InitBASAL_TargetVolume"] == (25 if injury else 40)
    assert controls["EGF_BASAL_HalfMaxValue"] == (7 if injury else 8)
    assert controls["EGF_SUPERDiffCoef"] == (20 if injury else 12)
    assert manifest["parameter_overrides"] == (
        {"IsInjury": True, "SimTime": 50400, "InjuryTime": 7200, "SLS_Concentration": dose}
        if injury
        else {}
    )
    tree = ElementTree.parse(root / "project/Simulation/vCornea_v2.xml")
    assert tree.findtext("./Potts/Temperature") == "8.0"
    assert tree.findtext("./Potts/Steps") == ("50402" if injury else "43202")
    assert tree.findtext("./Potts/RandomSeed") == "11"
    assert int(tree.findtext("./Metadata/NumberOfProcessors")) == (1 if injury else 4)
    assert "observation_overlay" not in manifest
    assert manifest["compatibility_overlay"]["version"] == "2.0.0"
    pristine = (SOURCE / family / "Simulation/vCornea_v2Steppables.py").read_bytes()
    assert b"self.bins_number = 10" in pristine
    # Initial state is copied from this tree, never replaced by another source version.
    state = manifest["initial_state"]
    assert state["external_replacement"] is False
    assert (SOURCE / state["source"]).read_bytes() == (
        root / "project" / Path(state["source"]).name
    ).read_bytes()
    assert bytes_digest((SOURCE / state["source"]).read_bytes()) == state["sha256"]
    if injury:
        assert state["sha256"] == STASH_INJURY_STATE
        # IsInjury=True enables the injury; InjuryType=False selects chemical, not abrasion.
        assert controls["InjuryType"] is False
        assert b"if self.injury:" in pristine and b"if self.injuryType == 1:" in pristine
    assert not (root / "record.json").exists()


def test_snapshot_rejects_substituted_source_pins_and_external_state(tmp_path):
    pins = json.loads(PINS.read_bytes())
    pins["git_commit"] = "0" * 40
    bad = tmp_path / "pins.json"
    bad.write_text(json.dumps(pins))
    with pytest.raises(Conflict, match="reviewed January"):
        prepare_publication(
            SOURCE, bad, tmp_path / "bad", "mild-injury", 7, parameter_profile="stash-hpc-5.0.0"
        )
    with pytest.raises(InvalidRequest, match="own original initial state"):
        prepare_publication(
            SOURCE,
            PINS,
            tmp_path / "external",
            "mild-injury",
            7,
            parameter_profile="stash-hpc-5.0.0",
            injury_state=tmp_path / "other.piff",
        )
    assert not (tmp_path / "bad").exists() and not (tmp_path / "external").exists()


def test_two_family_mechanisms_are_not_silently_unified():
    def method(family, classname, name):
        tree = ast.parse((SOURCE / family / "Simulation/vCornea_v2Steppables.py").read_bytes())
        cls = next(n for n in tree.body if isinstance(n, ast.ClassDef) and n.name == classname)
        return ast.unparse(
            next(n for n in cls.body if isinstance(n, ast.FunctionDef) and n.name == name)
        )

    formation = method(PROJECT, "GrowthSteppable", "step")
    injury = method(INJURY_PROJECT, "GrowthSteppable", "step")
    assert "cell.dict['Initial_Volume']" not in formation
    assert "cell.dict['Initial_Volume']" in injury
    assert formation != injury
    formation_death = method(PROJECT, "DeathSteppable", "step")
    injury_death = method(INJURY_PROJECT, "DeathSteppable", "step")
    assert "self.SLS_threshold / cell.dict['Initial_Volume']" not in formation_death
    assert "self.SLS_threshold / cell.dict['Initial_Volume']" in injury_death
    assert "random.random()" in formation_death and "random.random()" in injury_death


def test_original_injury_state_differs_from_later_archive_only_in_line_endings():
    archived = Path("vendor/models/vcornea-16884414/Local/Project/paper_version/Epithelium.piff")
    historical = SOURCE / INJURY_PROJECT / "Epithelium.piff"
    assert archived.read_bytes() != historical.read_bytes()
    assert archived.read_bytes().replace(b"\r\n", b"\n") == historical.read_bytes()


def test_serial_formation_preserves_original_biology_and_keeps_old_profile_immutable(tmp_path):
    a, b = tmp_path / "old", tmp_path / "serial"
    old = prepare_publication(
        SOURCE, PINS, a, "self-organization", 7, parameter_profile="stash-hpc-5.0.0"
    )
    new = prepare_publication(
        SOURCE,
        PINS,
        b,
        "self-organization",
        7,
        parameter_profile="stash-formation-serial-7.0.0",
    )
    for key in [
        "historical_batch_controls",
        "controls",
        "source_files",
        "initial_state",
        "parameter_overrides",
    ]:
        assert old[key] == new[key]
    assert old["publication_recipe_version"] == "5.0.0"
    assert new["publication_recipe_version"] == "7.0.0"
    assert old["native_workers_requested"] == 4 and new["native_workers_requested"] == 1
    left, right = (ElementTree.parse(root / "project/Simulation/vCornea_v2.xml") for root in [a, b])
    workers = right.find("./Metadata/NumberOfProcessors")
    assert workers.text == "1"
    workers.text = "4"
    assert ElementTree.tostring(left.getroot()) == ElementTree.tostring(right.getroot())
    for name in ["vCornea_v2Steppables.py", "vCornea_v2.py"]:
        assert (a / "project/Simulation" / name).read_bytes() == (
            b / "project/Simulation" / name
        ).read_bytes()
    assert (a / "project/Epithelium_stem_only.piff").read_bytes() == (
        b / "project/Epithelium_stem_only.piff"
    ).read_bytes()
    assert new["execution_control_overlay"]["source_workers"] == 4
    with pytest.raises(InvalidRequest, match="formation-only"):
        prepare_publication(
            SOURCE,
            PINS,
            tmp_path / "rejected",
            "mild-injury",
            7,
            parameter_profile="stash-formation-serial-7.0.0",
        )
    assert not (tmp_path / "rejected").exists()
