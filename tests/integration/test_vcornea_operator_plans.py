"""Operator orchestration preserves model/protocol identities across separate series."""

import importlib
import json
import sys
from pathlib import Path

import pytest

from peykar_core.hashing import bytes_digest


@pytest.fixture
def operators(monkeypatch):
    monkeypatch.syspath_prepend(str(Path("scripts").resolve()))
    return importlib.import_module("reproduce_vcornea"), importlib.import_module(
        "analyze_vcornea_series"
    )


def series(root, case, protocol=b'{"frozen":true}\n'):
    root.mkdir()
    (root / "comparison.json").write_bytes(protocol)
    value = {
        "status": "execution_completed",
        "protocol_digest": bytes_digest(protocol),
        "profile_digest": "1" * 64,
        "jobs": [
            {
                "case": case,
                "seed": seed,
                "directory": str(root / str(seed)),
                "manifest_digest": str(seed) * 32,
                "status": "succeeded",
            }
            for seed in [7, 11]
        ],
    }
    (root / "plan.json").write_text(json.dumps(value))
    return value


def test_combination_retains_original_plans_and_each_source_identity(
    tmp_path, operators, monkeypatch
):
    _, analysis = operators
    monkeypatch.setattr(analysis, "verified_completed", lambda *args: True)
    a, b = tmp_path / "injury", tmp_path / "formation"
    series(a, "mild-injury")
    series(b, "self-organization")
    original = [(p / "plan.json").read_bytes() for p in [a, b]]
    output = analysis.combine_completed_series(a, [b], tmp_path / "combined")
    result = json.loads((output / "plan.json").read_bytes())
    assert len(result["jobs"]) == 4
    assert {job["original_series"] for job in result["jobs"]} == {str(a), str(b)}
    assert [(output / f"input-plan-{i}.json").read_bytes() for i in [0, 1]] == original
    assert [(p / "plan.json").read_bytes() for p in [a, b]] == original


@pytest.mark.parametrize("failure", ["running", "runtime", "protocol", "duplicate", "unverified"])
def test_invalid_combination_is_rejected_before_any_output(
    tmp_path, operators, monkeypatch, failure
):
    _, analysis = operators
    monkeypatch.setattr(analysis, "verified_completed", lambda *args: failure != "unverified")
    a, b = tmp_path / "a", tmp_path / "b"
    series(a, "mild-injury")
    value = series(b, "mild-injury" if failure == "duplicate" else "self-organization")
    if failure == "running":
        value["status"] = "running"
    if failure == "runtime":
        value["profile_digest"] = "2" * 64
    if failure == "protocol":
        value["protocol_digest"] = "3" * 64
    (b / "plan.json").write_text(json.dumps(value))
    output = tmp_path / "rejected"
    with pytest.raises(ValueError):
        analysis.combine_completed_series(a, [b], output)
    assert not output.exists()


def test_wait_prerequisites_pin_job_definitions_and_stop_on_failed_execution(tmp_path, operators):
    execution, _ = operators
    root = tmp_path / "prior"
    plan = series(root, "self-organization")
    definition = execution.series_definition(plan)
    plan["status"] = "running"
    plan["jobs"][0]["elapsed_seconds"] = 100
    assert execution.series_definition(plan) == definition
    plan["jobs"][0]["manifest_digest"] = "4" * 64
    assert execution.series_definition(plan) != definition
    plan["status"] = "execution_failed"
    (root / "plan.json").write_text(json.dumps(plan))
    output = tmp_path / "waiting"
    output.mkdir()
    with pytest.raises(ValueError, match="series failed"):
        execution.wait_for_series([root], output, 60)
    assert (output / "prerequisite-0.json").read_bytes() == (root / "plan.json").read_bytes()


def test_follow_on_waits_for_initial_plan_publication_and_retains_failure(
    tmp_path,
    operators,
    monkeypatch,
):
    _, analysis = operators
    runs, output = tmp_path / "preparing", tmp_path / "analysis"

    def publish_failed_plan(_seconds):
        runs.mkdir()
        (runs / "plan.json").write_text('{"status":"execution_failed"}')

    monkeypatch.setattr(analysis.time, "sleep", publish_failed_plan)
    monkeypatch.setattr(
        sys,
        "argv",
        [
            "analyze_vcornea_series.py",
            "--runs",
            str(runs),
            "--python",
            sys.executable,
            "--references",
            str(tmp_path),
            "--original-source",
            str(tmp_path),
            "--output",
            str(output),
        ],
    )
    with pytest.raises(SystemExit) as result:
        analysis.main()
    assert result.value.code == 1
    assert json.loads((output / "status.json").read_bytes())["status"] == "native_execution_failed"
