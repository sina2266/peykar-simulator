"""Synthetic causality and algebra checks; no actual experiment is scored."""

import math

import pytest

from peykar.model_learning.corneal_online import forecast_corneal_radius_trend
from peykar_core.errors import InvalidRequest


def forecast(**changes):
    arguments = {"history": [(0, 1), (2, 0.81)], "origin_hour": 2, "future_hours": [3, 5]}
    return forecast_corneal_radius_trend(**{**arguments, **changes})


def fractions(result):
    return [point["area_fraction"] for point in result["predictions"]]


def test_exact_last_two_radius_trend_and_normalized_units():
    result = forecast()
    assert result["signed_radius_slope_per_hour"] == pytest.approx(-0.05)
    assert result["slope_unit"] == "normalized-radius/hour"
    assert result["time_unit"] == "hour"
    assert result["area_unit"] == "dimensionless"
    assert fractions(result) == pytest.approx([0.85**2, 0.75**2])
    assert result["history_used"] == result["observed_history"]
    assert result["origin_area_fraction"] == 0.81
    assert result["future_observations_used"] is False
    assert result["training_or_hyperparameter_search"] is False
    assert result["biological_validation"] is False
    assert "score" not in result
    assert "candidate_outcomes" not in result


def test_earlier_observations_do_not_affect_prediction_and_input_is_unmodified():
    prefix = [(0, 0.1), (1, 100), (4, 1), (6, 0.81)]
    before = list(prefix)
    result = forecast(history=prefix, origin_hour=6, future_hours=[7, 9])
    other = forecast(history=[(0, 20), (2, 0), *prefix[-2:]], origin_hour=6, future_hours=[7, 9])
    assert result["predictions"] == other["predictions"]
    assert result["signed_radius_slope_per_hour"] == other["signed_radius_slope_per_hour"]
    assert result["history_used"] == other["history_used"]
    assert prefix == before
    assert result["observed_history"] != other["observed_history"]


def test_expansion_and_actual_area_above_one_are_preserved():
    result = forecast(history=[(0, 1), (2, 1.21)], future_hours=[4])
    assert result["origin_area_fraction"] == 1.21
    assert result["signed_radius_slope_per_hour"] == pytest.approx(0.05)
    assert fractions(result) == pytest.approx([1.44])
    assert result["observed_history"][1]["area_fraction"] == 1.21


def test_forecast_clipping_is_zero_only_and_new_observation_can_reopen():
    closed = forecast(history=[(0, 1), (2, 0)], future_hours=[3, 4])
    assert fractions(closed) == [0, 0]
    reopened = forecast(history=[(0, 1), (2, 0), (3, 0.04)], origin_hour=3, future_hours=[4])
    assert reopened["signed_radius_slope_per_hour"] == pytest.approx(0.2)
    assert fractions(reopened) == pytest.approx([0.16])
    assert reopened["history_used"] == [
        {"hour": 2, "area_fraction": 0},
        {"hour": 3, "area_fraction": 0.04},
    ]


def test_irregular_real_hours_and_time_translation_use_actual_elapsed_duration():
    irregular = forecast(history=[(0.5, 1), (3, 0.25)], origin_hour=3, future_hours=[3.5, 4.25])
    shifted = forecast(
        history=[(100.5, 1), (103, 0.25)], origin_hour=103, future_hours=[103.5, 104.25]
    )
    assert irregular["signed_radius_slope_per_hour"] == pytest.approx(-0.2)
    assert fractions(irregular) == pytest.approx([0.16, 0.0625])
    assert fractions(irregular) == fractions(shifted)
    assert [p["horizon_hours"] for p in irregular["predictions"]] == [0.5, 1.25]


def test_zeros_and_equal_area_are_valid_without_inventing_migration():
    assert fractions(forecast(history=[(0, 0), (2, 0)])) == [0, 0]
    assert fractions(forecast(history=[(0, 4), (2, 4)])) == [4, 4]


@pytest.mark.parametrize(
    "change",
    [
        {"history": []},
        {"history": [(2, 0.81)]},
        {"history": [(0, 1), (2, 0.81)] * 50_001},
        {"history": [(0, 1), (0, 0.81)]},
        {"history": [(2, 1), (0, 0.81)]},
        {"history": [(0, 1), (3, 0.81)]},
        {"origin_hour": 3},
        {"origin_hour": True},
        {"history": [(-1, 1), (2, 0.81)]},
        {"history": [(0, -1), (2, 0.81)]},
        {"history": [(0, math.nan), (2, 0.81)]},
        {"history": [(0, math.inf), (2, 0.81)]},
        {"history": [(0, True), (2, 0.81)]},
        {"history": [(0, 1), (2,)]},
        {"future_hours": []},
        {"future_hours": [3] * 100_002},
        {"future_hours": [2]},
        {"future_hours": [3, 3]},
        {"future_hours": [4, 3]},
        {"future_hours": [math.inf]},
        {"future_hours": [math.nan]},
    ],
)
def test_invalid_prefixes_future_rows_times_and_nonfinite_values_are_rejected(change):
    with pytest.raises(InvalidRequest):
        forecast(**change)


def test_nonfinite_derived_slope_radius_and_area_fail_explicitly():
    with pytest.raises(InvalidRequest, match="signed slope"):
        forecast(history=[(0, 0), (5e-324, 1)], origin_hour=5e-324, future_hours=[1])
    with pytest.raises(InvalidRequest, match="forecast radius"):
        forecast(history=[(0, 0), (1, 1e308)], origin_hour=1, future_hours=[1e308])
    with pytest.raises(InvalidRequest, match="forecast area"):
        forecast(history=[(0, 0), (1, 1e308)], origin_hour=1, future_hours=[100])
