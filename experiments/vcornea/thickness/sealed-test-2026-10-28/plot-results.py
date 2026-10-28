"""Plot the one-shot sealed corneal thickness results from the public report only.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/vcornea/thickness/sealed-test-2026-10-28/plot-results.py

Values come only from studies whose values are public in ``report-public.json``;
redacted (NC/ND/SA) studies contribute to the pooled bars through the published
summary only.
"""

from __future__ import annotations

import json
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

HERE = Path(__file__).resolve().parent
report = json.loads((HERE / "report-public.json").read_text())
summary = report["summary"]
studies = {s["study_id"]: s for s in report["studies"]}
PRIMARY = "population_assimilation_robust"
methods = [
    PRIMARY,
    "analog_knn",
    "pooled_curve_ratio",
    "persistence",
    "preoperative_level",
]
labels = [
    "model (robust prior)",
    "analog kNN",
    "pooled-curve ratio",
    "persistence",
    "preoperative level (f = 1)",
]

fig, axes = plt.subplots(2, 2, figsize=(12.5, 9.5))
ax = axes[0, 0]
width = 0.38
for offset, task, colour in (
    (-width / 2, "next_observation", "#1f77b4"),
    (width / 2, "landmark", "#ff7f0e"),
):
    values = [summary[task][m]["pooled_rmse_pp"] for m in methods]
    ax.barh(
        [i + offset for i in range(len(methods))],
        values,
        height=width,
        color=colour,
        label=task.replace("_", " "),
    )
ax.axvline(5.0, color="grey", linestyle="--", linewidth=1)
ax.set_yticks(range(len(methods)), labels)
ax.invert_yaxis()
ax.set_xlabel("pooled RMSE (pp of preoperative CCT)")
ax.set_title("Sealed test: 15 studies, 29 forecast series")
ax.legend(loc="lower right")

ax = axes[0, 1]
for method, colour, name in (
    ("persistence", "#999999", "persistence"),
    (PRIMARY, "#1f77b4", "model"),
):
    xs, ys = [], []
    for study in report["studies"]:
        if study.get("values_redacted"):
            continue
        for entry in study["next_observation"]:
            for row in entry["rows"]:
                if method in row["predictions"]:
                    xs.append(100 * row["observed"])
                    ys.append(100 * row["predictions"][method])
    ax.scatter(xs, ys, s=18, color=colour, alpha=0.75, label=f"{name} ({len(xs)} targets)")
ax.plot([75, 120], [75, 120], color="black", linewidth=1)
ax.fill_between([75, 120], [70, 115], [80, 125], color="green", alpha=0.08, label="±5 pp")
ax.set_xlim(75, 120)
ax.set_ylim(75, 120)
ax.set_xlabel("observed next follow-up (% of preoperative CCT)")
ax.set_ylabel("forecast (% of preoperative CCT)")
ax.set_title("Next observation, public sealed studies")
ax.legend(loc="upper left", fontsize=8)


def series_panel(ax: plt.Axes, study_id: str, title: str, xlabel: str) -> None:
    study = studies[study_id]
    for index, initial in enumerate(study["initial_state"]):
        colour = ("black", "#8c564b")[index % 2]
        months = initial["months"]
        name = initial["series_id"].split("/")[1]
        ax.plot(
            months,
            [100 * v for v in initial["observed"]],
            "o-",
            color=colour,
            label=f"observed ({name})",
        )
        ax.plot(
            months,
            [100 * v for v in initial["predictions"]["population_initial_state"]],
            ":",
            color=colour,
            label=f"model before any follow-up, U1 ({name})",
        )
        forecast = next(
            e for e in study["next_observation"] if e["series_id"] == initial["series_id"]
        )
        targets = [row["target_month"] for row in forecast["rows"]]
        model = [100 * row["predictions"][PRIMARY] for row in forecast["rows"]]
        low = [100 * row["intervals"][PRIMARY]["0.05"] for row in forecast["rows"]]
        high = [100 * row["intervals"][PRIMARY]["0.95"] for row in forecast["rows"]]
        ax.plot(targets, model, "s--", color="#1f77b4", label="model next follow-up, U2 (90% band)")
        ax.fill_between(targets, low, high, color="#1f77b4", alpha=0.12)
    ax.set_xlabel(xlabel)
    ax.set_ylabel("CCT (% of preoperative)")
    ax.set_title(title)
    handles, names = ax.get_legend_handles_labels()
    unique = dict(zip(names, handles, strict=True))
    ax.legend(unique.values(), unique.keys(), fontsize=8)


series_panel(
    axes[1, 0],
    "cct-pmc9156484",
    "DMEK, 10 years: slow late thickening (not modelled)",
    "months after DMEK",
)
series_panel(
    axes[1, 1],
    "cct-pmc8775002",
    "YAG capsulotomy ('other'): assigned to the graft family",
    "months after laser",
)
fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150)
fig.savefig(HERE / "sealed-results.pdf")
print("wrote", HERE / "sealed-results.png")
