"""Shared platform services; source acquisition remains a local operator action."""

from collections.abc import Callable
from typing import Annotated

from fastapi import APIRouter, Depends, Header, Query, Response

from peykar.biology.persistent_graph import GraphService
from peykar.composition.experiments import ExperimentService
from peykar.model_import.libraries import LibraryService
from peykar.model_import.native_runtimes import NativeRuntimeService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    BiologicalGraphSpec,
    ExperimentRunRecord,
    ExperimentSpec,
    FrozenExperiment,
    Identifier,
    LibrarySourceReceipt,
    NativeRuntimeEvidence,
    Reference,
    Version,
)


def install_platform_routes(
    read: APIRouter, write: APIRouter, service: Callable[..., SimulationService]
) -> None:
    @read.get("/libraries", tags=["libraries"])
    def libraries(
        svc: Annotated[SimulationService, Depends(service)], capability: str | None = None
    ) -> tuple[LibrarySourceReceipt, ...]:
        return LibraryService(svc.db, svc.artifacts).discover(capability)

    @read.get("/libraries/{id}/versions/{version}", tags=["libraries"])
    def library(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> LibrarySourceReceipt:
        return LibraryService(svc.db, svc.artifacts).get(Reference(id=id, version=version))

    @read.get("/runtimes", tags=["runtimes"])
    def runtimes(
        svc: Annotated[SimulationService, Depends(service)], engine: str | None = None
    ) -> tuple[NativeRuntimeEvidence, ...]:
        return NativeRuntimeService(svc.db, svc.artifacts).discover(engine)

    @read.get("/runtimes/{id}/versions/{version}", tags=["runtimes"])
    def runtime(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> NativeRuntimeEvidence:
        return NativeRuntimeService(svc.db, svc.artifacts).get(Reference(id=id, version=version))

    @write.post("/biology/graphs", status_code=201, tags=["biology"])
    def register_graph(
        body: BiologicalGraphSpec,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> dict[str, str]:
        checksum = GraphService(svc.db).register(body)
        response.headers["Location"] = f"/v1/biology/graphs/{body.id}/versions/{body.version}"
        return {"digest": checksum}

    @read.get("/biology/graphs", tags=["biology"])
    def graphs(
        svc: Annotated[SimulationService, Depends(service)], species: str | None = None
    ) -> tuple[BiologicalGraphSpec, ...]:
        return GraphService(svc.db).discover(species)

    @read.get("/biology/graphs/{id}/versions/{version}", tags=["biology"])
    def graph(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> BiologicalGraphSpec:
        return GraphService(svc.db).get(Reference(id=id, version=version))

    @read.get("/biology/graphs/{id}/versions/{version}/neighborhood", tags=["biology"])
    def neighbors(
        id: Identifier,
        version: Version,
        entity_id: str,
        svc: Annotated[SimulationService, Depends(service)],
        depth: Annotated[int, Query(ge=1, le=4)] = 1,
        max_nodes: Annotated[int, Query(ge=1, le=500)] = 100,
    ) -> dict[str, object]:
        return GraphService(svc.db).neighborhood(
            Reference(id=id, version=version), entity_id, depth, max_nodes
        )

    @write.post("/experiments", status_code=201, tags=["experiments"])
    def freeze_experiment(
        body: ExperimentSpec,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> FrozenExperiment:
        result = ExperimentService(svc).freeze(body)
        response.headers["Location"] = f"/v1/experiments/{body.id}/versions/{body.version}"
        return result

    @read.get("/experiments", tags=["experiments"])
    def experiments(
        svc: Annotated[SimulationService, Depends(service)],
    ) -> tuple[FrozenExperiment, ...]:
        return ExperimentService(svc).discover()

    @read.get("/experiments/{id}/versions/{version}", tags=["experiments"])
    def experiment(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> FrozenExperiment:
        return ExperimentService(svc).get(Reference(id=id, version=version))

    @write.post("/experiment-runs", status_code=202, tags=["experiments"])
    def submit_experiment(
        body: Reference,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
        idempotency_key: Annotated[str | None, Header(max_length=128)] = None,
    ) -> ExperimentRunRecord:
        result = ExperimentService(svc).submit(body, idempotency_key)
        response.headers["Location"] = f"/v1/experiment-runs/{result.id}"
        return result

    @read.get("/experiment-runs/{id}", tags=["experiments"])
    def experiment_run(
        id: str, svc: Annotated[SimulationService, Depends(service)]
    ) -> ExperimentRunRecord:
        return ExperimentService(svc).record(id)

    @read.get("/experiment-runs/{id}/results", tags=["experiments"])
    def experiment_results(
        id: str, svc: Annotated[SimulationService, Depends(service)]
    ) -> dict[str, object]:
        return ExperimentService(svc).results(id)

    @read.get("/experiment-runs/{id}/events", tags=["experiments"])
    def experiment_events(
        id: str, svc: Annotated[SimulationService, Depends(service)]
    ) -> list[dict[str, object]]:
        return ExperimentService(svc).events(id)

    @write.post("/experiment-runs/{id}/cancel", tags=["experiments"])
    def cancel_experiment(
        id: str, svc: Annotated[SimulationService, Depends(service)]
    ) -> ExperimentRunRecord:
        return ExperimentService(svc).cancel(id)
