"""Immutable evidence-bearing graph snapshots and bounded neighborhood queries."""

from collections import deque
from graphlib import CycleError, TopologicalSorter

from peykar.biology.graph import BiologicalGraph
from peykar.infrastructure.database import Database
from peykar_core.contracts import BiologicalGraphSpec, Reference
from peykar_core.errors import InvalidRequest, NotFound
from peykar_core.hashing import digest


class GraphService:
    def __init__(self, db: Database) -> None:
        self.db = db

    def register(self, spec: BiologicalGraphSpec) -> str:
        graph = BiologicalGraph()
        ids = [node.id for node in spec.entities]
        if len(ids) != len(set(ids)):
            raise InvalidRequest("Duplicate entity IDs in graph snapshot")
        for node in spec.entities:
            if any(
                not value.strip() or len(value) > 500
                for value in (node.id, node.label, node.kind, node.species)
            ):
                raise InvalidRequest(
                    "Entity identifiers, labels, kinds and species must be bounded"
                )
            if node.species != spec.context.species:
                raise InvalidRequest("Graph entity species differs from its declared context")
            graph.add_entity(node)
        for edge in spec.relationships:
            if edge.context != spec.context:
                raise InvalidRequest("Relationship context differs from its graph snapshot")
            if not edge.evidence or any(
                not item.strip() or len(item) > 2000 for item in edge.evidence
            ):
                raise InvalidRequest("Relationships require bounded nonblank evidence references")
            if edge.source not in graph.entities or edge.target not in graph.entities:
                raise InvalidRequest("Relationship references an unknown entity")
        for relation in ("is_a", "part_of"):
            hierarchy: dict[str, set[str]] = {id: set() for id in ids}
            for edge in spec.relationships:
                if edge.relation == relation:
                    hierarchy[edge.source].add(edge.target)
            try:
                tuple(TopologicalSorter(hierarchy).static_order())
            except CycleError as exc:
                raise InvalidRequest(f"Cyclic {relation} hierarchy") from exc
        return self.db.register("biological_graph", spec.id, spec.version, spec.model_dump_json())

    def get(self, reference: Reference) -> BiologicalGraphSpec:
        return BiologicalGraphSpec.model_validate_json(
            self.db.object("biological_graph", reference.id, reference.version)
        )

    def discover(self, species: str | None = None) -> tuple[BiologicalGraphSpec, ...]:
        return tuple(
            BiologicalGraphSpec.model_validate_json(row["payload"])
            for row in self.db.objects("biological_graph", limit=200, species=species)
        )

    def neighborhood(
        self, reference: Reference, entity_id: str, depth: int = 1, max_nodes: int = 100
    ) -> dict[str, object]:
        if not 1 <= depth <= 4 or not 1 <= max_nodes <= 500:
            raise InvalidRequest("Neighborhood bounds are depth 1..4 and 1..500 nodes")
        graph = self.get(reference)
        entities = {entity.id: entity for entity in graph.entities}
        if entity_id not in entities:
            raise NotFound("Entity is absent from this graph version")
        adjacency: dict[str, set[str]] = {id: set() for id in entities}
        for edge in graph.relationships:
            adjacency[edge.source].add(edge.target)
            adjacency[edge.target].add(edge.source)
        seen = {entity_id}
        pending = deque([(entity_id, 0)])
        truncated = False
        while pending:
            node, distance = pending.popleft()
            if distance >= depth:
                continue
            for neighbor in sorted(adjacency[node]):
                if neighbor in seen:
                    continue
                if len(seen) >= max_nodes:
                    truncated = True
                    continue
                seen.add(neighbor)
                pending.append((neighbor, distance + 1))
        return {
            "reference": reference.model_dump(mode="json"),
            "graph_digest": digest(graph),
            "entities": [entities[id].model_dump(mode="json") for id in sorted(seen)],
            "relationships": [
                e.model_dump(mode="json")
                for e in graph.relationships
                if e.source in seen and e.target in seen
            ],
            "truncated": truncated,
            "interpretation": "Evidence-linked declarations; graph edges do not execute mechanisms",
        }
