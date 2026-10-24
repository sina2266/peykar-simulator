"""Synthetic engineering fixtures only; never fit retained experimental data."""

import math

import pytest

from peykar.model_learning.corneal_front import (
    HOURS,
    CornealFrontParameters,
    corneal_learning_algorithm,
    fit_corneal_front,
    predict_open_area_fraction,
)
from peykar_core.errors import Cancelled, InvalidRequest

SYNTHETIC = CornealFrontParameters(40, 60, 4, 2)


def synthetic_rows():
    return [
        {
            "group": group,
            "replicate": str(replicate),
            "hour": hour,
            "wound_area_fraction": predict_open_area_fraction(
                SYNTHETIC, initial_radius_um=1000, hour=hour, concentration_wv_percent=dose
            ),
        }
        for group, dose in [("Control", 0), ("2% SDP", 2)]
        for replicate in range(1, 4)
        for hour in HOURS
    ]


def fit(rows, **kwargs):
    return fit_corneal_front(
        rows,
        initial_radius_um=1000,
        calibration_source_sha256="a" * 64,
        protocol_sha256="b" * 64,
        provenance={"source": "Synthetic fixture only; no actual animal or experiment"},
        checkpoint=kwargs.pop("checkpoint", lambda: None),
        **kwargs,
    )


def test_synthetic_recovery_exact_algorithm_receipt_and_order_independence():
    actual = fit(synthetic_rows())
    assert actual.parameters == SYNTHETIC
    assert actual.evaluation_count == 41_640
    artifact = actual.as_artifact()
    assert artifact["objective_mean_square_area_fraction"] == 0
    assert artifact["sample_counts"] == {
        "actual_animals": 6,
        "rows_including_baseline": 60,
        "fitted_nonbaseline_values": 54,
        "fitted_times_per_animal": 9,
        "groups": 2,
    }
    assert artifact["biological_validation"] is False
    assert artifact["algorithm"] == corneal_learning_algorithm()
    reordered = fit(list(reversed(synthetic_rows())))
    assert reordered.parameters == actual.parameters
    assert reordered.normalized_calibration_sha256 == actual.normalized_calibration_sha256
    assert reordered.evaluation_count == actual.evaluation_count


def test_initial_state_dose_interpolation_monotone_and_absorbing_closure():
    # At1% the interpolated speed is50um/h and lag3h; no observed area is input.
    values = [
        predict_open_area_fraction(
            SYNTHETIC, initial_radius_um=1000, hour=hour, concentration_wv_percent=1
        )
        for hour in range(73)
    ]
    assert values[0] == 1
    assert values[3] == 1
    assert values[13] == 0.25
    assert values[23:] == [0] * 50
    assert values == sorted(values, reverse=True)


@pytest.mark.parametrize(
    "field,value",
    [
        ("rate0_um_per_h", -1),
        ("rate2_um_per_h", 201),
        ("lag0_h", 25),
        ("lag2_h", math.nan),
        ("lag2_h", True),
    ],
)
def test_parameter_units_and_bounds_are_explicit(field, value):
    parameters = {"rate0_um_per_h": 40, "rate2_um_per_h": 60, "lag0_h": 4, "lag2_h": 2}
    parameters[field] = value
    with pytest.raises(InvalidRequest):
        CornealFrontParameters(**parameters)


@pytest.mark.parametrize(
    "field,value",
    [
        ("initial_radius_um", 0),
        ("initial_radius_um", math.inf),
        ("hour", -1),
        ("hour", 73),
        ("concentration_wv_percent", 2.1),
        ("concentration_wv_percent", True),
    ],
)
def test_prediction_rejects_unreviewed_inputs_or_dose_extrapolation(field, value):
    arguments = {"initial_radius_um": 1000, "hour": 12, "concentration_wv_percent": 1}
    arguments[field] = value
    with pytest.raises(InvalidRequest):
        predict_open_area_fraction(SYNTHETIC, **arguments)


def test_holdout_rows_rejected_instead_of_silently_filtered():
    rows = synthetic_rows()
    rows[0]["group"] = "0.5% SDP"
    with pytest.raises(InvalidRequest, match="heldout leakage"):
        fit(rows)
    with pytest.raises(InvalidRequest, match="heldout"):
        fit([*synthetic_rows(), {"group": "0.5% SDP"}])


@pytest.mark.parametrize(
    "mutation",
    [
        "missing",
        "duplicate",
        "off_grid",
        "invented_rabbit",
        "bad_baseline",
        "nonfinite",
        "negative",
    ],
)
def test_complete_unique_actual_animal_time_grid_required(mutation):
    rows = synthetic_rows()
    if mutation == "missing":
        rows.pop()
    elif mutation == "duplicate":
        rows[-1] = dict(rows[0])
    elif mutation == "off_grid":
        rows[-1]["hour"] = 53
    elif mutation == "invented_rabbit":
        rows[-1]["replicate"] = "4"
    elif mutation == "bad_baseline":
        rows[0]["wound_area_fraction"] = 0.99
    elif mutation == "nonfinite":
        rows[-1]["wound_area_fraction"] = math.inf
    elif mutation == "negative":
        rows[-1]["wound_area_fraction"] = -0.1
    with pytest.raises(InvalidRequest):
        fit(rows)


def test_objective_cap_and_cooperative_cancellation_never_return_partial_fit():
    with pytest.raises(InvalidRequest, match="cap reached"):
        fit(synthetic_rows(), max_evaluations=1)
    calls = 0

    def checkpoint():
        nonlocal calls
        calls += 1
        if calls == 20:
            raise Cancelled("Synthetic cancellation")

    with pytest.raises(Cancelled, match="Synthetic cancellation"):
        fit(synthetic_rows(), checkpoint=checkpoint)
    assert calls == 20


def test_even_finite_observed_expansion_is_preserved_not_clamped():
    # Validation must accept an actual normalized area above1, not silently alter it.
    # A deliberately tiny evaluation cap stops before any completed fit.
    rows = synthetic_rows()
    rows[1]["wound_area_fraction"] = 1.02
    with pytest.raises(InvalidRequest, match="cap reached"):
        fit(rows, max_evaluations=1)


def test_exact_source_protocol_and_nonempty_provenance_are_required():
    arguments = {
        "initial_radius_um": 1000,
        "calibration_source_sha256": "a" * 64,
        "protocol_sha256": "b" * 64,
        "provenance": {"source": "Synthetic"},
        "checkpoint": lambda: None,
    }
    for field, value in [
        ("calibration_source_sha256", "bad"),
        ("protocol_sha256", "B" * 64),
        ("provenance", {}),
    ]:
        with pytest.raises(InvalidRequest):
            fit_corneal_front(synthetic_rows(), **{**arguments, field: value})
