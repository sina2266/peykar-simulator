"""Optional projected-radius kinematics for superficial rabbit corneal repair.

Published lag/radius-motion behavior, not a cellular/chemical model. An explicit
offline operator must register this adapter; default API engines are unchanged.
"""

import math
from collections.abc import Callable

from peykar_core.contracts import ModelPackage, Quantity, Series, SimulationPlan, SimulationResult
from peykar_core.planning import parameter_values, time_grid, validate_plan
from peykar_core.units import convert


class CornealProjectedFrontEngine:
    id = "cornea.projected_front"
    version = "1.0.0"

    @staticmethod
    def parameters(model: ModelPackage, plan: SimulationPlan) -> dict[str, float]:
        units = {
            "initial_radius": "micrometer",
            "migration_rate": "micrometer / hour",
            "lag": "hour",
        }
        values = parameter_values(model, plan)
        return {
            spec.name: convert(
                Quantity(value=values[spec.name], unit=spec.default.unit), units[spec.name]
            )
            for spec in model.parameters
        }

    def validate(self, model: ModelPackage, plan: SimulationPlan) -> tuple[str, ...]:
        validate_plan(model, plan)
        if (model.engine, model.engine_version) != (self.id, self.version):
            return ("Projected corneal front requires its exact reviewed engine identity",)
        if model.context.species != "Oryctolagus cuniculus":
            return ("This engine's current applicability is rabbit corneal debridement",)
        if "mechanical epithelial debridement" not in model.context.conditions:
            return ("Chemical or unspecified injury is not the reviewed repair context",)
        if {p.name for p in model.parameters} != {"initial_radius", "migration_rate", "lag"}:
            return ("Projected corneal front requires radius, migration_rate and lag",)
        if plan.interventions:
            return ("Drug/dose/time-varying interventions require a separately reviewed model",)
        values = self.parameters(model, plan)
        if (
            not 0 < values["initial_radius"] <= 10000
            or not 0 <= values["migration_rate"] <= 10000
            or not 0 <= values["lag"] <= 72
            or convert(plan.duration, "hour") > 72
        ):
            return ("Require bounded nonnegative kinetics, radius in(0,10000]µm and at most72h",)
        return ()

    def execute(
        self, model: ModelPackage, plan: SimulationPlan, checkpoint: Callable[[], None]
    ) -> SimulationResult:
        errors = self.validate(model, plan)
        if errors:
            raise ValueError("; ".join(errors))
        values = self.parameters(model, plan)
        times = time_grid(plan)
        initial = values["initial_radius"]
        radii = []
        areas = []
        fractions = []
        for second in times:
            checkpoint()
            migration_hours = max(0.0, second / 3600 - values["lag"])
            radius = max(0.0, initial - values["migration_rate"] * migration_hours)
            radii.append(radius)
            areas.append(math.pi * radius * radius)
            fractions.append((radius / initial) ** 2)
        return SimulationResult(
            time=Series(name="time", unit="second", values=times),
            outputs=(
                Series(name="projected_radius", unit="micrometer", values=tuple(radii)),
                Series(name="projected_area", unit="micrometer ** 2", values=tuple(areas)),
                Series(name="open_area_fraction", unit="dimensionless", values=tuple(fractions)),
                Series(
                    name="closure_fraction",
                    unit="dimensionless",
                    values=tuple(1 - value for value in fractions),
                ),
            ),
            seed=plan.seed,
            limitations=(
                "Deterministic projected-radius law; seed does not create biological replicates.",
                "Circular planar observation, not curvature-corrected corneal surface area.",
                "No cell-layer, chemical transport, stromal, barrier or drug-response validation.",
                "Published point estimates; uncertainty and protocol differences remain.",
                "Experimental checks are scoped evidence, not a global clinical approval.",
            ),
        )
