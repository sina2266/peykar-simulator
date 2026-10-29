"""Plot the one-shot sealed results of retinal visual-acuity model 1.1 from public files.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/retina-acuity/model-1.1/sealed-test-2026-10-29/plot-results.py

Pooled bars come from the published summaries of ``report-public.json`` (model 1.1)
and ``reference-1.0.0-public.json`` (model 1.0.0 on the same studies). The per-series
scatter panels use the per-series metrics of ``sealed-analysis.json``, which are public
for every study (values of redacted studies are not). The series panel uses a public
(CC BY) study only. Reported values are ``g = 1 + change / 2 logMAR`` (one pp = one
ETDRS letter = 0.02 logMAR), so the acuity is ``logMAR0 + 2 (g - 1)``.
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
reference = json.loads((HERE / "reference-1.0.0-public.json").read_text())
analysis = json.loads((HERE / "sealed-analysis.json").read_text())
PRIMARY = "population_anchored_t4"
REFERENCE = "population_assimilation_t4"
INITIAL = "population_initial_state"
# Chosen after unsealing: the public study with a day-1 visit (the failure mode of 1.0).
SERIES_STUDY = "rac-pmc8230427"
COLOURS = {"steroid_implant": "#d62728", "anti_vegf": "#1f77b4", "other": "#7f7f7f"}


def treatment_class(context: str) -> str:
    treatment = context.rsplit("/", 1)[-1]
    return treatment if treatment in COLOURS else "other"


def pooled(source: dict, task: str, method: str) -> float:
    return source["summary"][f"{task}_group_means"][method]["pooled_rmse_pp"]


fig, axes = plt.subplots(2, 2, figsize=(12.5, 10.5))

ax = axes[0, 0]
rows = [
    ("model 1.1 (anchored t4)", report, PRIMARY),
    ("model 1.0.0 (same training)", reference, REFERENCE),
    ("persistence", report, "persistence"),
    ("pooled-curve increment", report, "pooled_curve_ratio"),
    ("analog kNN", report, "analog_knn"),
]
width = 0.38
largest = 0.0
for offset, task, colour in (
    (-width / 2, "next_observation", "#1f77b4"),
    (width / 2, "landmark", "#ff7f0e"),
):
    values = [pooled(source, task, method) for _, source, method in rows]
    positions = [i + offset for i in range(len(rows))]
    ax.barh(positions, values, height=width, color=colour, label=task.replace("_", " "))
    for y, v in zip(positions, values, strict=True):
        ax.text(v + 0.1, y, f"{v:.2f}", va="center", fontsize=8)
    largest = max(largest, *values)
ax.axvline(5.0, color="grey", linestyle="--", linewidth=1)
ax.set_xlim(0, largest * 1.2)
ax.set_yticks(range(len(rows)), [r[0] for r in rows])
ax.invert_yaxis()
ax.set_xlabel("pooled RMSE, group means (ETDRS letters; 5 letters = 0.10 logMAR)")
series = report["summary"]["next_observation_group_means"][PRIMARY]["series"]
initial = report["summary"]["initial_state"]["group_mean"]
initial_old = reference["summary"]["initial_state"]["group_mean"][INITIAL]["pooled_rmse_pp"]
coverage = analysis["H1_calibration_next_observation_group_means"]
ax.set_title(
    f"Sealed test, {series} group-mean series: H1 passes, H2–H5 fail\n"
    f"initial state {initial[INITIAL]['pooled_rmse_pp']:.2f} (1.0.0 {initial_old:.2f}) vs "
    f"pooled training curve {initial['pooled_training_curve']['pooled_rmse_pp']:.2f} "
    f"letters; 80% coverage {coverage['80']['coverage']:.2f}",
    fontsize=10,
)
ax.legend(loc="upper right")


def scatter(ax: plt.Axes, task: str, method: str, comparator: str, label: str) -> None:
    points = [
        row
        for row in analysis["per_series"][task]
        if row["kind"] == "group_mean" and method in row and comparator in row
    ]
    top = 0.0
    for row in points:
        x, y = row[comparator]["rmse_pp"], row[method]["rmse_pp"]
        ax.scatter(x, y, color=COLOURS[treatment_class(row["context"])], s=28, alpha=0.8)
        top = max(top, x, y)
    top *= 1.08
    ax.plot([0, top], [0, top], color="black", linewidth=0.8)
    ax.set_xlim(0, top)
    ax.set_ylim(0, top)
    ax.set_xlabel(f"{label}, RMSE per series (letters)")
    ax.set_ylabel(f"model 1.1 {method.replace('population_', '')}, RMSE per series (letters)")
    worse = sum(row[method]["rmse_pp"] > row[comparator]["rmse_pp"] for row in points)
    handles = [
        Line2D([], [], color=c, marker="o", linestyle="", label=k.replace("_", " "))
        for k, c in COLOURS.items()
    ]
    ax.legend(handles=handles, fontsize=8, loc="upper left", title="treatment", title_fontsize=8)
    ax.text(
        0.97 * top,
        0.04 * top,
        f"above the line: model worse\n({worse} of {len(points)} series)",
        ha="right",
        fontsize=8,
    )


scatter(axes[0, 1], "landmark", PRIMARY, "persistence", "persistence")
axes[0, 1].set_title(
    "Landmark forecasts per group mean: the dexamethasone-implant\n"
    "DME series (red) carry the largest model errors",
    fontsize=10,
)
scatter(axes[1, 1], "initial_state", INITIAL, "pooled_training_curve", "pooled training curve")
axes[1, 1].set_title(
    "Initial state per control group mean: the context priors\n"
    "do not beat one pooled training curve",
    fontsize=10,
)

ax = axes[1, 0]
study = next(s for s in report["studies"] if s["study_id"] == SERIES_STUDY)
old = next(s for s in reference["studies"] if s["study_id"] == SERIES_STUDY)
entry = study["initial_state"][0]
logmar0 = entry["reference_value"]
times = [0.0, *entry["months"]]
observed = [logmar0, *(logmar0 + 2 * (g - 1) for g in entry["observed"])]
ax.plot(times, observed, "o-", color="black", label=f"observed (logMAR0 {logmar0:.2f})")
landmark = study["landmark"][0]["rows"]
landmark_old = old["landmark"][0]["rows"]
targets = [r["target_month"] for r in landmark]
for rows_, method, colour, name in (
    (landmark, PRIMARY, "#1f77b4", "model 1.1 landmark forecast"),
    (landmark_old, REFERENCE, "#d62728", "model 1.0.0 landmark forecast"),
):
    ax.plot(
        targets,
        [logmar0 + 2 * (r["predictions"][method] - 1) for r in rows_],
        "s--",
        color=colour,
        label=name,
    )
    ax.fill_between(
        targets,
        [logmar0 + 2 * (r["intervals"][method]["0.05"] - 1) for r in rows_],
        [logmar0 + 2 * (r["intervals"][method]["0.95"] - 1) for r in rows_],
        color=colour,
        alpha=0.10,
    )
ax.axvline(landmark[0]["origin_month"], color="grey", linestyle=":", linewidth=1)
ax.text(
    landmark[0]["origin_month"] + 0.015,
    0.97,
    "day-1 visit\n(forecast origin)",
    fontsize=8,
    va="top",
    transform=ax.get_xaxis_transform(),
)
ax.invert_yaxis()
ax.set_xlabel("months after the first treatment")
ax.set_ylabel("best-corrected acuity (logMAR; up is better)")
ax.set_title(
    "nAMD, switch to brolucizumab (public study): from a day-1 visit\n"
    "1.1 holds, 1.0.0 drifts (shaded: 90% intervals)",
    fontsize=10,
)
ax.legend(fontsize=8, loc="lower left")

fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150, bbox_inches="tight")
fig.savefig(HERE / "sealed-results.pdf", bbox_inches="tight")
print("wrote", HERE / "sealed-results.png", "verdicts", analysis["verdicts"])
