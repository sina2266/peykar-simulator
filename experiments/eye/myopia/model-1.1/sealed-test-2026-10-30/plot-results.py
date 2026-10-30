"""Plot the one-shot sealed results of the myopia axial-length model 1.1 from public files
only.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/myopia/model-1.1/sealed-test-2026-10-30/plot-results.py

Every panel uses published per-series metrics only (RMSE, target counts and interval
coverage counts), which redacted (NC/ND/SA) studies keep without values; no axial-length
value is plotted. ``reference-1.0.0-public.json`` is model 1.0.0 fitted on the same
training records in the same sealed run. One pp is 0.01 mm.
"""

from __future__ import annotations

import json
import math
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

HERE = Path(__file__).resolve().parent
report = json.loads((HERE / "report-public.json").read_text())
reference = json.loads((HERE / "reference-1.0.0-public.json").read_text())
analysis = json.loads((HERE / "sealed-analysis.json").read_text())
PRIMARY = "population_assimilation"
INITIAL = "population_initial_state"
verdicts = analysis["verdicts"]
# The study that carries the landmark loss against 1.0.0 (post hoc breakdown).
H5_STUDY = "myo-pmc11253518"


def _group_means(source: dict, task: str) -> list[dict]:
    return [
        {**entry, "study_id": study["study_id"]}
        for study in source["studies"]
        for entry in study[task]
        if entry["context"].startswith("human/") and entry["kind"] == "group_mean"
    ]


def _pooled(entries: list[dict], method: str) -> float:
    scored = [e for e in entries if method in e["scores"] and e["scores"][method]["n"]]
    total = sum(e["scores"][method]["n"] for e in scored)
    squares = sum(e["scores"][method]["n"] * e["scores"][method]["rmse_pp"] ** 2 for e in scored)
    return math.sqrt(squares / total)


def _coverage(entries: list[dict], level: str) -> float:
    inside = sum(e["interval_coverage"][PRIMARY][level]["inside"] for e in entries)
    total = sum(e["interval_coverage"][PRIMARY][level]["n"] for e in entries)
    return inside / total


fig, axes = plt.subplots(2, 2, figsize=(12.5, 11.0))

ax = axes[0, 0]
rows = [
    ("model 1.1 (Gaussian primary)", report, PRIMARY),
    ("model 1.0.0, same training", reference, PRIMARY),
    ("pooled-curve increment", report, "pooled_curve_ratio"),
    ("analog kNN", report, "analog_knn"),
    ("last-two trend", report, "last_two_log_linear"),
    ("persistence", report, "persistence"),
]
width = 0.38
largest = 0.0
for offset, task, colour in (
    (-width / 2, "next_observation", "#1f77b4"),
    (width / 2, "landmark", "#ff7f0e"),
):
    values = [_pooled(_group_means(source, task), method) for _, source, method in rows]
    positions = [i + offset for i in range(len(rows))]
    ax.barh(positions, values, height=width, color=colour, label=task.replace("_", " "))
    for y, v in zip(positions, values, strict=True):
        ax.text(v + 0.2, y, f"{v:.2f}", va="center", fontsize=8)
    largest = max(largest, *values)
ax.axvline(5.0, color="grey", linestyle="--", linewidth=1)
ax.set_xlim(0, largest * 1.15)
ax.set_yticks(range(len(rows)), [r[0] for r in rows])
ax.invert_yaxis()
ax.set_xlabel("pooled RMSE, group means (pp = 0.01 mm of axial-length change)")
ax.set_title(
    f"Sealed test, {len(_group_means(report, 'next_observation'))} group-mean forecast series: "
    "H2 and H3 pass;\nH5 fails on the landmark task (model 1.1 above model 1.0.0 by more "
    "than 0.05 pp)",
    fontsize=10,
)
ax.legend(loc="lower right")

ax = axes[0, 1]
windows = {"80": (0.70, 0.90), "90": (0.80, 0.97)}
for i, (level, (low, high)) in enumerate(windows.items()):
    ax.add_patch(
        plt.Rectangle((i - 0.4, low), 0.8, high - low, color="#2ca02c", alpha=0.15, zorder=0)
    )
    for shift, source, colour, label in (
        (-0.12, report, "#1f77b4", "model 1.1"),
        (0.12, reference, "#7f7f7f", "model 1.0.0, same training"),
    ):
        value = _coverage(_group_means(source, "next_observation"), level)
        ax.plot(
            [i + shift], [value], "o", color=colour, markersize=9, label=label if i == 0 else None
        )
        ax.text(i + shift + 0.05, value, f"{value:.3f}", va="center", fontsize=8)
ax.set_xticks([0, 1], ["80% interval\n(window 0.70–0.90)", "90% interval\n(window 0.80–0.97)"])
ax.set_xlim(-0.6, 1.6)
ax.set_ylim(0.6, 1.02)
ax.set_ylabel("next-observation coverage, group means")
ax.set_title(
    "H1 fails: the intervals are too wide (both coverages above the upper limits;\n"
    "model 1.0.0's too); the green bands are the declared windows",
    fontsize=10,
)
ax.legend(loc="lower left", fontsize=8)

ax = axes[1, 0]
by_context: dict[str, list[dict]] = defaultdict(list)
for entry in _group_means(report, "initial_state"):
    by_context[entry["context"].split("/")[1]].append(entry)
order = sorted(by_context, key=lambda c: -sum(e["scores"][INITIAL]["n"] for e in by_context[c]))
labels, model_values, curve_values = [], [], []
for context in order:
    entries = by_context[context]
    targets = sum(e["scores"][INITIAL]["n"] for e in entries)
    groups = f"{len(entries)} group" + ("s" if len(entries) > 1 else "")
    labels.append(
        f"{context.replace('_', ' ')}\n({groups}, {targets} target{'s' if targets > 1 else ''})"
    )
    model_values.append(_pooled(entries, INITIAL))
    curve_values.append(_pooled(entries, "pooled_training_curve"))
positions = range(len(labels))
ax.barh(
    [p - width / 2 for p in positions],
    curve_values,
    height=width,
    color="#999999",
    label="pooled training curve",
)
ax.barh(
    [p + width / 2 for p in positions],
    model_values,
    height=width,
    color="#1f77b4",
    label="model 1.1 initial state",
)
for p, a, b in zip(positions, curve_values, model_values, strict=True):
    ax.text(a + 0.3, p - width / 2, f"{a:.1f}", va="center", fontsize=7)
    ax.text(b + 0.3, p + width / 2, f"{b:.1f}", va="center", fontsize=7)
ax.set_yticks(list(positions), labels, fontsize=7)
ax.invert_yaxis()
ax.set_xlabel("initial-state RMSE, control group means (pp = 0.01 mm)")
ax.set_xlim(0, max(*model_values, *curve_values) * 1.15)
model_all = _pooled(_group_means(report, "initial_state"), INITIAL)
curve_all = _pooled(_group_means(report, "initial_state"), "pooled_training_curve")
ax.set_title(
    f"H4 fails by {model_all - curve_all:.2f} pp ({model_all:.2f} against the pooled curve's "
    f"{curve_all:.2f});\nsingle vision is worse than the pooled curve",
    fontsize=10,
)
ax.legend(loc="lower right", fontsize=8)

ax = axes[1, 1]
change: dict[str, float] = defaultdict(float)
for source, sign in ((report, 1.0), (reference, -1.0)):
    for entry in _group_means(source, "landmark"):
        score = entry["scores"].get(PRIMARY)
        if score and score["n"]:
            change[entry["study_id"]] += sign * score["n"] * score["rmse_pp"] ** 2
ranked = sorted(change.items(), key=lambda kv: kv[1])
colours = ["#d62728" if k == H5_STUDY else "#1f77b4" for k, _ in ranked]
ax.barh(range(len(ranked)), [v for _, v in ranked], color=colours)
ax.set_yticks(range(len(ranked)), [k.removeprefix("myo-pmc") for k, _ in ranked], fontsize=7)
ax.axvline(0.0, color="black", linewidth=0.8)
ax.set_xlabel("landmark squared error per study, 1.1 minus 1.0.0 (pp²)")
ax.set_title(
    f"H5: one orthokeratology study ({H5_STUDY.removeprefix('myo-pmc')}, four groups seen at 1, 3\n"
    "and 6 months) carries the landmark loss; without it 1.1 is lower",
    fontsize=10,
)

fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150, bbox_inches="tight")
fig.savefig(HERE / "sealed-results.pdf", bbox_inches="tight")
print("wrote", HERE / "sealed-results.png", "verdicts", verdicts)
