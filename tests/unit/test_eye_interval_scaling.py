"""Model-only interval scaling 1.0.0 (synthetic axial-length series)."""

from __future__ import annotations

import math

import numpy as np
import pytest

from peykar.model_learning.eye_stacking import StackingSpec
from peykar.model_learning.eye_stacking2 import HistoryStacking, HistoryStackingSpec
from peykar.model_learning.myopia_population_1_1 import DEVELOPMENT_CANDIDATES
from peykar.validation import eye_additive_evaluation5 as additive
from peykar.validation.eye_interval_scaling import (
    MODEL_ONLY,
    fit_scale,
    nested_scale_report,
    scale_report,
)
from peykar.validation.myopia_axial_data import (
    COVARIATE_UNIT_MM,
    MODEL_UNIT_MM,
    REPORT_UNIT_MM,
    AxialSeries,
)
from peykar_core.errors import InvalidRequest

MODEL = "population_assimilation"
SCALE = additive.AdditiveScale(
    quantity="axial length",
    unit="mm",
    model_unit=MODEL_UNIT_MM,
    covariate_unit=COVARIATE_UNIT_MM,
    report_unit=REPORT_UNIT_MM,
)


def _synthetic() -> list[AxialSeries]:
    rng = np.random.default_rng(7)
    rates = {"human/single_vision/control": 0.36, "human/orthokeratology/optical": 0.18}
    months = (6.0, 12.0, 18.0, 24.0)
    out = []
    for c, (context, rate) in enumerate(sorted(rates.items())):
        for s in range(3):
            study = f"st-{c}-{s}"
            for k in range(2):
                t = np.asarray(months)
                change = rate * 4.0 * (1.0 - np.exp(-t / 48.0)) + rng.normal(0.0, 0.02, t.size)
                out.append(
                    AxialSeries(
                        series_id=f"{study}/{k}",
                        study_id=study,
                        context=context,
                        reference_mm=float(rng.uniform(23.5, 25.5)),
                        months=months,
                        changes_mm=tuple(float(x) for x in change),
                        aggregate_n=60,
                    )
                )
    return out


def _report() -> dict:
    series = _synthetic()
    return additive.leave_one_study_out(
        DEVELOPMENT_CANDIDATES["n0"],
        series,
        scale=SCALE,
        clusters=[["st-0-0", "st-0-1"]],
    )


def _spec(grid: tuple[str, str, str] = ("0.30", "3.00", "0.01")) -> HistoryStackingSpec:
    return HistoryStackingSpec(members=StackingSpec(history_model=MODEL), scale_grid=grid)


def test_nested_scaling_keeps_the_points_and_scales_the_intervals() -> None:
    report = _report()
    scaled = nested_scale_report(report, _spec())
    block = scaled["interval_scaling"]
    assert block["evaluation_version"] == "1.0.0"
    assert set(block["scaling_by_held_out_study"]) == {s["study_id"] for s in report["studies"]}
    everything = fit_scale(report["studies"], _spec())
    assert block["scaling_all_studies"] == everything.describe()
    checked = 0
    for study in scaled["studies"]:
        factor = block["scaling_by_held_out_study"][study["study_id"]]["interval_scale"]
        assert 0.30 <= factor <= 3.00
        for task in ("next_observation", "landmark"):
            for entry in study[task]:
                for row in entry["rows"]:
                    model = row["predictions"][MODEL]
                    assert row["predictions"]["stacked"] == model
                    for q, value in row["intervals"]["stacked"].items():
                        expected = model + factor * (row["intervals"][MODEL][q] - model)
                        assert value == pytest.approx(expected, abs=1e-12)
                    checked += 1
    assert checked > 0
    # The excluded cluster: both linked studies were held out together.
    assert "st-0-1" in scaled["clusters"][0]


def test_a_unit_grid_reproduces_the_model_intervals_and_frozen_scales_apply() -> None:
    report = _report()
    unit = nested_scale_report(report, _spec(("1.00", "1.00", "0.01")))
    for study in unit["studies"]:
        for entry in study["next_observation"]:
            for row in entry["rows"]:
                for q, value in row["intervals"]["stacked"].items():
                    assert value == pytest.approx(row["intervals"][MODEL][q], abs=1e-12)
    frozen = HistoryStacking(weights=MODEL_ONLY, interval_scale=0.5)
    applied = scale_report(report, frozen, _spec())
    assert applied["interval_scaling"]["mode"] == "frozen scale"
    row = applied["studies"][0]["next_observation"][0]["rows"][0]
    m = row["predictions"][MODEL]
    width = row["intervals"]["stacked"]["0.9"] - row["intervals"]["stacked"]["0.1"]
    assert width == pytest.approx(
        0.5 * (row["intervals"][MODEL]["0.9"] - row["intervals"][MODEL]["0.1"])
    )
    assert math.isclose(row["predictions"]["stacked"], m)
    with pytest.raises(InvalidRequest):
        scale_report(report, HistoryStacking(weights=(0.5, 0.5, 0.0), interval_scale=1.0), _spec())
