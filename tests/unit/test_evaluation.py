import pytest

from peykar.examples import analytical_dataset
from peykar.simulation.worker import Worker
from peykar.validation.evaluation import evaluate
from peykar_core.contracts import Quantity, Series
from peykar_core.errors import InvalidRequest


@pytest.mark.parametrize(
    "change", ["time_grid", "value_count", "species", "unit", "negative_tolerance"]
)
def test_observation_mapping_constraints(service, plan, change):
    record = service.submit(plan)
    Worker(service).tick()
    spec, obs = analytical_dataset()
    tol = Quantity(value=0.01, unit="mole/liter")
    if change == "time_grid":
        obs = obs.model_copy(update={"time": Series(name="time", unit="s", values=(0, 1))})
    elif change == "value_count":
        obs = obs.model_copy(
            update={"values": Series(name="concentration", unit="mole/liter", values=(1, 2))}
        )
    elif change == "species":
        spec = spec.model_copy(
            update={"context": spec.context.model_copy(update={"species": "human"})}
        )
    elif change == "unit":
        obs = obs.model_copy(update={"values": obs.values.model_copy(update={"unit": "kg"})})
    else:
        tol = Quantity(value=-1, unit="mole/liter")
    with pytest.raises(InvalidRequest):
        evaluate(service.db.manifest(record.id), service.result(record.id), spec, obs, tol)


def test_equivalent_observation_units(service, plan):
    record = service.submit(plan)
    Worker(service).tick()
    spec, obs = analytical_dataset()
    obs = obs.model_copy(
        update={
            "values": Series(
                name="concentration",
                unit="millimole/liter",
                values=tuple(v * 1000 for v in obs.values.values),
            )
        }
    )
    report = evaluate(
        service.db.manifest(record.id),
        service.result(record.id),
        spec,
        obs,
        Quantity(value=1e-9, unit="millimole/liter"),
    )
    assert report.passed and report.evidence_class == "synthetic_verification"
