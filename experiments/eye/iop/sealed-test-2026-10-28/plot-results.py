"""Plot the one-shot sealed IOP-after-surgery results from the public report only.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/iop/sealed-test-2026-10-28/plot-results.py

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
methods = [PRIMARY, "analog_knn", "pooled_curve_ratio", "persistence", "preoperative_level"]
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
    block = summary[f"{task}_group_means"]
    values = [block[m]["pooled_rmse_pp"] for m in methods]
    positions = [i + offset for i in range(len(methods))]
    ax.barh(
        positions,
        [min(v, 19.0) for v in values],
        height=width,
        color=colour,
        label=task.replace("_", " "),
    )
    for y, v in zip(positions, values, strict=True):
        ax.text(min(v, 19.0) + 0.2, y, f"{v:.2f}", va="center", fontsize=8)
ax.axvline(5.0, color="grey", linestyle="--", linewidth=1)
ax.set_xlim(0, 21.5)
ax.set_yticks(range(len(methods)), labels)
ax.invert_yaxis()
ax.set_xlabel("pooled RMSE (pp of preoperative IOP; bars clipped at 19)")
groups = summary["next_observation_group_means"][PRIMARY]["series"]
ax.set_title(f"Sealed test, group means: 25 studies, {groups} forecast series")
ax.legend(loc="upper right")

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
            if entry["kind"] != "group_mean":
                continue
            for row in entry["rows"]:
                if method in row["predictions"]:
                    xs.append(100 * row["observed"])
                    ys.append(100 * row["predictions"][method])
    ax.scatter(xs, ys, s=18, color=colour, alpha=0.75, label=f"{name} ({len(xs)} targets)")
ax.plot([20, 140], [20, 140], color="black", linewidth=1)
ax.fill_between([20, 140], [15, 135], [25, 145], color="green", alpha=0.08, label="±5 pp")
ax.set_xlim(20, 140)
ax.set_ylim(20, 140)
ax.set_xlabel("observed next follow-up (% of preoperative IOP)")
ax.set_ylabel("forecast (% of preoperative IOP)")
ax.set_title("Next observation, group means of public sealed studies")
ax.legend(loc="upper left", fontsize=8)


def series_panel(ax: plt.Axes, study_id: str, names: tuple[str, ...], title: str) -> None:
    study = studies[study_id]
    colours = ("black", "#d62728", "#2ca02c")
    chosen = [e for e in study["initial_state"] if e["series_id"].split("/")[1] in names]
    for index, initial in enumerate(chosen):
        colour = colours[index % len(colours)]
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
        ax.plot(
            targets,
            model,
            "s--",
            color=colour,
            alpha=0.6,
            label=f"model next follow-up, U2 ({name})",
        )
        ax.fill_between(targets, low, high, color=colour, alpha=0.08)
    ax.set_xscale("log")
    ax.set_xlabel("months after surgery (log scale; first point = day 1)")
    ax.set_ylabel("IOP (% of preoperative)")
    ax.set_title(f"{title}\n(shaded: 90% interval of the next-follow-up forecast)", fontsize=10)
    handles, labels_ = ax.get_legend_handles_labels()
    unique = dict(zip(labels_, handles, strict=True))
    ax.legend(unique.values(), unique.keys(), fontsize=7)


series_panel(
    axes[1, 0],
    "iop-pmc12109802",
    ("acg-phaco-gatt", "oag-solo-gatt"),
    "GATT: early pressure rise after day 1 (not modelled)",
)
series_panel(
    axes[1, 1],
    "iop-pmc5642877",
    ("canaloplasty", "phacocanaloplasty"),
    "Canaloplasty: very low day 1, then rebound",
)
fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150)
fig.savefig(HERE / "sealed-results.pdf")
print("wrote", HERE / "sealed-results.png")
