"""Synthetic inverse-family proofs only; no actual future data or solver fit."""

import math

import pytest

from peykar.validation.cornea_front_identifiability import corneal_front_identifiability
from peykar_core.errors import InvalidRequest


def diagnostic(**changes):
    arguments = {
        "baseline_area_mm2": 4 * math.pi,
        "anchor_area_mm2": 3.24 * math.pi,
        "anchor_hour": 2,
        "maximum_lag_hours": 2,
        "maximum_speed_um_per_hour": 1000,
        "future_hours": [3, 4, 5],
    }
    return corneal_front_identifiability(**{**arguments, **changes})


def test_partial_shrink_inverts_units_lag_coupling_and_exact_extreme_areas():
    result = diagnostic()
    assert result["admissible_speed_interval_um_per_hour"] == pytest.approx([100, 1000])
    assert result["admissible_lag_marginal_interval_hours"] == pytest.approx([0, 1.8])
    assert result["equivalent_baseline_radius_um"] == pytest.approx(2000)
    assert result["equivalent_anchor_radius_um"] == pytest.approx(1800)
    assert result["both_parameters_identified_within_family"] is False
    for bound, low_radius, high_radius in zip(
        result["future_bounds"], [800, 0, 0], [1700, 1600, 1500], strict=True
    ):
        assert bound["open_area_fraction_lower"] == pytest.approx((low_radius / 2000) ** 2)
        assert bound["open_area_fraction_upper"] == pytest.approx((high_radius / 2000) ** 2)
        assert bound["projected_area_upper_mm2"] == pytest.approx(
            math.pi * (high_radius / 1000) ** 2
        )


def test_lag_bound_below_anchor_restricts_maximum_speed():
    result = diagnostic(maximum_lag_hours=1)
    assert result["admissible_speed_interval_um_per_hour"] == pytest.approx([100, 200])
    assert result["admissible_lag_marginal_interval_hours"] == pytest.approx([0, 1])
    fixed = diagnostic(maximum_lag_hours=0)
    assert fixed["admissible_speed_interval_um_per_hour"] == pytest.approx([100, 100])
    assert fixed["both_parameters_identified_within_family"] is True
    assert fixed["optimistic_minimax_point_rmse_lower_bound_fraction"] == 0
    assert fixed["precision_can_be_guaranteed_within_declared_family"] is True
    assert fixed["biological_validation"] is False


def test_extreme_vectors_and_bruteforce_admissible_family_prove_minimax_bounds():
    result = diagnostic()
    lower = [b["open_area_fraction_lower"] for b in result["future_bounds"]]
    upper = [b["open_area_fraction_upper"] for b in result["future_bounds"]]
    midpoint = [b["optimistic_minimax_midpoint_fraction"] for b in result["future_bounds"]]
    bound = result["optimistic_minimax_point_rmse_lower_bound_fraction"]
    maximum = result["optimistic_minimax_point_maximum_error_lower_bound_fraction"]
    assert bound == pytest.approx(
        math.sqrt(sum((a - b) ** 2 for a, b in zip(lower, upper, strict=True)) / 3) / 2
    )
    assert maximum == pytest.approx(max(b - a for a, b in zip(lower, upper, strict=True)) / 2)
    # Both endpoint vectors are attainable, forcing any point forecast to at least
    # half their Euclidean separation. Midpoint attains that lower bound exactly.
    for endpoint in [lower, upper]:
        rmse = math.sqrt(sum((a - b) ** 2 for a, b in zip(endpoint, midpoint, strict=True)) / 3)
        assert rmse == pytest.approx(bound)
    for rate in range(100, 1001, 10):
        lag = 2 - 200 / rate
        assert 0 <= lag <= 2
        prediction = [(max(0, 2000 - rate * max(0, t - lag)) / 2000) ** 2 for t in [3, 4, 5]]
        assert all(
            a - 1e-12 <= value <= b + 1e-12
            for a, b, value in zip(lower, upper, prediction, strict=True)
        )
        rmse = math.sqrt(sum((a - b) ** 2 for a, b in zip(prediction, midpoint, strict=True)) / 3)
        assert rmse <= bound + 1e-12
    assert result["abstain_from_claiming_target_precision"] is True
    assert result["future_observations_used"] is False
    assert result["fit_or_solver_run"] is False
    assert result["statistical_or_confidence_interval"] is False


def test_no_change_can_hide_unstarted_lag_or_identify_zero_speed():
    hidden = diagnostic(anchor_area_mm2=4 * math.pi)
    assert hidden["admissible_speed_interval_um_per_hour"] == [0, 1000]
    assert hidden["future_bounds"][0]["open_area_fraction_lower"] == pytest.approx(0.25)
    assert all(b["open_area_fraction_upper"] == 1 for b in hidden["future_bounds"])
    stopped = diagnostic(anchor_area_mm2=4 * math.pi, maximum_lag_hours=1)
    assert stopped["admissible_speed_interval_um_per_hour"] == [0, 0]
    assert all(
        b["open_area_fraction_lower"] == b["open_area_fraction_upper"] == 1
        for b in stopped["future_bounds"]
    )
    assert stopped["speed_identified_within_family"] is True
    assert stopped["lag_identified_within_family"] is False


def test_closed_anchor_has_absorbing_future_and_only_lower_rate_constraint():
    closed = diagnostic(anchor_area_mm2=0, maximum_speed_um_per_hour=2000, maximum_lag_hours=0.5)
    assert closed["admissible_speed_interval_um_per_hour"] == pytest.approx([1000, 2000])
    assert closed["admissible_lag_marginal_interval_hours"] == [0, 0.5]
    assert all(
        b["open_area_fraction_lower"] == b["open_area_fraction_upper"] == 0
        for b in closed["future_bounds"]
    )
    assert closed["optimistic_minimax_point_rmse_lower_bound_fraction"] == 0
    assert closed["precision_can_be_guaranteed_within_declared_family"] is True
    with pytest.raises(InvalidRequest, match="cannot reproduce"):
        diagnostic(anchor_area_mm2=0, maximum_speed_um_per_hour=999)


def test_insufficient_speed_bound_does_not_manufacture_admissible_state():
    with pytest.raises(InvalidRequest, match="cannot reproduce"):
        diagnostic(maximum_speed_um_per_hour=99)
    with pytest.raises(InvalidRequest, match="cannot reproduce"):
        diagnostic(maximum_speed_um_per_hour=0)


@pytest.mark.parametrize(
    "change",
    [
        {"baseline_area_mm2": 0},
        {"baseline_area_mm2": 401},
        {"anchor_area_mm2": 5 * math.pi},
        {"anchor_area_mm2": -1},
        {"anchor_hour": 0},
        {"anchor_hour": 97},
        {"maximum_lag_hours": -1},
        {"maximum_speed_um_per_hour": -1},
        {"maximum_speed_um_per_hour": math.inf},
        {"maximum_lag_hours": math.nan},
        {"anchor_area_mm2": True},
        {"future_hours": []},
        {"future_hours": [2, 3]},
        {"future_hours": [3, 3]},
        {"future_hours": [4, 3]},
        {"future_hours": [3, math.nan]},
        {"future_hours": [169]},
        {"future_hours": [3] * 100_002},
    ],
)
def test_bad_units_domain_expansion_and_future_grids_are_rejected(change):
    with pytest.raises(InvalidRequest):
        diagnostic(**change)


def test_zero_speed_and_lag_zero_identify_unmoving_state_without_biology_claim():
    result = diagnostic(
        anchor_area_mm2=4 * math.pi, maximum_lag_hours=0, maximum_speed_um_per_hour=0
    )
    assert result["both_parameters_identified_within_family"] is True
    assert result["biological_validation"] is False


def test_tiny_positive_areas_do_not_underflow_radius_or_make_nonfinite_report():
    result = diagnostic(baseline_area_mm2=5e-324, anchor_area_mm2=0)
    assert result["equivalent_baseline_radius_um"] > 0
    assert math.isfinite(result["admissible_speed_interval_um_per_hour"][0])
