from pathlib import Path

import pytest

from peykar.examples import bootstrap, reference_model, reference_plan
from peykar.simulation.service import SimulationService
from peykar_core.contracts import ModelPackage, SimulationPlan


@pytest.fixture
def service(tmp_path: Path) -> SimulationService:
    value = SimulationService(tmp_path / "state")
    bootstrap(value)
    return value


@pytest.fixture
def model() -> ModelPackage:
    return reference_model()


@pytest.fixture
def plan() -> SimulationPlan:
    return reference_plan()
