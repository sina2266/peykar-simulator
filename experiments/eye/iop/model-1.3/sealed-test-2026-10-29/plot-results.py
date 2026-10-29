"""Plot the one-shot tranche-4 sealed results of IOP model 1.3 from public files only.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/iop/model-1.3/sealed-test-2026-10-29/plot-results.py

Values come only from studies whose values are public in ``report-public.json`` and
``reference-1.2.0-public.json``. Redacted (NC/ND/SA) studies enter the pooled bars only
through their published per-series RMSE and target counts, which carry no values.
"""

from __future__ import annotations

import json
import math
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

HERE = Path(__file__).resolve().parent
report = json.loads((HERE / "report-public.json").read_text())
reference = json.loads((HERE / "reference-1.2.0-public.json").read_text())
analysis = json.loads((HERE / "sealed-analysis.json").read_text())
PRIMARY = "population_assimilation"
INITIAL = "population_initial_state"
studies = {s["study_id"]: s for s in report["studies"]}
reference_studies = {s["study_id"]: s for s in reference["studies"]}
h5 = analysis["H5_versus_model_1_2_same_training"]

# Panels chosen after unsealing from public (CC BY) control group means.
PANELS = [
    (
        [("iop-pmc8625108", "mp-tscpc"), ("iop-pmc10900656", "mp-cpc")],
        "Micropulse cyclophotocoagulation (two studies):\n"
        "the sub-procedure level improves the initial state",
    ),
    (
        [("iop-pmc11722142", "su-pap-grade-0"), ("iop-pmc11722142", "su-pap-grade-3")],
        "Valved tube (one study, two baseline subgroups):\n"
        "the valved-tube level makes the initial state worse",
    ),
]


def _human_group_means(source: dict, task: str) -> list[dict]:
    return [
        entry
        for study in source["studies"]
        for entry in study[task]
        if entry["context"].startswith("human/") and entry["kind"] == "group_mean"
    ]


def _pooled(entries: list[dict], method: str) -> tuple[float, int]:
    total = sum(e["scores"][method]["n"] for e in entries)
    squares = sum(e["scores"][method]["n"] * e["scores"][method]["rmse_pp"] ** 2 for e in entries)
    return math.sqrt(squares / total), total


fig, axes = plt.subplots(2, 2, figsize=(12.5, 11.0))

ax = axes[0, 0]
rows = [
    ("model 1.3", report, PRIMARY),
    ("model 1.2.0 (same training)", reference, PRIMARY),
    ("analog kNN", report, "analog_knn"),
    ("pooled-curve ratio", report, "pooled_curve_ratio"),
    ("persistence", report, "persistence"),
]
width = 0.38
largest = 0.0
for offset, task, colour in (
    (-width / 2, "next_observation", "#1f77b4"),
    (width / 2, "landmark", "#ff7f0e"),
):
    values = [_pooled(_human_group_means(source, task), m)[0] for _, source, m in rows]
    positions = [i + offset for i in range(len(rows))]
    ax.barh(positions, values, height=width, color=colour, label=task.replace("_", " "))
    for y, v in zip(positions, values, strict=True):
        ax.text(v + 0.15, y, f"{v:.2f}", va="center", fontsize=8)
    largest = max(largest, *values)
ax.axvline(5.0, color="grey", linestyle="--", linewidth=1)
ax.set_xlim(0, largest * 1.12)
ax.set_yticks(range(len(rows)), [r[0] for r in rows])
ax.invert_yaxis()
ax.set_xlabel("pooled RMSE, human group means (pp of preoperative IOP)")
groups = len(_human_group_means(report, "next_observation"))
ax.set_title(
    f"Tranche-4 sealed test: {groups} group-mean forecast series; H1–H4 pass\n"
    f"H5 fails: initial state {h5['initial_state']['model_1_3_rmse_pp']:.2f} vs "
    f"{h5['initial_state']['model_1_2_rmse_pp']:.2f} pp for 1.2.0",
    fontsize=10,
)
ax.legend(loc="upper right")

ax = axes[0, 1]
new_initial = _human_group_means(report, "initial_state")
old_by_id = {e["series_id"]: e for e in _human_group_means(reference, "initial_state")}
by_procedure: dict[str, list[tuple[dict, dict]]] = defaultdict(list)
for entry in new_initial:
    by_procedure[entry["context"].rsplit("/", 1)[-1]].append((entry, old_by_id[entry["series_id"]]))
order = sorted(
    by_procedure, key=lambda p: -sum(e["scores"][INITIAL]["n"] for e, _ in by_procedure[p])
)
labels, new_values, old_values = [], [], []
for procedure in order:
    pairs = by_procedure[procedure]
    new_rmse, targets = _pooled([e for e, _ in pairs], INITIAL)
    old_rmse, _ = _pooled([o for _, o in pairs], INITIAL)
    labels.append(f"{procedure.replace('_', ' ')}\n({len(pairs)} groups, {targets} targets)")
    new_values.append(new_rmse)
    old_values.append(old_rmse)
positions = range(len(labels))
ax.barh(
    [p - width / 2 for p in positions], old_values, height=width, color="#999999", label="1.2.0"
)
ax.barh([p + width / 2 for p in positions], new_values, height=width, color="#1f77b4", label="1.3")
for p, a, b in zip(positions, old_values, new_values, strict=True):
    ax.text(a + 0.2, p - width / 2, f"{a:.1f}", va="center", fontsize=7)
    ax.text(b + 0.2, p + width / 2, f"{b:.1f}", va="center", fontsize=7)
ax.set_yticks(list(positions), labels, fontsize=7)
ax.invert_yaxis()
ax.set_xlabel("initial-state RMSE, human control group means (pp)")
ax.set_title(
    "Initial state by procedure: where the sub-procedure level helps and hurts", fontsize=10
)
ax.set_xlim(0, max(*new_values, *old_values) * 1.12)
ax.legend(loc="upper right")


def series_panel(ax: plt.Axes, chosen: list[tuple[str, str]], title: str) -> None:
    colours = ("black", "#d62728", "#2ca02c")
    for index, (study_id, name) in enumerate(chosen):
        colour = colours[index % len(colours)]
        new = next(
            e for e in studies[study_id]["initial_state"] if e["series_id"].split("/")[1] == name
        )
        old = next(
            e
            for e in reference_studies[study_id]["initial_state"]
            if e["series_id"] == new["series_id"]
        )
        tag = f"{study_id.removeprefix('iop-pmc')}:{name}"
        months = new["months"]
        ax.plot(
            months,
            [100 * v for v in new["observed"]],
            "o-",
            color=colour,
            label=f"observed ({tag})",
        )
        ax.plot(
            months,
            [100 * v for v in new["predictions"][INITIAL]],
            "s--",
            color=colour,
            alpha=0.7,
            label=f"1.3 initial state ({tag})",
        )
        ax.plot(
            months,
            [100 * v for v in old["predictions"][INITIAL]],
            "^:",
            color=colour,
            alpha=0.7,
            label=f"1.2.0 initial state ({tag})",
        )
    ax.set_xscale("log")
    ax.set_xlabel("months after surgery (log scale)")
    ax.set_ylabel("IOP (% of preoperative)")
    ax.set_title(title, fontsize=10)
    handles, labels_ = ax.get_legend_handles_labels()
    unique = dict(zip(labels_, handles, strict=True))
    ax.legend(
        unique.values(),
        unique.keys(),
        fontsize=7,
        loc="upper center",
        bbox_to_anchor=(0.5, -0.13),
        ncol=2,
    )


for ax, (chosen, title) in zip(axes[1], PANELS, strict=True):
    series_panel(ax, chosen, title)
fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150, bbox_inches="tight")
fig.savefig(HERE / "sealed-results.pdf", bbox_inches="tight")
print("wrote", HERE / "sealed-results.png", "verdicts", analysis["verdicts"])
