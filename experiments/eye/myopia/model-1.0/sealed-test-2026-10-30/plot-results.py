"""Plot the one-shot sealed results of the myopia axial-length model 1.0 from public files
only.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/myopia/model-1.0/sealed-test-2026-10-30/plot-results.py

Values come only from studies whose values are public in ``report-public.json``.
Redacted (NC/ND/SA) studies enter the pooled bars only through their published
per-series RMSE and target counts, which carry no values. One pp is 0.01 mm.
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
analysis = json.loads((HERE / "sealed-analysis.json").read_text())
PRIMARY = "population_assimilation"
INITIAL = "population_initial_state"
studies = {s["study_id"]: s for s in report["studies"]}
redacted = set(analysis["studies_with_redacted_values"])
verdicts = analysis["verdicts"]

# Panels chosen after unsealing from public (CC BY) group means.
LONG_TRIAL = "myo-pmc6990884"
COHORT = "myo-pmc7447800"
# The article's group letters (A orthokeratology, B atropine 0.01%, C atropine 0.05%).
COHORT_GROUPS = {
    "orthokeratology-group-a": "orthokeratology, A",
    "atropine-group-b": "atropine 0.01%, B",
    "atropine-group-c": "atropine 0.05%, C",
}


def _group_means(task: str) -> list[dict]:
    return [
        entry
        for study in report["studies"]
        for entry in study[task]
        if entry["context"].startswith("human/") and entry["kind"] == "group_mean"
    ]


def _pooled(entries: list[dict], method: str) -> float:
    scored = [e for e in entries if method in e["scores"] and e["scores"][method]["n"]]
    total = sum(e["scores"][method]["n"] for e in scored)
    squares = sum(e["scores"][method]["n"] * e["scores"][method]["rmse_pp"] ** 2 for e in scored)
    return math.sqrt(squares / total)


def _mm(fractions: list[float]) -> list[float]:
    # Reported values are 1 + change / (100 x 0.01 mm).
    return [v - 1.0 for v in fractions]


def _public(study_id: str) -> dict:
    if study_id in redacted:
        raise SystemExit(f"{study_id} is redacted and cannot be plotted")
    return studies[study_id]


fig, axes = plt.subplots(2, 2, figsize=(12.5, 11.0))

ax = axes[0, 0]
rows = [
    ("model 1.0 (Gaussian primary)", PRIMARY),
    ("pooled-curve increment", "pooled_curve_ratio"),
    ("analog kNN", "analog_knn"),
    ("last-two trend", "last_two_log_linear"),
    ("persistence", "persistence"),
]
width = 0.38
largest = 0.0
for offset, task, colour in (
    (-width / 2, "next_observation", "#1f77b4"),
    (width / 2, "landmark", "#ff7f0e"),
):
    values = [_pooled(_group_means(task), method) for _, method in rows]
    positions = [i + offset for i in range(len(rows))]
    ax.barh(positions, values, height=width, color=colour, label=task.replace("_", " "))
    for y, v in zip(positions, values, strict=True):
        ax.text(v + 0.3, y, f"{v:.1f}", va="center", fontsize=8)
    largest = max(largest, *values)
ax.axvline(5.0, color="grey", linestyle="--", linewidth=1)
ax.set_xlim(0, largest * 1.12)
ax.set_yticks(range(len(rows)), [r[0] for r in rows])
ax.invert_yaxis()
ax.set_xlabel("pooled RMSE, group means (pp = 0.01 mm of axial-length change)")
failed = [k.split("_")[0] for k, v in verdicts.items() if not v]
ax.set_title(
    f"Sealed test, {len(_group_means('next_observation'))} group-mean forecast series:\n"
    f"{', '.join(failed)} fail (pooled-curve increment and kNN beat the model)",
    fontsize=10,
)
ax.legend(loc="upper right")

ax = axes[0, 1]
by_context: dict[str, list[dict]] = defaultdict(list)
for entry in _group_means("initial_state"):
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
    label="model 1.0 initial state",
)
for p, a, b in zip(positions, curve_values, model_values, strict=True):
    ax.text(a + 0.4, p - width / 2, f"{a:.1f}", va="center", fontsize=7)
    ax.text(b + 0.4, p + width / 2, f"{b:.1f}", va="center", fontsize=7)
ax.set_yticks(list(positions), labels, fontsize=7)
ax.invert_yaxis()
ax.set_xlabel("initial-state RMSE, control group means (pp = 0.01 mm)")
ax.set_xlim(0, max(*model_values, *curve_values) * 1.15)
ax.set_title(
    "Initial state by treatment: better than the pooled curve for most treatments,\n"
    "worse for single vision and atropine (H4 fails)",
    fontsize=10,
)
ax.legend(loc="lower right", fontsize=8)

ax = axes[1, 0]
trial = _public(LONG_TRIAL)
for entry, colour in zip(trial["initial_state"], ("#d62728", "black"), strict=False):
    name = entry["series_id"].split("/")[1]
    months = entry["months"]
    ax.plot(
        [0, *months], [0, *_mm(entry["observed"])], "o-", color=colour, label=f"observed ({name})"
    )
    ax.plot(
        [0, *months],
        [0, *_mm(entry["predictions"][INITIAL])],
        "s--",
        color=colour,
        alpha=0.7,
        label=f"model initial state ({name})",
    )
    ax.plot(
        [0, *months],
        [0, *_mm(entry["predictions"]["pooled_training_curve"])],
        "^:",
        color=colour,
        alpha=0.6,
        label=f"pooled training curve ({name})",
    )
ax.set_xlabel("months from baseline")
ax.set_ylabel("axial-length change (mm)")
ax.set_title(
    f"A 4-year trial ({LONG_TRIAL.removeprefix('myo-pmc')}): the control eyes grew 0.42 mm a year\n"
    "throughout; the law levels off and few training visits lie after 2 years",
    fontsize=10,
)
ax.legend(fontsize=7, loc="upper left")

ax = axes[1, 1]
cohort = _public(COHORT)
colours = ("#2ca02c", "#9467bd", "#8c564b")
for entry, colour in zip(cohort["next_observation"], colours, strict=False):
    name = COHORT_GROUPS[entry["series_id"].split("/")[1]]
    whole = next(e for e in cohort["initial_state"] if e["series_id"] == entry["series_id"])
    ax.plot(
        [0.0, *whole["months"]],
        [0.0, *_mm(whole["observed"])],
        "o-",
        color=colour,
        label=f"observed ({name})",
    )
    targets = [r["target_month"] for r in entry["rows"]]
    point = [r["predictions"][PRIMARY] - 1.0 for r in entry["rows"]]
    low = [r["intervals"][PRIMARY]["0.1"] - 1.0 for r in entry["rows"]]
    high = [r["intervals"][PRIMARY]["0.9"] - 1.0 for r in entry["rows"]]
    ax.errorbar(
        targets,
        point,
        yerr=[
            [p - lo for p, lo in zip(point, low, strict=True)],
            [hi - p for p, hi in zip(point, high, strict=True)],
        ],
        fmt="s",
        color=colour,
        alpha=0.6,
        capsize=3,
        label=f"next-visit forecast, 80% interval ({name})",
    )
ax.set_xlabel("months from baseline")
ax.set_ylabel("axial-length change (mm)")
ax.set_title(
    f"A retrospective anisomyopia cohort ({COHORT.removeprefix('myo-pmc')}; 18 and 13 of 36\n"
    "atropine children left at month 24): second-year jumps outside the intervals",
    fontsize=10,
)
ax.legend(fontsize=7, loc="upper left")

fig.tight_layout()
fig.savefig(HERE / "sealed-results.png", dpi=150, bbox_inches="tight")
fig.savefig(HERE / "sealed-results.pdf", bbox_inches="tight")
print("wrote", HERE / "sealed-results.png", "verdicts", verdicts)
