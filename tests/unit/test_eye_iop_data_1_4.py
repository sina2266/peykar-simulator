"""IOP series reader of model 1.4 (selective sub-procedure contexts), on the public
development records of the IOP dataset."""

from __future__ import annotations

import json
from dataclasses import replace
from pathlib import Path

import pytest

from peykar.validation.eye_iop_data_1_2 import to_iop_series
from peykar.validation.eye_iop_data_1_3 import SUB_PROCEDURE_RULES, to_iop_series_1_3
from peykar.validation.eye_iop_data_1_4 import to_iop_series_1_4
from peykar_core.errors import InvalidRequest

RECORDS = (
    Path(__file__).resolve().parents[2]
    / "experiments/data/iop-glaucoma-surgery-2026-10-28/development/records"
)


def _records() -> list[dict]:
    paths = sorted(RECORDS.glob("*.json"))
    if not paths:
        pytest.skip("public IOP development records not present")
    return [json.loads(p.read_text()) for p in paths]


def test_all_rule_procedures_reproduce_model_1_3_and_none_keeps_model_1_2() -> None:
    contexts = set()
    for record in _records():
        every = to_iop_series_1_4(record, set(SUB_PROCEDURE_RULES))
        assert every == to_iop_series_1_3(record)
        plain = to_iop_series(record)
        none = to_iop_series_1_4(record, ())
        for a, b in zip(plain, none, strict=True):
            species, _, procedure = a.context.partition("/")
            assert b == replace(a, context=f"{species}/all/{procedure}")
        one = to_iop_series_1_4(record, {"cyclodestruction"})
        for a, b, c in zip(plain, one, every, strict=True):
            procedure = a.context.partition("/")[2]
            assert b.context == (
                c.context
                if procedure == "cyclodestruction"
                else f"{a.context.partition('/')[0]}/all/{procedure}"
            )
            contexts.add(b.context)
    assert any("/all/" in c for c in contexts)


def test_unknown_procedures_are_refused() -> None:
    record = _records()[0]
    with pytest.raises(InvalidRequest):
        to_iop_series_1_4(record, {"trabeculectomy"})
