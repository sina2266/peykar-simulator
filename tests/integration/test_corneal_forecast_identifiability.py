import importlib.util
import json
from pathlib import Path

import pytest


def test_actual_prefix_audit_abstains_without_reading_future_data(monkeypatch):
    path = Path("scripts/audit_corneal_forecast_identifiability.py")
    spec = importlib.util.spec_from_file_location("prefix_precision", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    retained = json.loads(
        (module.BASE / "identifiability-observed-2026-10-27/report.json").read_bytes()
    )
    allowed = {
        path.resolve(),
        module.PROTOCOL.resolve(),
        module.SOURCE.resolve(),
        (module.BASE / "ijms2019-control-prefix.json").resolve(),
    }
    original_read = Path.read_bytes

    def read_only_prefix(self):
        assert self.resolve() in allowed, f"Unexpected source/future access: {self}"
        return original_read(self)

    monkeypatch.setattr(Path, "read_bytes", read_only_prefix)
    report = module.audit()
    assert report == retained
    assert report["abstain_from_claiming_target_precision"] is True
    assert report["optimistic_minimax_point_rmse_lower_bound_fraction"] == pytest.approx(
        0.14510388870513824
    )
    assert report["future_observations_used"] is False
    assert report["fit_or_solver_run"] is False
    assert report["statistical_or_confidence_interval"] is False
    assert report["biological_validation"] is False
