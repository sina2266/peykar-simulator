"""Two different model structures behind one contract.

Decay: exact scalar first-order decay, with piecewise parameter interventions.
Logistic: discrete stochastic birth/death events via bounded binomial steps.
Neither is a scientifically validated model of a specific biological system.
"""

import math
from collections.abc import Callable

import numpy as np

from peykar_core.contracts import ModelPackage, Quantity, Series, SimulationPlan, SimulationResult
from peykar_core.planning import check_parameter, parameter_values, time_grid, validate_plan
from peykar_core.units import convert


def _apply_events(
    model: ModelPackage, plan: SimulationPlan, at: float, values: dict[str, float]
) -> None:
    for event in plan.interventions:
        if math.isclose(convert(event.at, "second"), at, rel_tol=0, abs_tol=1e-8):
            values[event.parameter] = check_parameter(model, event.parameter, event.quantity)


class DecayEngine:
    id = "reference.decay"
    version = "0.1.0"

    def validate(self, model: ModelPackage, plan: SimulationPlan) -> tuple[str, ...]:
        validate_plan(model, plan)
        expected = {"initial": "mole / liter", "rate": "1 / second"}
        if {p.name: p.default.unit for p in model.parameters} != expected:
            return ("Decay engine requires initial [mole / liter] and rate [1 / second]",)
        if any(e.parameter != "rate" for e in plan.interventions):
            return ("Only rate can change during a decay run",)
        values = parameter_values(model, plan)
        if values["initial"] < 0 or values["rate"] < 0:
            return ("Initial concentration and decay rate must be nonnegative",)
        return ()

    def execute(
        self, model: ModelPackage, plan: SimulationPlan, checkpoint: Callable[[], None]
    ) -> SimulationResult:
        errors = self.validate(model, plan)
        if errors:
            raise ValueError("; ".join(errors))
        params = parameter_values(model, plan)
        times = time_grid(plan)
        value = params["initial"]
        output: list[float] = []
        for i, at in enumerate(times):
            checkpoint()
            if i:
                value *= math.exp(-params["rate"] * (at - times[i - 1]))
            _apply_events(model, plan, at, params)
            # 'initial' is a state initialization parameter, never a runtime input.
            output.append(value)
        return SimulationResult(
            time=Series(name="time", unit="second", values=times),
            outputs=(Series(name="concentration", unit="mole / liter", values=tuple(output)),),
            seed=plan.seed,
            limitations=("Analytical synthetic verification only; no biological validation.",),
        )


class LogisticEngine:
    id = "reference.logistic"
    version = "0.1.0"

    def validate(self, model: ModelPackage, plan: SimulationPlan) -> tuple[str, ...]:
        validate_plan(model, plan)
        expected = {"initial", "capacity", "birth_rate", "death_rate"}
        if {p.name for p in model.parameters} != expected:
            return ("Logistic engine parameter names do not match its contract",)
        for spec in model.parameters:
            expected_unit = "dimensionless" if spec.name in ("initial", "capacity") else "1/second"
            if not math.isclose(
                convert(Quantity(value=1.0, unit=spec.default.unit), expected_unit), 1.0
            ):
                return ("Logistic model parameters must use canonical units",)
        if any(e.parameter not in ("birth_rate", "death_rate") for e in plan.interventions):
            return ("Only birth_rate and death_rate can change during a logistic run",)
        params = parameter_values(model, plan)
        for name in ("initial", "capacity"):
            if not params[name].is_integer():
                return ("Population counts and capacity must be integers",)
        if min(params.values()) < 0 or not 1 <= params["capacity"] <= 10_000_000:
            return ("Nonnegative parameters and capacity in [1, 10000000] required",)
        if params["initial"] > params["capacity"]:
            return ("Initial population exceeds capacity",)
        return ()

    def execute(
        self, model: ModelPackage, plan: SimulationPlan, checkpoint: Callable[[], None]
    ) -> SimulationResult:
        errors = self.validate(model, plan)
        if errors:
            raise ValueError("; ".join(errors))
        params = parameter_values(model, plan)
        times = time_grid(plan)
        rng = np.random.default_rng(plan.seed)
        population = int(params["initial"])
        output: list[float] = []
        for i, at in enumerate(times):
            checkpoint()
            if i:
                dt = at - times[i - 1]
                capacity = int(params["capacity"])
                birth_p = -math.expm1(-params["birth_rate"] * dt)
                birth_p *= max(0.0, 1 - population / capacity)
                births = int(rng.binomial(population, birth_p))
                deaths = int(rng.binomial(population, -math.expm1(-params["death_rate"] * dt)))
                population = min(capacity, max(0, population + births - deaths))
            _apply_events(model, plan, at, params)
            output.append(float(population))
        return SimulationResult(
            time=Series(name="time", unit="second", values=times),
            outputs=(Series(name="population", unit="dimensionless", values=tuple(output)),),
            seed=plan.seed,
            limitations=(
                "Synthetic bounded discrete-time model; time-step-dependent approximation.",
                "No spatial, mechanistic, tissue-specific or clinical validity.",
            ),
        )
