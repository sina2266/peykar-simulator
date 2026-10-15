"""HomBon-compatible lifecycle facade; no HomBon dependency or domain logic."""

from peykar import __version__
from peykar.simulation.service import SimulationService
from peykar_core.contracts import RunRecord, SimulationPlan, SimulationResult
from peykar_core.errors import PeykarError


class ScientificInstrument:
    def __init__(self, service: SimulationService) -> None:
        self.service = service

    def describe(self) -> dict[str, object]:
        return {
            "id": "peykar",
            "type": "computational_simulator",
            "version": __version__,
            "asynchronous": True,
            "engines": self.service.engines.describe(),
            "default_validity": "unvalidated",
            "input_schema": "SimulationPlan-1.0.0.json",
        }

    def validate(self, plan: SimulationPlan) -> tuple[str, ...]:
        try:
            self.service.validate(plan)
        except PeykarError as exc:
            return (str(exc),)
        return ()

    def estimate_cost(self, plan: SimulationPlan) -> dict[str, int | float | str]:
        return self.service.estimate(plan)

    def submit(self, plan: SimulationPlan, idempotency_key: str | None = None) -> RunRecord:
        return self.service.submit(plan, idempotency_key)

    def status(self, run_id: str) -> RunRecord:
        return self.service.db.run(run_id)

    def collect(self, run_id: str) -> SimulationResult:
        return self.service.result(run_id)

    def cancel(self, run_id: str) -> RunRecord:
        return self.service.db.cancel(run_id)
