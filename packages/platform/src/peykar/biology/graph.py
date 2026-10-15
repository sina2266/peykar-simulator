from graphlib import CycleError, TopologicalSorter

from peykar_core.contracts import BiologicalEntity, BiologicalRelationship
from peykar_core.errors import InvalidRequest


class BiologicalGraph:
    """Small in-memory reference graph; persisted graph queries are a future port."""

    def __init__(self) -> None:
        self.entities: dict[str, BiologicalEntity] = {}
        self.relationships: list[BiologicalRelationship] = []

    def add_entity(self, entity: BiologicalEntity) -> None:
        if entity.id in self.entities and self.entities[entity.id] != entity:
            raise InvalidRequest("Entity identifier already has a different definition")
        self.entities[entity.id] = entity

    def add_relationship(self, edge: BiologicalRelationship) -> None:
        if edge.source not in self.entities or edge.target not in self.entities:
            raise InvalidRequest("Relationship references an unknown entity")
        if not edge.evidence:
            raise InvalidRequest("Biological relationships require evidence references")
        if any(
            self.entities[id].species != edge.context.species for id in (edge.source, edge.target)
        ):
            raise InvalidRequest("Cross-species relationships need an explicit mapping")
        candidate = [*self.relationships, edge]
        for relation in ("is_a", "part_of"):
            graph: dict[str, set[str]] = {key: set() for key in self.entities}
            for item in candidate:
                if item.relation == relation:
                    graph[item.source].add(item.target)
            try:
                tuple(TopologicalSorter(graph).static_order())
            except CycleError as exc:
                raise InvalidRequest(f"Cyclic {relation} hierarchy") from exc
        if edge not in self.relationships:
            self.relationships.append(edge)
