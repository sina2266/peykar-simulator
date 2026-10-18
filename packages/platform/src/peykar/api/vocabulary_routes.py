"""Authenticated vocabulary queries and unreviewed contextual annotation writes."""

from collections.abc import Callable
from typing import Annotated

from fastapi import APIRouter, Depends, Query, Response

from peykar.biology.vocabularies import VocabularyService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    EntityVocabularyMapping,
    EntityVocabularyMappingSpec,
    Identifier,
    Reference,
    Version,
    VocabularyMappingConflict,
    VocabularyPackage,
    VocabularyTermRecord,
)


def install_vocabulary_routes(
    read: APIRouter, write: APIRouter, service: Callable[..., SimulationService]
) -> None:
    def vocab(svc: SimulationService) -> VocabularyService:
        return VocabularyService(svc.db, svc.artifacts)

    @read.get("/biology/vocabularies", tags=["vocabularies"])
    def discover(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[VocabularyPackage, ...]:
        return vocab(svc).discover(limit, offset)

    @read.get("/biology/vocabularies/{id}/versions/{version}", tags=["vocabularies"])
    def get(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> VocabularyPackage:
        return vocab(svc).get(Reference(id=id, version=version))

    @read.get("/biology/vocabularies/{id}/versions/{version}/terms", tags=["vocabularies"])
    def search(
        id: Identifier,
        version: Version,
        svc: Annotated[SimulationService, Depends(service)],
        q: Annotated[str, Query(max_length=200)] = "",
        namespace: Annotated[str | None, Query(max_length=200)] = None,
        obsolete: bool | None = None,
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[VocabularyTermRecord, ...]:
        return vocab(svc).search(
            Reference(id=id, version=version), q, namespace, obsolete, limit, offset
        )

    @read.get("/biology/vocabularies/{id}/versions/{version}/term", tags=["vocabularies"])
    def term(
        id: Identifier,
        version: Version,
        term_id: Annotated[str, Query(max_length=200)],
        svc: Annotated[SimulationService, Depends(service)],
    ) -> VocabularyTermRecord:
        return vocab(svc).term(Reference(id=id, version=version), term_id)

    @read.get(
        "/biology/vocabularies/{id}/versions/{version}/artifacts/{kind}", tags=["vocabularies"]
    )
    def artifact(
        id: Identifier,
        version: Version,
        kind: str,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> Response:
        from peykar_core.errors import NotFound

        package = vocab(svc).get(Reference(id=id, version=version))
        if kind not in ("raw", "normalized"):
            raise NotFound("Vocabulary artifact kind not found")
        checksum = package.spec.raw_digest if kind == "raw" else package.normalized_digest
        return Response(
            svc.artifacts.get(checksum),
            media_type="application/octet-stream",
            headers={"Content-Disposition": "attachment", "X-Content-Type-Options": "nosniff"},
        )

    @write.post("/biology/vocabulary-mappings", status_code=201, tags=["vocabularies"])
    def map_entity(
        body: EntityVocabularyMappingSpec, svc: Annotated[SimulationService, Depends(service)]
    ) -> EntityVocabularyMapping:
        return vocab(svc).map_entity(body)

    @read.get("/biology/vocabulary-mappings", tags=["vocabularies"])
    def mappings(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[EntityVocabularyMapping, ...]:
        return vocab(svc).mappings(limit, offset)

    @read.get("/biology/vocabulary-mapping-conflicts", tags=["vocabularies"])
    def conflicts(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=50)] = 20,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[VocabularyMappingConflict, ...]:
        return vocab(svc).conflicts(limit, offset)

    @read.get("/biology/vocabulary-mappings/{id}/versions/{version}", tags=["vocabularies"])
    def mapping(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> EntityVocabularyMapping:
        return vocab(svc).mapping(Reference(id=id, version=version))
