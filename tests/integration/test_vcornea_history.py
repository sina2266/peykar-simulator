import ast
import json
from pathlib import Path
from xml.etree import ElementTree

import pytest

from peykar.integrations.vcornea_history import (
    BATCH,
    HISTORICAL_COMMIT,
    HISTORICAL_TREE,
    historical_controls,
    literal_control,
)
from peykar.integrations.vcornea_publication import prepare_publication
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, digest

SOURCE = Path("vendor/models/vcornea-history-b6123adf")
PINS = Path("engine_adapters/compucell3d/vcornea-history-b6123adf.json")


def test_historical_project_uses_original_hpc_physics_without_running_launcher(tmp_path):
    destination = tmp_path / "historical"
    original = {
        p.relative_to(SOURCE).as_posix(): bytes_digest(p.read_bytes())
        for p in SOURCE.rglob("*")
        if p.is_file()
    }
    assert digest(original) == HISTORICAL_TREE
    manifest = prepare_publication(
        SOURCE,
        PINS,
        destination,
        "self-organization",
        11,
        parameter_profile="historical-hpc-3.0.0",
    )
    assert not (destination / "record.json").exists()
    assert manifest["source_git_commit"] == HISTORICAL_COMMIT
    assert manifest["source_files"] == original
    assert manifest["controls"] == {"steps_mcs": 43200, "seed": 11}
    controls = manifest["historical_batch_controls"]
    assert controls == historical_controls((SOURCE / BATCH).read_bytes())
    assert len(controls) == 95
    assert controls["EGF_SUPERDiffCoef"] == 10  # Not the later archive's twelve.
    assert controls["IsInjury"] is False
    tree = ElementTree.parse(destination / "project/Simulation/vCornea_v2.xml")
    assert float(tree.findtext("./Potts/Temperature")) == 10
    assert tree.findtext("./Potts/Steps") == "43202"
    assert tree.findtext("./Potts/RandomSeed") == "11"
    assert manifest["compatibility_overlay"]["version"] == "2.0.0"
    assert (
        b"self.bins_number = 10"
        in (destination / "project/Simulation/vCornea_v2Steppables.py").read_bytes()
    )
    assert all(bytes_digest((SOURCE / p).read_bytes()) == sha for p, sha in original.items())
    with pytest.raises(Conflict, match="Preserve"):
        prepare_publication(
            SOURCE,
            PINS,
            destination,
            "self-organization",
            7,
            parameter_profile="historical-hpc-3.0.0",
        )


def test_historical_identity_cannot_be_substituted_by_a_changed_pin(tmp_path):
    pins = json.loads(PINS.read_bytes())
    pins["git_commit"] = "0" * 40
    path = tmp_path / "pins.json"
    path.write_text(json.dumps(pins))
    with pytest.raises(Conflict, match="reviewed historical"):
        prepare_publication(
            SOURCE,
            path,
            tmp_path / "run",
            "self-organization",
            7,
            parameter_profile="historical-hpc-3.0.0",
        )
    assert not (tmp_path / "run").exists()


@pytest.mark.parametrize("expression", ["__import__('os')", "1/0", "name", "[1]", "'text'"])
def test_batch_control_reader_never_evaluates_code(expression):
    with pytest.raises(InvalidRequest, match="numeric literal"):
        literal_control(ast.parse(expression, mode="eval").body)


def test_batch_controls_reject_ambiguous_sweeps_and_duplicate_keys():
    for raw in [b"var_dict={'x':[1,2]}", b"var_dict={'x':[1], 'x':[2]}", b"var_dict=f()"]:
        with pytest.raises(InvalidRequest):
            historical_controls(raw)
    assert historical_controls(b"var_dict={'SimTime':[7200*6], 'IsInjury':[False]}") == {
        "SimTime": 43200,
        "IsInjury": False,
    }


def test_historical_injury_state_is_not_silently_mixed_with_later_source(tmp_path):
    with pytest.raises(InvalidRequest, match="separate source review"):
        prepare_publication(
            SOURCE,
            PINS,
            tmp_path / "injury",
            "mild-injury",
            7,
            parameter_profile="historical-hpc-3.0.0",
        )
    assert not (tmp_path / "injury").exists()
