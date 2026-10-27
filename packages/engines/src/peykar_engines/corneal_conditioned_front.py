"""Optional post-injury-conditioned corneal projected-front forecast.

A baseline and a later observed area define an effective interval-average radius
speed. Forecast time zero is the later observation, not the time of injury.
This is a distinct empirical forecast, never a validated initial-state simulator.
"""

import math
from collections.abc import Callable

from peykar_core.contracts import ModelPackage, Quantity, Series, SimulationPlan, SimulationResult
from peykar_core.planning import parameter_values, time_grid, validate_plan
from peykar_core.units import convert


class CornealConditionedFrontEngine:
    id = "cornea.conditioned_front"
    version = "1.0.0"

    @staticmethod
    def parameters(model: ModelPackage, plan: SimulationPlan) -> dict[str, float]:
        units = {
            "baseline_area": "millimeter ** 2",
            "anchor_area": "millimeter ** 2",
            "anchor_time": "hour",
        }
        values = parameter_values(model, plan)
        return {
            spec.name: convert(
                Quantity(value=values[spec.name], unit=spec.default.unit), units[spec.name]
            )
            for spec in model.parameters
        }

    def validate(self, model: ModelPackage, plan: SimulationPlan) -> tuple[str, ...]:
        if (model.engine, model.engine_version) != (self.id, self.version):
            return ("Conditioned corneal forecast requires its exact reviewed engine identity",)
        if model.context.species != "Oryctolagus cuniculus":
            return ("This forecast's current applicability is rabbit corneal debridement",)
        if "mechanical epithelial debridement" not in model.context.conditions:
            return ("Chemical or unspecified injury is not the reviewed mechanical context",)
        if len(model.parameters) != 3 or {p.name for p in model.parameters} != {
            "baseline_area",
            "anchor_area",
            "anchor_time",
        }:
            return ("Conditioned forecast requires baseline_area, anchor_area and anchor_time",)
        if plan.interventions:
            return ("Time-varying interventions require a separately reviewed forecast model",)
        validate_plan(model, plan)
        values = self.parameters(model, plan)
        baseline, anchor, at = (
            values["baseline_area"],
            values["anchor_area"],
            values["anchor_time"],
        )
        if (
            not 0 < baseline <= 400
            or not 0 <= anchor <= baseline
            or not 0 < at <= 96
            or at + convert(plan.duration, "hour") > 168
        ):
            return (
                "Require baseline area in(0,400]mm², anchor area in[0,baseline], "
                "anchor time in(0,96]h and absolute horizon at most168h; expansion is unsupported",
            )
        speed = ((math.sqrt(baseline) - math.sqrt(anchor)) / math.sqrt(math.pi)) / at
        if not math.isfinite(speed):
            return ("Conditioning observations imply a nonfinite interval-average radius speed",)
        return ()

    def execute(
        self, model: ModelPackage, plan: SimulationPlan, checkpoint: Callable[[], None]
    ) -> SimulationResult:
        errors = self.validate(model, plan)
        if errors:
            raise ValueError("; ".join(errors))
        values = self.parameters(model, plan)
        baseline = values["baseline_area"]
        anchor = values["anchor_area"]
        initial_radius_mm = math.sqrt(baseline) / math.sqrt(math.pi)
        anchor_radius_mm = math.sqrt(anchor) / math.sqrt(math.pi)
        speed_mm_per_hour = (initial_radius_mm - anchor_radius_mm) / values["anchor_time"]
        times = time_grid(plan)
        radii_um = []
        areas_mm2 = []
        fractions = []
        for second in times:
            checkpoint()
            radius_mm = max(0.0, anchor_radius_mm - speed_mm_per_hour * (second / 3600))
            # This equivalent area formula preserves the measured anchor exactly
            # and prevents roundoff from creating area above it when speed is zero.
            remaining_radius_fraction = radius_mm / anchor_radius_mm if anchor_radius_mm else 0.0
            area_mm2 = anchor * remaining_radius_fraction**2
            radii_um.append(radius_mm * 1000)
            areas_mm2.append(area_mm2)
            fractions.append(area_mm2 / baseline)
        return SimulationResult(
            time=Series(name="time", unit="second", values=times),
            outputs=(
                Series(
                    name="time_since_injury",
                    unit="hour",
                    values=tuple(values["anchor_time"] + second / 3600 for second in times),
                ),
                Series(name="projected_radius", unit="micrometer", values=tuple(radii_um)),
                Series(name="projected_area", unit="millimeter ** 2", values=tuple(areas_mm2)),
                Series(name="open_area_fraction", unit="dimensionless", values=tuple(fractions)),
                Series(
                    name="closure_fraction",
                    unit="dimensionless",
                    values=tuple(1 - value for value in fractions),
                ),
            ),
            seed=plan.seed,
            limitations=(
                "Requires a post-injury anchor observation; "
                "not a from-injury initial-state prediction.",
                "Time is forecast elapsed time from the anchor, not absolute time from injury.",
                "Area normalization always uses the original baseline area, not the anchor area.",
                "Equivalent cohort areas imply equivalent radii, not mean individual-animal radii.",
                "Effective early-interval average speed is not "
                "measured instantaneous cell velocity.",
                "No inferred lag, biological replicates, uncertainty or causal drug effect.",
                "Circular planar projected area; no modality conversion or curvature correction.",
                "No cell-layer, chemical injury, barrier "
                "or universal biological/clinical validity.",
            ),
        )
