import math

import pytest

from peykar.validation.cornea_compatibility import (
    CircularAreaObservation as Observation,
)
from peykar.validation.cornea_compatibility import (
    shared_front_feasibility,
)
from peykar_core.errors import InvalidRequest


def test_common_displacement_recovers_exact_synthetic_geometry_without_kinetic_fit():
    report = shared_front_feasibility(
        (Observation("small", 2, 0.25), Observation("large", 4, 0.5625)),
        maximum_error_fraction=0,
    )
    assert report["feasible"]
    assert report["admissible_shared_displacement_um"] == [1, 1]
    assert report["optimistic_minimum_maximum_error_fraction"] == 0
    assert report["diagnostic_inverse_fit"] is True
    assert report["kinetic_parameters_fitted"] is False
    assert report["forward_prediction_or_validation"] is False


def test_equal_radius_conflict_has_analytical_minimax_error_half_observed_difference():
    report = shared_front_feasibility(
        (Observation("a", 10, 0.2), Observation("b", 10, 0.8)),
        maximum_error_fraction=0.1,
    )
    assert report["feasible"] is False
    assert report["admissible_shared_displacement_um"] is None
    assert report["optimistic_minimum_maximum_error_fraction"] == pytest.approx(0.3)
    assert report["optimistic_diagnostic_displacement_um"] == pytest.approx(
        10 * (1 - math.sqrt(0.5))
    )


def test_absorbing_closure_keeps_feasibility_after_small_wound_closes():
    report = shared_front_feasibility(
        (Observation("small closed", 2, 0), Observation("large open", 4, 0.25)),
        maximum_error_fraction=0,
    )
    assert report["feasible"]
    assert report["admissible_shared_displacement_um"] == [2, 2]


def test_published_rounding_is_optimistic_but_above_one_observations_are_not_clamped():
    report = shared_front_feasibility(
        (Observation("rounded", 10, 1.02, 0.005), Observation("initial", 10, 1)),
        maximum_error_fraction=0.01,
    )
    assert report["observations"][0]["measured_area_fraction"] == 1.02
    assert report["feasible"] is False
    assert report["optimistic_minimum_maximum_error_fraction"] == pytest.approx(0.015)


def test_smaller_wound_cannot_remain_more_open_under_any_shared_time_dependent_front():
    report = shared_front_feasibility(
        (Observation("small", 3000, 0.7, 0.005), Observation("large", 3500, 0.4)),
        maximum_error_fraction=0.1,
    )
    # Even ignoring the stronger radius constraint, ordering gives this lower bound.
    assert report["optimistic_minimum_maximum_error_fraction"] >= (0.7 - 0.005 - 0.4) / 2
    assert report["feasible"] is False


@pytest.mark.parametrize(
    "observation,tolerance",
    [
        (Observation("bad", 0, 0.5), 0.1),
        (Observation("bad", float("nan"), 0.5), 0.1),
        (Observation("bad", 10, float("inf")), 0.1),
        (Observation("bad", 10, -0.5), 0.1),
        (Observation("bad", 10, 0.5, -0.1), 0.1),
        (Observation("bad", 10, 0.5), float("nan")),
        (Observation("bad", 10, 0.5), -0.1),
    ],
)
def test_unreported_or_invalid_measurements_cannot_create_a_feasible_hypothesis(
    observation, tolerance
):
    with pytest.raises(InvalidRequest):
        shared_front_feasibility(
            (Observation("valid", 10, 0.5), observation), maximum_error_fraction=tolerance
        )


def test_duplicate_or_single_observations_are_not_cross_study_evidence():
    observation = Observation("one", 10, 0.5)
    for data in [(observation,), (observation, observation)]:
        with pytest.raises(InvalidRequest):
            shared_front_feasibility(data, maximum_error_fraction=0.1)
