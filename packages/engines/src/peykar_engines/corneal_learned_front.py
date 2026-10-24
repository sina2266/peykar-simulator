"""Optional offline empirical corneal front with learned dose endpoints.

No training, remote code or automatic registration. A reviewed model supplies its
frozen endpoint parameters. Linear dose interpolation is an empirical assumption,
not a causal pharmacological mechanism or biological/clinical approval.
"""

import math
from collections.abc import Callable

from peykar_core.contracts import ModelPackage, Quantity, Series, SimulationPlan, SimulationResult
from peykar_core.planning import parameter_values, time_grid, validate_plan
from peykar_core.units import convert


class CornealLearnedFrontEngine:
    id = "cornea.learned_front"
    version = "1.0.0"
    units = {
        "initial_radius": "micrometer",
        "concentration": "percent",
        "rate0": "micrometer/hour",
        "rate2": "micrometer/hour",
        "lag0": "hour",
        "lag2": "hour",
    }

    def parameters(self, model: ModelPackage, plan: SimulationPlan) -> dict[str, float]:
        values = parameter_values(model, plan)
        return {
            p.name: convert(Quantity(value=values[p.name], unit=p.default.unit), self.units[p.name])
            for p in model.parameters
        }

    def validate(self, model: ModelPackage, plan: SimulationPlan) -> tuple[str, ...]:
        validate_plan(model, plan)
        if (model.engine, model.engine_version) != (self.id, self.version):
            return ("Learned corneal front requires its exact reviewed engine identity",)
        if model.context.species != "Oryctolagus cuniculus":
            return ("The current corneal learner is scoped to rabbits",)
        if "mechanical epithelial debridement" not in model.context.conditions:
            return ("Chemical or unspecified injury is outside the reviewed learned context",)
        if {p.name for p in model.parameters} != set(self.units):
            return ("Learned front requires radius, concentration and four endpoint parameters",)
        if plan.interventions:
            return ("Time-varying interventions require another reviewed learned model",)
        values = self.parameters(model, plan)
        if (
            not 0 < values["initial_radius"] <= 10000
            or not 0 <= values["concentration"] <= 2
            or any(not 0 <= values[k] <= 200 for k in ["rate0", "rate2"])
            or any(not 0 <= values[k] <= 24 for k in ["lag0", "lag2"])
            or convert(plan.duration, "hour") > 72
        ):
            return ("Require bounded kinetics, SDP dose0..2percent and duration at most72h",)
        return ()

    def execute(
        self, model: ModelPackage, plan: SimulationPlan, checkpoint: Callable[[], None]
    ) -> SimulationResult:
        errors = self.validate(model, plan)
        if errors:
            raise ValueError("; ".join(errors))
        values = self.parameters(model, plan)
        dose = values["concentration"] / 2
        rate = (1 - dose) * values["rate0"] + dose * values["rate2"]
        lag = (1 - dose) * values["lag0"] + dose * values["lag2"]
        initial = values["initial_radius"]
        times = time_grid(plan)
        radii, fractions, areas = [], [], []
        for second in times:
            checkpoint()
            radius = max(0.0, initial - rate * max(0.0, second / 3600 - lag))
            radii.append(radius)
            areas.append(math.pi * radius**2)
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
                "Empirical endpoint kinetics; linear SDP dose interpolation is an assumption.",
                "Observed post-injury area and study identity are not prediction inputs.",
                "Deterministic cohort parameters do not create independent animal replicates.",
                "Circular projection; unmeasured substrate, care and injury effects unresolved.",
                "No cell-layer, chemical, barrier, causal treatment or clinical validation.",
                "Calibration and all later evidence must remain separately identified.",
            ),
        )
