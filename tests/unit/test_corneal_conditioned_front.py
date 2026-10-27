"""Synthetic conditioned-front contracts; no real experimental data are read."""

import math

import pytest
from pydantic import ValidationError

from peykar.model_registry.catalog import EngineCatalog
from peykar_core.contracts import (
    Intervention,
    ModelPackage,
    ParameterSpec,
    ParameterValue,
    Quantity,
    Reference,
    ScientificContext,
    SimulationPlan,
    ValidityProfile,
)
from peykar_core.errors import Cancelled, InvalidRequest, Unsupported
from peykar_engines.corneal_conditioned_front import CornealConditionedFrontEngine


def fixtures():
    context = ScientificContext(
        species="Oryctolagus cuniculus",
        biological_system="Synthetic rabbit corneal wound fixture",
        conditions=("mechanical epithelial debridement",),
    )
    model = ModelPackage(
        id="synthetic.corneal-conditioned-front",
        version="1.0.0",
        title="Synthetic projected-front forecast fixture; no experimental parameter values",
        engine="cornea.conditioned_front",
        engine_version="1.0.0",
        license="LicenseRef-Peykar-Noncommercial-1.1.0",
        source="python:peykar_engines.corneal_conditioned_front",
        source_digest="a" * 64,
        capabilities=("conditioned_forecast",),
        context=context,
        validity=ValidityProfile(context=context, limitations=("Synthetic fixture only",)),
        parameters=(
            ParameterSpec(name="baseline_area", default=Quantity(value=4 * math.pi, unit="mm²")),
            ParameterSpec(name="anchor_area", default=Quantity(value=math.pi, unit="mm²")),
            ParameterSpec(name="anchor_time", default=Quantity(value=2, unit="hour")),
        ),
    )
    plan = SimulationPlan(
        model=Reference(id=model.id, version=model.version),
        context=context,
        duration=Quantity(value=4, unit="hour"),
        sample_interval=Quantity(value=1, unit="hour"),
        purpose="Synthetic conditioned-front contract verification only",
        seed=7,
    )
    return model, plan


def override(plan, name, value, unit):
    return plan.model_copy(
        update={
            "parameters": (ParameterValue(name=name, quantity=Quantity(value=value, unit=unit)),)
        }
    )


def outputs(result):
    return {series.name: series for series in result.outputs}


def test_forecast_starts_at_anchor_and_normalizes_to_original_baseline():
    model, plan = fixtures()
    result = CornealConditionedFrontEngine().execute(model, plan, lambda: None)
    values = outputs(result)
    assert result.time.values == (0, 3600, 7200, 10800, 14400)
    assert result.time.unit == "second"
    assert values["time_since_injury"].unit == "hour"
    assert values["time_since_injury"].values == (2, 3, 4, 5, 6)
    assert values["projected_radius"].unit == "micrometer"
    assert values["projected_radius"].values == (1000, 500, 0, 0, 0)
    assert values["projected_area"].unit == "millimeter ** 2"
    assert values["projected_area"].values == (math.pi, math.pi / 4, 0, 0, 0)
    assert values["open_area_fraction"].values == (0.25, 0.0625, 0, 0, 0)
    assert values["closure_fraction"].values == (0.75, 0.9375, 1, 1, 1)
    assert result.seed == 7
    assert result.validity_status == "unvalidated"
    assert any("post-injury" in text for text in result.limitations)
    assert any("interval average" in text for text in result.limitations)


def test_area_and_time_overrides_convert_equivalent_units():
    model, plan = fixtures()
    alternate = plan.model_copy(
        update={
            "parameters": (
                ParameterValue(
                    name="baseline_area", quantity=Quantity(value=0.04 * math.pi, unit="cm²")
                ),
                ParameterValue(
                    name="anchor_area", quantity=Quantity(value=1_000_000 * math.pi, unit="µm²")
                ),
                ParameterValue(name="anchor_time", quantity=Quantity(value=120, unit="minute")),
            ),
            "duration": Quantity(value=240, unit="minute"),
            "sample_interval": Quantity(value=60, unit="minute"),
        }
    )
    engine = CornealConditionedFrontEngine()
    expected = engine.execute(model, plan, lambda: None)
    actual = engine.execute(model, alternate, lambda: None)
    assert actual.time == expected.time
    for left, right in zip(actual.outputs, expected.outputs, strict=True):
        assert left.name == right.name
        assert left.unit == right.unit
        assert left.values == pytest.approx(right.values)


@pytest.mark.parametrize("anchor,fraction", [(4 * math.pi, 1), (0, 0)])
def test_equal_area_stays_open_and_closed_anchor_stays_closed(anchor, fraction):
    model, plan = fixtures()
    result = CornealConditionedFrontEngine().execute(
        model, override(plan, "anchor_area", anchor, "mm²"), lambda: None
    )
    assert outputs(result)["open_area_fraction"].values == pytest.approx((fraction,) * 5)


@pytest.mark.parametrize(
    "name,value,unit",
    [
        ("baseline_area", 0, "mm²"),
        ("baseline_area", 401, "mm²"),
        ("anchor_area", -1, "mm²"),
        ("anchor_area", 5 * math.pi, "mm²"),
        ("anchor_time", 0, "hour"),
        ("anchor_time", 97, "hour"),
    ],
)
def test_expansion_and_out_of_applicability_inputs_are_rejected(name, value, unit):
    model, plan = fixtures()
    with pytest.raises(ValueError, match="Require baseline"):
        CornealConditionedFrontEngine().execute(
            model, override(plan, name, value, unit), lambda: None
        )


def test_anchor_plus_forecast_horizon_is_bounded():
    model, plan = fixtures()
    plan = override(plan, "anchor_time", 96, "hour").model_copy(
        update={"duration": Quantity(value=73, unit="hour")}
    )
    with pytest.raises(ValueError, match="absolute horizon"):
        CornealConditionedFrontEngine().execute(model, plan, lambda: None)
    at_boundary = plan.model_copy(update={"duration": Quantity(value=72, unit="hour")})
    assert CornealConditionedFrontEngine().validate(model, at_boundary) == ()


@pytest.mark.parametrize("value", [math.nan, math.inf, -math.inf, True])
def test_nonfinite_and_boolean_observations_fail_quantity_contract(value):
    with pytest.raises(ValidationError):
        Quantity(value=value, unit="mm²")


def test_invalid_dimensions_and_nonfinite_derived_speed_fail_before_output():
    model, plan = fixtures()
    with pytest.raises(InvalidRequest, match="Cannot convert"):
        CornealConditionedFrontEngine().execute(
            model, override(plan, "anchor_area", 1, "millimeter"), lambda: None
        )
    with pytest.raises(ValueError, match="nonfinite interval-average"):
        CornealConditionedFrontEngine().execute(
            model, override(plan, "anchor_time", 5e-324, "hour"), lambda: None
        )


@pytest.mark.parametrize(
    "change,expected",
    [
        ({"species": "Homo sapiens"}, "rabbit"),
        ({"conditions": ("chemical injury",)}, "mechanical context"),
    ],
)
def test_wrong_species_and_injury_are_not_silent_transfers(change, expected):
    model, plan = fixtures()
    context = model.context.model_copy(update=change)
    model = model.model_copy(update={"context": context})
    plan = plan.model_copy(update={"context": context})
    with pytest.raises(ValueError, match=expected):
        CornealConditionedFrontEngine().execute(model, plan, lambda: None)


def test_wrong_engine_missing_parameter_and_time_varying_intervention_are_rejected():
    model, plan = fixtures()
    engine = CornealConditionedFrontEngine()
    for change in ({"engine": "cornea.projected_front"}, {"engine_version": "2.0.0"}):
        with pytest.raises(ValueError, match="engine identity"):
            engine.execute(model.model_copy(update=change), plan, lambda: None)
    with pytest.raises(ValueError, match="requires baseline_area"):
        engine.execute(
            model.model_copy(update={"parameters": model.parameters[:2]}), plan, lambda: None
        )
    intervention = Intervention(
        at=Quantity(value=1, unit="hour"),
        parameter="anchor_area",
        quantity=Quantity(value=2, unit="mm²"),
    )
    with pytest.raises(ValueError, match="interventions"):
        engine.execute(
            model, plan.model_copy(update={"interventions": (intervention,)}), lambda: None
        )


def test_checkpoint_propagates_cancellation_at_each_sample():
    model, plan = fixtures()
    calls = 0

    def checkpoint():
        nonlocal calls
        calls += 1
        if calls == 3:
            raise Cancelled("Synthetic cancellation")

    with pytest.raises(Cancelled, match="Synthetic cancellation"):
        CornealConditionedFrontEngine().execute(model, plan, checkpoint)
    assert calls == 3


def test_conditioned_adapter_is_not_registered_in_default_api_catalog():
    with pytest.raises(Unsupported):
        EngineCatalog().get("cornea.conditioned_front", "1.0.0")


def test_equal_area_has_no_roundoff_generated_closure_or_expansion():
    model, plan = fixtures()
    plan = plan.model_copy(
        update={
            "parameters": tuple(
                ParameterValue(name=name, quantity=Quantity(value=11.03, unit="mm²"))
                for name in ("baseline_area", "anchor_area")
            )
        }
    )
    result = outputs(CornealConditionedFrontEngine().execute(model, plan, lambda: None))
    assert result["projected_area"].values == (11.03,) * 5
    assert result["open_area_fraction"].values == (1,) * 5
    assert result["closure_fraction"].values == (0,) * 5
