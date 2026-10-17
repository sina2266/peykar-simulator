"""Authenticated cohort admission, execution lifecycle and complete/partial reports."""

from collections.abc import Callable
from typing import Annotated

from fastapi import APIRouter, Depends, Header, Query, Response

from peykar.composition.batch_results import report
from peykar.composition.batches import BatchService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    BatchReport,
    BatchRunRecord,
    BatchSpec,
    FrozenBatch,
    Identifier,
    Reference,
    Version,
)
from peykar_core.hashing import digest


def install_batch_routes(
    read: APIRouter, write: APIRouter, service: Callable[..., SimulationService]
) -> None:
    @write.post("/batches", status_code=201, tags=["batches"])
    def freeze(
        body: BatchSpec, response: Response, svc: Annotated[SimulationService, Depends(service)]
    ) -> FrozenBatch:
        result = BatchService(svc).freeze(body)
        response.headers["Location"] = f"/v1/batches/{body.id}/versions/{body.version}"
        return result

    @read.get("/batches", tags=["batches"])
    def discover(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=200)] = 100,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[FrozenBatch, ...]:
        return BatchService(svc).discover(limit, offset)

    @read.get("/batches/{id}/versions/{version}", tags=["batches"])
    def get(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> FrozenBatch:
        return BatchService(svc).get(Reference(id=id, version=version))

    @write.post("/batch-runs", status_code=202, tags=["batches"])
    def submit(
        body: Reference,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
        idempotency_key: Annotated[str | None, Header(max_length=128)] = None,
    ) -> BatchRunRecord:
        result = BatchService(svc).submit(body, idempotency_key)
        response.headers["Location"] = "/v1/batch-runs/" + result.id
        response.headers["Retry-After"] = "1"
        return result

    @read.get("/batch-runs", tags=["batches"])
    def runs(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=200)] = 100,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[BatchRunRecord, ...]:
        return BatchService(svc).runs(limit, offset)

    @read.get("/batch-runs/{id}", tags=["batches"])
    def status(id: str, svc: Annotated[SimulationService, Depends(service)]) -> BatchRunRecord:
        return BatchService(svc).record(id)

    @write.post("/batch-runs/{id}/cancel", tags=["batches"])
    def cancel(id: str, svc: Annotated[SimulationService, Depends(service)]) -> BatchRunRecord:
        return BatchService(svc).cancel(id)

    @read.get("/batch-runs/{id}/events", tags=["batches"])
    def events(
        id: str, svc: Annotated[SimulationService, Depends(service)]
    ) -> list[dict[str, object]]:
        return BatchService(svc).events(id)

    @read.get("/batch-runs/{id}/results", tags=["batches"])
    def results(
        id: str, response: Response, svc: Annotated[SimulationService, Depends(service)]
    ) -> BatchReport:
        result = report(BatchService(svc), id)
        response.headers["ETag"] = '"' + digest(result) + '"'
        return result
