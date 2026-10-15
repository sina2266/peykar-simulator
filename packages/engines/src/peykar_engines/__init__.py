"""Reference engines verify infrastructure; they are not validated tissue models."""

from peykar_core.protocols import EngineAdapter
from peykar_engines.reference import DecayEngine, LogisticEngine


def reference_engines() -> tuple[EngineAdapter, ...]:
    return DecayEngine(), LogisticEngine()
