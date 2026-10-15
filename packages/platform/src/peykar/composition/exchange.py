"""Analytical two-compartment exchange: a conservation verification primitive.

This is not a pharmacokinetic or organ model. Volumes remain constant, there are
no sources or sinks, and clearance is symmetric and nonnegative.
"""

import math

from peykar_core.contracts import Quantity
from peykar_core.errors import InvalidRequest
from peykar_core.units import convert


def exchange(
    concentration_a: Quantity,
    concentration_b: Quantity,
    volume_a: Quantity,
    volume_b: Quantity,
    clearance: Quantity,
    elapsed: Quantity,
) -> tuple[Quantity, Quantity]:
    a, b = convert(concentration_a, "mole/liter"), convert(concentration_b, "mole/liter")
    va, vb = convert(volume_a, "liter"), convert(volume_b, "liter")
    flow, dt = convert(clearance, "liter/second"), convert(elapsed, "second")
    if min(a, b, flow, dt) < 0 or va <= 0 or vb <= 0:
        raise InvalidRequest("Exchange requires positive volumes and nonnegative other inputs")
    equilibrium = (va * a + vb * b) / (va + vb)
    delta = (a - b) * math.exp(-flow * (1 / va + 1 / vb) * dt)
    return (
        Quantity(value=equilibrium + vb / (va + vb) * delta, unit="mole/liter"),
        Quantity(value=equilibrium - va / (va + vb) * delta, unit="mole/liter"),
    )
