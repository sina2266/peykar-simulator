"""Immutable vocabulary snapshots, indexed reads and contextual mapping declarations."""

import sqlite3
from graphlib import CycleError, TopologicalSorter
from pathlib import Path

from pydantic import ValidationError

from peykar.biology.persistent_graph import GraphService
from peykar.biology.vocabulary_import import parse_vocabulary
from peykar.infrastructure.artifacts import ArtifactStore
from peykar.infrastructure.database import Database
from peykar_core.contracts import (
    EntityVocabularyMapping,
    EntityVocabularyMappingSpec,
    Reference,
    VocabularyMappingConflict,
    VocabularyPackage,
    VocabularySourceSpec,
    VocabularyTerm,
    VocabularyTermRecord,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import canonical_json, digest


class VocabularyService:
    def __init__(self, db: Database, artifacts: ArtifactStore) -> None:
        self.db, self.artifacts = db, artifacts
        with db.connection() as connection:
            count = connection.execute(
                "SELECT count(*) FROM sqlite_master WHERE type='table' "
                "AND name IN ('vocabulary_terms','vocabulary_aliases')"
            ).fetchone()[0]
            if count != 2:
                connection.executescript(
                    (
                        Path(__file__).parents[1] / "infrastructure/migrations/006_vocabularies.sql"
                    ).read_text()
                )

    def admit(self, spec: VocabularySourceSpec, raw: bytes) -> VocabularyPackage:
        terms, metadata = parse_vocabulary(spec, raw)
        ids = {t.id for t in terms}
        if len(ids) != len(terms):
            raise InvalidRequest("Duplicate vocabulary term identities")
        aliases: dict[str, str] = {}
        for term in terms:
            for alias in term.aliases:
                if alias in ids or alias in aliases:
                    raise InvalidRequest("Ambiguous or duplicate vocabulary alias")
                aliases[alias] = term.id
        try:
            tuple(TopologicalSorter({t.id: set(t.parents) for t in terms}).static_order())
        except CycleError as error:
            raise InvalidRequest("Cyclic vocabulary is_a declarations") from error
        serialized = [(t, canonical_json(t)) for t in sorted(terms, key=lambda t: t.id)]
        normalized = ("\n".join(raw for _, raw in serialized) + "\n").encode()
        if len(normalized) > 64 * 1024**2:
            raise InvalidRequest("Normalized vocabulary exceeds 64 MiB")
        self.artifacts.put(raw)
        package = VocabularyPackage(
            spec=spec,
            normalized_digest=self.artifacts.put(normalized),
            term_count=len(terms),
            alias_count=len(aliases),
            obsolete_count=sum(t.obsolete for t in terms),
            unresolved_parent_ids=tuple(
                sorted({p for t in terms for p in t.parents if p not in ids})
            ),
            source_metadata=metadata,
        )
        with self.db.connection(immediate=True) as connection:
            previous = connection.execute(
                "SELECT 1 FROM objects WHERE kind='vocabulary' AND id=? AND version=?",
                (spec.id, spec.version),
            ).fetchone()
            Database.register_in_transaction(
                connection, "vocabulary", spec.id, spec.version, package.model_dump_json()
            )
            if previous:
                # An immutable re-import never silently repairs/replaces a damaged index.
                self._count(connection, package)
                return package
            connection.executemany(
                "INSERT INTO vocabulary_terms VALUES (?,?,?,?,?,?,?,?)",
                (
                    (
                        spec.id,
                        spec.version,
                        t.id,
                        t.label,
                        t.namespace,
                        int(t.obsolete),
                        digest(t),
                        payload,
                    )
                    for t, payload in serialized
                ),
            )
            connection.executemany(
                "INSERT INTO vocabulary_aliases VALUES (?,?,?,?)",
                (
                    (spec.id, spec.version, alias, target)
                    for alias, target in sorted(aliases.items())
                ),
            )
        return package

    @staticmethod
    def _count(connection: sqlite3.Connection, package: VocabularyPackage) -> None:
        count = connection.execute(
            "SELECT count(*) FROM vocabulary_terms WHERE vocabulary_id=? AND vocabulary_version=?",
            (package.spec.id, package.spec.version),
        ).fetchone()[0]
        if count != package.term_count:
            raise Conflict(
                "Vocabulary index is incomplete; use an explicit verified recovery workflow"
            )
        aliases = connection.execute(
            "SELECT count(*) FROM vocabulary_aliases WHERE vocabulary_id=? "
            "AND vocabulary_version=?",
            (package.spec.id, package.spec.version),
        ).fetchone()[0]
        if aliases != package.alias_count:
            raise Conflict("Vocabulary alias index is incomplete")

    def get(self, reference: Reference) -> VocabularyPackage:
        package = VocabularyPackage.model_validate_json(
            self.db.object("vocabulary", reference.id, reference.version)
        )
        with self.db.connection() as connection:
            self._count(connection, package)
        return package

    def discover(self, limit: int = 20, offset: int = 0) -> tuple[VocabularyPackage, ...]:
        return tuple(
            self.get(Reference(id=r["id"], version=r["version"]))
            for r in self.db.objects("vocabulary", limit=limit, offset=offset)
        )

    @staticmethod
    def _record(
        package: VocabularyPackage, requested: str, row: sqlite3.Row
    ) -> VocabularyTermRecord:
        try:
            term = VocabularyTerm.model_validate_json(row["payload"])
            matches = (
                digest(term) == row["digest"]
                and term.id == row["term_id"]
                and term.label == row["label"]
                and term.namespace == row["namespace"]
                and int(term.obsolete) == row["obsolete"]
            )
        except (ValueError, ValidationError) as error:
            raise Conflict("Vocabulary term index is corrupt") from error
        if not matches or (requested != term.id and requested not in term.aliases):
            raise Conflict("Vocabulary term/alias index integrity check failed")
        return VocabularyTermRecord(
            vocabulary=Reference(id=package.spec.id, version=package.spec.version),
            vocabulary_digest=digest(package),
            requested_id=requested,
            matched_alias=requested != term.id,
            term=term,
            term_digest=digest(term),
        )

    def term(self, reference: Reference, term_id: str) -> VocabularyTermRecord:
        package = self.get(reference)
        with self.db.connection() as connection:
            row = connection.execute(
                "SELECT * FROM vocabulary_terms WHERE vocabulary_id=? "
                "AND vocabulary_version=? AND term_id=?",
                (reference.id, reference.version, term_id),
            ).fetchone()
            if row is None:
                row = connection.execute(
                    "SELECT t.* FROM vocabulary_terms t JOIN vocabulary_aliases a "
                    "ON (t.vocabulary_id=a.vocabulary_id "
                    "AND t.vocabulary_version=a.vocabulary_version AND t.term_id=a.term_id) "
                    "WHERE a.vocabulary_id=? AND a.vocabulary_version=? AND a.alias_id=?",
                    (reference.id, reference.version, term_id),
                ).fetchone()
        if row is None:
            raise NotFound("Term is absent from the pinned vocabulary snapshot")
        return self._record(package, term_id, row)

    def search(
        self,
        reference: Reference,
        query: str = "",
        namespace: str | None = None,
        obsolete: bool | None = None,
        limit: int = 20,
        offset: int = 0,
    ) -> tuple[VocabularyTermRecord, ...]:
        if not 1 <= limit <= 50 or offset < 0 or len(query) > 200:
            raise InvalidRequest(
                "Vocabulary search requires limit 1..50, nonnegative offset and bounded query"
            )
        package = self.get(reference)
        clauses = ["vocabulary_id=?", "vocabulary_version=?"]
        values: list[str | int] = [reference.id, reference.version]
        if query:
            escaped = query.replace("\\", "\\\\").replace("%", "\\%").replace("_", "\\_")
            clauses.append("(term_id LIKE ? ESCAPE '\\' OR label LIKE ? ESCAPE '\\')")
            values.extend(("%" + escaped + "%", "%" + escaped + "%"))
        if namespace is not None:
            clauses.append("namespace=?")
            values.append(namespace)
        if obsolete is not None:
            clauses.append("obsolete=?")
            values.append(int(obsolete))
        values.extend((limit, offset))
        with self.db.connection() as connection:
            rows = connection.execute(
                "SELECT * FROM vocabulary_terms WHERE "
                + " AND ".join(clauses)
                + " ORDER BY term_id LIMIT ? OFFSET ?",
                values,
            ).fetchall()
        return tuple(self._record(package, row["term_id"], row) for row in rows)

    def map_entity(self, spec: EntityVocabularyMappingSpec) -> EntityVocabularyMapping:
        if not spec.rationale.strip() or any(not e.strip() or len(e) > 2000 for e in spec.evidence):
            raise InvalidRequest("Mappings require nonblank bounded rationale and evidence")
        graph = GraphService(self.db).get(spec.graph)
        entity = next((e for e in graph.entities if e.id == spec.entity_id), None)
        if entity is None or graph.context != spec.context:
            raise InvalidRequest("Mapping requires an exact graph entity and scientific context")
        target = self.term(spec.vocabulary, spec.term_id)
        if target.matched_alias or target.term.obsolete:
            raise InvalidRequest(
                "Mappings require current canonical IDs; replacements are never followed"
            )
        if target.term.species is not None and target.term.species != entity.species:
            raise InvalidRequest("Mapping target species differs from the graph entity")
        domain = self.get(spec.vocabulary).spec.domain
        exact_kinds = {
            "gene": {"gene"},
            "protein": {"protein", "receptor", "enzyme"},
            "pathway": {"pathway"},
            "cell": {"cell", "cell_type"},
            "anatomy": {"anatomical_structure", "tissue", "organ"},
            "function": {
                "process",
                "biological_process",
                "molecular_function",
                "cellular_component",
            },
        }
        if spec.relation == "exact_match" and entity.kind not in exact_kinds[domain]:
            raise InvalidRequest(
                "Exact mapping requires a compatible entity kind; use a qualified relation"
            )
        primary_prefix = {
            "gene": "HGNC",
            "protein": "UniProtKB",
            "pathway": "Reactome",
            "cell": "CL",
            "anatomy": "UBERON",
            "function": "GO",
        }[domain]
        if spec.relation == "exact_match" and target.term.id.split(":")[0] != primary_prefix:
            raise InvalidRequest(
                "Exact target is outside the supported domain identifier namespace"
            )
        if (
            spec.relation == "exact_match"
            and domain == "function"
            and (
                target.term.namespace
                in ("biological_process", "molecular_function", "cellular_component")
                and entity.kind != target.term.namespace
                and not (entity.kind == "process" and target.term.namespace == "biological_process")
            )
        ):
            raise InvalidRequest("Exact GO annotations must retain the original ontology aspect")
        record = EntityVocabularyMapping(spec=spec, graph_digest=digest(graph), target=target)
        with self.db.connection(immediate=True) as connection:
            rows = connection.execute(
                "SELECT id,version,payload,digest FROM objects WHERE kind='vocabulary_mapping' "
                "AND json_extract(payload,'$.spec.graph.id')=? "
                "AND json_extract(payload,'$.spec.graph.version')=? "
                "AND json_extract(payload,'$.spec.entity_id')=?",
                (spec.graph.id, spec.graph.version, spec.entity_id),
            ).fetchall()
            if len(rows) > 1000:
                raise InvalidRequest(
                    "Mapping subject exceeds the bounded local declaration profile"
                )
            Database.register_in_transaction(
                connection, "vocabulary_mapping", spec.id, spec.version, record.model_dump_json()
            )
            for row in rows:
                prior = EntityVocabularyMapping.model_validate_json(row["payload"])
                if digest(prior) != row["digest"]:
                    raise Conflict("Existing mapping integrity check failed")
                if (
                    spec.relation == prior.spec.relation == "exact_match"
                    and prior.spec.context == spec.context
                    and prior.spec.term_id != spec.term_id
                    and prior.spec.term_id.split(":")[0] == spec.term_id.split(":")[0]
                ):
                    pair = tuple(
                        sorted(
                            (
                                Reference(id=spec.id, version=spec.version),
                                Reference(id=prior.spec.id, version=prior.spec.version),
                            ),
                            key=lambda r: (r.id, r.version),
                        )
                    )
                    conflict = VocabularyMappingConflict(
                        id="mapping.conflict."
                        + digest([r.model_dump(mode="json") for r in pair])[:32],
                        version="1.0.0",
                        mappings=pair,
                    )
                    Database.register_in_transaction(
                        connection,
                        "vocabulary_mapping_conflict",
                        conflict.id,
                        conflict.version,
                        conflict.model_dump_json(),
                    )
        return record

    def mappings(self, limit: int = 20, offset: int = 0) -> tuple[EntityVocabularyMapping, ...]:
        return tuple(
            self.mapping(Reference(id=row["id"], version=row["version"]))
            for row in self.db.objects("vocabulary_mapping", limit=limit, offset=offset)
        )

    def mapping(self, reference: Reference) -> EntityVocabularyMapping:
        record = EntityVocabularyMapping.model_validate_json(
            self.db.object("vocabulary_mapping", reference.id, reference.version)
        )
        graph = GraphService(self.db).get(record.spec.graph)
        target = self.term(record.spec.vocabulary, record.spec.term_id)
        if digest(graph) != record.graph_digest or target != record.target:
            raise Conflict("Vocabulary mapping frozen lineage differs")
        return record

    def conflicts(self, limit: int = 20, offset: int = 0) -> tuple[VocabularyMappingConflict, ...]:
        records = tuple(
            VocabularyMappingConflict.model_validate_json(row["payload"])
            for row in self.db.objects("vocabulary_mapping_conflict", limit=limit, offset=offset)
        )
        for record in records:
            for reference in record.mappings:
                self.mapping(reference)
        return records
