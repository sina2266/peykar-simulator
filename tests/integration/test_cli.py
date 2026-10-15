from pathlib import Path

from typer.testing import CliRunner

from peykar.cli import app


def test_cli_demo(tmp_path: Path, monkeypatch):
    monkeypatch.setenv("PEYKAR_DATA_DIR", str(tmp_path))
    result = CliRunner().invoke(app, ["demo"])
    assert result.exit_code == 0, result.output
    assert '"passed": true' in result.output
    assert '"status": "succeeded"' in result.output
