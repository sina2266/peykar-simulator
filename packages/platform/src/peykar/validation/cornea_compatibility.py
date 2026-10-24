"""Inverse diagnostic for shared circular-front hypotheses, never biological approval.

For a nominal radius R and common inward displacement d, A/A0=max(0,1-d/R)^2.
Each measured-area interval gives an admissible d interval. Empty intersection
rejects the shared-displacement hypothesis even with arbitrary time dependence;
changing a shared lag or speed cannot remove this necessary-condition failure.
"""

import math
from dataclasses import dataclass
from typing import Any

from peykar_core.errors import InvalidRequest


@dataclass(frozen=True)
class CircularAreaObservation:
    label: str
    nominal_radius_um: float
    measured_area_fraction: float
    published_rounding_fraction: float = 0.0


def _intervals(
    observations: tuple[CircularAreaObservation, ...], error: float
) -> tuple[list[dict[str, Any]], tuple[float, float] | None]:
    rows: list[dict[str, Any]] = []
    lower, upper = 0.0, max(o.nominal_radius_um for o in observations)
    for o in observations:
        area_lower = max(0.0, o.measured_area_fraction - o.published_rounding_fraction - error)
        area_upper = min(1.0, o.measured_area_fraction + o.published_rounding_fraction + error)
        if area_lower > area_upper:
            bounds = None
            lower, upper = 1.0, 0.0
        else:
            start = o.nominal_radius_um * (1 - math.sqrt(area_upper))
            # Once closed, larger displacements remain admissible if zero area is allowed.
            stop = (
                max(a.nominal_radius_um for a in observations)
                if area_lower == 0
                else o.nominal_radius_um * (1 - math.sqrt(area_lower))
            )
            bounds = [start, stop]
            lower, upper = max(lower, start), min(upper, stop)
        rows.append(
            {
                "label": o.label,
                "nominal_radius_um": o.nominal_radius_um,
                "measured_area_fraction": o.measured_area_fraction,
                "published_rounding_fraction": o.published_rounding_fraction,
                "admissible_displacement_um": bounds,
            }
        )
    return rows, (lower, upper) if lower <= upper else None


def shared_front_feasibility(
    observations: tuple[CircularAreaObservation, ...], *, maximum_error_fraction: float
) -> dict[str, Any]:
    """Exact interval inversion, plus optimistic minimum achievable max error.

    Published rounding is measurement representation only, not SD/SEM or a CI.
    Inferred displacement is a diagnostic fit, never a forward model prediction.
    No time law, kinetic parameter or causal mechanism is estimated here.
    """
    if (
        len(observations) < 2
        or len(observations) > 100
        or len({o.label for o in observations}) != len(observations)
        or not all(o.label.strip() for o in observations)
        or not math.isfinite(maximum_error_fraction)
        or not 0 <= maximum_error_fraction <= 1
        or any(
            not all(
                math.isfinite(v)
                for v in [
                    o.nominal_radius_um,
                    o.measured_area_fraction,
                    o.published_rounding_fraction,
                ]
            )
            or not 0 < o.nominal_radius_um <= 10000
            or not 0 <= o.measured_area_fraction <= 2
            or not 0 <= o.published_rounding_fraction <= 1
            for o in observations
        )
    ):
        raise InvalidRequest("Shared-front diagnosis needs bounded, distinct finite observations")
    rows, intersection = _intervals(observations, maximum_error_fraction)
    lo, hi = 0.0, 2.0
    _, at_zero = _intervals(observations, 0)
    if at_zero is None:
        for _ in range(80):
            middle = (lo + hi) / 2
            _, feasible = _intervals(observations, middle)
            if feasible is None:
                lo = middle
            else:
                hi = middle
    else:
        hi = 0.0
    _, best = _intervals(observations, hi)
    assert best is not None
    displacement = sum(best) / 2
    predictions = []
    for o in observations:
        area = max(0.0, 1 - displacement / o.nominal_radius_um) ** 2
        error = max(0.0, abs(area - o.measured_area_fraction) - o.published_rounding_fraction)
        predictions.append(
            {
                "label": o.label,
                "optimistic_diagnostic_fraction": area,
                "error_beyond_published_rounding_fraction": error,
            }
        )
    return {
        "hypothesis": "one shared inward displacement at this time, nominal circular projection",
        "maximum_error_fraction": maximum_error_fraction,
        "feasible": intersection is not None,
        "admissible_shared_displacement_um": None if intersection is None else list(intersection),
        "observations": rows,
        "optimistic_minimum_maximum_error_fraction": hi,
        "optimistic_diagnostic_displacement_um": displacement,
        "optimistic_diagnostic_areas": predictions,
        "diagnostic_inverse_fit": True,
        "forward_prediction_or_validation": False,
        "kinetic_parameters_fitted": False,
        "independent_biological_validation": False,
    }
