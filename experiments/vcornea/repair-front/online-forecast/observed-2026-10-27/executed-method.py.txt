"""Pure causal observation-state forecasting from the last two available areas.

This generic radius-trend predictor is not a trained biological simulator. It
accepts an explicitly bounded observed prefix and future query times, never
future observations, files, fitted parameters or an automatically registered model.
"""

import math
from collections.abc import Sequence
from typing import Any

from peykar_core.errors import InvalidRequest

MAX_OBSERVATIONS = 100_001
MAX_FORECASTS = 100_001


def _number(value: object, name: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise InvalidRequest(f"{name} must be a finite nonnegative number")
    try:
        result = float(value)
    except OverflowError as error:
        raise InvalidRequest(f"{name} must be a finite nonnegative number") from error
    if not math.isfinite(result) or result < 0:
        raise InvalidRequest(f"{name} must be a finite nonnegative number")
    return result


def forecast_corneal_radius_trend(
    *,
    history: Sequence[tuple[float, float]],
    origin_hour: float,
    future_hours: Sequence[float],
) -> dict[str, Any]:
    """Forecast normalized area using exactly the two last observed prefix points.

    Each history pair is ``(absolute_hour, observed_area_fraction)``. The origin
    must be the last observed hour; all supplied history must be available then.
    A fraction above1 or an observed expansion is preserved. Forecast radius is
    clipped only at zero; a later new observation may reopen a previously closed
    wound. No irreversible closure state is carried between calls.
    """
    origin = _number(origin_hour, "origin_hour")
    if not 2 <= len(history) <= MAX_OBSERVATIONS:
        raise InvalidRequest("Radius trend requires2..100001 observed prefix points")
    observed: list[tuple[float, float]] = []
    for point in history:
        if not isinstance(point, (tuple, list)) or len(point) != 2:
            raise InvalidRequest("Each observation requires an hour and area-fraction pair")
        hour = _number(point[0], "observation_hour")
        fraction = _number(point[1], "observed_area_fraction")
        if hour > origin:
            raise InvalidRequest("Future observations are forbidden in the causal prefix")
        if observed and hour <= observed[-1][0]:
            raise InvalidRequest("Observed prefix hours must be unique and strictly increasing")
        observed.append((hour, fraction))
    if observed[-1][0] != origin:
        raise InvalidRequest("Origin must equal the last available observation hour")
    if not 1 <= len(future_hours) <= MAX_FORECASTS:
        raise InvalidRequest("Radius trend requires1..100001 future query times")
    targets = tuple(_number(hour, "future_hour") for hour in future_hours)
    if targets[0] <= origin or any(b <= a for a, b in zip(targets, targets[1:], strict=False)):
        raise InvalidRequest("Future query hours must be unique, increasing and after the origin")

    previous, last = observed[-2:]
    previous_radius, last_radius = math.sqrt(previous[1]), math.sqrt(last[1])
    slope = (last_radius - previous_radius) / (last[0] - previous[0])
    if not math.isfinite(slope):
        raise InvalidRequest("Observed radius trend implies a nonfinite signed slope")
    predictions = []
    for hour in targets:
        horizon = hour - origin
        projected_radius = last_radius + slope * horizon
        if not math.isfinite(projected_radius):
            raise InvalidRequest("Requested horizon implies a nonfinite forecast radius")
        radius = max(0.0, projected_radius)
        try:
            fraction = radius**2
        except OverflowError as error:
            raise InvalidRequest("Requested horizon implies a nonfinite forecast area") from error
        if not math.isfinite(fraction):
            raise InvalidRequest("Requested horizon implies a nonfinite forecast area")
        predictions.append(
            {
                "hour": hour,
                "horizon_hours": horizon,
                "normalized_radius": radius,
                "area_fraction": fraction,
            }
        )
    return {
        "schema_version": "1.0.0",
        "method_id": "cornea.causal_last_two_radius_trend",
        "method_version": "1.0.0",
        "class": "causal_observation_conditioned_forecast_not_initial_state_simulation",
        "formula": "q(t)=max(0,sqrt(q_last)+signed_slope*(t-origin_hour))^2",
        "origin_hour": origin,
        "origin_area_fraction": last[1],
        "observed_history": [{"hour": hour, "area_fraction": q} for hour, q in observed],
        "history_used": [{"hour": hour, "area_fraction": q} for hour, q in observed[-2:]],
        "history_rule": "Exactly the last two available observations; no window or tuning",
        "signed_radius_slope_per_hour": slope,
        "slope_unit": "normalized-radius/hour",
        "time_unit": "hour",
        "area_unit": "dimensionless",
        "predictions": predictions,
        "future_observations_used": False,
        "training_or_hyperparameter_search": False,
        "biological_validation": False,
        "limitations": [
            "Requires two available observations; not an initial-state simulator.",
            "Signed trend is an observation-state estimate, not measured cell velocity.",
            "Observed fractions above1 and expansion are not clamped or removed.",
            "Forecast radius clips at zero; later actual observations may show reopening.",
            "No manufactured sample IDs, uncertainty, SD, causal treatment effect or score.",
            "No cellular, chemical, barrier or universal biological/clinical validity.",
        ],
    }
