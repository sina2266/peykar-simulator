"""Plot the one-shot sealed results of refraction model 1.0 from public files only.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/refraction/model-1.0/sealed-test-2026-10-29/plot-results.py

Values come only from studies whose values are public in ``report-public.json``;
redacted (NC/ND or publisher-licensed) studies contribute to the pooled bars and
coverage only through the published summaries. Reported values are ``g = 1 +
change / 10 D`` (one pp = 0.1 D), so the spherical equivalent is ``SE0 + 10 (g - 1)``.
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
reference = json.loads((HERE / "reference-f0-public.json").read_text())
analysis = json.loads((HERE / "sealed-analysis.json").read_text())
PRIMARY = "population_assimilation_robust"
studies = {s["study_id"]: s for s in report["studies"]}
# Panels chosen after unsealing from public (CC BY) control group means.
PANELS = [
    (
        "ref-pmc12668699",
        "LASIK by degree of myopia: after severe myopia the late regression\n"
        "falls outside the 90% landmark interval",
    ),
    ("ref-pmc5006606", "SMILE followed to 4 years: refraction stays near plano"),
]


def _pp(summary: dict, task: str, method: str) -> float:
    return summary[f"{task}_group_means"][method]["pooled_rmse_pp"] / 10


fig, axes = plt.subplots(2, 2, figsize=(12.5, 10.5))
ax = axes[0, 0]
rows = [
    ("model 1.0 (f1)", report["summary"], PRIMARY),
    ("f0 (same training)", reference["summary"], PRIMARY),
    ("persistence", report["summary"], "persistence"),
    ("plano target (full correction)", report["summary"], "target_level"),
    ("analog kNN", report["summary"], "analog_knn"),
]
width = 0.38
largest = 0.0
for offset, task, colour in (
    (-width / 2, "next_observation", "#1f77b4"),
    (width / 2, "landmark", "#ff7f0e"),
):
    values = [_pp(summary, task, m) for _, summary, m in rows]
    positions = [i + offset for i in range(len(rows))]
    ax.barh(positions, values, height=width, color=colour, label=task.replace("_", " "))
    for y, v in zip(positions, values, strict=True):
        ax.text(v + 0.01, y, f"{v:.2f}", va="center", fontsize=8)
    largest = max(largest, *values)
ax.axvline(0.5, color="grey", linestyle="--", linewidth=1)
ax.set_xlim(0, largest * 1.12)
ax.set_yticks(range(len(rows)), [r[0] for r in rows])
ax.invert_yaxis()
ax.set_xlabel("pooled RMSE, group means (D of spherical equivalent)")
series = report["summary"]["next_observation_group_means"][PRIMARY]["series"]
ax.set_title(f"Sealed test: {series} group-mean forecast series (dashed: 0.5 D)")
ax.legend(loc="upper right")

ax = axes[0, 1]
h4 = analysis["H4_versus_plano_target"]
h5 = analysis["H5_versus_f0_same_training"]["initial_state"]
labels = ["model 1.0 (f1)", "f0", "plano target"]
values = [
    h4["population_initial_state_rmse_pp"] / 10,
    h5["f0_rmse_pp"] / 10,
    h4["target_level_rmse_pp"] / 10,
]
bars = ax.bar(labels, values, color=["#1f77b4", "#999999", "#d62728"])
for bar, v in zip(bars, values, strict=True):
    ax.text(bar.get_x() + bar.get_width() / 2, v + 0.01, f"{v:.2f}", ha="center", fontsize=9)
coverage = analysis["H1_calibration_next_observation_group_means"]
ax.set_ylabel("pooled RMSE (D)")
ax.set_title(
    f"Initial state, {h4['common_targets']} control group-mean targets\n"
    f"next-observation coverage {coverage['80']['coverage']:.2f} (80%), "
    f"{coverage['90']['coverage']:.2f} (90%)",
    fontsize=10,
)


def series_panel(ax: plt.Axes, study_id: str, title: str) -> None:
    colours = ("black", "#d62728", "#2ca02c", "#9467bd")
    study = studies[study_id]
    handles = []
    last_month = 0.0
    for index, initial in enumerate(study["initial_state"]):
        colour = colours[index % len(colours)]
        se0 = initial["reference_value"]
        tag = initial["series_id"].split("/", 1)[1].removesuffix("/se")
        months = [0.0, *initial["months"]]
        last_month = max(last_month, *months)
        observed = [se0, *(se0 + 10 * (g - 1) for g in initial["observed"])]
        (line,) = ax.plot(months, observed, "o-", color=colour, label=f"{tag} (SE0 {se0:+.2f} D)")
        handles.append(line)
        prior = [se0 + 10 * (g - 1) for g in initial["predictions"]["population_initial_state"]]
        ax.plot(initial["months"], prior, "^:", color=colour, alpha=0.7)
        landmark = next(
            (e for e in study["landmark"] if e["series_id"] == initial["series_id"]), None
        )
        if landmark is not None and landmark["rows"]:
            targets = [r["target_month"] for r in landmark["rows"]]
            ax.plot(
                targets,
                [se0 + 10 * (r["predictions"][PRIMARY] - 1) for r in landmark["rows"]],
                "s--",
                color=colour,
                alpha=0.6,
            )
            low = [se0 + 10 * (r["intervals"][PRIMARY]["0.05"] - 1) for r in landmark["rows"]]
            high = [se0 + 10 * (r["intervals"][PRIMARY]["0.95"] - 1) for r in landmark["rows"]]
            if len(targets) > 1:
                ax.fill_between(targets, low, high, color=colour, alpha=0.10)
            else:  # one target: draw the interval as a bar
                ax.vlines(targets, low, high, color=colour, alpha=0.4, linewidth=6)
    ax.axhline(0.0, color="grey", linewidth=0.8)
    ax.set_xscale("symlog", linthresh=0.1)
    ax.set_xlim(-0.02, last_month * 1.4)
    ax.set_xlabel("months after surgery (log scale above 0.1 month)")
    ax.set_ylabel("spherical equivalent (D)")
    ax.set_title(f"{title}\n(shaded: 90% interval of the landmark forecast)", fontsize=10)
    handles += [
        Line2D([], [], color="grey", marker="o", linestyle="-", label="observed"),
        Line2D(
            [], [], color="grey", marker="^", linestyle=":", label="initial state (from SE0 only)"
        ),
        Line2D(
            [],
            [],
            color="grey",
            marker="s",
            linestyle="--",
            label="landmark forecast (from first visit)",
        ),
    ]
    ax.legend(handles=handles, fontsize=7, loc="lower right")


for ax, (study_id, title) in zip(axes[1], PANELS, strict=True):
    series_panel(ax, study_id, title)
fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150, bbox_inches="tight")
fig.savefig(HERE / "sealed-results.pdf", bbox_inches="tight")
print("wrote", HERE / "sealed-results.png", "verdicts", analysis["verdicts"])
