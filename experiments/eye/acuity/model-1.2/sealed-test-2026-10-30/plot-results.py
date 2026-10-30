"""Plot the one-shot sealed results of visual-acuity model 1.2 from public files.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/acuity/model-1.2/sealed-test-2026-10-30/plot-results.py

Pooled values come from the published summary of ``report-public.json`` (model 1.2,
stacked; it also carries model 1.0.0's forecasts from the same fit) and the verdicts
from ``sealed-analysis.json``; the per-series panels use the per-series scores of
``report-public.json``, which are public for every study (values of redacted studies
are not). One reported pp is one ETDRS letter (0.02 logMAR).
"""

from __future__ import annotations

import json
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402

HERE = Path(__file__).resolve().parent
report = json.loads((HERE / "report-public.json").read_text())
analysis = json.loads((HERE / "sealed-analysis.json").read_text())
STACKED, MODEL = "stacked", "population_assimilation_robust"
STACKED_INITIAL, INITIAL = "stacked_initial_state", "population_initial_state"
COLOURS = {"cxl": "#2ca02c", "dmek": "#1f77b4", "dsaek": "#ff7f0e", "other": "#7f7f7f"}


def procedure(context: str) -> str:
    name = context.rsplit("/", 1)[-1]
    return name if name in COLOURS else "other"


def pooled(task: str, method: str) -> float:
    return report["summary"][f"{task}_group_means"][method]["pooled_rmse_pp"]


fig, axes = plt.subplots(2, 2, figsize=(12.5, 10.5))

ax = axes[0, 0]
rows = [
    ("model 1.2 (stacked)", STACKED),
    ("model 1.0.0 (same fit)", MODEL),
    ("persistence", "persistence"),
    ("pooled-curve increment", "pooled_curve_ratio"),
    ("analog kNN", "analog_knn"),
]
width = 0.38
largest = 0.0
for offset, task, colour in (
    (-width / 2, "next_observation", "#1f77b4"),
    (width / 2, "landmark", "#ff7f0e"),
):
    values = [pooled(task, method) for _, method in rows]
    positions = [i + offset for i in range(len(rows))]
    ax.barh(positions, values, height=width, color=colour, label=task.replace("_", " "))
    for y, v in zip(positions, values, strict=True):
        ax.text(v + 0.1, y, f"{v:.2f}", va="center", fontsize=8)
    largest = max(largest, *values)
ax.axvline(5.0, color="grey", linestyle="--", linewidth=1)
ax.set_xlim(0, largest * 1.45)
ax.set_yticks(range(len(rows)), [r[0] for r in rows])
ax.invert_yaxis()
ax.set_xlabel("pooled RMSE, group means (ETDRS letters; 5 letters = 0.10 logMAR)")
coverage = analysis["H1_calibration_next_observation_group_means"]
series = report["summary"]["next_observation_group_means"][STACKED]["series"]
ax.set_title(
    f"Sealed test, {series} group-mean series: H1-H4 pass, H5 fails\n"
    f"stacked next-observation coverage {coverage['80']['coverage']:.2f} (80%) and "
    f"{coverage['90']['coverage']:.2f} (90%)",
    fontsize=10,
)
ax.legend(loc="upper right")

ax = axes[0, 1]
contexts = [
    c
    for c, block in sorted(report["summary"]["by_context"].items())
    if INITIAL in block.get("initial_state_group_mean", {})
]
methods = [
    (STACKED_INITIAL, "model 1.2 (stacked)", "#9467bd"),
    (INITIAL, "model 1.0.0", "#1f77b4"),
    ("pooled_training_curve", "pooled training curve", "#bcbd22"),
]
width = 0.26
top = 0.0
for k, (method, label, colour) in enumerate(methods):
    values = [
        report["summary"]["by_context"][c]["initial_state_group_mean"][method]["pooled_rmse_pp"]
        for c in contexts
    ]
    positions = [i + (k - 1) * width for i in range(len(contexts))]
    ax.bar(positions, values, width=width, color=colour, label=label)
    for x, v in zip(positions, values, strict=True):
        ax.text(x, v + 0.3, f"{v:.1f}", ha="center", fontsize=7)
    top = max(top, *values)
targets = [
    report["summary"]["by_context"][c]["initial_state_group_mean"][INITIAL]["targets"]
    for c in contexts
]
ax.set_xticks(
    range(len(contexts)),
    [f"{c.split('/')[-1]}\n({n} targets)" for c, n in zip(contexts, targets, strict=True)],
)
ax.set_ylim(0, top * 1.2)
ax.set_ylabel("pooled RMSE, control group means (letters)")
initial = report["summary"]["initial_state"]["group_mean"]
ax.set_title(
    "Initial state by procedure: the pooled curve helps cross-linking\n"
    f"and hurts keratoplasty (all: stacked {initial[STACKED_INITIAL]['pooled_rmse_pp']:.2f}, "
    f"1.0.0 {initial[INITIAL]['pooled_rmse_pp']:.2f}, curve "
    f"{initial['pooled_training_curve']['pooled_rmse_pp']:.2f})",
    fontsize=10,
)
ax.legend(fontsize=8, loc="upper left")


def scatter(ax: plt.Axes, task: str, new: str, old: str) -> None:
    # Per-series scores are public for every study (redacted ones keep only metrics).
    points = [
        entry
        for study in report["studies"]
        for entry in study[task]
        if entry["kind"] == "group_mean" and new in entry["scores"] and old in entry["scores"]
    ]
    top = 0.0
    for entry in points:
        x, y = entry["scores"][old]["rmse_pp"], entry["scores"][new]["rmse_pp"]
        ax.scatter(x, y, color=COLOURS[procedure(entry["context"])], s=30, alpha=0.8)
        top = max(top, x, y)
    top *= 1.08
    ax.plot([0, top], [0, top], color="black", linewidth=0.8)
    ax.set_xlim(0, top)
    ax.set_ylim(0, top)
    ax.set_xlabel("model 1.0.0, RMSE per series (letters)")
    ax.set_ylabel("model 1.2 stacked, RMSE per series (letters)")
    worse = sum(e["scores"][new]["rmse_pp"] > e["scores"][old]["rmse_pp"] for e in points)
    handles = [
        Line2D([], [], color=c, marker="o", linestyle="", label=k) for k, c in COLOURS.items()
    ]
    ax.legend(handles=handles, fontsize=8, loc="upper left", title="procedure", title_fontsize=8)
    ax.text(
        0.97 * top,
        0.04 * top,
        f"above the line: stacking worse\n({worse} of {len(points)} series)",
        ha="right",
        fontsize=8,
    )


scatter(axes[1, 0], "landmark", STACKED, MODEL)
axes[1, 0].set_title("Landmark forecasts per group mean: stacked against model 1.0.0", fontsize=10)
scatter(axes[1, 1], "initial_state", STACKED_INITIAL, INITIAL)
axes[1, 1].set_title(
    "Initial state per control group mean: stacked against model 1.0.0", fontsize=10
)

fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150, bbox_inches="tight")
fig.savefig(HERE / "sealed-results.pdf", bbox_inches="tight")
print("wrote", HERE / "sealed-results.png", "verdicts", analysis["verdicts"])
