"""Native model/job HTTP boundary; Docker remains exclusively in the operator agent."""

import base64
import binascii
from collections.abc import Callable
from typing import Annotated, Literal

from fastapi import APIRouter, Depends, Header, Query, Response

from peykar.model_import.native_models import NativeModelService
from peykar.simulation.native_jobs import NativeJobService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    Contract,
    NativeAgentClaim,
    NativeExecutionProfile,
    NativeModelPackage,
    NativeModelSpec,
    NativeRunManifest,
    NativeRunRecord,
    NativeRunResult,
    NativeRunSpec,
    Reference,
)
from peykar_core.errors import InvalidRequest, NotFound


class NativeModelImport(Contract):
    spec: NativeModelSpec
    xml_text: str


class NativeClaimRequest(Contract):
    profiles: tuple[Reference, ...]


class NativeCompletionRequest(Contract):
    status: Literal["succeeded", "failed", "cancelled"]
    result: NativeRunResult | None = None
    blobs_base64: dict[str, str] = {}


def install_native_routes(
    read: APIRouter, write: APIRouter, service: Callable[..., SimulationService]
) -> None:
    @write.post("/native-models", status_code=201, tags=["native models"])
    def admit_model(
        body: NativeModelImport,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> NativeModelPackage:
        result = NativeModelService(svc.db, svc.artifacts).admit(body.spec, body.xml_text.encode())
        response.headers["Location"] = (
            f"/v1/native-models/{body.spec.id}/versions/{body.spec.version}"
        )
        return result

    @read.get("/native-models", tags=["native models"])
    def models(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=200)] = 200,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[NativeModelPackage, ...]:
        return NativeModelService(svc.db, svc.artifacts).discover(limit, offset)

    @read.get("/native-models/{id}/versions/{version}", tags=["native models"])
    def model(
        id: str, version: str, svc: Annotated[SimulationService, Depends(service)]
    ) -> NativeModelPackage:
        return NativeModelService(svc.db, svc.artifacts).get(Reference(id=id, version=version))

    @read.get("/native-models/{id}/versions/{version}/content", tags=["native models"])
    def model_content(
        id: str, version: str, svc: Annotated[SimulationService, Depends(service)]
    ) -> Response:
        model = NativeModelService(svc.db, svc.artifacts).get(Reference(id=id, version=version))
        return Response(
            svc.artifacts.get(model.content_digest),
            media_type="application/octet-stream",
            headers={
                "Content-Disposition": 'attachment; filename="model.xml"',
                "X-Content-Type-Options": "nosniff",
            },
        )

    @read.get("/native-profiles", tags=["native models"])
    def profiles(
        svc: Annotated[SimulationService, Depends(service)],
    ) -> tuple[NativeExecutionProfile, ...]:
        return NativeModelService(svc.db, svc.artifacts).profiles()

    @write.post("/native-runs", status_code=201, tags=["native simulations"])
    def submit(
        body: NativeRunSpec,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
        idempotency_key: Annotated[str | None, Header()] = None,
    ) -> NativeRunRecord:
        result = NativeJobService(svc).submit(body, idempotency_key)
        response.headers["Location"] = f"/v1/native-runs/{result.id}"
        return result

    @write.post("/native-runs/claim", tags=["native agent"])
    def claim(
        body: NativeClaimRequest, svc: Annotated[SimulationService, Depends(service)]
    ) -> NativeAgentClaim | None:
        return NativeJobService(svc).claim(body.profiles)

    @write.post("/native-runs/recover", tags=["native agent"])
    def recover(svc: Annotated[SimulationService, Depends(service)]) -> dict[str, int]:
        return {"failed": NativeJobService(svc).recover_stale()}

    @read.get("/native-runs", tags=["native simulations"])
    def runs(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=200)] = 100,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[NativeRunRecord, ...]:
        return NativeJobService(svc).runs(limit, offset)

    @read.get("/native-runs/{id}", tags=["native simulations"])
    def run(id: str, svc: Annotated[SimulationService, Depends(service)]) -> NativeRunRecord:
        return NativeJobService(svc).run(id)

    @read.get("/native-runs/{id}/manifest", tags=["native simulations"])
    def manifest(id: str, svc: Annotated[SimulationService, Depends(service)]) -> NativeRunManifest:
        return NativeJobService(svc).manifest(id)

    @read.get("/native-runs/{id}/results", tags=["native simulations"])
    def results(id: str, svc: Annotated[SimulationService, Depends(service)]) -> NativeRunResult:
        return NativeJobService(svc).result(id)

    @read.get("/native-runs/{id}/artifacts/{name:path}", tags=["native simulations"])
    def artifact(
        id: str, name: str, svc: Annotated[SimulationService, Depends(service)]
    ) -> Response:
        result = NativeJobService(svc).result(id)
        if name not in result.artifacts:
            raise NotFound("Native result artifact not found")
        return Response(
            svc.artifacts.get(result.artifacts[name]),
            media_type="application/octet-stream",
            headers={"Content-Disposition": "attachment", "X-Content-Type-Options": "nosniff"},
        )

    @read.get("/native-runs/{id}/events", tags=["native simulations"])
    def events(
        id: str, svc: Annotated[SimulationService, Depends(service)]
    ) -> list[dict[str, object]]:
        return NativeJobService(svc).events(id)

    @write.post("/native-runs/{id}/cancel", tags=["native simulations"])
    def cancel(id: str, svc: Annotated[SimulationService, Depends(service)]) -> NativeRunRecord:
        return NativeJobService(svc).cancel(id)

    @write.post("/native-runs/{id}/heartbeat", tags=["native agent"])
    def heartbeat(
        id: str,
        svc: Annotated[SimulationService, Depends(service)],
        token: Annotated[str, Header(alias="Native-Claim-Token")],
    ) -> dict[str, bool]:
        return {"cancel_requested": NativeJobService(svc).heartbeat(id, token)}

    @write.post("/native-runs/{id}/complete", tags=["native agent"])
    def complete(
        id: str,
        body: NativeCompletionRequest,
        svc: Annotated[SimulationService, Depends(service)],
        token: Annotated[str, Header(alias="Native-Claim-Token")],
    ) -> NativeRunRecord:
        try:
            blobs = {
                name: base64.b64decode(value, validate=True)
                for name, value in body.blobs_base64.items()
            }
        except (ValueError, binascii.Error) as error:
            raise InvalidRequest("Native artifacts require valid base64") from error
        return NativeJobService(svc).finish(id, token, body.status, body.result, blobs)
