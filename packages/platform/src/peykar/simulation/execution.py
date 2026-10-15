"""Bounded execution of trusted, installed engines. This is not a security sandbox."""

import os
import signal
import subprocess
import sys
import tempfile
import time
from collections.abc import Callable
from contextlib import suppress
from pathlib import Path
from typing import Protocol

from peykar_core.contracts import RunManifest, SimulationResult
from peykar_core.errors import InvalidRequest, Unsupported
from peykar_core.hashing import canonical_json
from peykar_core.protocols import EngineAdapter

MAX_OUTPUT_BYTES = 32 * 1024 * 1024
MAX_INPUT_BYTES = 8 * 1024 * 1024


class Executor(Protocol):
    def execute(
        self, engine: EngineAdapter, manifest: RunManifest, checkpoint: Callable[[], None]
    ) -> SimulationResult: ...


class InProcessExecutor:
    """Explicit cooperative mode for trusted operator integrations and tests only."""

    def execute(
        self, engine: EngineAdapter, manifest: RunManifest, checkpoint: Callable[[], None]
    ) -> SimulationResult:
        return engine.execute(manifest.model_snapshot, manifest.plan, checkpoint)


def _run_child(
    command: tuple[str, ...], payload: bytes, wall_seconds: float, checkpoint: Callable[[], None]
) -> bytes:
    """No shell, inherited secrets, inherited cwd, or unbounded output pipes."""
    if not sys.platform.startswith("linux"):
        raise Unsupported("Bounded child execution currently requires Linux")
    if len(payload) > MAX_INPUT_BYTES:
        raise InvalidRequest("Execution manifest exceeds child input limit")
    deadline = time.monotonic() + wall_seconds
    environment = {
        "PATH": str(Path(sys.executable).parent),
        "OPENBLAS_NUM_THREADS": "1",
        "OMP_NUM_THREADS": "1",
        "MKL_NUM_THREADS": "1",
        "LANG": "C.UTF-8",
    }
    with tempfile.TemporaryDirectory(prefix="peykar-run-") as directory:
        root = Path(directory)
        request, output = root / "request.json", root / "result.json"
        request.write_bytes(payload)
        with request.open("rb") as incoming, output.open("wb") as outgoing:
            process = subprocess.Popen(
                command,
                stdin=incoming,
                stdout=outgoing,
                stderr=subprocess.DEVNULL,
                cwd=root,
                env=environment,
                start_new_session=True,
            )
            try:
                while True:
                    checkpoint()
                    if time.monotonic() >= deadline:
                        raise InvalidRequest("Hard execution deadline exceeded")
                    if output.stat().st_size > MAX_OUTPUT_BYTES:
                        raise InvalidRequest("Child output exceeds limit")
                    if process.poll() is not None:
                        break
                    time.sleep(min(0.05, max(0.0, deadline - time.monotonic())))
                if process.returncode != 0:
                    raise InvalidRequest("Child execution failed or exceeded a resource limit")
            finally:
                # Also kill any descendants, including after an apparently successful exit.
                with suppress(ProcessLookupError):
                    os.killpg(process.pid, signal.SIGKILL)
                process.wait()
        with output.open("rb") as stream:
            raw = stream.read(MAX_OUTPUT_BYTES + 1)
        if len(raw) > MAX_OUTPUT_BYTES:
            raise InvalidRequest("Child output exceeds limit")
        return raw


class SubprocessExecutor:
    def __init__(self, approved_plugins: frozenset[str] = frozenset()) -> None:
        self.approved_plugins = approved_plugins

    def execute(
        self, engine: EngineAdapter, manifest: RunManifest, checkpoint: Callable[[], None]
    ) -> SimulationResult:
        payload = canonical_json(
            {"manifest": manifest.model_dump(mode="json"), "plugins": sorted(self.approved_plugins)}
        ).encode()
        raw = _run_child(
            (sys.executable, "-I", "-m", "peykar.simulation.child"),
            payload,
            manifest.plan.budget.wall_seconds,
            checkpoint,
        )
        try:
            return SimulationResult.model_validate_json(raw)
        except ValueError as exc:
            raise InvalidRequest("Child returned an invalid result contract") from exc
