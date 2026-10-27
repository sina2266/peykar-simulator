"""Render a corneal population evaluation report as Markdown tables (read-only)."""

from __future__ import annotations

import argparse
import json
import math
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any

METHODS = (
    "cohort_assimilation",
    "population_assimilation_t4",
    "population_assimilation",
    "radius_lag_flat_prior",
    "analog_knn",
    "last_two_radius_trend",
    "first_to_current_trend",
    "crosson1986_fixed",
    "persistence",
)


def rmse(values: list[float]) -> float:
    return math.sqrt(sum(v * v for v in values) / len(values)) if values else float("nan")


def render(report: dict[str, Any]) -> str:
    lines = []
    per_study: dict[tuple[str, str], list[float]] = defaultdict(list)
    passes: dict[str, list[int]] = defaultdict(lambda: [0, 0])
    floors: dict[str, list[int]] = defaultdict(lambda: [0, 0])
    for study in report["studies"]:
        for entry in study["next_observation"]:
            floor = entry["oracle_fit_floor"]["within_5_10_pp"]
            floors[study["study_id"]][0] += int(floor)
            floors[study["study_id"]][1] += 1
            for method, score in entry["scores"].items():
                per_study[(study["study_id"], method)].extend(score["signed_errors_pp"])
                ok = score["meets_5_10_pp"] and score["improves_persistence_by_0_5_pp"]
                passes[method][0] += int(ok)
                passes[method][1] += 1
    studies = sorted({s for s, _ in per_study})
    header = "| Method | " + " | ".join(studies) + " | Series passing |"
    lines += ["### Next observation: RMSE (pp) per study", "", header]
    lines.append("|" + "---|" * (len(studies) + 2))
    for method in METHODS:
        if method not in passes:
            continue
        cells = [
            f"{rmse(per_study[(s, method)]):.2f}" if per_study.get((s, method)) else "—"
            for s in studies
        ]
        lines.append(
            f"| {method} | " + " | ".join(cells) + f" | {passes[method][0]}/{passes[method][1]} |"
        )
    lines.append("")
    lines.append(
        "In-sample radius–lag fit within 5/10 pp (diagnostic floor): "
        + ", ".join(f"{s} {floors[s][0]}/{floors[s][1]}" for s in studies)
    )
    lines += ["", "### Interval coverage (pooled next-observation targets)", ""]
    for level, data in report["summary"]["coverage"].items():
        cells = ", ".join(
            f"{m} {v['inside']}/{v['n']} ({v['inside'] / v['n']:.0%})"
            for m, v in data["next_observation"].items()
            if v["n"]
        )
        animal = data["initial_state_individual"]
        animal_text = (
            f"; initial-state individual {animal['inside']}/{animal['n']}" if animal["n"] else ""
        )
        lines.append(f"- {level}%: {cells}{animal_text}")
    lines += ["", "### Initial state only: control-group mean curves", ""]
    lines.append("| Study / group | Animals | Population model | Crosson 1986 | Pooled curve |")
    lines.append("|---|---:|---|---|---|")
    for study in report["studies"]:
        for entry in study["initial_state"]:
            s = entry["scores"]
            cell = {
                k: f"{v['rmse_pp']:.1f} / {v['max_abs_pp']:.1f}{' ✓' if v['meets_5_10_pp'] else ''}"
                for k, v in s.items()
            }
            lines.append(
                f"| {study['study_id']} / {entry['group']} | {entry['animals']} | "
                f"{cell['population_initial_state']} | {cell['crosson1986_fixed']} | "
                f"{cell['pooled_training_curve']} |"
            )
    lines.append("")
    lines.append("Cells are RMSE / maximum absolute error in percentage points; ✓ meets 5/10.")
    return "\n".join(lines) + "\n"


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("report", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args(argv)
    text = render(json.loads(args.report.read_text()))
    if args.output:
        args.output.write_text(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
