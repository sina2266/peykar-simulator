"""Plot the one-shot sealed results of retinal-thickness model 1.4 from public files.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/retina/model-1.4/sealed-test-2026-10-30/plot-results.py

The pooled values are the paired values of the declared checks in
``sealed-analysis.json`` (common targets with the stacked forecast). The per-series
panels use the per-series scores of ``report-public.json``, public for every study
(values of redacted studies are not). Values are percentage points of each series'
baseline central retinal thickness.
"""

from __future__ import annotations

import json
import math
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402

HERE = Path(__file__).resolve().parent
report = json.loads((HERE / "report-public.json").read_text())
analysis = json.loads((HERE / "sealed-analysis.json").read_text())
STACKED, MODEL = "stacked", "population_assimilation"
CLASSES = {
    "anti_vegf": "#1f77b4",
    "steroid_implant": "#d62728",
    "combination": "#9467bd",
    "other": "#7f7f7f",
}


def treatment(context: str) -> str:
    name = context.rsplit("/", 1)[-1]
    return name if name in CLASSES else "other"


def group_means(task: str) -> list[dict]:
    return [
        entry
        for study in report["studies"]
        for entry in study[task]
        if entry["kind"] == "group_mean" and STACKED in entry["scores"] and MODEL in entry["scores"]
    ]


fig, axes = plt.subplots(2, 2, figsize=(12.5, 10.5))

ax = axes[0, 0]
h5 = analysis["H5_versus_model_1_1_same_fit"]
paired = {
    "next_observation": analysis["H2_accuracy_next_observation_group_means"],
    "landmark": analysis["H3_landmark_group_means"]["comparisons"],
}
rows = [
    ("model 1.4 (stacked)", STACKED),
    ("model 1.1.0 (same fit)", MODEL),
    ("persistence", "persistence"),
    ("pooled-curve increment", "pooled_curve_ratio"),
    ("analog kNN", "analog_knn"),
]


def paired_value(task: str, method: str) -> float:
    if method == STACKED:
        return h5[task]["model_1_4_rmse_pp"]
    if method == MODEL:
        return h5[task]["model_1_1_rmse_pp"]
    return paired[task][method][f"{method}_rmse_pp"]


width = 0.38
largest = 0.0
for offset, task, colour in (
    (-width / 2, "next_observation", "#1f77b4"),
    (width / 2, "landmark", "#ff7f0e"),
):
    values = [paired_value(task, method) for _, method in rows]
    positions = [i + offset for i in range(len(rows))]
    ax.barh(positions, values, height=width, color=colour, label=task.replace("_", " "))
    for y, v in zip(positions, values, strict=True):
        ax.text(v + 0.1, y, f"{v:.2f}", va="center", fontsize=8)
    largest = max(largest, *values)
ax.axvline(5.0, color="grey", linestyle="--", linewidth=1)
ax.set_xlim(0, largest * 1.45)
ax.set_yticks(range(len(rows)), [r[0] for r in rows])
ax.invert_yaxis()
targets = h5["landmark"]["common_targets"]
ax.set_xlabel(f"pooled RMSE, human group means, {targets} common targets (pp of the baseline)")
series = len(group_means("next_observation"))
initial = h5["initial_state"]
curve = report["summary"]["initial_state"]["group_mean"]["pooled_training_curve"]
ax.set_title(
    f"Sealed test, {series} group-mean series: H1, H2, H4 pass; H3 and H5 fail\n"
    f"initial state (1.1.0's own) {initial['model_1_4_rmse_pp']:.2f}, "
    f"pooled curve {curve['pooled_rmse_pp']:.2f} pp",
    fontsize=10,
)
ax.legend(loc="lower right")

ax = axes[0, 1]
stacked_h1 = analysis["H1_calibration_next_observation_group_means"]
stacked_lm = analysis["H3_landmark_group_means"]["calibration_reported"]
reference = analysis["model_1_1_own_H1_H4_reported_not_tested"]
model_h1 = reference["H1_calibration_next_observation_group_means"]
model_lm = reference["H3_landmark_group_means"]["calibration_reported"]
groups = [
    ("next obs. 80%\n(H1, tested)", stacked_h1["80"], model_h1["80"]),
    ("next obs. 90%\n(H1, tested)", stacked_h1["90"], model_h1["90"]),
    ("landmark 80%\n(reported)", stacked_lm["80"], model_lm["80"]),
    ("landmark 90%\n(reported)", stacked_lm["90"], model_lm["90"]),
]
for k, (_, new, old) in enumerate(groups):
    low, high = new["window"]
    ax.add_patch(
        plt.Rectangle((k - 0.45, low), 0.9, high - low, color="#2ca02c", alpha=0.12, zorder=0)
    )
    for j, (block, label, colour) in enumerate(
        ((new, "model 1.4 (scaled interval)", "#9467bd"), (old, "model 1.1.0", "#1f77b4"))
    ):
        x = k + (j - 0.5) * 0.35
        ax.bar(x, block["coverage"], width=0.33, color=colour, label=label if k == 0 else None)
        ax.text(
            x,
            block["coverage"] - 0.03,
            f"{block['coverage']:.3f}",
            ha="center",
            fontsize=8,
            color="white",
        )
ax.set_xticks(range(len(groups)), [g[0] for g in groups], fontsize=8)
ax.set_ylim(0.55, 1.0)
ax.set_ylabel("coverage, human group means (windows shaded)")
ax.set_title(
    "Next-observation intervals scaled by 0.79: 80% coverage 0.793 (1.1.0: 0.839);\n"
    "landmark intervals (scale 1.00) under-cover for both models",
    fontsize=10,
)
ax.legend(fontsize=8, loc="upper right")

ax = axes[1, 0]
names = list(CLASSES)
gains: dict[str, list[float]] = {task: [] for task in ("next_observation", "landmark")}
counts = []
for name in names:
    total = 0
    for task in gains:
        new = old = 0.0
        n = 0
        for entry in group_means(task):
            if treatment(entry["context"]) != name:
                continue
            scores = entry["scores"]
            new += scores[STACKED]["rmse_pp"] ** 2 * scores[STACKED]["n"]
            old += scores[MODEL]["rmse_pp"] ** 2 * scores[MODEL]["n"]
            n += scores[STACKED]["n"]
        gains[task].append(math.sqrt(old / n) - math.sqrt(new / n) if n else 0.0)
        total = n
    counts.append(total)
for offset, task, colour in ((-0.2, "next_observation", "#1f77b4"), (0.2, "landmark", "#ff7f0e")):
    positions = [i + offset for i in range(len(names))]
    ax.bar(positions, gains[task], width=0.38, color=colour, label=task.replace("_", " "))
    for x, v in zip(positions, gains[task], strict=True):
        ax.text(x, v + (0.03 if v >= 0 else -0.12), f"{v:+.2f}", ha="center", fontsize=7)
ax.axhline(0, color="black", linewidth=0.8)
lowest = min(min(v) for v in gains.values())
highest = max(max(v) for v in gains.values())
ax.set_ylim(min(lowest, 0.0) - 0.3, max(highest, 0.0) + 0.2)
ax.set_xticks(
    range(len(names)),
    [f"{n.replace('_', ' ')}\n({c} targets)" for n, c in zip(names, counts, strict=True)],
    fontsize=8,
)
ax.set_ylabel("RMSE of 1.1.0 minus RMSE of 1.4 (pp; above 0: stacking better)")
ax.set_title("Gain of stacking by treatment, human group means", fontsize=10)
ax.legend(fontsize=8, loc="lower left")

ax = axes[1, 1]
points = group_means("landmark")
top = 0.0
for entry in points:
    x, y = entry["scores"][MODEL]["rmse_pp"], entry["scores"][STACKED]["rmse_pp"]
    ax.scatter(x, y, color=CLASSES[treatment(entry["context"])], s=24, alpha=0.75)
    top = max(top, x, y)
top *= 1.05
ax.plot([0, top], [0, top], color="black", linewidth=0.8)
ax.set_xlim(0, top)
ax.set_ylim(0, top)
ax.set_xlabel("model 1.1.0, landmark RMSE per series (pp)")
ax.set_ylabel("model 1.4 stacked, landmark RMSE per series (pp)")
worse = sum(e["scores"][STACKED]["rmse_pp"] > e["scores"][MODEL]["rmse_pp"] for e in points)
handles = [
    Line2D([], [], color=c, marker="o", linestyle="", label=k.replace("_", " "))
    for k, c in CLASSES.items()
]
ax.legend(handles=handles, fontsize=8, loc="upper left", title="treatment", title_fontsize=8)
ax.text(
    0.97 * top,
    0.04 * top,
    f"above the line: stacking worse\n({worse} of {len(points)} series)",
    ha="right",
    fontsize=8,
)
ax.set_title("Landmark forecasts per group mean: stacked against 1.1.0", fontsize=10)

fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150, bbox_inches="tight")
fig.savefig(HERE / "sealed-results.pdf", bbox_inches="tight")
print("wrote", HERE / "sealed-results.png", "verdicts", analysis["verdicts"])
