"""Childhood-myopia axial-length model 1.1 configurations (synthetic record)."""

from __future__ import annotations

import pytest

from peykar.model_learning import myopia_population as version1
from peykar.model_learning.myopia_population_1_1 import (
    DEVELOPMENT_CANDIDATES,
    REFERENCES,
    adapter,
)


def _record() -> dict:
    return {
        "schema": "eye.myopia-axial-record-1.0.0",
        "study_id": "myo-pmc1",
        "doi": "10.1/x",
        "species": "human",
        "treatment": "atropine",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [
            {
                "group": "atropine-0.01",
                "role": "control",
                "region": "east_asia",
                "atropine_percent": 0.01,
            },
            {
                "group": "placebo",
                "role": "control",
                "treatment": "single_vision",
                "region": "east_asia",
            },
        ],
        "series": [
            {
                "series_id": "atropine-0.01/al",
                "group": "atropine-0.01",
                "kind": "group_mean",
                "n": 110,
                "device": "iolmaster",
                "value_kind": "axial_absolute",
                "months": [0, 6, 12],
                "values": [24.70, 24.85, 24.98],
                "source_cells": ["a", "b", "c"],
            },
            {
                "series_id": "placebo/al",
                "group": "placebo",
                "kind": "group_mean",
                "n": 111,
                "device": "iolmaster",
                "value_kind": "axial_absolute",
                "months": [0, 12],
                "values": [24.82, 25.20],
                "source_cells": ["d", "e"],
            },
        ],
    }


def test_round_zero_candidate_is_model_one_with_a_new_version_string() -> None:
    frozen = version1.DEVELOPMENT_CANDIDATES["m4"]
    assert REFERENCES["1.0.0"] is frozen
    n0 = DEVELOPMENT_CANDIDATES["n0"]
    assert n0.model_version == "1.1.0-n0" and frozen.model_version == "1.0.0-m4"
    differing = {key for key, value in n0.describe().items() if frozen.describe()[key] != value}
    assert differing == {"model_version"}


def test_every_candidate_reads_family_contexts() -> None:
    for name in (*DEVELOPMENT_CANDIDATES, *REFERENCES):
        contexts = [s.context for s in adapter(name)(_record())]
        assert contexts == ["human/atropine/atropine", "human/single_vision/control"]
    with pytest.raises(KeyError):
        adapter("m0")
