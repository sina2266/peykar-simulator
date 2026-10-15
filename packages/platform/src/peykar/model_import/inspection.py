from peykar.model_registry.catalog import EngineCatalog
from peykar_core.contracts import ModelPackage
from peykar_core.errors import InvalidRequest, Unsupported
from peykar_core.hashing import digest


def inspect_package(data: bytes, engines: EngineCatalog) -> dict[str, object]:
    """Parse a JSON descriptor. Never eval, unpickle, extract archives, or follow URLs."""
    if len(data) > 2_000_000:
        raise InvalidRequest("Descriptor exceeds 2 MB")
    model = ModelPackage.model_validate_json(data)
    installed = True
    try:
        engines.get(model.engine, model.engine_version)
    except Unsupported:
        installed = False
    return {
        "id": model.id,
        "version": model.version,
        "digest": digest(model),
        "engine_available": installed,
        "declared_license": model.license,
        "scientific_validity": "not_established_by_import",
        "source_fetched": False,
    }
