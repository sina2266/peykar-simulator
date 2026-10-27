"""Replay retained actual evidence, never run a new experimental candidate."""

import importlib.util
import json
import shutil
from pathlib import Path

import pytest

BASE = Path("experiments/vcornea/repair-front/conditioned-forecast")


def operator():
    spec = importlib.util.spec_from_file_location(
        "conditioned_forecast", "scripts/check_corneal_conditioned_forecast.py"
    )
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def retained_copy(tmp_path):
    for p in (BASE / "observed-2026-10-27").glob("*.json"):
        if p.name != "report.json":
            shutil.copyfile(p, tmp_path / p.name)
    return tmp_path


def test_actual_future_only_comparison_replays_and_retains_failure(tmp_path):
    module = operator()
    report = module.analyze(retained_copy(tmp_path))
    assert report["passed"] is False
    assert report["primary_future_only_metrics"]["rmse_fraction"] == pytest.approx(
        0.14988200098795731
    )
    assert report["primary_future_only_metrics"]["maximum_absolute_error_fraction"] > 0.19
    assert [r["hour"] for r in report["rows"]] == [36, 48, 60, 72, 84]
    assert len(report["all_source_observations"]) == 28
    assert report["conditioning_hours_excluded_from_score"] == [0, 24]
    assert report["future_data_used_for_state_estimation"] is False
    assert report["initial_state_simulator_validated"] is False
    assert report["independent_biological_validation"] is False


def test_actual_grid_starts_at_anchor_and_finishes_full_injury_horizon():
    output = BASE / "observed-2026-10-27"
    candidate = json.loads((output / "candidate.json").read_bytes())
    receipt = json.loads((output / "child-execution.json").read_bytes())
    series = {s["name"]: s for s in candidate["outputs"]}
    assert candidate["time"]["values"][0] == 0
    assert candidate["time"]["values"][-1] == 60 * 3600
    assert len(candidate["time"]["values"]) == 241
    assert series["time_since_injury"]["values"][0] == 24
    assert series["time_since_injury"]["values"][-1] == 84
    assert series["open_area_fraction"]["values"][0] == 47.02 / 72.32
    assert len(receipt["verified_denied_read_probes"]) == 3


def test_failed_actual_child_cannot_be_scored_as_success(tmp_path):
    module = operator()
    output = retained_copy(tmp_path)
    path = output / "execution-record.json"
    record = json.loads(path.read_bytes())
    record["returncode"] = 9
    path.write_text(json.dumps(record))
    with pytest.raises(ValueError, match="actual successful isolated"):
        module.analyze(output)


def test_changed_retained_prediction_is_rejected_before_comparison(tmp_path):
    module = operator()
    output = retained_copy(tmp_path)
    path = output / "candidate.json"
    path.write_bytes(path.read_bytes() + b" ")
    with pytest.raises(ValueError, match="Frozen artifact changed"):
        module.analyze(output)
