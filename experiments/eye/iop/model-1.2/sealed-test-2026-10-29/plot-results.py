"""Plot the one-shot tranche-3 sealed results of IOP model 1.2 from public files only.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/iop/model-1.2/sealed-test-2026-10-29/plot-results.py

Values come only from studies whose values are public in ``report-public.json`` and
``reference-1.1.0-public.json``; redacted (NC/ND/SA) studies contribute to the pooled
bars and coverage only through the published summaries.
"""

from __future__ import annotations

import json
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

HERE = Path(__file__).resolve().parent
report = json.loads((HERE / "report-public.json").read_text())
reference = json.loads((HERE / "reference-1.1.0-public.json").read_text())
analysis = json.loads((HERE / "sealed-analysis.json").read_text())
PRIMARY = "population_assimilation"
studies = {s["study_id"]: s for s in report["studies"]}
reference_studies = {s["study_id"]: s for s in reference["studies"]}

# Panels chosen after unsealing from public (CC BY) control group means.
PANELS = [
    (
        [("iop-pmc12249627", "phaco-kdb"), ("iop-pmc12249627", "phaco-tanito-microhook")],
        "Goniotomy with phaco (two blades): wider first-month intervals, same forecasts",
    ),
    (
        [("iop-pmc7603617", "agv-conventional"), ("iop-pmc7647454", "ahmed-conventional")],
        "Ahmed valve (two studies): the month-1 pressure rise is still missed",
    ),
]

fig, axes = plt.subplots(2, 2, figsize=(12.5, 11.0))

ax = axes[0, 0]
rows = [
    ("model 1.2", report["summary"], PRIMARY),
    ("model 1.1.0 (same training)", reference["summary"], PRIMARY),
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
ax.set_title(f"Tranche-3 sealed test: {groups} group-mean forecast series")
ax.legend(loc="upper right")

ax = axes[0, 1]
h5 = analysis["H5_versus_model_1_1_same_training"]["first_month_next_observation_coverage"]
labels, new_values, old_values, nominal = [], [], [], []
for level in ("80", "90"):
    labels.append(f"first month\n{level}%\n({h5['targets']} targets)")
    new_values.append(h5[level]["model_1_2"])
    old_values.append(h5[level]["model_1_1"])
    nominal.append(h5[level]["nominal"])
for level in ("80", "90"):
    new = report["summary"]["coverage"][level]["next_observation"][PRIMARY]
    old = reference["summary"]["coverage"][level]["next_observation"][PRIMARY]
    labels.append(f"all months\n{level}%\n({new['n']} targets)")
    new_values.append(new["inside"] / new["n"])
    old_values.append(old["inside"] / old["n"])
    nominal.append(int(level) / 100)
positions = range(len(labels))
ax.bar([p - width / 2 for p in positions], old_values, width=width, color="#999999", label="1.1.0")
ax.bar([p + width / 2 for p in positions], new_values, width=width, color="#1f77b4", label="1.2")
for p, a, b, n in zip(positions, old_values, new_values, nominal, strict=True):
    ax.hlines(n, p - 0.45, p + 0.45, color="black", linestyle="--", linewidth=1)
    ax.text(p - width / 2, a + 0.01, f"{a:.2f}", ha="center", fontsize=8)
    ax.text(p + width / 2, b + 0.01, f"{b:.2f}", ha="center", fontsize=8)
ax.set_xticks(list(positions), labels, fontsize=8)
ax.set_ylim(0.5, 1.02)
ax.set_ylabel("share of next follow-ups inside the interval")
ax.set_title(
    "Next-observation interval coverage (dashed: nominal)\n"
    "first month: group means; all months: all series",
    fontsize=10,
)
ax.legend(loc="lower right")


def series_panel(ax: plt.Axes, chosen: list[tuple[str, str]], title: str) -> None:
    colours = ("black", "#d62728", "#2ca02c")
    for index, (study_id, name) in enumerate(chosen):
        colour = colours[index % len(colours)]
        study = studies[study_id]
        initial = next(e for e in study["initial_state"] if e["series_id"].split("/")[1] == name)
        tag = f"{study_id.removeprefix('iop-pmc')}:{name}"
        ax.plot(
            initial["months"],
            [100 * v for v in initial["observed"]],
            "o-",
            color=colour,
            label=f"observed ({tag})",
        )
        forecast = next(
            e for e in study["next_observation"] if e["series_id"] == initial["series_id"]
        )
        targets = [row["target_month"] for row in forecast["rows"]]
        ax.plot(
            targets,
            [100 * row["predictions"][PRIMARY] for row in forecast["rows"]],
            "s--",
            color=colour,
            alpha=0.6,
            label=f"model 1.2 next follow-up ({tag})",
        )
        ax.fill_between(
            targets,
            [100 * row["intervals"][PRIMARY]["0.05"] for row in forecast["rows"]],
            [100 * row["intervals"][PRIMARY]["0.95"] for row in forecast["rows"]],
            color=colour,
            alpha=0.10,
        )
        old = next(
            e
            for e in reference_studies[study_id]["next_observation"]
            if e["series_id"] == initial["series_id"]
        )
        for quantile in ("0.05", "0.95"):
            ax.plot(
                [row["target_month"] for row in old["rows"]],
                [100 * row["intervals"][PRIMARY][quantile] for row in old["rows"]],
                ":",
                color=colour,
                alpha=0.8,
                label=f"model 1.1.0 90% interval ({tag})",
            )
    ax.set_xscale("log")
    ax.set_xlabel("months after surgery (log scale)")
    ax.set_ylabel("IOP (% of preoperative)")
    ax.set_title(f"{title}\n(shaded: 90% interval of the 1.2 next-follow-up forecast)", fontsize=10)
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
