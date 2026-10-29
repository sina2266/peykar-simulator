"""Scenario forecasts, stored fits and the eye CLI on synthetic records."""

from __future__ import annotations

import json
import math
from pathlib import Path

import numpy as np
import pytest
from typer.testing import CliRunner

from peykar import eye_simulation
from peykar.eye_cli import eye_app
from peykar.model_learning import eye_recovery_population2 as population
from peykar.model_learning.eye_forecast import (
    check_support,
    fit_from_json,
    fit_to_json,
    forecast_scenario,
    time_support,
)
from peykar.model_learning.retina_thickness_population_1_1 import DEVELOPMENT_CANDIDATES
from peykar.validation.eye_retina_data import RetinaSeries
from peykar_core.errors import InvalidRequest

CONFIG = DEVELOPMENT_CANDIDATES["s5"]
MONTHS = (0.25, 1.0, 2.0, 4.0, 6.0)


def _series() -> list[RetinaSeries]:
    rng = np.random.default_rng(17)
    contexts = {
        "human/dme/anti_vegf": (-0.3, 0.05, -0.1, 2.0, None),
        "human/rvo/anti_vegf": (-0.4, 0.05, -0.1, 2.0, None),
        "human/dme/steroid_implant": (-0.4, 0.1, -0.05, 0.5, 3.5),
    }
    out = []
    for c, (context, (level, excess, transient, tau, duration)) in enumerate(
        sorted(contexts.items())
    ):
        for s in range(3):
            for k, n in enumerate((25, None)):
                reference = float(rng.uniform(320, 600))
                x = math.log(reference / 300.0)
                beta = np.array([level - 0.5 * x + rng.normal(0, 0.03), excess, transient])
                curve = np.exp(
                    population.design(CONFIG, MONTHS, np.array([math.log(tau)]), duration)[0] @ beta
                )
                values = curve * np.exp(rng.normal(0, 0.03 if n else 0.06, len(MONTHS)))
                out.append(
                    RetinaSeries(
                        series_id=f"st{c}{s}/{k}",
                        study_id=f"st{c}{s}",
                        context=context,
                        reference_value=reference,
                        months=MONTHS,
                        fractions=tuple(float(v) for v in values),
                        aggregate_n=n,
                        metadata={"effect_duration_months": duration},
                    )
                )
    return out


def test_stored_fit_round_trips_and_forecasts_match_the_harness_path() -> None:
    series = _series()
    fit = population.fit_population(CONFIG, series)
    stored = json.loads(json.dumps(fit_to_json(fit, note="test")))
    again = fit_from_json(CONFIG, stored)
    for name, family in fit.families.items():
        np.testing.assert_array_equal(family.solution, again.families[name].solution)
        np.testing.assert_array_equal(family.covariance, again.families[name].covariance)
    assert dict(again.noise_centers.centers) == dict(fit.noise_centers.centers)
    assert again.forecast_inflation("aggregate", "human/dme/anti_vegf", 30) == pytest.approx(
        fit.forecast_inflation("aggregate", "human/dme/anti_vegf", 30)
    )
    with pytest.raises(InvalidRequest):
        fit_from_json(DEVELOPMENT_CANDIDATES["s2"], stored)
    result = forecast_scenario(
        CONFIG,
        again,
        context="human/dme/anti_vegf",
        baseline=450.0,
        months=[4.0, 6.0],
        group_size=30,
        observed=[(1.0, 360.0), (2.0, 350.0)],
    )
    # The same computation through the public model functions.
    observed = RetinaSeries(
        series_id="x/1",
        study_id="x",
        context="human/dme/anti_vegf",
        reference_value=450.0,
        months=(1.0, 2.0),
        fractions=(360.0 / 450.0, 350.0 / 450.0),
        aggregate_n=30,
    )
    prior = fit.prior(observed.context, observed.kind, population.level_covariate(CONFIG, observed))
    direct = population.predictive(
        CONFIG,
        population.posterior(
            CONFIG,
            population.gaussian(prior["mean"], prior["covariance"]),
            series=observed,
            noise=fit.noise("aggregate", observed.context, 30),
        ),
        months=[4.0, 6.0],
        quantiles=(0.05, 0.1, 0.5, 0.9, 0.95),
        extra_noise=fit.forecast_inflation("aggregate", observed.context, 30),
    )
    assert result["method"] == "population_assimilation"
    for i, row in enumerate(result["forecast"]):
        assert row["fraction"]["point"] == pytest.approx(direct["quantiles"]["0.5"][i])
        assert row["value"]["interval_90"][1] == pytest.approx(450 * direct["quantiles"]["0.95"][i])


def test_initial_state_uses_the_duration_and_warns_for_untrained_contexts() -> None:
    fit = population.fit_population(CONFIG, _series())
    with_rule = forecast_scenario(
        CONFIG,
        fit,
        context="human/dme/steroid_implant",
        baseline=500.0,
        months=[2.0, 6.0],
        group_size=30,
        effect_duration_months=3.5,
    )
    without = forecast_scenario(
        CONFIG,
        fit,
        context="human/dme/steroid_implant",
        baseline=500.0,
        months=[2.0, 6.0],
        group_size=30,
    )
    assert with_rule["method"] == "population_initial_state" and not with_rule["warnings"]
    assert (
        with_rule["forecast"][1]["fraction"]["point"] > without["forecast"][1]["fraction"]["point"]
    )
    unknown = forecast_scenario(
        CONFIG,
        fit,
        context="human/csc/laser",
        baseline=400.0,
        months=[1.0],
        observed=[(2.0, 300.0)],
    )
    assert unknown["level_used"] == "family:human/all"
    assert len(unknown["warnings"]) == 3
    with pytest.raises(InvalidRequest):
        forecast_scenario(CONFIG, fit, context="human/dme/anti_vegf", baseline=-1, months=[1.0])
    with pytest.raises(InvalidRequest):
        forecast_scenario(
            CONFIG,
            fit,
            context="human/dme/anti_vegf",
            baseline=400,
            months=[3.0],
            observed=[(1.0, 300.0), (1.0, 310.0)],
        )


def _record(index: int, context_treatment: str, agent: str) -> dict:
    rng = np.random.default_rng(index)
    months = [0.0, 0.5, 1.0, 2.0, 4.0, 6.0]
    base = float(rng.uniform(350, 550))
    values = [base, *(base * float(v) for v in rng.uniform(0.6, 0.9, 5))]
    return {
        "schema": "eye.retina-thickness-record-1.0.0",
        "study_id": f"ret-synthetic-{index}",
        "doi": f"10.0/{index}",
        "species": "human",
        "indication": "dme",
        "treatment": context_treatment,
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [{"group": "g", "role": "control", "agent": agent}],
        "series": [
            {
                "series_id": "g/cst",
                "group": "g",
                "kind": "group_mean",
                "n": 30,
                "value_kind": "thickness_absolute",
                "measure": "central_subfield",
                "device": "spectralis",
                "regimen": "single" if context_treatment == "steroid_implant" else "prn",
                "prior_treatment": "naive",
                "months": months,
                "values": values,
                "source_cells": [f"a.xml::T1!R{i}C1" for i in range(len(months))],
            }
        ],
    }


def test_cli_fits_and_forecasts_from_record_folders(tmp_path: Path) -> None:
    folder = tmp_path / "records"
    folder.mkdir()
    for i in range(6):
        treatment, agent = (
            ("steroid_implant", "dexamethasone implant (Ozurdex)")
            if i % 2
            else ("anti_vegf", "ranibizumab")
        )
        (folder / f"r{i}.json").write_text(json.dumps(_record(i, treatment, agent)))
    runner = CliRunner()
    fit_path = tmp_path / "fit.json"
    result = runner.invoke(
        eye_app,
        [
            "fit",
            "--component",
            "retina-thickness",
            "--records",
            str(folder),
            "--output",
            str(fit_path),
        ],
    )
    assert result.exit_code == 0, result.output
    assert json.loads(result.output)["records"] == 6
    again = runner.invoke(
        eye_app,
        [
            "fit",
            "--component",
            "retina-thickness",
            "--records",
            str(folder),
            "--output",
            str(fit_path),
        ],
    )
    assert again.exit_code != 0
    result = runner.invoke(
        eye_app,
        [
            "forecast",
            "--component",
            "retina-thickness",
            "--fit",
            str(fit_path),
            "--context",
            "human/dme/steroid_implant",
            "--baseline",
            "480",
            "--months",
            "2,6",
            "--n",
            "30",
            "--regimen",
            "single",
            "--agent",
            "Ozurdex",
        ],
    )
    assert result.exit_code == 0, result.output
    payload = json.loads(result.output)
    assert payload["effect_duration_months"] == 3.5 and payload["model_version"] == "1.1.0"
    assert payload["fit_records"] == 6 and len(payload["forecast"]) == 2
    listed = runner.invoke(eye_app, ["components"])
    assert listed.exit_code == 0
    assert {c["component"] for c in json.loads(listed.output)} == {
        *eye_simulation.COMPONENTS,
        *eye_simulation.CORNEAL_COMPONENTS,
        *eye_simulation.ADDITIVE_COMPONENTS,
    }
    with pytest.raises(InvalidRequest):
        eye_simulation.component("lens")


def _corneal_series(kind: str) -> list:
    from peykar.validation.cornea_endothelium_data import DensitySeries
    from peykar.validation.cornea_thickness_data import ThicknessSeries

    rng = np.random.default_rng(23)
    months = (0.0329, 0.23, 1.0, 3.0, 6.0, 12.0)
    out = []
    for s in range(4):
        for context in ("human/cataract", "human/glaucoma"):
            for k, n in enumerate((30, None)):
                if kind == "endothelium":
                    fractions = tuple(
                        float(v)
                        for v in 0.9
                        * (1 + np.asarray(months) / 6.0) ** -0.05
                        * np.exp(rng.normal(0, 0.02, len(months)))
                    )
                    out.append(
                        DensitySeries(
                            series_id=f"e{s}/{context}/{k}",
                            study_id=f"e{s}",
                            context=context,
                            reference_ecd=2500.0,
                            months=months,
                            fractions=fractions,
                            aggregate_n=n,
                        )
                    )
                else:
                    fractions = tuple(
                        float(v)
                        for v in np.exp(0.1 / (1 + np.asarray(months) / 0.2))
                        * np.exp(rng.normal(0, 0.01, len(months)))
                    )
                    out.append(
                        ThicknessSeries(
                            series_id=f"t{s}/{context}/{k}",
                            study_id=f"t{s}",
                            context=context,
                            reference_um=540.0,
                            months=months,
                            fractions=fractions,
                            aggregate_n=n,
                        )
                    )
    return out


@pytest.mark.parametrize("name", ["corneal-endothelium", "corneal-thickness"])
def test_corneal_components_use_their_frozen_primaries(name: str) -> None:
    from peykar.model_learning import endothelial_population, thickness_population

    item = eye_simulation.CORNEAL_COMPONENTS[name]
    series = _corneal_series("endothelium" if "endothelium" in name else "thickness")
    fit = eye_simulation.fit_corneal(item, series)
    initial = eye_simulation.forecast_corneal(
        item,
        fit,
        context="human/cataract",
        baseline=2500.0 if "endo" in name else 540.0,
        months=[1.0, 6.0],
        group_size=30,
    )
    assert initial["method"].startswith("population_initial_state") and not initial["warnings"]
    observed = eye_simulation.forecast_corneal(
        item,
        fit,
        context="human/cataract",
        baseline=2500.0 if "endo" in name else 540.0,
        months=[6.0],
        group_size=30,
        observed=[
            (1.0, 2200.0 if "endo" in name else 545.0),
            (3.0, 2150.0 if "endo" in name else 541.0),
        ],
    )
    prefix = type(series[0])(
        **{
            **{f: getattr(series[0], f) for f in series[0].__dataclass_fields__},
            "series_id": "scenario/series",
            "study_id": "scenario",
            "context": "human/cataract",
            "months": (1.0, 3.0),
            "fractions": (2200 / 2500, 2150 / 2500) if "endo" in name else (545 / 540, 541 / 540),
            "aggregate_n": 30,
        }
    )
    if "endo" in name:
        model = endothelial_population
        prior = fit.prior("human/cataract", "aggregate")
        direct = model.predictive(
            model.anchored_posterior(
                prior, series=prefix, noise=fit.noise("aggregate", "human/cataract"), student_df=4.0
            ),
            months=[6.0],
            draws=2000,
            seed=0,
        )
    else:
        tmodel = thickness_population
        prior = fit.prior("human/cataract", "aggregate")
        direct = tmodel.predictive(
            tmodel.posterior(
                tmodel.robust(tmodel.gaussian(prior["mean"], prior["covariance"])),
                series=prefix,
                noise=fit.noise("aggregate", "human/cataract"),
                offset=0.0,
            ),
            months=[6.0],
        )
    assert observed["forecast"][0]["fraction"]["point"] == pytest.approx(direct["mean_fraction"][0])
    assert observed["point"] == "predictive mean"
    unknown = eye_simulation.forecast_corneal(
        item,
        fit,
        context="human/vitreoretinal",
        baseline=2500.0 if "endo" in name else 540.0,
        months=[3.0],
    )
    assert unknown["level_used"].startswith("family:") and len(unknown["warnings"]) == 2
    with pytest.raises(InvalidRequest):
        eye_simulation.forecast_corneal(
            item, fit, context="human/cataract", baseline=500.0, months=[1.0], reference_kind="x"
        )


def _refraction_record(index: int) -> dict:
    rng = np.random.default_rng(100 + index)
    se0 = float(rng.uniform(-8.0, -2.0))
    months = [0.0, 0.0329, 1.0, 3.0, 6.0, 12.0]
    drift = [0.3, 0.1, 0.0, -0.1, -0.2]
    values = [se0, *(0.1 * se0 + d + float(rng.normal(0, 0.05)) for d in drift)]
    return {
        "schema": "cornea.refraction-record-1.0.0",
        "study_id": f"ref-synthetic-{index}",
        "doi": f"10.0/ref{index}",
        "species": "human",
        "procedure": "lasik",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [
            {"group": "g", "role": "control", "refractive_error": "myopia", "target": "plano"}
        ],
        "series": [
            {
                "series_id": "g/se",
                "group": "g",
                "kind": "group_mean",
                "n": 40,
                "value_kind": "se_absolute",
                "method": "manifest",
                "months": months,
                "values": values,
                "source_cells": [f"a.xml::T1!R{i}C1" for i in range(len(months))],
            }
        ],
    }


def test_refraction_component_forecasts_in_dioptres(tmp_path: Path) -> None:
    folder = tmp_path / "refraction"
    folder.mkdir()
    for i in range(5):
        (folder / f"r{i}.json").write_text(json.dumps(_refraction_record(i)))
    item = eye_simulation.any_component("refraction")
    assert isinstance(item, eye_simulation.AdditiveComponent)
    runner = CliRunner()
    result = runner.invoke(
        eye_app,
        [
            "forecast",
            "--component",
            "refraction",
            "--context",
            "human/myopia/lasik",
            "--baseline",
            "-5",
            "--months",
            "6,12",
            "--n",
            "40",
            "--records",
            str(folder),
        ],
    )
    assert result.exit_code == 0, result.output
    payload = json.loads(result.output)
    assert payload["unit"] == "D" and payload["method"] == "population_initial_state"
    for row in payload["forecast"]:
        low, high = row["value"]["interval_90"]
        assert low < row["value"]["point"] < high
        assert -1.5 < row["value"]["point"] < 0.5
    observed = runner.invoke(
        eye_app,
        [
            "forecast",
            "--component",
            "refraction",
            "--context",
            "human/myopia/lasik",
            "--baseline",
            "-5",
            "--months",
            "12",
            "--n",
            "40",
            "--observed",
            "1:-0.5,3:-0.6",
            "--records",
            str(folder),
        ],
    )
    assert observed.exit_code == 0, observed.output
    forecast = json.loads(observed.output)
    assert forecast["method"] == "population_assimilation_robust"
    assert forecast["observed"][0]["change"] == pytest.approx(4.5)
    assert abs(forecast["forecast"][0]["value"]["point"] + 0.6) < 0.5
    refused = runner.invoke(
        eye_app,
        [
            "forecast",
            "--component",
            "refraction",
            "--context",
            "human/myopia/lasik",
            "--baseline",
            "-5",
            "--months",
            "12",
            "--regimen",
            "single",
            "--records",
            str(folder),
        ],
    )
    assert refused.exit_code != 0


def _acuity_record(index: int) -> dict:
    rng = np.random.default_rng(200 + index)
    logmar0 = float(rng.uniform(0.6, 1.2))
    months = [0.0, 0.25, 1.0, 3.0, 6.0, 12.0]
    recovery = [0.7, 0.5, 0.35, 0.3, 0.28]
    values = [logmar0, *(r * logmar0 + float(rng.normal(0, 0.02)) for r in recovery)]
    return {
        "schema": "cornea.acuity-record-1.0.0",
        "study_id": f"acu-synthetic-{index}",
        "doi": f"10.0/acu{index}",
        "species": "human",
        "procedure": "dmek",
        "split": "development",
        "license": {"statement": "CC BY 4.0"},
        "source_files": [{"name": "a.xml", "sha256": "0" * 64}],
        "groups": [{"group": "g", "role": "control", "indication": "fuchs", "combined": None}],
        "series": [
            {
                "series_id": "g/bcva",
                "group": "g",
                "kind": "group_mean",
                "n": 30,
                "value_kind": "logmar_absolute",
                "months": months,
                "values": values,
                "source_cells": [f"a.xml::T1!R{i}C1" for i in range(len(months))],
            }
        ],
    }


def test_acuity_component_forecasts_in_logmar(tmp_path: Path) -> None:
    folder = tmp_path / "acuity"
    folder.mkdir()
    for i in range(5):
        (folder / f"a{i}.json").write_text(json.dumps(_acuity_record(i)))
    item = eye_simulation.any_component("acuity")
    assert isinstance(item, eye_simulation.AdditiveComponent)
    assert item.describe()["candidate"] == "a0" and item.unit == "logMAR"
    runner = CliRunner()
    base = ["forecast", "--component", "acuity", "--context", "human/dmek", "--baseline", "0.9"]
    result = runner.invoke(
        eye_app, [*base, "--months", "6,12", "--n", "30", "--records", str(folder)]
    )
    assert result.exit_code == 0, result.output
    payload = json.loads(result.output)
    assert payload["unit"] == "logMAR" and payload["method"] == "population_initial_state"
    for row in payload["forecast"]:
        low, high = row["value"]["interval_90"]
        assert low < row["value"]["point"] < high
        assert 0.0 < row["value"]["point"] < 0.6
    observed = runner.invoke(
        eye_app,
        [*base, "--months", "12", "--observed", "1:0.5,3:0.35", "--records", str(folder)],
    )
    assert observed.exit_code == 0, observed.output
    forecast = json.loads(observed.output)
    assert forecast["method"] == "population_assimilation_robust"
    assert forecast["observed"][0]["change"] == pytest.approx(-0.4)
    assert abs(forecast["forecast"][0]["value"]["point"] - 0.3) < 0.15
    assert forecast["warnings"] == ["single-eye forecasts were scored but not tested"]


def test_time_support_flags_months_outside_the_training_visits() -> None:
    series = _series()
    support = time_support(series)
    assert support["earliest_month"] == 0.25 and support["latest_month"] == 6.0
    assert support["contexts"]["human/dme/anti_vegf"] == [0.25, 6.0]
    fit = population.fit_population(CONFIG, series)
    common = {"context": "human/dme/anti_vegf", "baseline": 450.0, "group_size": 30}
    inside = forecast_scenario(CONFIG, fit, months=[1.0, 6.0], support=support, **common)
    assert not inside["warnings"]
    assert inside["training_support"] == {
        "earliest_month": 0.25,
        "latest_month": 6.0,
        "context_months": [0.25, 6.0],
    }
    outside = forecast_scenario(
        CONFIG, fit, months=[0.1, 12.0], observed=[(0.05, 430.0)], support=support, **common
    )
    texts = outside["warnings"]
    assert len(texts) == 3
    assert texts[0].startswith("observed follow-ups before the earliest training visit (0.25")
    assert texts[1].startswith("forecast months before the earliest training visit (0.25")
    assert texts[2].startswith("forecast months after the latest training visit (6 months)")
    # The check only adds warnings: the forecast is the same without it.
    plain = forecast_scenario(CONFIG, fit, months=[0.1, 12.0], observed=[(0.05, 430.0)], **common)
    assert plain["forecast"] == outside["forecast"] and plain["training_support"] is None
    view, none = check_support(
        support, context="human/csc/laser", targets=[1.0], observed_months=[]
    )
    assert view is not None and view["context_months"] is None and none == []
    with pytest.raises(InvalidRequest):
        time_support([])


def test_cli_stores_the_training_support_and_warns_outside_it(tmp_path: Path) -> None:
    folder = tmp_path / "records"
    folder.mkdir()
    for i in range(6):
        (folder / f"r{i}.json").write_text(json.dumps(_record(i, "anti_vegf", "ranibizumab")))
    runner = CliRunner()
    fit_path = tmp_path / "fit.json"
    fitted = runner.invoke(
        eye_app,
        [
            "fit",
            "--component",
            "retina-thickness",
            "--records",
            str(folder),
            "--output",
            str(fit_path),
        ],
    )
    assert fitted.exit_code == 0, fitted.output
    assert json.loads(fitted.output)["training_months"] == [0.5, 6.0]
    stored = json.loads(fit_path.read_text())
    assert stored["provenance"]["training_support"]["earliest_month"] == 0.5
    base = [
        "forecast",
        "--component",
        "retina-thickness",
        "--context",
        "human/dme/anti_vegf",
        "--baseline",
        "480",
        "--n",
        "30",
    ]
    early = runner.invoke(eye_app, [*base, "--months", "0.25,2", "--fit", str(fit_path)])
    assert early.exit_code == 0, early.output
    payload = json.loads(early.output)
    assert payload["training_support"]["earliest_month"] == 0.5
    assert any("before the earliest training visit (0.5 months)" in w for w in payload["warnings"])
    fresh = runner.invoke(eye_app, [*base, "--months", "2", "--records", str(folder)])
    assert fresh.exit_code == 0, fresh.output
    assert json.loads(fresh.output)["warnings"] == []
    del stored["provenance"]["training_support"]
    old_fit = tmp_path / "old-fit.json"
    old_fit.write_text(json.dumps(stored))
    legacy = runner.invoke(eye_app, [*base, "--months", "2", "--fit", str(old_fit)])
    assert legacy.exit_code == 0, legacy.output
    legacy_payload = json.loads(legacy.output)
    assert legacy_payload["training_support"] is None
    assert legacy_payload["warnings"] == [
        "this stored fit does not record its training visit months; refit it to check the "
        "forecast months against them"
    ]


def test_additive_and_corneal_forecasts_report_the_training_support(tmp_path: Path) -> None:
    folder = tmp_path / "acuity"
    folder.mkdir()
    for i in range(5):
        (folder / f"a{i}.json").write_text(json.dumps(_acuity_record(i)))
    runner = CliRunner()
    base = ["forecast", "--component", "acuity", "--context", "human/dmek", "--baseline", "0.9"]
    result = runner.invoke(
        eye_app,
        [*base, "--months", "24", "--n", "30", "--observed", "0.1:0.8", "--records", str(folder)],
    )
    assert result.exit_code == 0, result.output
    payload = json.loads(result.output)
    assert payload["training_support"]["earliest_month"] == 0.25
    assert payload["training_support"]["latest_month"] == 12.0
    assert len(payload["warnings"]) == 2
    item = eye_simulation.CORNEAL_COMPONENTS["corneal-thickness"]
    series = _corneal_series("thickness")
    support = eye_simulation.training_support(item, series)
    assert support["earliest_month"] == pytest.approx(0.0329)
    corneal = eye_simulation.forecast_corneal(
        item,
        eye_simulation.fit_corneal(item, series),
        context="human/cataract",
        baseline=540.0,
        months=[1.0, 24.0],
        group_size=30,
        support=support,
    )
    assert corneal["training_support"]["context_months"] == [pytest.approx(0.0329), 12.0]
    assert corneal["warnings"] == [
        "forecast months after the latest training visit (12 months) extrapolate the law "
        "beyond its training data"
    ]
