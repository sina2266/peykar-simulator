import importlib.util
from pathlib import Path


def test_actual_studies_rule_out_shared_front_without_tuning_or_new_candidate():
    spec = importlib.util.spec_from_file_location(
        "corneal_shared_audit", Path("scripts/audit_corneal_shared_front.py")
    )
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    report = module.audit()
    assert report["shared_hypothesis_feasible_at_all_common_times"] is False
    assert [row["hour"] for row in report["shared_time_results"]] == [18, 24]
    assert all(len(row["joint"]["observations"]) == 4 for row in report["shared_time_results"])
    assert all(
        not pair["feasible"] for row in report["shared_time_results"] for pair in row["pairwise"]
    )
    assert report["lower_bound_on_any_shared_front_maximum_error_fraction"] > 0.21
    assert report["diagnostic_inverse_fit"] is True
    assert report["model_execution_performed"] is False
    assert report["kinetic_parameters_fitted"] is False
    assert report["forward_prediction_or_validation"] is False
