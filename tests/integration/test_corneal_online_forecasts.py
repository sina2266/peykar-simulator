"""Check retained real executions; never launch a new scientific child in tests."""

import hashlib
import importlib.util
import json
import shutil
import zipfile
from pathlib import Path, PurePosixPath

import pytest

BASE = Path("experiments/vcornea/repair-front/online-forecast/observed-2026-10-27")


def operator():
    spec = importlib.util.spec_from_file_location(
        "online_replay", "scripts/replay_corneal_online_forecasts.py"
    )
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@pytest.fixture
def retained(tmp_path):
    for path in BASE.glob("*.json"):
        shutil.copyfile(path, tmp_path / path.name)
    inventory = json.loads((BASE / "archive-inventory.json").read_bytes())
    archive = BASE / inventory["path"]
    assert hashlib.sha256(archive.read_bytes()).hexdigest() == inventory["sha256"]
    with zipfile.ZipFile(archive) as stored:
        assert len(stored.infolist()) == inventory["member_count"] == 784
        for info in stored.infolist():
            name = PurePosixPath(info.filename)
            assert name.parts[0] == "jobs" and not name.is_absolute() and ".." not in name.parts
            assert info.file_size < 100_000
            path = tmp_path / str(name)
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(stored.read(info))
    return tmp_path


def rewrite(path, transform):
    content = json.loads(path.read_bytes())
    transform(content)
    path.write_text(json.dumps(content))


def test_actual98_children_replay_with_every_numeric_and_categorical_target(retained, monkeypatch):
    module = operator()

    def forbidden_process(*args, **kwargs):
        raise AssertionError("Evidence replay must not execute a new scientific process")

    monkeypatch.setattr(module.subprocess, "run", forbidden_process)
    report = module.analyze(retained)
    assert report == json.loads((BASE / "report.json").read_bytes())
    assert report["passed"] is False
    assert report["forecast_count"] == 98 and report["numeric_forecast_count"] == 96
    assert report["series_count"] == 16
    assert sum(s["passed"] for s in report["series_reports"]) == 5
    assert all(s["passes_improvement"] for s in report["series_reports"])
    assert sum(len(s["observations"]) for s in report["excluded_observations"]) == 32
    categorical = [row for row in report["rows"] if not row["scored"]]
    assert len(categorical) == report["categorical_unscored_count"] == 2
    assert all(row["actual_area_fraction"] is None and row["errors"] is None for row in categorical)
    assert all(row["original"]["reported"] == "Completely Healed" for row in categorical)
    assert report["previously_consumed_studies"] is True
    assert report["independent_biological_validation"] is False
    assert report["initial_state_simulator_validated"] is False


def test_every_actual_child_retains_read_network_and_write_denial(retained):
    for path in sorted((retained / "jobs").glob("*/child-receipt.json")):
        receipt = json.loads(path.read_bytes())
        assert len(receipt["verified_denied_read_probes"]) == 5
        assert receipt["network_denial_verified"] is receipt["write_denial_verified"] is True
        assert receipt["future_values_in_input"] is False
        prefix = json.loads((path.parent / "prefix.json").read_bytes())
        assert all(p["hour"] <= prefix["origin_hour"] for p in prefix["history"])
        assert all(p["eligible_for_conditioning"] is True for p in prefix["history"])
        assert "target_area_fraction" not in prefix


def test_failed_child_is_rejected_even_with_complete_looking_predictions(retained):
    rewrite(retained / "jobs/000/execution-record.json", lambda value: value.update(returncode=7))
    with pytest.raises(ValueError, match="actual successful isolated"):
        operator().analyze(retained)


def test_changed_prediction_bytes_are_rejected(retained):
    path = retained / "jobs/000/prediction.json"
    path.write_bytes(path.read_bytes() + b" ")
    with pytest.raises(ValueError, match="Frozen artifact changed"):
        operator().analyze(retained)


def test_forged_rehashed_comparator_is_rejected_by_frozen_arithmetic(retained):
    path = retained / "jobs/000/prediction.json"
    rewrite(path, lambda value: value["forecasts"].update(persistence=0))
    rewrite(
        retained / "jobs/000/child-receipt.json",
        lambda value: value.update(prediction_sha256=hashlib.sha256(path.read_bytes()).hexdigest()),
    )
    with pytest.raises(ValueError, match="frozen prefix arithmetic"):
        operator().analyze(retained)


def test_rehashed_future_value_in_prefix_is_rejected_against_original_observations(retained):
    path = retained / "jobs/000/prefix.json"
    rewrite(path, lambda value: value.update(target_area_fraction=0))
    rewrite(
        retained / "jobs/000/child-receipt.json",
        lambda value: value.update(prefix_sha256=hashlib.sha256(path.read_bytes()).hexdigest()),
    )
    with pytest.raises(ValueError, match="Candidate prefix"):
        operator().analyze(retained)
