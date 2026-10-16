"""Explicit local unknowns, separate from execution and scientific approval."""

import json
from typing import Literal

from peykar.infrastructure.database import Database
from peykar.model_registry.registry import ModelRegistry
from peykar_core.contracts import KnowledgeGap
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import digest

Severity = Literal["blocking", "limitation", "informational"]
STORAGE_VERSION = "1.0.0"


class KnowledgeGapService:
    def __init__(self, db: Database) -> None:
        self.db = db
        self.models = ModelRegistry(db)

    def register(self, gap: KnowledgeGap) -> str:
        if not gap.description.strip() or len(gap.description) > 20_000:
            raise InvalidRequest(
                "Knowledge gap description requires 1 to 20000 nonblank characters"
            )
        if not gap.context.species.strip() or not gap.context.biological_system.strip():
            raise InvalidRequest("Knowledge gap context must be explicit and nonblank")
        if len(gap.affected_models) > 200 or len(set(gap.affected_models)) != len(
            gap.affected_models
        ):
            raise InvalidRequest("At most 200 unique affected model references are supported")
        for reference in gap.affected_models:
            self.models.get(reference)
        return self.db.register("knowledge_gap", gap.id, STORAGE_VERSION, gap.model_dump_json())

    def get(self, id: str) -> KnowledgeGap:
        gap = KnowledgeGap.model_validate_json(self.db.object("knowledge_gap", id, STORAGE_VERSION))
        if gap.id != id:
            raise Conflict("Knowledge gap identifier differs from its stored contents")
        return gap

    def discover(
        self,
        *,
        species: str | None = None,
        biological_system: str | None = None,
        condition: str | None = None,
        severity: Severity | None = None,
        model_id: str | None = None,
        model_version: str | None = None,
        limit: int = 100,
        offset: int = 0,
    ) -> tuple[KnowledgeGap, ...]:
        if not 1 <= limit <= 200 or offset < 0:
            raise InvalidRequest("Knowledge gap pagination requires limit 1..200 and offset >= 0")
        if severity is not None and severity not in ("blocking", "limitation", "informational"):
            raise InvalidRequest("Unknown knowledge gap severity")
        if model_version is not None and model_id is None:
            raise InvalidRequest("Model version filter requires a model ID")
        clauses = ["kind='knowledge_gap'", "version=?"]
        parameters: list[str | int] = [STORAGE_VERSION]
        for field, value in (
            ("$.context.species", species),
            ("$.context.biological_system", biological_system),
            ("$.severity", severity),
        ):
            if value is not None:
                clauses.append("json_extract(payload,?)=?")
                parameters.extend((field, value))
        if condition is not None:
            clauses.append(
                "EXISTS(SELECT 1 FROM json_each(payload,'$.context.conditions') WHERE value=?)"
            )
            parameters.append(condition)
        if model_id is not None:
            model_clause = "json_extract(value,'$.id')=?"
            parameters.append(model_id)
            if model_version is not None:
                model_clause += " AND json_extract(value,'$.version')=?"
                parameters.append(model_version)
            clauses.append(
                "EXISTS(SELECT 1 FROM json_each(payload,'$.affected_models') WHERE "
                + model_clause
                + ")"
            )
        query = (
            "SELECT id,payload,digest FROM objects WHERE "
            + " AND ".join(clauses)
            + " ORDER BY id LIMIT ? OFFSET ?"
        )
        parameters.extend((limit, offset))
        with self.db.connection() as db:
            rows = db.execute(query, parameters).fetchall()
        gaps = []
        for row in rows:
            if digest(json.loads(row["payload"])) != row["digest"]:
                raise Conflict("Knowledge gap integrity verification failed")
            gap = KnowledgeGap.model_validate_json(row["payload"])
            if gap.id != row["id"]:
                raise Conflict("Knowledge gap identifier differs from its stored contents")
            gaps.append(gap)
        return tuple(gaps)
