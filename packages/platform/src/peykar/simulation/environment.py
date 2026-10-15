"""Capture and verify the installed runtime independently of the process cwd."""

import inspect
import platform
import sys
from importlib.metadata import version
from pathlib import Path

from peykar import __version__
from peykar_core.contracts import RuntimeEnvironment
from peykar_core.errors import InvalidRequest
from peykar_core.hashing import bytes_digest
from peykar_core.protocols import EngineAdapter


def runtime_lock() -> Path:
    """The workspace/image lock lives beside its virtual environment, never in a run cwd."""
    return Path(sys.prefix).parent / "uv.lock"


def capture_environment(engine: EngineAdapter) -> RuntimeEnvironment:
    lock = runtime_lock()
    return RuntimeEnvironment(
        platform_version=__version__,
        python_version=platform.python_version(),
        engine_id=engine.id,
        engine_version=engine.version,
        engine_digest=bytes_digest(Path(inspect.getfile(type(engine))).read_bytes()),
        dependencies=tuple(f"{name}=={version(name)}" for name in ("pydantic", "pint", "numpy")),
        lock_digest=bytes_digest(lock.read_bytes()) if lock.is_file() else None,
    )


def verify_environment(expected: RuntimeEnvironment, engine: EngineAdapter) -> None:
    current = capture_environment(engine)
    # Old installed manifests did not contain a lock digest. Keep those readable and
    # executable, while enforcing every digest actually pinned by a submitter.
    if expected.lock_digest is None:
        current = current.model_copy(update={"lock_digest": None})
    if current != expected:
        raise InvalidRequest("Worker environment differs from the pinned manifest")
