"""Operator orchestration preserves model/protocol identities across separate series."""

import importlib
import json
import sys
from pathlib import Path

import pytest

from peykar_core.hashing import bytes_digest, digest


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


@pytest.mark.parametrize(
    "workers,maximum,accepted",
    [
        ([1] * 6, 6, True),
        ([4] * 6, 4, True),
        ([4] * 6, 6, False),
        ([1, 4], 5, False),
        ([2], 1, False),
    ],
)
def test_parallelism_checks_native_xml_instead_of_blas_environment(
    tmp_path, operators, workers, maximum, accepted
):
    execution, _ = operators
    jobs = []
    for ordinal, count in enumerate(workers):
        root = tmp_path / str(ordinal)
        simulation = root / "project/Simulation"
        simulation.mkdir(parents=True)
        (simulation / "vCornea_v2.xml").write_text(
            "<CompuCell3D><Metadata><NumberOfProcessors>"
            + str(count)
            + "</NumberOfProcessors></Metadata></CompuCell3D>"
        )
        jobs.append({"directory": str(root)})
    if not accepted:
        with pytest.raises(ValueError):
            execution.validate_parallelism(jobs, maximum)
    else:
        result = execution.validate_parallelism(jobs, maximum)
        assert result["requested_workers_per_job"] == workers
        assert result["maximum_simultaneous_requested_workers"] == sum(
            sorted(workers, reverse=True)[:maximum]
        )


def test_resume_rejects_six_multiworker_retries_before_starting_any_child(
    tmp_path, operators, monkeypatch
):
    resume = importlib.import_module("resume_vcornea")
    previous, output = tmp_path / "previous", tmp_path / "retry"
    plan = series(previous, "mild-injury")
    profile = tmp_path / "profile.json"
    profile.write_text('{"scope":"synthetic capacity fixture"}')
    plan["jobs"] = [
        dict(plan["jobs"][0], seed=seed, directory=str(previous / str(seed)))
        for seed in range(1, 7)
    ]
    plan.update(
        status="execution_failed",
        profile_digest=digest(json.loads(profile.read_bytes())),
        parameter_profile="historical-hpc-3.0.0",
        wall_seconds_per_run=60,
    )
    (previous / "plan.json").write_text(json.dumps(plan))
    monkeypatch.setattr(resume, "verified_completed", lambda *args: False)

    def prepare(_source, _pins, root, *args, **kwargs):
        simulation = root / "project/Simulation"
        simulation.mkdir(parents=True)
        (simulation / "vCornea_v2.xml").write_text(
            "<CompuCell3D><Metadata><NumberOfProcessors>4</NumberOfProcessors>"
            "</Metadata></CompuCell3D>"
        )
        return {"scope": "synthetic retry preparation"}

    monkeypatch.setattr(resume, "prepare_publication", prepare)
    monkeypatch.setattr(resume, "execute_native", lambda *a, **kw: pytest.fail("child started"))
    monkeypatch.setattr(
        sys,
        "argv",
        [
            "resume_vcornea.py",
            "--previous",
            str(previous),
            "--source",
            str(tmp_path),
            "--pins",
            str(tmp_path),
            "--prefix",
            str(tmp_path),
            "--profile",
            str(profile),
            "--output",
            str(output),
            "--max-parallel",
            "6",
        ],
    )
    with pytest.raises(ValueError, match="multiworker"):
        resume.main()
    assert (output / "previous-plan.json").read_bytes() == (previous / "plan.json").read_bytes()
    assert not list(output.rglob("record.json"))


@pytest.mark.parametrize("stale_status", ["prepared", "running", "failed"])
def test_resume_reuses_verified_exits_even_when_the_old_plan_status_is_stale(
    tmp_path, operators, monkeypatch, stale_status
):
    resume = importlib.import_module("resume_vcornea")
    previous, output = tmp_path / "old", tmp_path / "resumed"
    plan = series(previous, "mild-injury")
    profile = tmp_path / "runtime.json"
    profile.write_text('{"scope":"synthetic completed-child fixture"}')
    plan.update(
        status="execution_failed",
        profile_digest=digest(json.loads(profile.read_bytes())),
        parameter_profile="observed-injury-4.0.0",
        wall_seconds_per_run=60,
    )
    for job in plan["jobs"]:
        job.update(status=stale_status, failure="stale_parent_observation")
        root = Path(job["directory"])
        root.mkdir()
        (root / "record.json").write_text(
            '{"status":"succeeded","returncode":0,"elapsed_seconds":12.5}'
        )
    (previous / "plan.json").write_text(json.dumps(plan))
    original = (previous / "plan.json").read_bytes()
    monkeypatch.setattr(resume, "verified_completed", lambda *args: True)
    monkeypatch.setattr(
        resume, "prepare_publication", lambda *a, **kw: pytest.fail("verified job prepared again")
    )
    monkeypatch.setattr(
        resume, "execute_native", lambda *a, **kw: pytest.fail("verified child executed again")
    )
    monkeypatch.setattr(
        sys,
        "argv",
        [
            "resume_vcornea.py",
            "--previous",
            str(previous),
            "--source",
            str(tmp_path),
            "--pins",
            str(tmp_path),
            "--prefix",
            str(tmp_path),
            "--profile",
            str(profile),
            "--output",
            str(output),
        ],
    )
    with pytest.raises(SystemExit) as result:
        resume.main()
    assert result.value.code == 0
    after = json.loads((output / "plan.json").read_bytes())
    assert after["status"] == "execution_completed"
    assert all(
        job["status"] == "succeeded" and job["elapsed_seconds"] == 12.5 for job in after["jobs"]
    )
    assert all(job["reused_verified_completed"] and "failure" not in job for job in after["jobs"])
    assert after["native_worker_budget"]["maximum_simultaneous_requested_workers"] == 0
    assert (previous / "plan.json").read_bytes() == original
    assert (output / "previous-plan.json").read_bytes() == original
