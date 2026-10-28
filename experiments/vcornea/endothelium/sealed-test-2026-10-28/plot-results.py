"""Plot the one-shot sealed endothelium results from the public report only.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/vcornea/endothelium/sealed-test-2026-10-28/plot-results.py

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
methods = [
    "population_anchored_t4",
    "analog_knn",
    "pooled_curve_ratio",
    "persistence",
    "last_two_log_linear",
]
labels = [
    "model (anchored, t4)",
    "analog kNN",
    "pooled-curve ratio",
    "persistence",
    "last-two trend",
]

fig, axes = plt.subplots(1, 3, figsize=(15, 4.8))
ax = axes[0]
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
ax.set_xlabel("pooled RMSE (pp of reference ECD)")
ax.set_title("Sealed test: 17 studies, 30 series")
ax.legend(loc="upper right")

ax = axes[1]
for method, colour, name in (
    ("persistence", "#999999", "persistence"),
    ("population_anchored_t4", "#1f77b4", "model"),
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
ax.plot([20, 105], [20, 105], color="black", linewidth=1)
ax.fill_between([20, 105], [15, 100], [25, 110], color="green", alpha=0.08, label="±5 pp")
ax.set_xlim(20, 105)
ax.set_ylim(20, 105)
ax.set_xlabel("observed next follow-up (% of reference)")
ax.set_ylabel("forecast (% of reference)")
ax.set_title("Next observation, public sealed studies")
ax.legend(loc="upper left", fontsize=8)

ax = axes[2]
study = next(s for s in report["studies"] if s["study_id"] == "ecd-pmc9156484")
entry = study["next_observation"][0]
months = [entry["rows"][0]["origin_month"]] + [row["target_month"] for row in entry["rows"]]
first = study["landmark"][0]["rows"][0]["predictions"]["persistence"]
observed = [first] + [row["observed"] for row in entry["rows"]]
ax.plot(months, [100 * v for v in observed], "o-", color="black", label="observed (group mean)")
targets = [row["target_month"] for row in entry["rows"]]
model = [100 * row["predictions"]["population_anchored_t4"] for row in entry["rows"]]
low = [100 * row["intervals"]["population_anchored_t4"]["0.05"] for row in entry["rows"]]
high = [100 * row["intervals"]["population_anchored_t4"]["0.95"] for row in entry["rows"]]
ax.plot(targets, model, "s--", color="#1f77b4", label="model next-follow-up forecast")
ax.fill_between(targets, low, high, color="#1f77b4", alpha=0.15, label="90% interval")
ax.set_xlabel("months after DMEK")
ax.set_ylabel("ECD (% of donor count)")
ax.set_title("Worst public series: late drop after 60 months")
ax.legend(fontsize=8)
fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150)
fig.savefig(HERE / "sealed-results.pdf")
print("wrote", HERE / "sealed-results.png")
