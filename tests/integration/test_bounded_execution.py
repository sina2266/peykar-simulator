import os
import sqlite3
import sys
import time

import pytest

from peykar.simulation.execution import SubprocessExecutor, _run_child
from peykar_core.errors import Cancelled, InvalidRequest


def test_hung_child_is_killed_and_reaped(tmp_path):
    pid_file = tmp_path / "pid"
    command = (
        sys.executable,
        "-I",
        "-c",
        "import os,time,pathlib; pathlib.Path("
        + repr(str(pid_file))
        + ").write_text(str(os.getpid())); time.sleep(60)",
    )
    started = time.monotonic()
    with pytest.raises(InvalidRequest, match="deadline"):
        _run_child(command, b"{}", 0.4, lambda: None)
    assert time.monotonic() - started < 3
    with pytest.raises(ProcessLookupError):
        os.kill(int(pid_file.read_text()), 0)


def test_cancellation_interrupts_noncooperative_child():
    calls = 0

    def checkpoint():
        nonlocal calls
        calls += 1
        if calls > 2:
            raise Cancelled("requested")

    with pytest.raises(Cancelled):
        _run_child((sys.executable, "-I", "-c", "import time; time.sleep(60)"), b"", 5, checkpoint)


def test_child_has_private_cwd_and_does_not_inherit_secrets(monkeypatch, tmp_path):
    monkeypatch.setenv("PEYKAR_API_TOKEN", "never-pass-me")
    output = (
        _run_child(
            (
                sys.executable,
                "-I",
                "-c",
                "import os; print(os.getenv('PEYKAR_API_TOKEN')); print(os.getcwd())",
            ),
            b"",
            5,
            lambda: None,
        )
        .decode()
        .splitlines()
    )
    assert output[0] == "None"
    assert "peykar-run-" in output[1]
    assert not os.path.exists(output[1])


def test_child_output_limit(monkeypatch):
    monkeypatch.setattr("peykar.simulation.execution.MAX_OUTPUT_BYTES", 100)
    with pytest.raises(InvalidRequest, match="output"):
        _run_child((sys.executable, "-I", "-c", "print('x'*101)"), b"", 5, lambda: None)


def test_invalid_child_result_is_rejected(service, plan, monkeypatch):
    run = service.submit(plan)
    monkeypatch.setattr("peykar.simulation.execution._run_child", lambda *args: b"{}")
    engine = service.engines.get("reference.decay", "0.1.0")
    with pytest.raises(InvalidRequest, match="result contract"):
        SubprocessExecutor().execute(engine, service.db.manifest(run.id), lambda: None)


def test_heartbeat_lock_contention_does_not_stall_resource_supervisor(service, plan):
    service.submit(plan)
    run_id, token = service.db.claim()
    with service.db.connection(immediate=True):
        started = time.monotonic()
        with pytest.raises(sqlite3.OperationalError, match="locked"):
            service.db.heartbeat(run_id, token)
        assert time.monotonic() - started < 1.0
