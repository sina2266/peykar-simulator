"""Dimensional compatibility alone cannot establish a scientifically valid coupling."""

from graphlib import CycleError, TopologicalSorter

from peykar_core.contracts import Contract, ModelPackage, Quantity, ScientificPort
from peykar_core.errors import InvalidRequest, Unsupported
from peykar_core.units import convert


class Connection(Contract):
    source_model: str
    source_port: str
    target_model: str
    target_port: str


def _port(model: ModelPackage, name: str) -> ScientificPort:
    port = next((p for p in model.ports if p.name == name), None)
    if port is None:
        raise InvalidRequest(f"Unknown port {model.id}.{name}")
    return port


def check_connection(source: ScientificPort, target: ScientificPort) -> float:
    if source.direction != "output" or target.direction != "input":
        raise InvalidRequest("Connections must run from output to input")
    if source.semantic_id != target.semantic_id:
        raise InvalidRequest("A unit conversion cannot replace a semantic coupling model")
    if (source.scale, source.shape, source.frame) != (target.scale, target.shape, target.frame):
        raise InvalidRequest("Scale, shape or coordinate frame requires an explicit mapping")
    # Affine conversions cannot be applied as a scalar multiplicative mapping.
    zero = convert(Quantity(value=0.0, unit=source.unit), target.unit)
    if zero != 0:
        raise InvalidRequest("Affine unit conversion needs a dedicated mapping")
    return convert(Quantity(value=1.0, unit=source.unit), target.unit)


def composition_order(
    models: dict[str, ModelPackage], connections: tuple[Connection, ...]
) -> tuple[str, ...]:
    graph: dict[str, set[str]] = {key: set() for key in models}
    targets: set[tuple[str, str]] = set()
    for edge in connections:
        if edge.source_model not in models or edge.target_model not in models:
            raise InvalidRequest("Connection references an unknown model instance")
        source, target = models[edge.source_model], models[edge.target_model]
        if source.context.species != target.context.species:
            raise InvalidRequest("Cross-species composition requires an evidence-backed mapping")
        check_connection(_port(source, edge.source_port), _port(target, edge.target_port))
        key = (edge.target_model, edge.target_port)
        if key in targets:
            raise InvalidRequest("Multiple writers target the same input")
        targets.add(key)
        graph[edge.target_model].add(edge.source_model)
    try:
        return tuple(TopologicalSorter(graph).static_order())
    except CycleError as exc:
        raise Unsupported(
            "Feedback coupling requires a numerical solver; DAG scheduling is unsafe"
        ) from exc
