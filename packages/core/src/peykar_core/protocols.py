"""Dependency inversion boundaries. Implementations are trusted installed code."""

from collections.abc import Callable
from typing import Protocol

from peykar_core.contracts import ModelPackage, SimulationPlan, SimulationResult


class EngineAdapter(Protocol):
    id: str
    version: str

    def validate(self, model: ModelPackage, plan: SimulationPlan) -> tuple[str, ...]: ...

    def execute(
        self, model: ModelPackage, plan: SimulationPlan, checkpoint: Callable[[], None]
    ) -> SimulationResult: ...
