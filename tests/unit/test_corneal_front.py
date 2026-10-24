from pathlib import Path

import pytest

from peykar.model_registry.catalog import EngineCatalog
from peykar_core.contracts import (
    Intervention,
    ModelPackage,
    ParameterValue,
    Quantity,
    Reference,
    SimulationPlan,
)
from peykar_core.errors import Unsupported
from peykar_core.hashing import bytes_digest
from peykar_engines.corneal_front import CornealProjectedFrontEngine


def fixtures():
    model = ModelPackage.model_validate_json(
        Path("model_packages/cornea/projected-repair/1.0.0/model.json").read_bytes()
    )
    # Explicit synthetic unit fixture; not an experimental parameter estimate.
    plan = SimulationPlan(
        model=Reference(id=model.id, version=model.version),
        context=model.context,
        duration=Quantity(value=14, unit="hour"),
        sample_interval=Quantity(value=1, unit="hour"),
        parameters=(
            ParameterValue(name="initial_radius", quantity=Quantity(value=10, unit="micrometer")),
            ParameterValue(
                name="migration_rate", quantity=Quantity(value=1, unit="micrometer/hour")
            ),
            ParameterValue(name="lag", quantity=Quantity(value=2, unit="hour")),
        ),
        purpose="Synthetic kinematic contract and numerical verification only",
        seed=0,
    )
    return model, plan


def test_radius_lag_absorbing_closure_and_actual_area_observation():
    model, plan = fixtures()
    assert model.source_digest == bytes_digest(
        Path("packages/engines/src/peykar_engines/corneal_front.py").read_bytes()
    )
    result = CornealProjectedFrontEngine().execute(model, plan, lambda: None)
    values = {s.name: s for s in result.outputs}
    radius = values["projected_radius"].values
    assert radius[:3] == (10, 10, 10)
    assert radius[7] == 5
    assert values["open_area_fraction"].values[7] == 0.25
    assert values["closure_fraction"].values[7] == 0.75
    assert radius[12:] == (0, 0, 0)
    assert values["projected_area"].values[-1] == 0
    assert values["projected_area"].unit == "micrometer ** 2"
    assert result.time.unit == "second"
    assert result.seed == 0


def test_unit_equivalent_initial_radius_preserves_prediction():
    model, plan = fixtures()
    equivalent = plan.model_copy(
        update={
            "parameters": (
                ParameterValue(
                    name="initial_radius", quantity=Quantity(value=0.01, unit="millimeter")
                ),
                *plan.parameters[1:],
            )
        }
    )
    engine = CornealProjectedFrontEngine()
    assert engine.execute(model, plan, lambda: None) == engine.execute(
        model, equivalent, lambda: None
    )


def test_zero_migration_does_not_return_fake_closure():
    model, plan = fixtures()
    plan = plan.model_copy(
        update={
            "parameters": (
                plan.parameters[0],
                ParameterValue(
                    name="migration_rate", quantity=Quantity(value=0, unit="micrometer/hour")
                ),
                plan.parameters[2],
            )
        }
    )
    result = CornealProjectedFrontEngine().execute(model, plan, lambda: None)
    assert next(s for s in result.outputs if s.name == "open_area_fraction").values == (1,) * 15


def test_checkpoint_cancellation_is_not_ignored():
    model, plan = fixtures()
    calls = []

    def cancel():
        calls.append(1)
        if len(calls) == 3:
            raise RuntimeError("stop actual execution")

    with pytest.raises(RuntimeError, match="stop actual execution"):
        CornealProjectedFrontEngine().execute(model, plan, cancel)
    assert len(calls) == 3


@pytest.mark.parametrize(
    "change,expected",
    [
        ({"species": "Homo sapiens"}, "rabbit"),
        ({"conditions": ("chemical injury",)}, "Chemical"),
    ],
)
def test_other_species_or_chemical_injury_cannot_reuse_this_model(change, expected):
    model, plan = fixtures()
    context = model.context.model_copy(update=change)
    changed = model.model_copy(update={"context": context})
    with pytest.raises(ValueError, match=expected):
        CornealProjectedFrontEngine().execute(
            changed, plan.model_copy(update={"context": context}), lambda: None
        )


def test_drug_or_parameter_interventions_require_another_reviewed_model():
    model, plan = fixtures()
    intervention = Intervention(
        at=Quantity(value=1, unit="hour"),
        parameter="migration_rate",
        quantity=Quantity(value=2, unit="micrometer/hour"),
    )
    with pytest.raises(ValueError, match="interventions"):
        CornealProjectedFrontEngine().execute(
            model, plan.model_copy(update={"interventions": (intervention,)}), lambda: None
        )


def test_optional_scientific_adapter_is_not_automatically_enabled_in_api_catalog():
    with pytest.raises(Unsupported):
        EngineCatalog().get("cornea.projected_front", "1.0.0")
