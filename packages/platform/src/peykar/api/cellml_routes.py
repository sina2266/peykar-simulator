"""CellML declarative writes and retained results; execution stays operator-only."""

from collections.abc import Callable
from typing import Annotated

from fastapi import APIRouter, Depends, Query, Response

from peykar.model_import.cellml import CellMLService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    CellMLExecutionReceipt,
    CellMLModelPackage,
    CellMLModelSpec,
    Contract,
    Identifier,
    Reference,
    Version,
)
from peykar_core.errors import NotFound


class CellMLAdmission(Contract):
    spec: CellMLModelSpec
    xml_text: str


def install_cellml_routes(
    read: APIRouter, write: APIRouter, service: Callable[..., SimulationService]
) -> None:
    def models(svc: SimulationService) -> CellMLService:
        return CellMLService(svc.db, svc.artifacts)

    @write.post("/cellml-models", status_code=201, tags=["CellML"])
    def admit(
        body: CellMLAdmission,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> CellMLModelPackage:
        value = models(svc).admit(body.spec, body.xml_text.encode("utf-8"))
        response.headers["Location"] = (
            f"/v1/cellml-models/{body.spec.id}/versions/{body.spec.version}"
        )
        return value

    @read.get("/cellml-models", tags=["CellML"])
    def discover(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[CellMLModelPackage, ...]:
        return models(svc).discover(limit, offset)

    @read.get("/cellml-models/{id}/versions/{version}", tags=["CellML"])
    def get(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> CellMLModelPackage:
        return models(svc).get(Reference(id=id, version=version))

    @read.get("/cellml-models/{id}/versions/{version}/content", tags=["CellML"])
    def content(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> Response:
        package = models(svc).get(Reference(id=id, version=version))
        return Response(
            svc.artifacts.get(package.content_digest),
            media_type="application/octet-stream",
            headers={
                "Content-Disposition": 'attachment; filename="model.cellml"',
                "X-Content-Type-Options": "nosniff",
            },
        )

    @read.get("/cellml-executions", tags=["CellML"])
    def executions(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[CellMLExecutionReceipt, ...]:
        return models(svc).executions(limit, offset)

    @read.get("/cellml-executions/{id}/versions/{version}", tags=["CellML"])
    def execution(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> CellMLExecutionReceipt:
        return models(svc).execution(Reference(id=id, version=version))

    @read.get("/cellml-executions/{id}/versions/{version}/artifacts/{name}", tags=["CellML"])
    def artifact(
        id: Identifier,
        version: Version,
        name: str,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> Response:
        receipt = models(svc).execution(Reference(id=id, version=version))
        if name not in receipt.artifacts:
            raise NotFound("CellML execution artifact not found")
        return Response(
            svc.artifacts.get(receipt.artifacts[name]),
            media_type="application/octet-stream",
            headers={"Content-Disposition": "attachment", "X-Content-Type-Options": "nosniff"},
        )
