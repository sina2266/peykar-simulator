from __future__ import annotations

from pathlib import Path

import pyvcell_odesolver as pvco

RESOURCE_DIR = Path(__file__).parent / "resources"
TOLERANCE = 1e-7


def _compare(actual: Path, expected: Path, tolerance: float) -> None:
    actual_lines = actual.read_text().splitlines()
    expected_lines = expected.read_text().splitlines()
    assert len(actual_lines) == len(expected_lines), (
        f"line counts differ: {actual} ({len(actual_lines)}) "
        f"vs {expected} ({len(expected_lines)})"
    )
    for i, (line_a, line_e) in enumerate(
        zip(actual_lines, expected_lines, strict=True)
    ):
        if line_a == line_e:
            continue
        values_a = [float(v) for v in line_a.split()]
        values_e = [float(v) for v in line_e.split()]
        assert len(values_a) == len(values_e), (
            f"line {i} widths differ: {line_a!r} vs {line_e!r}"
        )
        for v_a, v_e in zip(values_a, values_e, strict=True):
            adjusted = tolerance * max(abs(v_a), abs(v_e))
            assert abs(v_a - v_e) <= adjusted, (
                f"value mismatch on line {i}: {v_a} vs {v_e} "
                f"(adjusted tolerance {adjusted})"
            )


def test_solve_smoke_without_messaging(tmp_path: Path) -> None:
    hash_id = 1489333437
    input_file = RESOURCE_DIR / f"SimID_{hash_id}_0_.cvodeInput"
    expected_file = RESOURCE_DIR / f"SimID_{hash_id}_0_.ida.expected"
    output_file = tmp_path / f"SimID_{hash_id}_0_.ida"

    assert input_file.exists(), f"missing input: {input_file}"
    assert expected_file.exists(), f"missing expected: {expected_file}"

    return_code = pvco.solve(str(input_file), str(output_file))
    assert return_code == 0
    assert output_file.exists()

    _compare(output_file, expected_file, TOLERANCE)
