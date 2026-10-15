import math

import pytest

from peykar.biology.graph import BiologicalGraph
from peykar.data_fabric.service import DatasetService
from peykar.examples import SYNTHETIC, analytical_dataset
from peykar.model_import.inspection import inspect_package
from peykar.model_learning.calibration import estimate_decay_rate
from peykar.simulation.worker import Worker
from peykar.validation.evaluation import evaluate
from peykar_core.contracts import (
    BiologicalEntity,
    BiologicalRelationship,
    Observation,
    Quantity,
    Reference,
    Scenario,
    Series,
)
from peykar_core.errors import InvalidRequest, NotFound


def test_model_import_never_executes_or_fetches(service, model):
    report = inspect_package(model.model_dump_json().encode(), service.engines)
    assert report["engine_available"] and not report["source_fetched"]
    unsupported = model.model_copy(update={"engine": "unknown"})
    assert not inspect_package(unsupported.model_dump_json().encode(), service.engines)[
        "engine_available"
    ]


def test_dataset_digest_required(service):
    spec, obs = analytical_dataset()
    with pytest.raises(InvalidRequest):
        DatasetService(service.db, service.artifacts).register(
            spec.model_copy(update={"artifact_digest": "a" * 64}), obs
        )


def test_scenario_branch_history(service, plan):
    parent = Scenario(id="baseline", version="0.1.0", plan=plan, rationale="baseline")
    service.register_scenario(parent)
    child = Scenario(
        id="candidate",
        version="0.1.0",
        plan=plan.model_copy(update={"seed": 43}),
        parent=Reference(id=parent.id, version=parent.version),
        rationale="new seed",
    )
    service.register_scenario(child)
    assert parent.plan.seed == 42 and child.plan.seed == 43
    with pytest.raises(NotFound):
        service.register_scenario(
            child.model_copy(update={"parent": Reference(id="missing", version="0.1.0")})
        )


def test_dependency_pin_and_cycle(service, model):
    derived = model.model_copy(
        update={"id": "derived", "dependencies": (Reference(id=model.id, version=model.version),)}
    )
    service.models.register(derived)
    assert service.models.dependencies(derived) == (model,)
    with pytest.raises(InvalidRequest):
        service.models.register(
            model.model_copy(
                update={
                    "id": "cyclic",
                    "dependencies": (Reference(id="cyclic", version=model.version),),
                }
            )
        )


def test_calibration_is_candidate_only():
    spec, _ = analytical_dataset()
    spec = spec.model_copy(update={"role": "calibration"})
    obs = Observation(
        dataset=Reference(id=spec.id, version=spec.version),
        endpoint="concentration",
        time=Series(name="time", unit="s", values=(0, 1, 2, 3)),
        values=Series(
            name="concentration",
            unit="mole/liter",
            values=tuple(2 * math.exp(-0.3 * t) for t in range(4)),
        ),
    )
    assert estimate_decay_rate(spec, obs).value == pytest.approx(0.3)
    with pytest.raises(InvalidRequest):
        estimate_decay_rate(spec.model_copy(update={"role": "validation"}), obs)


def test_holdout_leakage_rejected(service, plan):
    record = service.submit(plan)
    Worker(service).tick()
    spec, obs = analytical_dataset()
    args = (service.db.manifest(record.id), service.result(record.id))
    tolerance = Quantity(value=1e-12, unit="mole/liter")
    with pytest.raises(InvalidRequest):
        evaluate(*args, spec, obs, tolerance, frozenset({spec.independence_group}))
    with pytest.raises(InvalidRequest):
        evaluate(*args, spec.model_copy(update={"role": "calibration"}), obs, tolerance)
    with pytest.raises(InvalidRequest):
        evaluate(*args, spec, obs.model_copy(update={"endpoint": "different"}), tolerance)


def test_hierarchy_cycles_and_missing_evidence():
    graph = BiologicalGraph()
    for id in ("cell", "tissue"):
        graph.add_entity(BiologicalEntity(id=id, label=id, kind=id, species="synthetic"))
    edge = BiologicalRelationship(
        source="cell",
        target="tissue",
        relation="part_of",
        context=SYNTHETIC,
        evidence=("synthetic-example",),
    )
    graph.add_relationship(edge)
    with pytest.raises(InvalidRequest):
        graph.add_relationship(edge.model_copy(update={"source": "tissue", "target": "cell"}))
    with pytest.raises(InvalidRequest):
        graph.add_relationship(edge.model_copy(update={"evidence": ()}))
