"""Childhood-myopia axial-length model 1.2 configurations and operator helpers."""

from __future__ import annotations

import importlib.util
from pathlib import Path

import pytest

from peykar.model_learning import myopia_population as version1
from peykar.model_learning import myopia_population_1_1 as version1_1
from peykar.model_learning.myopia_population_1_2 import (
    DEVELOPMENT_CANDIDATES,
    REFERENCES,
    SCALED_CANDIDATES,
    adapter,
)

ROOT = Path(__file__).resolve().parents[2]


def _operator():  # type: ignore[no-untyped-def]
    path = ROOT / "scripts/evaluate_myopia_population_1_2.py"
    spec = importlib.util.spec_from_file_location("evaluate_myopia_population_1_2", path)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _differing(a: dict, b: dict) -> set[str]:
    return {key for key in a if a[key] != b[key]}


def test_round_zero_candidates_are_the_frozen_versions_refitted() -> None:
    assert REFERENCES["1.0.0"] is version1.DEVELOPMENT_CANDIDATES["m4"]
    assert REFERENCES["1.1.0"] is version1_1.DEVELOPMENT_CANDIDATES["n3"]
    p0, p0m = DEVELOPMENT_CANDIDATES["p0"], DEVELOPMENT_CANDIDATES["p0m"]
    assert _differing(p0.describe(), REFERENCES["1.1.0"].describe()) == {"model_version"}
    assert _differing(p0m.describe(), REFERENCES["1.0.0"].describe()) == {"model_version"}
    assert p0.model_version == "1.2.0-p0" and p0m.model_version == "1.2.0-p0m"
    for name in (*DEVELOPMENT_CANDIDATES, *REFERENCES, *SCALED_CANDIDATES):
        assert adapter(name) is version1.to_axial_series_by_family
    with pytest.raises(KeyError):
        adapter("n3")


def test_round_one_candidates_scale_declared_bases() -> None:
    p0g = DEVELOPMENT_CANDIDATES["p0g"]
    differing = _differing(p0g.describe(), DEVELOPMENT_CANDIDATES["p0"].describe())
    assert differing == {"model_version", "max_noise_exponent"}
    assert p0g.max_noise_exponent == 0.0
    bases = {name: c.base for name, c in SCALED_CANDIDATES.items()}
    assert bases == {"p1": "p0", "p2": "p0g", "p3": "p0m"}
    for candidate in SCALED_CANDIDATES.values():
        spec = candidate.spec
        assert spec.members.history_model == "population_assimilation"
        assert spec.scale_tasks == ("next_observation", "landmark")
        assert (spec.calibration_level, spec.target_coverage) == ("80", "0.80")
        assert candidate.describe()["base"] == candidate.base


def test_overlap_clusters_are_connected_components_among_training_studies() -> None:
    operator = _operator()
    overlap = {
        "pairs": [
            {"studies": ["a", "b"]},
            {"studies": ["b", "c"]},
            {"studies": ["d", "x"]},
            {"studies": ["e", "f"]},
        ],
        "development_clusters": [["f", "g"], ["h", "y"]],
    }
    present = {"a", "b", "c", "d", "e", "f", "g", "h"}
    assert operator.clusters(overlap, present) == [["a", "b", "c"], ["e", "f", "g"]]
