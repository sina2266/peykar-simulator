"""Plot the one-shot sealed results of retinal-thickness model 1.1.0 from public files only.

Run with a Python that has Matplotlib (the repository's native analysis environment),
from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/retina/model-1.1/sealed-test-2026-10-28/plot-results.py

Values come only from studies whose values are public in ``report-public.json`` and
``reference-public.json``; redacted (NC/ND/SA) studies contribute to the pooled bars
through the published summaries only.
"""

from __future__ import annotations

import json
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

HERE = Path(__file__).resolve().parent
report = json.loads((HERE / "report-public.json").read_text())
reference = json.loads((HERE / "reference-public.json").read_text())
analysis = json.loads((HERE / "sealed-analysis.json").read_text())
PRIMARY = "population_assimilation"
studies = {s["study_id"]: s for s in report["studies"]}
reference_studies = {s["study_id"]: s for s in reference["studies"]}
# Panels chosen after unsealing from public (CC BY) studies: (study, series groups, title).
PANELS = [
    (
        "ret-pmc10926656",
        "dexamethasone-implant",
        "Single dexamethasone implant (DME): 1.1.0's dose rule follows the fall",
    ),
    (
        "ret-pmc10970839",
        "brolucizumab-naive,aflibercept-naive,brolucizumab-recalcitrant",
        "Anti-VEGF in DME (four-arm cohort): forecasts close to 1.0.0's",
    ),
]

fig, axes = plt.subplots(2, 2, figsize=(12.5, 9.5))
ax = axes[0, 0]
rows = [
    ("model 1.1.0", report["summary"], PRIMARY),
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
ax.set_xlim(
    0,
    max(
        14.0,
        1.15
        * max(
            v
            for _, s, m in rows
            for v in (
                s["next_observation_group_means"][m]["pooled_rmse_pp"],
                s["landmark_group_means"][m]["pooled_rmse_pp"],
            )
        ),
    ),
)
ax.set_yticks(range(len(rows)), [r[0] for r in rows])
ax.invert_yaxis()
ax.set_xlabel("pooled RMSE, group means (pp of baseline thickness)")
groups = report["summary"]["next_observation_group_means"][PRIMARY]["series"]
ax.set_title(f"Tranche-2 sealed test: {groups} group-mean forecast series")
ax.legend(loc="upper right", fontsize=8)

ax = axes[0, 1]
for source, colour, name in (
    (reference_studies, "#999999", "model 1.0.0"),
    (studies, "#1f77b4", "model 1.1.0"),
):
    xs, ys = [], []
    for study in source.values():
        if study.get("values_redacted"):
            continue
        for entry in study["next_observation"]:
            if entry["kind"] != "group_mean":
                continue
            for row in entry["rows"]:
                if PRIMARY in row["predictions"]:
                    xs.append(100 * row["observed"])
                    ys.append(100 * row["predictions"][PRIMARY])
    ax.scatter(xs, ys, s=16, color=colour, alpha=0.7, label=f"{name} ({len(xs)} targets)")
ax.plot([30, 130], [30, 130], color="black", linewidth=1)
ax.fill_between([30, 130], [25, 125], [35, 135], color="green", alpha=0.08, label="±5 pp")
ax.set_xlim(30, 130)
ax.set_ylim(30, 130)
ax.set_xlabel("observed next follow-up (% of baseline thickness)")
ax.set_ylabel("forecast (% of baseline thickness)")
ax.set_title("Next observation, group means of public sealed studies")
ax.legend(loc="upper left", fontsize=8)


def series_panel(ax: plt.Axes, study_id: str, names: list[str], title: str) -> None:
    study = studies[study_id]
    old = reference_studies[study_id]
    colours = ("black", "#d62728", "#2ca02c")
    chosen = [e for e in study["next_observation"] if e["series_id"].split("/")[1] in names]
    for index, forecast in enumerate(chosen):
        colour = colours[index % len(colours)]
        name = forecast["series_id"].split("/")[1]
        initial = next(
            (e for e in study["initial_state"] if e["series_id"] == forecast["series_id"]), None
        )
        months = [row["target_month"] for row in forecast["rows"]]
        observed = [100 * row["observed"] for row in forecast["rows"]]
        if initial is not None:
            months, observed = initial["months"], [100 * v for v in initial["observed"]]
        ax.plot([0, *months], [100, *observed], "o-", color=colour, label=f"observed ({name})")
        targets = [row["target_month"] for row in forecast["rows"]]
        model = [100 * row["predictions"][PRIMARY] for row in forecast["rows"]]
        low = [100 * row["intervals"][PRIMARY]["0.05"] for row in forecast["rows"]]
        high = [100 * row["intervals"][PRIMARY]["0.95"] for row in forecast["rows"]]
        ax.plot(
            targets, model, "s--", color=colour, alpha=0.8, label=f"1.1.0 next follow-up ({name})"
        )
        ax.fill_between(targets, low, high, color=colour, alpha=0.08)
        before = next(e for e in old["next_observation"] if e["series_id"] == forecast["series_id"])
        ax.plot(
            [row["target_month"] for row in before["rows"]],
            [100 * row["predictions"][PRIMARY] for row in before["rows"]],
            "x:",
            color=colour,
            alpha=0.6,
            label=f"1.0.0 next follow-up ({name})",
        )
    ax.set_xlabel("months after the first treatment")
    ax.set_ylabel("central thickness (% of baseline)")
    ax.set_title(
        f"{title}\n(shaded: 90% interval of the 1.1.0 next-follow-up forecast)", fontsize=10
    )
    handles, labels = ax.get_legend_handles_labels()
    unique = dict(zip(labels, handles, strict=True))
    ax.legend(unique.values(), unique.keys(), fontsize=7)


for ax, (study_id, names, title) in zip(axes[1], PANELS, strict=True):
    series_panel(ax, study_id, names.split(","), title)
fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150)
fig.savefig(HERE / "sealed-results.pdf")
print("wrote", HERE / "sealed-results.png", "verdicts", analysis["verdicts"])
