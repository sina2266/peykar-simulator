"""Generic eye model version 3 (point rule), its harness, and the IOP 1.3 series."""

from __future__ import annotations

import importlib.util
import math
from dataclasses import replace
from pathlib import Path
from types import ModuleType
from typing import Any

import numpy as np
import pytest

from peykar.model_learning import eye_recovery_population2 as population
from peykar.model_learning import eye_recovery_population3 as version3
from peykar.model_learning import iop_population_1_3 as iop13
from peykar.model_learning.iop_population_1_2 import DEVELOPMENT_CANDIDATES as IOP12
from peykar.model_learning.retina_thickness_population_1_1 import DEVELOPMENT_CANDIDATES
from peykar.validation import eye_recovery_evaluation2 as evaluation2
from peykar.validation import eye_recovery_evaluation3 as evaluation3
from peykar.validation.eye_iop_data_1_3 import sub_procedure, to_iop_series_1_3
from peykar.validation.eye_retina_data import RetinaSeries
from peykar_core.errors import InvalidRequest

ROOT = Path(__file__).resolve().parents[2]
CONFIG = DEVELOPMENT_CANDIDATES["s2"]
MONTHS = (0.25, 1.0, 2.0, 4.0, 6.0)


def test_point_rule() -> None:
    assert version3.point_forecast(0.7, 0.5, 0.9, "median") == 0.7
    spread = math.log(0.9 / 0.5) / (2 * version3.Z95)
    assert version3.point_forecast(0.7, 0.5, 0.9, "lognormal_mean") == pytest.approx(
        0.7 * math.exp(spread**2 / 2)
    )
    with pytest.raises(InvalidRequest):
        version3.point_forecast(0.7, 0.0, 0.9, "lognormal_mean")
    with pytest.raises(InvalidRequest):
        version3.RecoveryConfig(
            model_id="m",
            model_version="v",
            quantity="q",
            covariate_reference=1.0,
            covariate_unit="u",
            point="mode",
        )
    config = version3.RecoveryConfig(**{**_fields(CONFIG), "point": "lognormal_mean"})
    described = config.describe()
    assert described["point"] == "lognormal_mean" and described["generic_model_version"] == "3.0.0"


def _fields(config: population.RecoveryConfig) -> dict:
    return {name: getattr(config, name) for name in config.__dataclass_fields__}


def _series() -> list[RetinaSeries]:
    rng = np.random.default_rng(17)
    contexts = {
        "human/dme/anti_vegf": (-0.3, 0.05, -0.1, 2.0),
        "human/rvo/anti_vegf": (-0.4, 0.05, -0.1, 2.0),
    }
    out = []
    for c, (context, (level, excess, transient, tau)) in enumerate(sorted(contexts.items())):
        for s in range(3):
            study = f"ret-v3{c}{s}"
            for k, n in enumerate((25, None)):
                reference = float(rng.uniform(320, 600))
                x = math.log(reference / 300.0)
                beta = np.array([level - 0.5 * x + rng.normal(0, 0.03), excess, transient])
                curve = np.exp(
                    population.design(CONFIG, MONTHS, np.array([math.log(tau)]))[0] @ beta
                )
                values = curve * np.exp(rng.normal(0, 0.03 if n else 0.06, len(MONTHS)))
                out.append(
                    RetinaSeries(
                        series_id=f"{study}/{k}",
                        study_id=study,
                        context=context,
                        reference_value=reference,
                        months=MONTHS,
                        fractions=tuple(float(v) for v in values),
                        aggregate_n=n,
                    )
                )
    return out


def _strip(report: dict) -> dict:
    return {
        **report,
        "evaluation_version": None,
        "studies": [{**s, "evaluation_version": None} for s in report["studies"]],
    }


def test_version_3_harness_reproduces_version_2_with_the_median() -> None:
    series = _series()
    two = evaluation2.leave_one_study_out(CONFIG, series, studies=["ret-v300"])
    three = evaluation3.leave_one_study_out(CONFIG, series, studies=["ret-v300"])
    assert three["evaluation_version"] == "3.0.0"
    assert _strip(two) == _strip(three)


def test_lognormal_mean_moves_only_population_points() -> None:
    series = _series()
    median = evaluation3.leave_one_study_out(CONFIG, series, studies=["ret-v310"])
    config = version3.RecoveryConfig(**{**_fields(CONFIG), "point": "lognormal_mean"})
    mean = evaluation3.leave_one_study_out(config, series, studies=["ret-v310"])
    a, b = median["studies"][0], mean["studies"][0]
    for task in ("next_observation", "landmark"):
        for ea, eb in zip(a[task], b[task], strict=True):
            for ra, rb in zip(ea["rows"], eb["rows"], strict=True):
                assert ra["intervals"] == rb["intervals"]
                assert ra["predictions"]["persistence"] == rb["predictions"]["persistence"]
                assert (
                    rb["predictions"]["population_assimilation"]
                    > (ra["predictions"]["population_assimilation"])
                )
    ia = next(e for e in a["initial_state"] if e["kind"] == "group_mean")
    ib = next(e for e in b["initial_state"] if e["kind"] == "group_mean")
    for p, q in zip(
        ia["predictions"]["population_initial_state"],
        ib["predictions"]["population_initial_state"],
        strict=True,
    ):
        assert q > p


def test_iop_sub_procedures() -> None:
    assert sub_procedure("tube", "Ahmed valve (FP7)") == "valved"
    assert sub_procedure("tube", "non-valved Baerveldt 350") == "non_valved"
    assert sub_procedure("tube", "Paul glaucoma implant") == "non_valved"
    assert sub_procedure("angle_mims", "phaco + iStent inject W") == "stent"
    assert sub_procedure("angle_mims", "Kahook Dual Blade goniotomy") == "goniotomy"
    assert sub_procedure("angle_mims", "Trabectome ab interno trabeculectomy") == "goniotomy"
    assert sub_procedure("angle_mims", "GATT, 360 degrees") == "trabeculotomy"
    assert sub_procedure("angle_mims", "ab externo canaloplasty") == "canaloplasty"
    assert sub_procedure("angle_mims", "lens extraction and goniosynechialysis") == "other"
    assert sub_procedure("angle_mims", "POAG eyes", "Long-term outcomes of KDB") == "goniotomy"
    assert sub_procedure("cyclodestruction", "MicroPulse transscleral laser") == "micropulse"
    assert sub_procedure("cyclodestruction", "endoscopic cyclophotocoagulation") == "endoscopic"
    assert sub_procedure("cyclodestruction", "ultrasound cycloplasty (EyeOP1)") == "ultrasound"
    assert sub_procedure("cyclodestruction", "slow-coagulation TSCPC (G-Probe)") == "continuous"
    assert sub_procedure("subconjunctival_mims", "PreserFlo MicroShunt") == "preserflo"
    assert sub_procedure("subconjunctival_mims", "XEN 45 gel stent") == "xen"
    assert sub_procedure("trabeculectomy", "trabeculectomy with MMC") == "all"
    record = {
        "schema": "eye.iop-record-1.0.0",
        "study_id": "iop-synthetic",
        "doi": "10.0/x",
        "pmcid": "PMC0",
        "title": "Ahmed versus Baerveldt",
        "species": "human",
        "procedure": "tube",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "indication": "poag",
        "groups": [
            {"group": "agv", "role": "control", "n_reported": 30, "description": "Ahmed valve"},
            {"group": "bgi", "role": "control", "n_reported": 30, "description": "Baerveldt"},
        ],
        "extraction_protocol": "x",
        "exclusions": [],
        "notes": "",
        "series": [
            {
                "series_id": f"{group}/iop",
                "group": group,
                "kind": "group_mean",
                "n": 30,
                "value_kind": "iop_absolute",
                "reference_kind": "preoperative_medicated",
                "method": "goldmann",
                "months": [0.0, 1.0, 6.0],
                "values": [28.0, 14.0, 15.0],
                "source_cells": ["a.xml::T1!R1C1", "a.xml::T1!R1C2", "a.xml::T1!R1C3"],
            }
            for group in ("agv", "bgi")
        ],
    }
    try:
        series = to_iop_series_1_3(record)
    except InvalidRequest:
        pytest.skip("synthetic record does not satisfy the full IOP record schema")
    assert sorted(s.context for s in series) == ["human/non_valved/tube", "human/valved/tube"]


def test_iop_1_3_candidates() -> None:
    assert iop13.DEVELOPMENT_CANDIDATES["d0"] is IOP12["c1"]
    d1 = iop13.DEVELOPMENT_CANDIDATES["d1"]
    assert d1.grouping == ("treatment", "context")
    assert replace(d1, model_version="1.2.0-c1", grouping=("context",)) == IOP12["c1"]
    assert set(iop13.SUB_PROCEDURE_CANDIDATES) == {"d1"}


def _publisher() -> ModuleType:
    path = ROOT / "scripts/publish_iop_population_1_3.py"
    spec = importlib.util.spec_from_file_location("publish_iop_population_1_3", path)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def test_iop_1_3_publisher_tests_human_series_and_allow_lists_redacted_fields() -> None:
    publish = _publisher()
    score = {"n": 1, "rmse_pp": 2.0, "max_abs_pp": 2.0, "signed_errors_pp": [2.0]}

    def entry(context: str) -> dict[str, Any]:
        return {
            "series_id": f"x/{context}",
            "context": context,
            "role": "control",
            "kind": "group_mean",
            "rows": [
                {
                    "origin_month": 0.5,
                    "target_month": 1.0,
                    "observed": 0.8,
                    "history_points": 1,
                    "predictions": {"persistence": 0.78},
                    "intervals": {},
                }
            ],
            "scores": {"persistence": dict(score)},
            "interval_coverage": {},
        }

    initial = {
        "series_id": "x/human",
        "context": "human/valved/tube",
        "kind": "group_mean",
        "reference_value": 24.0,
        "months": [1.0],
        "observed": [0.7],
        "predictions": {"population_initial_state": [0.72]},
        "population_level_used": "human/valved/tube",
        "scores": {"population_initial_state": dict(score)},
    }
    study = {
        "study_id": "x",
        "initial_state": [initial],
        "next_observation": [entry("human/valved/tube"), entry("rabbit/trabeculectomy")],
        "landmark": [entry("rabbit/trabeculectomy")],
    }
    tested = publish._split({"studies": [study]}, tested=True)[0]
    assert [e["context"] for e in tested["next_observation"]] == ["human/valved/tube"]
    assert tested["landmark"] == [] and len(tested["initial_state"]) == 1
    untested = publish._split({"studies": [study]}, tested=False)[0]
    assert [e["context"] for e in untested["next_observation"]] == ["rabbit/trabeculectomy"]
    assert untested["initial_state"] == []
    redacted = publish.redact(study)
    kept = redacted["initial_state"][0]
    assert set(kept) <= publish.INITIAL_STATE_PUBLIC_FIELDS
    assert not {"reference_value", "observed", "predictions"} & set(kept)
    assert all("signed_errors_pp" not in s for s in kept["scores"].values())
    assert set(redacted["next_observation"][0]["rows"][0]) == {
        "origin_month",
        "target_month",
        "history_points",
    }
    assert study["initial_state"][0]["reference_value"] == 24.0


def test_iop_1_3_h5_rule() -> None:
    publish = _publisher()

    def h5(next_rmse: float, landmark: float, initial: float) -> dict[str, Any]:
        return {
            "next_observation": publish._comparison(10, next_rmse, 6.0),
            "landmark": publish._comparison(10, landmark, 8.0),
            "initial_state": publish._comparison(10, initial, 10.0),
        }

    assert publish.h5_passes(h5(6.04, 7.9, 9.9))
    assert not publish.h5_passes(h5(6.06, 7.9, 9.9))
    assert not publish.h5_passes(h5(5.0, 8.1, 9.9))
    assert not publish.h5_passes(h5(5.0, 7.9, 10.1))
    empty = h5(5.0, 7.9, 9.9)
    empty["landmark"]["common_targets"] = 0
    assert not publish.h5_passes(empty)
