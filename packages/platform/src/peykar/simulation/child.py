"""Trusted engine subprocess entry point. Apply Linux limits before heavy imports."""

import json
import math
import sys


def main() -> None:
    import resource

    maximum = 8 * 1024 * 1024
    raw = sys.stdin.buffer.read(maximum + 1)
    if len(raw) > maximum:
        raise ValueError("Input limit")
    request = json.loads(raw)
    seconds = float(request["manifest"]["plan"]["budget"]["wall_seconds"])
    resource.setrlimit(resource.RLIMIT_CPU, (max(1, math.ceil(seconds)),) * 2)
    resource.setrlimit(resource.RLIMIT_AS, (1024 * 1024 * 1024,) * 2)
    resource.setrlimit(resource.RLIMIT_FSIZE, (32 * 1024 * 1024,) * 2)
    resource.setrlimit(resource.RLIMIT_NOFILE, (64, 64))
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    from peykar.model_registry.catalog import EngineCatalog
    from peykar.simulation.environment import verify_environment
    from peykar_core.contracts import RunManifest
    from peykar_core.hashing import canonical_json
    from peykar_core.planning import validate_plan

    manifest = RunManifest.model_validate(request["manifest"])
    catalog = EngineCatalog()
    catalog.load_approved_plugins(frozenset(request["plugins"]))
    engine = catalog.get(manifest.environment.engine_id, manifest.environment.engine_version)
    verify_environment(manifest.environment, engine)
    validate_plan(manifest.model_snapshot, manifest.plan)
    if engine.validate(manifest.model_snapshot, manifest.plan):
        raise ValueError("Invalid engine plan")
    result = engine.execute(manifest.model_snapshot, manifest.plan, lambda: None)
    sys.stdout.write(canonical_json(result))


if __name__ == "__main__":
    main()
