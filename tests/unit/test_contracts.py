import json
import math

import pytest
from pydantic import ValidationError

from peykar_core.contracts import (
    ParameterValue,
    Quantity,
    Reference,
    Series,
    SimulationPlan,
    SimulationResult,
)
from peykar_core.errors import InvalidRequest
from peykar_core.hashing import canonical_json, digest
from peykar_core.planning import time_grid, validate_plan
from peykar_core.units import convert


def test_unknown_fields_rejected():
    with pytest.raises(ValidationError):
        Quantity(value=1, unit="second", typo=2)


@pytest.mark.parametrize("value", [True, False, "1.0"])
def test_quantity_rejects_non_numeric_inputs(value):
    with pytest.raises(ValidationError):
        Quantity(value=value, unit="second")


@pytest.mark.parametrize("value", [math.nan, math.inf, -math.inf])
def test_nonfinite_rejected(value):
    with pytest.raises(ValidationError):
        Quantity(value=value, unit="second")


def test_digest_json_roundtrip(plan):
    restored = SimulationPlan.model_validate_json(plan.model_dump_json())
    assert digest(plan) == digest(restored)
    assert canonical_json({"b": 2, "a": 1}) == '{"a":1,"b":2}'
    assert digest(json.loads(plan.model_dump_json())) == digest(plan)


def test_compatible_unit_conversion():
    assert convert(Quantity(value=2, unit="minute"), "second") == 120


@pytest.mark.parametrize("unit", ["kilogram", "nonsense_unit", "liter"])
def test_wrong_units_rejected(unit):
    with pytest.raises(InvalidRequest):
        convert(Quantity(value=1, unit=unit), "second")


@pytest.mark.parametrize("duration,dt", [(0, 1), (-1, 1), (1, 2), (10, 3), (10, 0)])
def test_invalid_time_grid(plan, duration, dt):
    updated = plan.model_copy(
        update={
            "duration": Quantity(value=duration, unit="s"),
            "sample_interval": Quantity(value=dt, unit="s"),
        }
    )
    with pytest.raises(InvalidRequest):
        time_grid(updated)


def test_budget_prevents_large_allocation(plan):
    updated = plan.model_copy(
        update={
            "duration": Quantity(value=1e308, unit="s"),
            "sample_interval": Quantity(value=1e-300, unit="s"),
        }
    )
    with pytest.raises(InvalidRequest):
        time_grid(updated)


def test_context_mismatch_rejected(model, plan):
    context = plan.context.model_copy(update={"species": "NCBITaxon:9606"})
    with pytest.raises(InvalidRequest):
        validate_plan(model, plan.model_copy(update={"context": context}))


def test_unknown_parameter_rejected(model, plan):
    updated = plan.model_copy(
        update={
            "parameters": (ParameterValue(name="mystery", quantity=Quantity(value=1, unit="s")),)
        }
    )
    with pytest.raises(InvalidRequest):
        validate_plan(model, updated)


def test_out_of_range_rejected(model, plan):
    updated = plan.model_copy(
        update={
            "parameters": (ParameterValue(name="rate", quantity=Quantity(value=-1, unit="1/s")),)
        }
    )
    with pytest.raises(InvalidRequest):
        validate_plan(model, updated)


def test_duplicate_parameters_rejected(plan):
    payload = plan.model_dump(mode="json")
    value = {"name": "rate", "quantity": {"value": 1, "unit": "1/s"}}
    payload["parameters"] = [value, value]
    with pytest.raises(ValidationError):
        SimulationPlan.model_validate(payload)


@pytest.mark.parametrize("seed", [True, -1, 2**32, 1.2, "42"])
def test_seed_strict(plan, seed):
    payload = plan.model_dump(mode="json")
    payload["seed"] = seed
    with pytest.raises(ValidationError):
        SimulationPlan.model_validate(payload)


def test_semver_required():
    with pytest.raises(ValidationError):
        Reference(id="example", version="latest")


def test_result_rejects_time_and_length_mismatch():
    with pytest.raises(ValidationError):
        SimulationResult(
            time=Series(name="t", unit="s", values=(0, 0)),
            outputs=(Series(name="x", unit="mole", values=(1, 2)),),
            seed=1,
            limitations=("test",),
        )
