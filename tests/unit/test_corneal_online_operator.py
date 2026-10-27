import importlib.util
from pathlib import Path

import pytest

from peykar_core.errors import InvalidRequest


def load(name):
    spec = importlib.util.spec_from_file_location(name, Path("scripts") / f"{name}.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def point(hour, q, eligible=True):
    return {"hour": hour, "area_fraction": q, "eligible_for_conditioning": eligible}


def request():
    return {
        "series_id": "synthetic",
        "history": [point(4, 1), point(18, 0.81), point(24, 0.64)],
        "origin_hour": 24,
        "target_hour": 50,
        "initial_radius_um": 3000,
    }


def test_same_prefix_comparators_use_actual_first_point_and_irregular_horizon():
    result = load("corneal_online_child").predict(request())
    assert result["forecasts"]["local_radius_trend"] == pytest.approx((0.8 - 0.1 / 6 * 26) ** 2)
    assert result["forecasts"]["first_to_current_radius"] == pytest.approx(
        (0.8 - 0.2 / 20 * 26) ** 2
    )
    assert result["forecasts"]["persistence"] == 0.64
    assert result["local_radius_trend"]["history_used"] == [
        {"hour": 18, "area_fraction": 0.81},
        {"hour": 24, "area_fraction": 0.64},
    ]


def test_child_rejects_future_outcome_field_instead_of_ignoring_it():
    value = request()
    value["target_area_fraction"] = 0
    with pytest.raises(ValueError, match="only prefix"):
        load("corneal_online_child").predict(value)


def test_child_rejects_future_observation_and_derived_baseline():
    value = request()
    value["history"].append(point(50, 0.1))
    with pytest.raises(InvalidRequest, match="Future observations"):
        load("corneal_online_child").predict(value)
    value = request()
    value["history"].insert(0, point(0, 1, False))
    with pytest.raises(ValueError, match="actual-observation prefix"):
        load("corneal_online_child").predict(value)


def test_origins_keep_categorical_targets_and_do_not_bootstrap_derived_zero_hour():
    series = {
        "series_id": "synthetic",
        "baseline_radius_um": 3000,
        "numeric_observations": [
            point(0, 1, False),
            point(4, 1.02),
            point(18, 0.86),
            point(24, 0.65),
        ],
        "categorical_observations": [{"hour": 50, "reported": "Completely Healed"}],
    }
    tasks, omitted = load("replay_corneal_online_forecasts").origins(series)
    assert [(r["origin_hour"], r["target_hour"]) for r in tasks] == [(18, 24), (24, 50)]
    assert [r["hour"] for r in omitted] == [4, 18]
    assert all(r["hour"] > 0 for task in tasks for r in task["history"])
    assert tasks[0]["history"][0]["area_fraction"] == 1.02
    assert all("target_area_fraction" not in task for task in tasks)
