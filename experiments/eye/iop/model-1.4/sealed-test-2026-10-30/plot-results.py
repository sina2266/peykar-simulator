"""Plot the one-shot tranche-5 sealed results of IOP model 1.4 from public files only.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/iop/model-1.4/sealed-test-2026-10-30/plot-results.py

Values come only from studies whose values are public in ``report-public.json`` and
``reference-1.2.0-public.json``. Redacted (NC/ND/SA) studies enter the pooled bars and
the H5 sensitivity panel only through their published per-series RMSE and target
counts, which carry no values.
"""

from __future__ import annotations

import json
import math
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
METHOD = {"next_observation": PRIMARY, "landmark": PRIMARY, "initial_state": INITIAL}
studies = {s["study_id"]: s for s in report["studies"]}
reference_studies = {s["study_id"]: s for s in reference["studies"]}
h5 = analysis["H5_versus_model_1_2_same_training"]
redacted = set(analysis["studies_with_redacted_values"])
# The study whose removal changes the H5 verdict (found after unsealing, private licence).
SENSITIVE = "iop-pmc8403082"

# Panels chosen after unsealing from public (CC BY) control group means.
PANELS = [
    (
        [("iop-pmc11288689", "preserflo-mmc"), ("iop-pmc9572517", "xen")],
        "PreserFlo and XEN (two studies): the device level\nimproves the initial state",
    ),
    (
        [("iop-pmc11204868", "mce"), ("iop-pmc11204868", "mue")],
        "XEN45 read as 'other' by the frozen keyword rule\n(one study, two subgroups)",
    ),
]


def _human_group_means(source: dict, task: str, skip: str | None = None) -> list[dict]:
    return [
        entry
        for study in source["studies"]
        if study["study_id"] != skip
        for entry in study[task]
        if entry["context"].startswith("human/") and entry["kind"] == "group_mean"
    ]


def _pooled(entries: list[dict], method: str) -> tuple[float, int]:
    scored = [e for e in entries if method in e["scores"] and e["scores"][method]["n"]]
    total = sum(e["scores"][method]["n"] for e in scored)
    squares = sum(e["scores"][method]["n"] * e["scores"][method]["rmse_pp"] ** 2 for e in scored)
    return math.sqrt(squares / total), total


def _difference(task: str, skip: str | None = None) -> float:
    new, _ = _pooled(_human_group_means(report, task, skip), METHOD[task])
    old, _ = _pooled(_human_group_means(reference, task, skip), METHOD[task])
    return new - old


# The per-series scores reproduce the declared H5 values.
for task in METHOD:
    expected = h5[task]["model_1_4_rmse_pp"] - h5[task]["model_1_2_rmse_pp"]
    if abs(_difference(task) - expected) > 1e-6:
        raise SystemExit(f"per-series scores do not reproduce H5 for {task}")

fig, axes = plt.subplots(2, 2, figsize=(12.5, 11.0))

ax = axes[0, 0]
rows = [
    ("model 1.4", report, PRIMARY),
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
    f"Tranche-5 sealed test: {groups} group-mean forecast series; H1–H4 pass\n"
    f"H5 fails: landmark {h5['landmark']['model_1_4_rmse_pp']:.2f} vs "
    f"{h5['landmark']['model_1_2_rmse_pp']:.2f} pp, next observation "
    f"{h5['next_observation']['model_1_4_rmse_pp']:.2f} vs "
    f"{h5['next_observation']['model_1_2_rmse_pp']:.2f} for 1.2.0",
    fontsize=10,
)
ax.legend(loc="upper right")

ax = axes[0, 1]
tasks = ("next_observation", "landmark", "initial_state")
every = [_difference(task) for task in tasks]
without = [_difference(task, SENSITIVE) for task in tasks]
positions = range(len(tasks))
ax.bar(
    [p - width / 2 for p in positions],
    every,
    width=width,
    color="#1f77b4",
    label=f"all {len(studies)} sealed studies",
)
ax.bar(
    [p + width / 2 for p in positions],
    without,
    width=width,
    color="#999999",
    label="without one private phaco-ECP study",
)
for p, a, b in zip(positions, every, without, strict=True):
    ax.text(p - width / 2, a + (0.006 if a >= 0 else -0.02), f"{a:+.3f}", ha="center", fontsize=8)
    ax.text(p + width / 2, b + (0.006 if b >= 0 else -0.02), f"{b:+.3f}", ha="center", fontsize=8)
ax.axhline(0.05, color="#d62728", linestyle="--", linewidth=1, label="allowed loss +0.05 pp")
ax.axhline(0.0, color="black", linewidth=0.8)
ax.set_ylim(min(*every, *without) - 0.05, max(*every, *without) + 0.05)
ax.set_xticks(list(positions), ["next observation", "landmark", "initial state\n(must be < 0)"])
ax.set_ylabel("RMSE of 1.4 minus RMSE of 1.2.0 (pp)")
ax.set_title(
    "H5 rests on one study: the sub-procedure level for endoscopic\n"
    "cyclophotocoagulation lowered its initial state error and raised its forecast errors",
    fontsize=10,
)
ax.legend(loc="lower left", fontsize=8)


def series_panel(ax: plt.Axes, chosen: list[tuple[str, str]], title: str) -> None:
    colours = ("black", "#d62728", "#2ca02c")
    for index, (study_id, name) in enumerate(chosen):
        if study_id in redacted:
            raise SystemExit(f"{study_id} is redacted and cannot be plotted")
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
            label=f"1.4 initial state ({tag})",
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
