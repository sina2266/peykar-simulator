from pathlib import Path

import pytest

from peykar.simulation.environment import capture_environment, verify_environment
from peykar.simulation.execution import InProcessExecutor
from peykar.simulation.worker import Worker
from peykar_core.errors import InvalidRequest
from peykar_core.hashing import bytes_digest


def test_runtime_lock_is_independent_of_cwd(service, plan, tmp_path: Path, monkeypatch):
    lock = tmp_path / "installed" / "uv.lock"
    lock.parent.mkdir()
    lock.write_bytes(b"pinned runtime")
    monkeypatch.setattr("peykar.simulation.environment.runtime_lock", lambda: lock)
    first = service.submit(plan)
    other = tmp_path / "other"
    other.mkdir()
    (other / "uv.lock").write_bytes(b"unrelated cwd lock")
    monkeypatch.chdir(other)
    second = service.submit(plan)
    assert service.db.manifest(first.id).environment == service.db.manifest(second.id).environment
    assert service.db.manifest(second.id).environment.lock_digest == bytes_digest(b"pinned runtime")
    assert Worker(service, InProcessExecutor()).tick().status == "succeeded"


@pytest.mark.parametrize("change", ["changed", "missing"])
def test_worker_rejects_runtime_lock_drift(service, plan, tmp_path, monkeypatch, change):
    lock = tmp_path / "uv.lock"
    lock.write_bytes(b"original lock")
    monkeypatch.setattr("peykar.simulation.environment.runtime_lock", lambda: lock)
    run = service.submit(plan)
    if change == "missing":
        lock.unlink()
    else:
        lock.write_bytes(b"modified lock")
    record = Worker(service, InProcessExecutor()).tick()
    assert record.id == run.id
    assert record.status == "failed"
    assert record.error == "execution_constraint"
    assert record.result_digest is None


def test_platform_version_is_enforced_and_legacy_missing_lock_supported(service):
    engine = service.engines.get("reference.decay", "0.1.0")
    environment = capture_environment(engine)
    verify_environment(environment.model_copy(update={"lock_digest": None}), engine)
    with pytest.raises(InvalidRequest, match="environment"):
        verify_environment(environment.model_copy(update={"platform_version": "different"}), engine)
