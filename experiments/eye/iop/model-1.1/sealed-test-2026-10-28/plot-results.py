"""Plot the one-shot tranche-2 sealed results of IOP model 1.1 from public files only.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/iop/model-1.1/sealed-test-2026-10-28/plot-results.py

Values come only from studies whose values are public in ``report-public.json`` and
``reference-1.0.0-public.json``; redacted (NC/ND/SA) studies contribute to the pooled
bars through the published summaries only.
"""

from __future__ import annotations

import json
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

HERE = Path(__file__).resolve().parent
report = json.loads((HERE / "report-public.json").read_text())
reference = json.loads((HERE / "reference-1.0.0-public.json").read_text())
analysis = json.loads((HERE / "sealed-analysis.json").read_text())
PRIMARY = "population_assimilation"
studies = {s["study_id"]: s for s in report["studies"]}
reference_studies = {s["study_id"]: s for s in reference["studies"]}
# Panels chosen after unsealing from public (CC BY) studies: (study, series groups, title).
PANELS = [
    (
        "iop-pmc7653954",
        "xen-alone,xen-phaco",
        "XEN gel stent: early course followed by the 1.1 transient",
    ),
    (
        "iop-pmc8594178",
        "kdb-goniotomy",
        "Goniotomy (KDB): low day 1, then a rise far above the prior",
    ),
]

fig, axes = plt.subplots(2, 2, figsize=(12.5, 9.5))
ax = axes[0, 0]
rows = [
    ("model 1.1", report["summary"], PRIMARY),
    ("model 1.0.0 (same training)", reference["summary"], PRIMARY),
    ("analog kNN", report["summary"], "analog_knn"),
    ("pooled-curve ratio", report["summary"], "pooled_curve_ratio"),
    ("persistence", report["summary"], "persistence"),
]
width = 0.38
for offset, task, colour in (
    (-width / 2, "next_observation", "#1f77b4"),
    (width / 2, "landmark", "#ff7f0e"),
):
    values = [summary[f"{task}_group_means"][m]["pooled_rmse_pp"] for _, summary, m in rows]
    positions = [i + offset for i in range(len(rows))]
    ax.barh(positions, values, height=width, color=colour, label=task.replace("_", " "))
    for y, v in zip(positions, values, strict=True):
        ax.text(v + 0.15, y, f"{v:.2f}", va="center", fontsize=8)
ax.axvline(5.0, color="grey", linestyle="--", linewidth=1)
ax.set_yticks(range(len(rows)), [r[0] for r in rows])
ax.invert_yaxis()
ax.set_xlabel("pooled RMSE, group means (pp of preoperative IOP)")
groups = report["summary"]["next_observation_group_means"][PRIMARY]["series"]
ax.set_title(f"Tranche-2 sealed test: {groups} group-mean forecast series")
ax.legend(loc="upper right")

ax = axes[0, 1]
for source, method, colour, name in (
    (reference_studies, PRIMARY, "#999999", "model 1.0.0"),
    (studies, PRIMARY, "#1f77b4", "model 1.1"),
):
    xs, ys = [], []
    for study in source.values():
        if study.get("values_redacted"):
            continue
        for entry in study["next_observation"]:
            if entry["kind"] != "group_mean" or not entry["context"].startswith("human/"):
                continue
            for row in entry["rows"]:
                if method in row["predictions"]:
                    xs.append(100 * row["observed"])
                    ys.append(100 * row["predictions"][method])
    ax.scatter(xs, ys, s=16, color=colour, alpha=0.7, label=f"{name} ({len(xs)} targets)")
ax.plot([10, 140], [10, 140], color="black", linewidth=1)
ax.fill_between([10, 140], [5, 135], [15, 145], color="green", alpha=0.08, label="±5 pp")
ax.set_xlim(10, 140)
ax.set_ylim(10, 140)
ax.set_xlabel("observed next follow-up (% of preoperative IOP)")
ax.set_ylabel("forecast (% of preoperative IOP)")
ax.set_title("Next observation, group means of public sealed studies")
ax.legend(loc="upper left", fontsize=8)


def series_panel(ax: plt.Axes, study_id: str, names: list[str], title: str) -> None:
    study = studies[study_id]
    colours = ("black", "#d62728", "#2ca02c")
    chosen = [e for e in study["initial_state"] if e["series_id"].split("/")[1] in names]
    for index, initial in enumerate(chosen):
        colour = colours[index % len(colours)]
        name = initial["series_id"].split("/")[1]
        months = initial["months"]
        ax.plot(
            months,
            [100 * v for v in initial["observed"]],
            "o-",
            color=colour,
            label=f"observed ({name})",
        )
        forecast = next(
            e for e in study["next_observation"] if e["series_id"] == initial["series_id"]
        )
        targets = [row["target_month"] for row in forecast["rows"]]
        model = [100 * row["predictions"][PRIMARY] for row in forecast["rows"]]
        low = [100 * row["intervals"][PRIMARY]["0.05"] for row in forecast["rows"]]
        high = [100 * row["intervals"][PRIMARY]["0.95"] for row in forecast["rows"]]
        label = f"model 1.1 next follow-up ({name})"
        ax.plot(targets, model, "s--", color=colour, alpha=0.6, label=label)
        ax.fill_between(targets, low, high, color=colour, alpha=0.08)
        old = next(
            e
            for e in reference_studies[study_id]["next_observation"]
            if e["series_id"] == initial["series_id"]
        )
        ax.plot(
            [row["target_month"] for row in old["rows"]],
            [100 * row["predictions"][PRIMARY] for row in old["rows"]],
            "x:",
            color=colour,
            alpha=0.6,
            label=f"model 1.0.0 next follow-up ({name})",
        )
    ax.set_xscale("log")
    ax.set_xlabel("months after surgery (log scale)")
    ax.set_ylabel("IOP (% of preoperative)")
    ax.set_title(f"{title}\n(shaded: 90% interval of the 1.1 next-follow-up forecast)", fontsize=10)
    handles, labels = ax.get_legend_handles_labels()
    unique = dict(zip(labels, handles, strict=True))
    ax.legend(unique.values(), unique.keys(), fontsize=7)


for ax, (study_id, names, title) in zip(axes[1], PANELS, strict=True):
    series_panel(ax, study_id, names.split(","), title)
fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150)
fig.savefig(HERE / "sealed-results.pdf")
print("wrote", HERE / "sealed-results.png", "verdicts", analysis["verdicts"])
