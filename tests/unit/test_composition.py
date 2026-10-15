import pytest

from peykar.composition.compatibility import Connection, check_connection, composition_order
from peykar.composition.exchange import exchange
from peykar_core.contracts import Quantity, ScientificPort
from peykar_core.errors import InvalidRequest, Unsupported


def port(direction="output", unit="mole/liter", **changes):
    values = dict(
        name="signal", direction=direction, unit=unit, semantic_id="CHEBI:15377", scale="molecular"
    )
    values.update(changes)
    return ScientificPort(**values)


def test_port_unit_scaling():
    assert check_connection(port(), port("input", "millimole/liter")) == 1000


@pytest.mark.parametrize(
    "changes",
    [
        {"semantic_id": "CHEBI:15378"},
        {"scale": "tissue"},
        {"shape": "field"},
        {"frame": "other"},
        {"unit": "kilogram"},
        {"direction": "output"},
    ],
)
def test_invalid_coupling(changes):
    with pytest.raises(InvalidRequest):
        check_connection(port(), port(**({"direction": "input"} | changes)))


def test_dag_and_feedback(model):
    out = port(name="out")
    inp = port("input", name="in")
    m = model.model_copy(update={"ports": (out, inp)})
    ab = Connection(source_model="a", source_port="out", target_model="b", target_port="in")
    ba = Connection(source_model="b", source_port="out", target_model="a", target_port="in")
    assert composition_order({"a": m, "b": m}, (ab,)) == ("a", "b")
    with pytest.raises(Unsupported):
        composition_order({"a": m, "b": m}, (ab, ba))
    with pytest.raises(InvalidRequest):
        composition_order({"a": m, "b": m}, (ab, ab))


def test_exchange_mass_conservation():
    a, b = exchange(
        Quantity(value=4, unit="mole/liter"),
        Quantity(value=0, unit="mole/liter"),
        Quantity(value=2, unit="liter"),
        Quantity(value=3, unit="liter"),
        Quantity(value=1, unit="liter/second"),
        Quantity(value=50, unit="second"),
    )
    assert 2 * a.value + 3 * b.value == pytest.approx(8)
    assert a.value == pytest.approx(b.value)


def test_exchange_time_partition_invariance():
    def q(x):
        return Quantity(value=x, unit="mole/liter")

    va, vb = Quantity(value=2, unit="liter"), Quantity(value=3, unit="liter")
    flow = Quantity(value=1, unit="liter/second")
    whole = exchange(q(4), q(0), va, vb, flow, Quantity(value=10, unit="s"))
    a, b = exchange(q(4), q(0), va, vb, flow, Quantity(value=3, unit="s"))
    parts = exchange(a, b, va, vb, flow, Quantity(value=7, unit="s"))
    assert [v.value for v in whole] == pytest.approx([v.value for v in parts])
