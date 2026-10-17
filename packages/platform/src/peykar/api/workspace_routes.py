"""Bounded discovery for the local research workbench; no new scientific claims."""

import json
from collections.abc import Callable
from typing import Annotated, Any, Literal

from fastapi import APIRouter, Depends, Query

from peykar.composition.experiments import ExperimentService
from peykar.composition.hybrid_flows import HybridFlowService
from peykar.simulation.service import SimulationService

CatalogKind = Literal["dataset", "scenario", "evaluation_set", "validation_record"]


def install_workspace_routes(
    read: APIRouter,
    service: Callable[..., SimulationService],
    role: Callable[..., str],
) -> None:
    @read.get("/workspace/session", tags=["workspace"])
    def session(access: Annotated[str, Depends(role)]) -> dict[str, str]:
        return {"role": access, "scope": "local_workspace"}

    @read.get("/workspace/catalog/{kind}", tags=["workspace"])
    def catalog(
        kind: CatalogKind,
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> list[dict[str, Any]]:
        # Metadata only: observation matrices and frozen payloads use their existing
        # checksum-verifying detail services. The kind allowlist excludes internals.
        result = []
        for row in svc.db.objects(kind, limit, offset):
            value = json.loads(row["payload"])
            spec = value.get("spec", value)
            result.append(
                {
                    "id": row["id"],
                    "version": row["version"],
                    "digest": row["digest"],
                    "title": spec.get("title", row["id"]),
                }
            )
        return result

    @read.get("/experiment-runs", tags=["experiments"])
    def experiments(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> list[dict[str, Any]]:
        engine = ExperimentService(svc)
        with svc.db.connection() as db:
            rows = db.execute(
                "SELECT id FROM experiment_runs ORDER BY created_at DESC,id LIMIT ? OFFSET ?",
                (limit, offset),
            ).fetchall()
        return [engine.record(row["id"]).model_dump(mode="json") for row in rows]

    @read.get("/hybrid-runs", tags=["hybrid flows"])
    def flows(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> list[dict[str, Any]]:
        engine = HybridFlowService(svc)
        with svc.db.connection() as db:
            rows = db.execute(
                "SELECT id FROM hybrid_runs ORDER BY created_at DESC,id LIMIT ? OFFSET ?",
                (limit, offset),
            ).fetchall()
        return [engine.record(row["id"]).model_dump(mode="json") for row in rows]
