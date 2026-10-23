import ast
from pathlib import Path
from types import SimpleNamespace

import pytest

from peykar.integrations.vcornea_publication import (
    RECIPES,
    native_command,
    publication_overrides,
    repair_link_lifetime,
)
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, canonical_json

FIXTURE = Path("tests/fixtures/vcornea-wall-links.py.txt")


class DeletedLinkAccess(RuntimeError):
    pass


class Link:
    def __init__(self, wall):
        self.wall = wall
        self.deleted = False

    def getOtherCell(self, cell):
        if self.deleted:
            raise DeletedLinkAccess("read of deleted native link")
        return SimpleNamespace(type="wall" if self.wall else "super")


class Links:
    def __init__(self, count):
        self.dim = SimpleNamespace(x=200)
        self.cell_field = {(0, 10, 0): "closest-wall"}
        self.WALL = "wall"
        self.AutoAdjustLinks = True
        self.Lambda_link_adjustment = False
        self.links = [Link(wall=(i == 0)) for i in range(count)]
        self.updated = []

    def get_fpp_link_by_cells(self, cell, wall):
        return next((link for link in self.links if link.wall and not link.deleted), None)

    def get_fpp_links_by_cell(self, cell):
        return list(self.links)

    def delete_fpp_link(self, link):
        link.deleted = True
        self.links.remove(link)

    def link_target_distance_update(self, link):
        if link.deleted:
            raise DeletedLinkAccess("update of deleted native link")
        self.updated.append(link)

    link_lambda_update = link_target_distance_update


def function(raw):
    tree = ast.parse(raw)
    method = tree.body[0].body[0]
    namespace = {}
    exec(
        compile(ast.Module(body=[method], type_ignores=[]), "reviewed-source-method", "exec"),
        namespace,
    )
    return namespace["update_wall_links"]


@pytest.fixture
def prepared(tmp_path):
    root = tmp_path / "run"
    path = root / "project/Simulation/vCornea_v2Steppables.py"
    path.parent.mkdir(parents=True)
    raw = FIXTURE.read_bytes()
    path.write_bytes(raw)
    manifest = {
        "scenario": "injury",
        "source_files": {
            "Local/Project/paper_version/Simulation/vCornea_v2Steppables.py": bytes_digest(raw)
        },
        "controls": {"steps_mcs": 20, "seed": 7},
    }
    (root / "manifest.json").write_text(canonical_json(manifest) + "\n")
    return root, path, raw


@pytest.mark.parametrize("count", [1, 3, 4])
def test_actual_upstream_deleted_link_failure_is_repaired(prepared, count):
    root, path, raw = prepared
    cell = SimpleNamespace(xCOM=0, yCOM=10)
    with pytest.raises(DeletedLinkAccess):
        function(raw)(Links(count), cell)
    manifest = repair_link_lifetime(root)
    owner = Links(count)
    function(path.read_bytes())(owner, cell)
    assert not owner.updated
    assert manifest["compatibility_overlay"]["upstream_digest"] == bytes_digest(raw)
    assert manifest["execution_files"]["Simulation/vCornea_v2Steppables.py"] == bytes_digest(
        path.read_bytes()
    )
    assert FIXTURE.read_bytes() == raw
    with pytest.raises(Conflict):
        repair_link_lifetime(root)


@pytest.mark.parametrize("lambda_adjustment", [True, False])
def test_live_wall_link_retains_original_force_update(prepared, lambda_adjustment):
    root, path, raw = prepared
    cell = SimpleNamespace(xCOM=0, yCOM=10)
    before = Links(2)
    before.Lambda_link_adjustment = lambda_adjustment
    function(raw)(before, cell)
    assert len(before.updated) == 1
    repair_link_lifetime(root)
    after = Links(2)
    after.Lambda_link_adjustment = lambda_adjustment
    function(path.read_bytes())(after, cell)
    assert len(after.updated) == 1 and not after.updated[0].deleted


def test_overlay_rejects_tampering_and_executed_runs(prepared):
    root, path, raw = prepared
    path.write_bytes(raw + b"# changed\n")
    with pytest.raises(Conflict, match="pristine"):
        repair_link_lifetime(root)
    path.write_bytes(raw)
    (root / "record.json").write_text("{}")
    with pytest.raises(Conflict, match="executed"):
        repair_link_lifetime(root)
    assert path.read_bytes() == raw


def test_explicit_native_boundary_and_seed(prepared, tmp_path):
    root, _, _ = prepared
    command = native_command(root, tmp_path / "runtime", 60)
    assert command[0] == "/usr/bin/sandbox-exec"
    assert "(deny network*)" in command[2] and "(deny file-write*)" in command[2]
    assert command[command.index("--seed") + 1] == "7"
    assert command[command.index("--wall-seconds") + 1] == "60"
    for value in [0, float("nan"), float("inf"), 86401]:
        with pytest.raises(InvalidRequest):
            native_command(root, tmp_path, value)


def test_full_publication_horizons_and_chemical_severity_are_explicit():
    assert RECIPES["self-organization"] == ("stem-only", 43200, {})
    for recipe, concentration in [
        ("slight-injury", 750.0),
        ("mild-injury", 1500.0),
        ("moderate-injury", 2500.0),
    ]:
        assert RECIPES[recipe] == (
            "injury",
            50400,
            {"InjuryTime": 7200, "SLS_Concentration": concentration},
        )


def test_hpc_formation_parameters_are_separate_from_historical_local_recipe():
    assert publication_overrides("self-organization", "local-1.0.0") == {}
    assert publication_overrides("self-organization", "hpc-2.0.0") == {
        "InitBASAL_TargetVolume": 40.0,
        "EGF_BASAL_HalfMaxValue": 8.0,
        "EGF_SUPERDiffCoef": 12.0,
    }
    assert publication_overrides("slight-injury", "hpc-2.0.0") == {
        "InjuryTime": 7200,
        "SLS_Concentration": 750.0,
    }
    with pytest.raises(InvalidRequest):
        publication_overrides("self-organization", "imagined-defaults")


def test_actual_membrane_deletion_stops_invalid_native_pixel_iterator(prepared):
    root, path, _ = prepared
    death = Path("tests/fixtures/vcornea-death-step.py.txt").read_bytes()
    original = path.read_bytes() + b"\n" + death
    path.write_bytes(original)
    manifest = {
        "scenario": "injury",
        "source_files": {
            "Local/Project/paper_version/Simulation/vCornea_v2Steppables.py": bytes_digest(original)
        },
    }
    (root / "manifest.json").write_text(canonical_json(manifest))

    class MembraneOwner:
        def __init__(self):
            self.injury, self.injuryTime, self.injuryType = True, 7200, False
            self.SLS_Gaussian_pulse, self.SLS_Injury, self.DeathControl = True, True, False
            self.SLS_X_Center, self.SLS_Y_Center, self.SLS_Concentration = 100, 65, 2500
            self.SLS_initialField = {}
            self.MEMB, self.LIMB, self.SUPER, self.WING, self.BASAL, self.STEM = range(1, 7)
            self.cell = SimpleNamespace(type=self.MEMB, deleted=False)
            self.SLS = {(1, 1, 0): 1, (2, 1, 0): 1}
            self.deletions = 0

        def cell_list_by_type(self, *types):
            return [self.cell]

        def get_cell_pixel_list(self, cell):
            for x in (1, 2):
                if cell.deleted:
                    raise DeletedLinkAccess("advance of deleted membrane pixel iterator")
                yield SimpleNamespace(pixel=SimpleNamespace(x=x, y=1, z=0))

        def delete_cell(self, cell):
            if cell.deleted:
                raise DeletedLinkAccess("second deletion of native cell")
            cell.deleted = True
            self.deletions += 1

    def death_step(raw):
        tree = ast.parse(raw)
        method = tree.body[1].body[0]
        namespace = {}
        exec(compile(ast.Module(body=[method], type_ignores=[]), "death-step", "exec"), namespace)
        return namespace["step"]

    with pytest.raises(DeletedLinkAccess, match="pixel iterator"):
        death_step(original)(MembraneOwner(), 7200)
    result = repair_link_lifetime(root, overlay_version="2.0.0")
    owner = MembraneOwner()
    death_step(path.read_bytes())(owner, 7200)
    assert owner.deletions == 1
    assert result["compatibility_overlay"]["version"] == "2.0.0"
