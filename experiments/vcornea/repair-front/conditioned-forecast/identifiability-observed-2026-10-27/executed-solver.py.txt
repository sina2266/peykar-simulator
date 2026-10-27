"""Prefix-only structural identifiability of an assumed circular wound front.

This is a retrospective inverse diagnostic, not another fitted candidate, a
statistical interval or a biological guarantee. It reads no files or future
observations. Bounds apply only to the explicitly declared lag/rate family.
"""

import math
from collections.abc import Sequence
from typing import Any

from peykar_core.errors import InvalidRequest

MAX_FUTURE_SAMPLES = 100_001
RMSE_TARGET = 0.05
MAXIMUM_ERROR_TARGET = 0.10


def _number(value: object, name: str, lower: float, upper: float = math.inf) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise InvalidRequest(f"{name} requires a finite number")
    try:
        result = float(value)
    except OverflowError as error:
        raise InvalidRequest(f"{name} requires a finite number") from error
    if not math.isfinite(result) or not lower <= result <= upper:
        raise InvalidRequest(f"{name} is outside the finite declared bounds")
    return result


def corneal_front_identifiability(
    *,
    baseline_area_mm2: float,
    anchor_area_mm2: float,
    anchor_hour: float,
    maximum_lag_hours: float,
    maximum_speed_um_per_hour: float,
    future_hours: Sequence[float],
) -> dict[str, Any]:
    """Invert two areas without inventing a separately identified lag or speed.

    Family: R(t)=max(0,R0-v*max(0,t-L)), 0<=L<=Lmax and0<=v<=Vmax.
    Areas are projected mm², times absolute hours since injury and rates µm/h.
    A0 andAa are actual cohort-equivalent areas, not fabricated animal radii.
    Every requested future time must be strictly beyond the observed anchor.
    """
    baseline = _number(baseline_area_mm2, "baseline_area_mm2", 0, 400)
    anchor = _number(anchor_area_mm2, "anchor_area_mm2", 0, baseline)
    at = _number(anchor_hour, "anchor_hour", 0, 96)
    lag_max = _number(maximum_lag_hours, "maximum_lag_hours", 0)
    rate_max = _number(maximum_speed_um_per_hour, "maximum_speed_um_per_hour", 0)
    if baseline == 0 or at == 0:
        raise InvalidRequest("A positive baseline area and anchor time are required")
    if not 1 <= len(future_hours) <= MAX_FUTURE_SAMPLES:
        raise InvalidRequest("Require1..100001 actual future times")
    times = tuple(_number(t, "future_hour", 0, 168) for t in future_hours)
    if times[0] <= at or any(b <= a for a, b in zip(times, times[1:], strict=False)):
        raise InvalidRequest(
            "Future times must be unique, increasing and strictly after the anchor"
        )

    # Taking sqrt before dividing avoids underflow for positive finite areas.
    radius0 = math.sqrt(baseline) / math.sqrt(math.pi) * 1000
    radius_anchor = math.sqrt(anchor) / math.sqrt(math.pi) * 1000
    # Stable difference of square roots preserves a small observed decrease.
    displacement = (
        (baseline - anchor) / (math.sqrt(baseline) + math.sqrt(anchor)) / math.sqrt(math.pi) * 1000
    )
    if anchor == 0:
        rate_min = radius0 / at
        rate_upper = rate_max
        regime = "closed_anchor_absorbing_future"
        coupling = "v*(anchor_hour-L)>=R0, with0<=L<=Lmax; clipping hides earlier closure"
        lag_upper = min(lag_max, max(0.0, at - radius0 / rate_max)) if rate_max else 0.0
    elif displacement == 0:
        rate_min = 0.0
        rate_upper = rate_max if lag_max >= at else 0.0
        regime = "unchanged_anchor_with_possible_unstarted_lag" if lag_max >= at else "zero_speed"
        coupling = "Eitherv=0, orL>=anchor_hour; joint lag/rate values are not independent"
        lag_upper = lag_max
    else:
        rate_min = displacement / at
        rate_upper = rate_max
        if lag_max < at:
            rate_upper = min(rate_upper, displacement / (at - lag_max))
        regime = "partially_shrunken_anchor"
        coupling = "L=anchor_hour-D/v; a different admissible speed implies a different lag"
        lag_upper = min(lag_max, max(0.0, at - displacement / rate_upper)) if rate_upper else 0.0
    if rate_min > rate_upper:
        raise InvalidRequest("The declared speed bound cannot reproduce the observed prefix")

    def area_fraction(hour: float, rate: float) -> float:
        if anchor == 0:
            return 0.0
        # For a partially shrunken anchor, all admissible fronts are already moving.
        # For an unchanged anchor, the extremal moving front can start exactly at it.
        remaining = max(0.0, radius_anchor - rate * (hour - at)) / radius_anchor
        return (anchor / baseline) * remaining**2

    bounds = []
    widths = []
    for hour in times:
        lower = area_fraction(hour, rate_upper)
        upper = area_fraction(hour, rate_min)
        width = upper - lower
        widths.append(width)
        bounds.append(
            {
                "hour_since_injury": hour,
                "open_area_fraction_lower": lower,
                "open_area_fraction_upper": upper,
                "projected_area_lower_mm2": baseline * lower,
                "projected_area_upper_mm2": baseline * upper,
                "optimistic_minimax_midpoint_fraction": (lower + upper) / 2,
            }
        )
    rmse_bound = math.sqrt(math.fsum(width**2 for width in widths) / len(widths)) / 2
    max_bound = max(widths) / 2
    can_guarantee = rmse_bound <= RMSE_TARGET and max_bound <= MAXIMUM_ERROR_TARGET
    lag_identified = lag_upper == 0
    speed_identified = rate_min == rate_upper
    return {
        "schema_version": "1.0.0",
        "id": "cornea.prefix-front-identifiability",
        "version": "1.0.0",
        "class": "retrospective_prefix_only_structural_diagnostic_not_forward_validation",
        "family": "R(t)=max(0,R0-v*max(0,t-L)); circular projected area=pi*R(t)^2",
        "inputs": {
            "baseline_area_mm2": baseline,
            "anchor_area_mm2": anchor,
            "anchor_hour": at,
            "maximum_lag_hours": lag_max,
            "maximum_speed_um_per_hour": rate_max,
            "future_hours": list(times),
        },
        "regime": regime,
        "equivalent_baseline_radius_um": radius0,
        "equivalent_anchor_radius_um": radius_anchor,
        "observed_equivalent_displacement_um": displacement,
        "admissible_speed_interval_um_per_hour": [rate_min, rate_upper],
        "admissible_lag_marginal_interval_hours": [0.0, lag_upper],
        "lag_speed_coupling": coupling,
        "speed_identified_within_family": speed_identified,
        "lag_identified_within_family": lag_identified,
        "both_parameters_identified_within_family": lag_identified and speed_identified,
        "future_bounds": bounds,
        "optimistic_minimax_point_rmse_lower_bound_fraction": rmse_bound,
        "optimistic_minimax_point_maximum_error_lower_bound_fraction": max_bound,
        "minimax_scope": (
            "Half theRMS/maximum distance between jointly attainable extremal future vectors. "
            "The unconstrained pointwise midpoint attains both bounds for this family; "
            "it need not itself be one constant-speed/lag trajectory."
        ),
        "targets": {"rmse_fraction": RMSE_TARGET, "maximum_error_fraction": MAXIMUM_ERROR_TARGET},
        "precision_can_be_guaranteed_within_declared_family": can_guarantee,
        "abstain_from_claiming_target_precision": not can_guarantee,
        "future_observations_used": False,
        "fit_or_solver_run": False,
        "statistical_or_confidence_interval": False,
        "biological_validation": False,
        "limitations": [
            "Model-assumption bounds only; true tissue need not belong to this front family.",
            "No statistical coverage, population distribution or clinical precision guarantee.",
            "Two areas generally do not separately identify lag and post-lag velocity.",
            "Cohort-equivalent radii are not mean individual-animal radii.",
            "No cell-density, substrate, pharmacology, imaging-bias or chemical mechanism.",
            "Retrospective diagnostic after a rejected forecast, not a new prospective result.",
        ],
    }
