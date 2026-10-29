"""Retinal visual-acuity model 1.2 publisher (synthetic data only)."""

from __future__ import annotations

import importlib.util
import math
from pathlib import Path


def _publisher():
    path = Path(__file__).resolve().parents[2] / "scripts/publish_retina_acuity_population_1_2.py"
    spec = importlib.util.spec_from_file_location("publish_retina_acuity_population_1_2", path)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _report(next_error: float, landmark_error: float, initial_error: float, method: str) -> dict:
    def entry(error: float) -> dict:
        return {
            "series_id": "g/bcva",
            "context": "human/dme/anti_vegf",
            "kind": "group_mean",
            "rows": [
                {
                    "origin_month": 1.0,
                    "target_month": 3.0,
                    "observed": 0.9,
                    "predictions": {method: 0.9 + error / 100.0},
                }
            ],
        }

    initial = {
        "series_id": "g/bcva",
        "context": "human/dme/anti_vegf",
        "kind": "group_mean",
        "months": [1.0],
        "observed": [0.9],
        "predictions": {"population_initial_state": [0.9 + initial_error / 100.0]},
    }
    return {
        "studies": [
            {
                "study_id": "rac-pmc1",
                "next_observation": [entry(next_error)],
                "landmark": [entry(landmark_error)],
                "initial_state": [initial],
            }
        ]
    }


def test_retina_acuity_1_2_publisher_constants_and_h5() -> None:
    module = _publisher()
    assert module.MODEL_VERSION == "1.2.0-x3"
    assert module.REFERENCE_VERSION == "1.1.0-w1"
    assert module.PRIMARY["next_observation"] == "population_anchored"
    assert module.REFERENCE_PRIMARY["landmark"] == "population_anchored_t4"
    assert module.base.PRIMARY == module.PRIMARY
    assert len(module.RECORD_DIRS["development"]) == 4
    assert "tranche34" in str(module.RECORD_DIRS["sealed"][0])
    new = _report(3.0, 4.0, 5.0, "population_anchored")
    old = _report(3.02, 6.0, 7.0, "population_anchored_t4")
    h5 = module.versus_reference(new, old)
    assert math.isclose(h5["landmark"]["model_1_1_rmse_pp"], 6.0)
    assert math.isclose(h5["initial_state"]["model_1_2_rmse_pp"], 5.0)
    assert module.h5_passes(h5)
    assert not module.h5_passes(
        module.versus_reference(_report(3.1, 4.0, 5.0, "population_anchored"), old)
    )
