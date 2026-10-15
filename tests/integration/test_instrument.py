import pytest

from peykar.integrations.scientific_instrument import ScientificInstrument
from peykar.simulation.worker import Worker
from peykar_core.contracts import Quantity
from peykar_core.errors import Conflict


def test_external_instrument_lifecycle(service, plan):
    instrument = ScientificInstrument(service)
    assert instrument.describe()["id"] == "peykar"
    assert instrument.validate(plan) == ()
    assert instrument.estimate_cost(plan)["samples"] == 11
    invalid = plan.model_copy(update={"duration": Quantity(value=-1, unit="s")})
    assert instrument.validate(invalid)
    submitted = instrument.submit(plan, "instrument-request")
    assert instrument.status(submitted.id).status == "queued"
    with pytest.raises(Conflict):
        instrument.collect(submitted.id)
    Worker(service).tick()
    assert instrument.collect(submitted.id).origin == "simulation"
    another = instrument.submit(plan)
    assert instrument.cancel(another.id).status == "cancelled"
