"""Shared resource, context, parameter and intervention checks before enqueueing."""

import math

from peykar_core.contracts import ModelPackage, Quantity, SimulationPlan
from peykar_core.errors import InvalidRequest
from peykar_core.units import convert


def parameter_values(model: ModelPackage, plan: SimulationPlan) -> dict[str, float]:
    specs = {p.name: p for p in model.parameters}
    values = {p.name: p.default.value for p in model.parameters}
    for override in plan.parameters:
        if override.name not in specs:
            raise InvalidRequest(f"Unknown parameter: {override.name}")
        spec = specs[override.name]
        values[override.name] = convert(override.quantity, spec.default.unit)
    for name, value in values.items():
        check_parameter(model, name, Quantity(value=value, unit=specs[name].default.unit))
    return values


def check_parameter(model: ModelPackage, name: str, quantity: Quantity) -> float:
    spec = next((p for p in model.parameters if p.name == name), None)
    if spec is None:
        raise InvalidRequest(f"Unknown parameter: {name}")
    value = convert(quantity, spec.default.unit)
    if spec.minimum is not None and value < spec.minimum:
        raise InvalidRequest(f"{name} is below its applicability range")
    if spec.maximum is not None and value > spec.maximum:
        raise InvalidRequest(f"{name} is above its applicability range")
    return value


def time_grid(plan: SimulationPlan) -> tuple[float, ...]:
    duration = convert(plan.duration, "second")
    dt = convert(plan.sample_interval, "second")
    if duration <= 0 or dt <= 0 or dt > duration:
        raise InvalidRequest("Require 0 < sample_interval <= duration")
    steps = duration / dt
    if not math.isfinite(steps) or steps > plan.budget.max_samples - 1:
        raise InvalidRequest("Requested samples exceed execution budget")
    rounded = round(steps)
    if not math.isclose(steps, rounded, rel_tol=0, abs_tol=1e-8):
        raise InvalidRequest("Duration must be an integer multiple of sample_interval")
    return tuple(i * dt for i in range(rounded + 1))


def validate_plan(model: ModelPackage, plan: SimulationPlan) -> None:
    if plan.model.id != model.id or plan.model.version != model.version:
        raise InvalidRequest("Plan does not reference this model version")
    if plan.context != model.context:
        raise InvalidRequest(
            "Cross-context or cross-species execution requires a new model version"
        )
    parameter_values(model, plan)
    times = time_grid(plan)
    seen: set[tuple[int, str]] = set()
    for intervention in plan.interventions:
        at = convert(intervention.at, "second")
        index = round(at / times[1])
        if (
            at < 0
            or at > times[-1]
            or not math.isclose(at, index * times[1], rel_tol=0, abs_tol=1e-8)
        ):
            raise InvalidRequest("Interventions must align with sample boundaries")
        key = (index, intervention.parameter)
        if key in seen:
            raise InvalidRequest("Ambiguous simultaneous interventions on one parameter")
        seen.add(key)
        check_parameter(model, intervention.parameter, intervention.quantity)
