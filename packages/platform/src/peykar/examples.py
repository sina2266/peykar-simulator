"""Self-contained synthetic fixtures, not vCornea approximations."""

import math
from pathlib import Path

from peykar.data_fabric.service import DatasetService
from peykar.simulation.service import SimulationService
from peykar.simulation.worker import Worker
from peykar_core.contracts import (
    DatasetSpec,
    ModelPackage,
    Observation,
    ParameterSpec,
    Quantity,
    Reference,
    ScientificContext,
    Series,
    SimulationPlan,
    ValidityProfile,
)
from peykar_core.hashing import bytes_digest, canonical_json

SYNTHETIC = ScientificContext(species="synthetic", biological_system="reference verification")


def reference_model(kind: str = "decay") -> ModelPackage:
    from peykar_engines import reference

    source = Path(reference.__file__).read_bytes()
    parameters: tuple[ParameterSpec, ...]
    if kind == "decay":
        parameters = (
            ParameterSpec(
                name="initial", default=Quantity(value=1.0, unit="mole / liter"), minimum=0
            ),
            ParameterSpec(
                name="rate", default=Quantity(value=0.1, unit="1 / second"), minimum=0, maximum=10
            ),
        )
    elif kind == "logistic":
        parameters = tuple(
            ParameterSpec(
                name=name,
                default=Quantity(value=value, unit=unit),
                minimum=minimum,
                maximum=maximum,
            )
            for name, value, unit, minimum, maximum in (
                ("initial", 20, "dimensionless", 0, 10000),
                ("capacity", 200, "dimensionless", 1, 10000),
                ("birth_rate", 0.2, "1/second", 0, 10),
                ("death_rate", 0.01, "1/second", 0, 10),
            )
        )
    else:
        raise ValueError("Unknown reference model")
    return ModelPackage(
        id=f"reference.{kind}",
        version="0.3.0",
        title=f"Synthetic {kind} verification model",
        engine=f"reference.{kind}",
        engine_version="0.1.0",
        license="LicenseRef-Peykar-Noncommercial-1.1.0",
        source="python:peykar_engines.reference",
        source_digest=bytes_digest(source),
        capabilities=("synthetic_verification",),
        context=SYNTHETIC,
        validity=ValidityProfile(
            context=SYNTHETIC, limitations=("Synthetic only. No biological validation.",)
        ),
        parameters=parameters,
    )


def reference_plan(kind: str = "decay", seed: int = 42) -> SimulationPlan:
    return SimulationPlan(
        model=Reference(id=f"reference.{kind}", version="0.3.0"),
        context=SYNTHETIC,
        duration=Quantity(value=10.0, unit="second"),
        sample_interval=Quantity(value=1.0, unit="second"),
        seed=seed,
        purpose="Synthetic infrastructure verification",
    )


def analytical_dataset() -> tuple[DatasetSpec, Observation]:
    observation = Observation(
        dataset=Reference(id="decay.analytic", version="0.3.0"),
        endpoint="concentration",
        time=Series(name="time", unit="second", values=tuple(float(i) for i in range(11))),
        values=Series(
            name="concentration",
            unit="mole / liter",
            values=tuple(math.exp(-0.1 * i) for i in range(11)),
        ),
    )
    spec = DatasetSpec(
        id=observation.dataset.id,
        version=observation.dataset.version,
        title="Analytical exponential decay reference",
        origin="synthetic",
        role="benchmark",
        context=SYNTHETIC,
        artifact_digest=bytes_digest(canonical_json(observation).encode()),
        license="LicenseRef-Peykar-Noncommercial-1.1.0",
        source="Analytical exp(-0.1*t); independent of engine recurrence",
        independence_group="analytic-decay-v1",
    )
    return spec, observation


def bootstrap(service: SimulationService) -> None:
    for kind in ("decay", "logistic"):
        service.models.register(reference_model(kind))
    spec, observation = analytical_dataset()
    DatasetService(service.db, service.artifacts).register(spec, observation)


def demo(service: SimulationService) -> dict[str, object]:
    from peykar.validation.evaluation import evaluate

    bootstrap(service)
    submitted = service.submit(reference_plan())
    worker = Worker(service)
    # There may be unrelated queued jobs; wait for this submitted job without assuming queue order.
    while service.db.run(submitted.id).status in ("queued", "running"):
        if worker.tick() is None:
            break
    result = service.result(submitted.id)
    spec, observation = analytical_dataset()
    report = evaluate(
        service.db.manifest(submitted.id),
        result,
        spec,
        observation,
        Quantity(value=1e-12, unit="mole/liter"),
    )
    return {
        "run": service.db.run(submitted.id).model_dump(mode="json"),
        "verification": report.model_dump(mode="json"),
    }
