"""Authenticated contextual process networks, ownership declarations and state frames."""

from collections.abc import Callable
from typing import Annotated, cast

from fastapi import APIRouter, Depends, Query, Response

from peykar.biology.processes import ProcessService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    BiologicalProcessNetwork,
    Identifier,
    MolecularStateFrame,
    MolecularStateFrameSpec,
    ProcessOwnershipPlan,
    ProcessOwnershipSpec,
    Reference,
    Version,
)


def install_process_routes(
    read: APIRouter, write: APIRouter, service: Callable[..., SimulationService]
) -> None:
    def processes(svc: SimulationService) -> ProcessService:
        return ProcessService(svc.db, svc.artifacts)

    @write.post("/biology/process-networks", status_code=201, tags=["processes"])
    def register(
        body: BiologicalProcessNetwork, svc: Annotated[SimulationService, Depends(service)]
    ) -> BiologicalProcessNetwork:
        return processes(svc).register(body)

    @read.get("/biology/process-networks", tags=["processes"])
    def discover(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[BiologicalProcessNetwork, ...]:
        return processes(svc).discover(limit, offset)

    @read.get("/biology/process-networks/{id}/versions/{version}", tags=["processes"])
    def get(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> BiologicalProcessNetwork:
        return processes(svc).get(Reference(id=id, version=version))

    @read.get("/biology/process-networks/{id}/versions/{version}/stoichiometry", tags=["processes"])
    def stoichiometry(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> dict[str, object]:
        return processes(svc).stoichiometry(Reference(id=id, version=version))

    @read.get(
        "/biology/process-networks/{id}/versions/{version}/sources/{source_id}", tags=["processes"]
    )
    def source(
        id: Identifier,
        version: Version,
        source_id: Identifier,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> Response:
        return Response(
            processes(svc).source(Reference(id=id, version=version), source_id),
            media_type="application/octet-stream",
            headers={"Content-Disposition": "attachment", "X-Content-Type-Options": "nosniff"},
        )

    @write.post("/biology/process-ownership", status_code=201, tags=["processes"])
    def own(
        body: ProcessOwnershipSpec, svc: Annotated[SimulationService, Depends(service)]
    ) -> ProcessOwnershipPlan:
        return processes(svc).own(body)

    @read.get("/biology/process-ownership", tags=["processes"])
    def plans(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[ProcessOwnershipPlan, ...]:
        return cast(
            tuple[ProcessOwnershipPlan, ...],
            processes(svc).records("process_ownership", limit, offset),
        )

    @read.get("/biology/process-ownership/{id}/versions/{version}", tags=["processes"])
    def plan(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> ProcessOwnershipPlan:
        return processes(svc).ownership(Reference(id=id, version=version))

    @write.post("/biology/state-frames", status_code=201, tags=["processes"])
    def frame(
        body: MolecularStateFrameSpec, svc: Annotated[SimulationService, Depends(service)]
    ) -> MolecularStateFrame:
        return processes(svc).frame(body)

    @read.get("/biology/state-frames", tags=["processes"])
    def frames(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[MolecularStateFrame, ...]:
        return cast(
            tuple[MolecularStateFrame, ...],
            processes(svc).records("molecular_frame", limit, offset),
        )

    @read.get("/biology/state-frames/{id}/versions/{version}", tags=["processes"])
    def get_frame(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> MolecularStateFrame:
        return processes(svc).get_frame(Reference(id=id, version=version))
