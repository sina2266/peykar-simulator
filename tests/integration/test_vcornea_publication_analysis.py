"""Scientific acceptance logic must retain disagreement and incomplete evidence."""

import importlib.util
from pathlib import Path

import pytest

spec = importlib.util.spec_from_file_location(
    "cornea_comparison", Path("runtime_profiles/vcornea-publication/compare.py")
)
comparison = importlib.util.module_from_spec(spec)
spec.loader.exec_module(comparison)


def test_reference_envelope_boundary_and_nonfinite_values():
    reference = {"mean": 10, "sample_sd": 2}
    assert comparison.mean_check(17, reference, 3, 1)["passed"]
    assert not comparison.mean_check(17.01, reference, 3, 1)["passed"]
    with pytest.raises(ValueError, match="Nonfinite"):
        comparison.mean_check(float("nan"), reference, 3, 1)
    with pytest.raises(ValueError, match="Negative"):
        comparison.mean_check(10, {"mean": 10, "sample_sd": -1}, 3, 1)


def test_trajectory_requires_complete_grid_and_preserves_failure():
    result = comparison.trajectory_check([10, 20], [10, 10], [0, 0], 3, 1, 0.9)
    assert result["coverage"] == 0.5 and not result["passed"]
    assert result["maximum_absolute_difference"] == 10
    with pytest.raises(ValueError, match="Full matching"):
        comparison.trajectory_check([10], [10, 10], [0, 0], 3, 1, 0.9)


def test_population_return_is_after_nadir_and_requires_full_sustained_window():
    # A one-hour spike and a final incomplete window are not sustained recovery.
    trajectory = [100] * 720 + [50] * 8 + [110] + [60] * 200 + [100] * 23
    result = comparison.recovery_summary(trajectory)
    assert result["first_24h_sustained_count_return_hours_after_injury"] is None
    assert not result["pre_registered_acceptance_endpoint"]
    trajectory.append(100)
    result = comparison.recovery_summary(trajectory)
    assert result["first_24h_sustained_count_return_hours_after_injury"] == 209
    assert result["classification"] == "descriptive_population_count_return_not_wound_closure"


def test_artifact_replacement_and_directory_escape_are_rejected(tmp_path):
    root = tmp_path / "run"
    root.mkdir()
    raw = root / "count.csv"
    raw.write_text("actual cells\n")
    checksum = comparison.checksum(raw)
    assert comparison.bounded_file(root, raw.name, checksum) == raw
    raw.write_text("substituted cells\n")
    with pytest.raises(ValueError, match="identity/confinement"):
        comparison.bounded_file(root, raw.name, checksum)
    external = tmp_path / "other.csv"
    external.write_text("actual cells\n")
    with pytest.raises(ValueError, match="identity/confinement"):
        comparison.bounded_file(root, "../other.csv", checksum)


def test_spatial_proxy_detects_a_real_open_fluid_path_and_its_reclosure():
    spec = importlib.util.spec_from_file_location(
        "cornea_spatial", Path("runtime_profiles/vcornea-publication/analyze_spatial.py")
    )
    spatial = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(spatial)
    # x,y axes: walls, membrane, two epithelial rows, tear and exterior medium.
    kinds = [[9] * 5] + [[6, 3, 5, 10, 0] for _ in range(3)] + [[9] * 5]
    baseline = spatial.membrane_surface(kinds)
    assert baseline == {1: 1, 2: 1, 3: 1}
    assert spatial.exposed_columns(kinds, baseline)["exposed_columns"] == []
    kinds[2][1:3] = [0, 0]
    result = spatial.exposed_columns(kinds, baseline)
    assert result["exposed_columns"] == [2]
    assert result["exposed_cross_section_width_um"] == 2
    assert not result["clinical_wound_closure"]
    kinds[2][2] = 5
    assert spatial.exposed_columns(kinds, baseline)["exposed_columns"] == []


def test_diagnostic_subset_cannot_pass_the_default_full_series_gate():
    protocol = {"groups": dict.fromkeys(["formation", "slight", "mild", "moderate"])}
    plan = {"jobs": [{"case": "formation", "seed": seed} for seed in [7, 11]]}
    with pytest.raises(ValueError, match="Every required cohort"):
        comparison.validate_series_jobs(plan, protocol)
    assert comparison.validate_series_jobs(plan, protocol, ["formation"]) == plan["jobs"]
    for cases in [[], ["unknown"], list(protocol["groups"])]:
        with pytest.raises(ValueError, match="proper subset"):
            comparison.validate_series_jobs(plan, protocol, cases)
    plan["jobs"].append(plan["jobs"][0])
    with pytest.raises(ValueError, match="distinct seeds"):
        comparison.validate_series_jobs(plan, protocol, ["formation"])
