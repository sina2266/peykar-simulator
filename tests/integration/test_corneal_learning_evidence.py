"""Read-only evidence replay: no new experimental fit or solver execution."""

import importlib.util
import json
import shutil
from pathlib import Path

import pytest

from peykar.validation.cornea_evidence import rabbit_wound_observations

BASE = Path("experiments/vcornea/repair-front/learned-front")


def operator(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_actual_source_split_uses_entire_animals_and_keeps_every_observation():
    raw = Path("experiments/data/cornea-evidence-20261024/raw/rabbit-figure2.xlsx").read_bytes()
    original = rabbit_wound_observations(raw)
    for name, groups, n in [
        ("training", {"Control", "2% SDP"}, 60),
        ("heldout", {"0.5% SDP"}, 30),
    ]:
        rows = json.loads((BASE / f"data/{name}.json").read_bytes())
        assert rows == [r for r in original if r["group"] in groups]
        assert len(rows) == n


def test_frozen_curve_constraints_are_reproducible_and_not_raw_animal_records():
    module = operator("curve_constraints", "scripts/extract_corneal_curve_constraints.py")
    actual = module.extract()
    retained = json.loads((BASE / "fischak2017/constraints.json").read_bytes())
    assert actual == retained
    assert [r["hour"] for r in actual["observations"]] == [12, 24, 36, 48, 72]
    assert all(not r["recovered_individual_animal_records"] for r in actual["observations"])
    assert 0.09 < actual["observations"][-1]["area_ratio_lower_bound"] < 0.10


def retained_copy(tmp_path):
    source = BASE / "observed-2026-10-24"
    for p in source.rglob("*.json"):
        if p.name == "report.json":
            continue
        target = tmp_path / p.relative_to(source)
        target.parent.mkdir(exist_ok=True, parents=True)
        shutil.copyfile(p, target)
    return tmp_path


def test_real_execution_replay_preserves_all_negative_outcomes(tmp_path):
    module = operator("learned_evidence", "scripts/learn_corneal_front.py")
    output = retained_copy(tmp_path)
    report = module.analyze(output)
    assert report["heldout2017"]["passed"] is False
    assert report["heldout2017"]["primary_metrics"]["rmse_fraction"] == pytest.approx(
        0.05958317282881904
    )
    assert report["heldout2017"]["rmse_improvement_fraction"] > 0.04
    assert len(report["heldout2017"]["individual_diagnostics"]) == 3
    assert report["challenge2021"]["passed"] is False
    assert len(report["challenge2021"]["categorical_observations"]) == 2
    assert sum(len(g["rows"]) for g in report["challenge2021"]["groups"]) == 10
    assert report["fischak2017"]["status"] == "rejected_by_conservative_constraints"
    assert report["fischak2017"]["optimistic_error_lower_bounds"]["rmse_fraction"] > 0.055
    assert report["independent_biological_validation"] is False


def test_failed_execution_or_changed_output_cannot_be_scientific_success(tmp_path):
    module = operator("failed_learned_evidence", "scripts/learn_corneal_front.py")
    output = retained_copy(tmp_path)
    path = output / "execution-record.json"
    record = json.loads(path.read_bytes())
    record["returncode"] = -9
    path.write_text(json.dumps(record))
    with pytest.raises(ValueError, match="actual successful"):
        module.analyze(output)
    record["returncode"] = 0
    path.write_text(json.dumps(record))
    candidate = output / "heldout2017/candidate.json"
    candidate.write_bytes(candidate.read_bytes() + b" ")
    with pytest.raises(ValueError, match="identity differs"):
        module.analyze(output)
