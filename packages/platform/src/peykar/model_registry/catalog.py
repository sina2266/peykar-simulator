"""No remote code loading. Entry points require an explicit operator allowlist."""

from importlib.metadata import entry_points

from peykar_core.errors import Conflict, Unsupported
from peykar_core.protocols import EngineAdapter
from peykar_engines import reference_engines


class EngineCatalog:
    def __init__(self) -> None:
        self._engines: dict[tuple[str, str], EngineAdapter] = {}
        for engine in reference_engines():
            self.register(engine)

    def register(self, engine: EngineAdapter) -> None:
        key = (engine.id, engine.version)
        if key in self._engines:
            raise Conflict("Duplicate engine version")
        self._engines[key] = engine

    def get(self, id: str, version: str) -> EngineAdapter:
        try:
            return self._engines[id, version]
        except KeyError as exc:
            raise Unsupported(f"Engine {id}@{version} is not installed and approved") from exc

    def load_approved_plugins(self, names: frozenset[str]) -> None:
        for entry in entry_points(group="peykar.engines"):
            if entry.name in names:
                for engine in entry.load()():
                    self.register(engine)

    def describe(self) -> tuple[dict[str, str], ...]:
        return tuple({"id": id, "version": version} for id, version in sorted(self._engines))
