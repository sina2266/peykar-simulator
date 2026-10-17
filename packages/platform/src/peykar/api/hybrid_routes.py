"""Native interfaces and explicit reference/native scalar workflow HTTP boundary."""

import json
from collections.abc import Callable
from typing import Annotated

from fastapi import APIRouter, Depends, Header, Query, Response

from peykar.composition.hybrid_flows import HybridFlowService
from peykar.model_import.native_interfaces import NativeInterfaceService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    FlowTransfer,
    FrozenHybridFlow,
    HybridFlowRunRecord,
    HybridFlowSpec,
    Identifier,
    NativeDerivationReceipt,
    NativeInterfaceSpec,
    Reference,
    Version,
)
from peykar_core.errors import Conflict
from peykar_core.hashing import digest


def install_hybrid_routes(
    read: APIRouter, write: APIRouter, service: Callable[..., SimulationService]
) -> None:
    @write.post("/native-interfaces", status_code=201, tags=["hybrid flows"])
    def interface(
        body: NativeInterfaceSpec,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> dict[str, str]:
        checksum = NativeInterfaceService(svc).register(body)
        response.headers["Location"] = f"/v1/native-interfaces/{body.id}/versions/{body.version}"
        return {"digest": checksum}

    @read.get("/native-interfaces", tags=["hybrid flows"])
    def interfaces(
        svc: Annotated[SimulationService, Depends(service)],
    ) -> tuple[NativeInterfaceSpec, ...]:
        return tuple(
            NativeInterfaceSpec.model_validate_json(r["payload"])
            for r in svc.db.objects("native_interface")
        )

    @read.get("/native-interfaces/{id}/versions/{version}", tags=["hybrid flows"])
    def get_interface(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> NativeInterfaceSpec:
        return NativeInterfaceService(svc).get(Reference(id=id, version=version))

    @read.get("/native-derivations/{id}/versions/{version}", tags=["hybrid flows"])
    def derivation(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> NativeDerivationReceipt:
        receipt = NativeDerivationReceipt.model_validate_json(
            svc.db.object("native_derivation", id, version)
        )
        if id != "derived." + digest(receipt):
            raise Conflict("Derivation identity differs")
        svc.artifacts.get(receipt.child_content_digest)
        return receipt

    @write.post("/hybrid-flows", status_code=201, tags=["hybrid flows"])
    def freeze(
        body: HybridFlowSpec,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> FrozenHybridFlow:
        result = HybridFlowService(svc).freeze(body)
        response.headers["Location"] = f"/v1/hybrid-flows/{body.id}/versions/{body.version}"
        return result

    @read.get("/hybrid-flows", tags=["hybrid flows"])
    def flows(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> list[dict[str, object]]:
        return [
            {
                "id": r["id"],
                "version": r["version"],
                "digest": r["digest"],
                "title": json.loads(r["payload"])["spec"]["title"],
            }
            for r in svc.db.objects("hybrid_flow", limit, offset)
        ]

    @read.get("/hybrid-flows/{id}/versions/{version}", tags=["hybrid flows"])
    def get(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> FrozenHybridFlow:
        return HybridFlowService(svc).get(Reference(id=id, version=version))

    @write.post("/hybrid-runs", status_code=202, tags=["hybrid flows"])
    def submit(
        body: Reference,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
        idempotency_key: Annotated[str | None, Header(max_length=128)] = None,
    ) -> HybridFlowRunRecord:
        result = HybridFlowService(svc).submit(body, idempotency_key)
        response.headers["Location"] = "/v1/hybrid-runs/" + result.id
        response.headers["Retry-After"] = "1"
        return result

    @read.get("/hybrid-runs/{id}", tags=["hybrid flows"])
    def record(id: str, svc: Annotated[SimulationService, Depends(service)]) -> HybridFlowRunRecord:
        return HybridFlowService(svc).record(id)

    @write.post("/hybrid-runs/{id}/cancel", tags=["hybrid flows"])
    def cancel(id: str, svc: Annotated[SimulationService, Depends(service)]) -> HybridFlowRunRecord:
        return HybridFlowService(svc).cancel(id)

    @read.get("/hybrid-runs/{id}/nodes/{node_id}/transfers", tags=["hybrid flows"])
    def transfers(
        id: str, node_id: Identifier, svc: Annotated[SimulationService, Depends(service)]
    ) -> tuple[FlowTransfer, ...]:
        return HybridFlowService(svc).transfers(id, node_id)

    @read.get("/hybrid-runs/{id}/events", tags=["hybrid flows"])
    def events(
        id: str, svc: Annotated[SimulationService, Depends(service)]
    ) -> list[dict[str, object]]:
        return HybridFlowService(svc).events(id)

    @read.get("/hybrid-runs/{id}/results", tags=["hybrid flows"])
    def results(id: str, svc: Annotated[SimulationService, Depends(service)]) -> dict[str, object]:
        return HybridFlowService(svc).results(id)
