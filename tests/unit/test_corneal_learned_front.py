from pathlib import Path

import pytest

from peykar.model_registry.catalog import EngineCatalog
from peykar_core.contracts import (
    Intervention,
    ModelPackage,
    ParameterSpec,
    ParameterValue,
    Quantity,
    Reference,
    SimulationPlan,
)
from peykar_core.errors import Unsupported
from peykar_engines.corneal_learned_front import CornealLearnedFrontEngine


def fixtures():
    original = ModelPackage.model_validate_json(
        Path("model_packages/cornea/projected-repair/1.0.0/model.json").read_bytes()
    )
    # Entirely synthetic engineering fixture, not learned experimental parameters.
    engine = CornealLearnedFrontEngine()
    values = {
        "initial_radius": 100,
        "concentration": 1,
        "rate0": 10,
        "rate2": 30,
        "lag0": 2,
        "lag2": 4,
    }
    model = original.model_copy(
        update={
            "id": "synthetic.learned-cornea",
            "engine": engine.id,
            "parameters": tuple(
                ParameterSpec(name=k, default=Quantity(value=v, unit=engine.units[k]))
                for k, v in values.items()
            ),
        }
    )
    plan = SimulationPlan(
        model=Reference(id=model.id, version=model.version),
        context=model.context,
        duration=Quantity(value=10, unit="hour"),
        sample_interval=Quantity(value=1, unit="hour"),
        seed=0,
        purpose="Synthetic learned-engine contract verification only",
    )
    return model, plan


def test_interpolated_kinetics_and_full_area_outputs_are_monotone_with_absorbing_closure():
    model, plan = fixtures()
    result = CornealLearnedFrontEngine().execute(model, plan, lambda: None)
    values = {s.name: s.values for s in result.outputs}
    assert values["projected_radius"][:4] == (100, 100, 100, 100)
    assert values["projected_radius"][5] == 60
    assert values["open_area_fraction"][5] == 0.36
    assert values["closure_fraction"][5] == 0.64
    assert values["projected_radius"][8:] == (0, 0, 0)
    assert result.seed == 0 and result.time.unit == "second"


def test_percent_conversion_and_dimensionless_dose_are_equivalent():
    model, plan = fixtures()
    a = plan.model_copy(
        update={
            "parameters": (
                ParameterValue(name="concentration", quantity=Quantity(value=0.5, unit="percent")),
            )
        }
    )
    b = plan.model_copy(
        update={
            "parameters": (
                ParameterValue(
                    name="concentration", quantity=Quantity(value=0.005, unit="dimensionless")
                ),
            )
        }
    )
    engine = CornealLearnedFrontEngine()
    assert engine.execute(model, a, lambda: None) == engine.execute(model, b, lambda: None)


@pytest.mark.parametrize(
    "name,value,unit",
    [("concentration", 2.1, "percent"), ("rate0", 201, "micrometer/hour"), ("lag2", 25, "hour")],
)
def test_outside_reviewed_input_ranges_are_rejected(name, value, unit):
    model, plan = fixtures()
    plan = plan.model_copy(
        update={
            "parameters": (ParameterValue(name=name, quantity=Quantity(value=value, unit=unit)),)
        }
    )
    with pytest.raises(ValueError, match="bounded kinetics"):
        CornealLearnedFrontEngine().execute(model, plan, lambda: None)


def test_interventions_and_default_registration_are_not_silently_enabled():
    model, plan = fixtures()
    change = Intervention(
        at=Quantity(value=1, unit="hour"),
        parameter="concentration",
        quantity=Quantity(value=2, unit="percent"),
    )
    with pytest.raises(ValueError, match="interventions"):
        CornealLearnedFrontEngine().execute(
            model, plan.model_copy(update={"interventions": (change,)}), lambda: None
        )
    with pytest.raises(Unsupported):
        EngineCatalog().get("cornea.learned_front", "1.0.0")


def test_checkpoint_failure_propagates():
    model, plan = fixtures()

    def stop():
        raise RuntimeError("cancel actual computation")

    with pytest.raises(RuntimeError, match="cancel actual"):
        CornealLearnedFrontEngine().execute(model, plan, stop)
