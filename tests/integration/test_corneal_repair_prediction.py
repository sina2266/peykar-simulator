"""Real retained evidence: preserve narrow agreement and independent disagreement."""

import importlib
import json
import shutil
from pathlib import Path

import pytest

OBSERVED = Path("experiments/vcornea/repair-front/observed-2026-10-24")


def operator(monkeypatch, name):
    monkeypatch.syspath_prepend(str(Path("scripts").resolve()))
    return importlib.import_module(name)


def completed(tmp_path, name):
    for file in [
        "candidate.json",
        "plan.json",
        "execution-record.json",
        "child-execution.json",
        "frozen-protocol.json",
    ]:
        shutil.copyfile(OBSERVED / name / file, tmp_path / file)
    return tmp_path


def test_real_pbs_cohort_excludes_baseline_keeps_every_rabbit_and_does_not_approve_biology(
    tmp_path, monkeypatch
):
    module = operator(monkeypatch, "check_corneal_repair_front")
    report = module.compare(completed(tmp_path, "pbs2017"), module.read_protocol())
    assert report["passed"] is True
    assert report["independent_biological_validation"] is False
    assert report["primary_nonbaseline_times"] == list(range(6, 55, 6))
    assert len(report["cohort_observations"]) == 10
    assert all(r["measured_animals"] == 3 for r in report["cohort_observations"])
    assert len(report["individual_diagnostics"]) == 3
    assert report["primary_metrics"]["rmse_fraction"] == pytest.approx(0.04021732891267079)
    # A cohort-mean pass cannot hide the less accurate second animal.
    assert report["individual_diagnostics"][1]["rmse_fraction"] > 0.05
    assert report["experimental_data_used_for_fitting"] is False


def test_real_independent_removal_methods_remain_negative_with_all_observations(
    tmp_path, monkeypatch
):
    module = operator(monkeypatch, "check_corneal_repair_challenge")
    report = module.compare(completed(tmp_path, "challenge2021"), module.protocol())
    assert report["status"] == "challenge_failed"
    assert report["passed"] is False
    assert len(report["observations"]) == 12
    assert {r["group"] for r in report["group_metrics"]} == {"Blade", "Ocu", "Amo"}
    assert all(not r["passed"] and r["rmse_fraction"] > 0.24 for r in report["group_metrics"])
    categorical = [r for r in report["observations"] if r["kind"] == "categorical"]
    assert len(categorical) == 2 and all(r["error_fraction"] is None for r in categorical)
    assert report["experimental_data_used_for_fitting"] is False


@pytest.mark.parametrize("change", ["candidate", "child_exit", "protocol"])
def test_changed_artifacts_or_failed_children_cannot_be_analyzed_as_success(
    tmp_path, monkeypatch, change
):
    module = operator(monkeypatch, "check_corneal_repair_front")
    output = completed(tmp_path, "pbs2017")
    data = module.read_protocol()
    if change == "candidate":
        with (output / "candidate.json").open("ab") as stream:
            stream.write(b" ")
    elif change == "child_exit":
        path = output / "execution-record.json"
        record = json.loads(path.read_bytes())
        record["returncode"] = -6
        path.write_text(json.dumps(record))
    else:
        data["maximum_rmse_fraction"] = 1
    with pytest.raises(ValueError):
        module.compare(output, data)
    assert not (output / "report.json").exists()
