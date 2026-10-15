"""Unit conversion is explicit. Semantic and spatial equivalence need separate checks."""

import pint

from peykar_core.contracts import Quantity
from peykar_core.errors import InvalidRequest

_REGISTRY = pint.UnitRegistry()


def convert(quantity: Quantity, unit: str) -> float:
    try:
        value = float(_REGISTRY.Quantity(quantity.value, quantity.unit).to(unit).magnitude)
        return Quantity(value=value, unit=unit).value
    except (pint.PintError, ValueError, TypeError) as exc:
        raise InvalidRequest(f"Cannot convert {quantity.unit!r} to {unit!r}") from exc


def validate_unit(unit: str) -> None:
    convert(Quantity(value=1.0, unit=unit), unit)
