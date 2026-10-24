from pathlib import Path

import pytest

from peykar.validation.cornea_evidence import rabbit_brush_aggregates
from peykar_core.errors import InvalidRequest

SOURCE = Path("experiments/vcornea/repair-front/chung2021-manuscript.xml")


def test_original_aggregates_preserve_all_methods_uncertainty_and_categorical_results():
    data = rabbit_brush_aggregates(SOURCE.read_bytes())
    rows = data["observations"]
    assert len(rows) == 12
    assert {(r["group"], r["hour"]) for r in rows} == {
        (g, t) for g in ["Blade", "Ocu", "Amo"] for t in [4, 18, 24, 50]
    }
    assert next(r for r in rows if (r["group"], r["hour"]) == ("Ocu", 4))["mean_fraction"] == 1.02
    categorical = [r for r in rows if r["kind"] == "categorical"]
    assert len(categorical) == 2
    assert all(r["reported"] == "Completely Healed" for r in categorical)
    assert all("mean_fraction" not in r for r in categorical)
    assert data["individual_observations_available"] is False
    assert data["per_time_independent_animal_count"] is None
    assert "unresolved" in data["plus_minus_definition"]


@pytest.mark.parametrize(
    "old,new",
    [
        (b"10.1155/2021/4668056", b"another-study"),
        (b"Blade group", b"Different group"),
        (b"18 hours", b"17 hours"),
        (b"0.99", b"NaN"),
        (b"0.99", b"-0.99"),
        (b"Healed", b"Unreported"),
    ],
)
def test_changed_identity_layout_nonfinite_or_categorical_values_are_not_silently_admitted(
    old, new
):
    with pytest.raises(InvalidRequest):
        rabbit_brush_aggregates(SOURCE.read_bytes().replace(old, new))


def test_inert_xml_never_resolves_entities():
    with pytest.raises(InvalidRequest, match="inert XML"):
        rabbit_brush_aggregates(b'<!DOCTYPE x [<!ENTITY e SYSTEM "file:///etc/passwd">]><x/>')
