"""Plot the one-shot sealed results of retinal visual-acuity model 1.0 from public files.

Run with a Python that has Matplotlib (the repository's native analysis
environment), from the repository root:

    .peykar/cornea-native47/bin/python \
        experiments/eye/retina-acuity/model-1.0/sealed-test-2026-10-29/plot-results.py

Values come only from studies whose values are public in ``report-public.json``;
redacted (NC/ND or publisher-licensed) studies contribute to the pooled bars only
through the published summaries. Reported values are ``g = 1 + change / 2 logMAR``
(one pp = one ETDRS letter = 0.02 logMAR), so the acuity is ``logMAR0 + 2 (g - 1)``.
The upper-right panel draws the fitted population law at its mean parameters (a
published model parameter set, no study values) against the visit times the model
was trained on.
"""

from __future__ import annotations

import json
import math
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.lines import Line2D  # noqa: E402

HERE = Path(__file__).resolve().parent
report = json.loads((HERE / "report-public.json").read_text())
analysis = json.loads((HERE / "sealed-analysis.json").read_text())
PRIMARY = "population_assimilation_t4"
studies = {s["study_id"]: s for s in report["studies"]}
# Earliest visit of any development (training) series, public or private: 0.986
# months (30 days). Timing only; see the README of model 1.0 for the command.
EARLIEST_TRAINING_MONTH = 0.986
DAY_MONTHS = 12.0 / 365.25
# Panels chosen after unsealing from public (CC BY) control group means.
PANELS = [
    ("rac-pmc10368507", "DME, aflibercept: forecasts from the first visit hold"),
    (
        "rac-pmc11921743",
        "DME, dexamethasone implant: the gain wears off\n(the law has no wear-off for acuity)",
    ),
]


def _rmse(summary: dict, task: str, method: str) -> float:
    return summary[f"{task}_group_means"][method]["pooled_rmse_pp"]


fig, axes = plt.subplots(2, 2, figsize=(12.5, 10.5))
ax = axes[0, 0]
rows = [
    ("model 1.0", PRIMARY),
    ("persistence", "persistence"),
    ("pooled-curve increment", "pooled_curve_ratio"),
    ("analog kNN", "analog_knn"),
]
width = 0.38
largest = 0.0
summary = report["summary"]
for offset, task, colour in (
    (-width / 2, "next_observation", "#1f77b4"),
    (width / 2, "landmark", "#ff7f0e"),
):
    values = [_rmse(summary, task, m) for _, m in rows]
    positions = [i + offset for i in range(len(rows))]
    ax.barh(positions, values, height=width, color=colour, label=task.replace("_", " "))
    for y, v in zip(positions, values, strict=True):
        ax.text(v + 0.1, y, f"{v:.2f}", va="center", fontsize=8)
    largest = max(largest, *values)
ax.axvline(5.0, color="grey", linestyle="--", linewidth=1)
ax.set_xlim(0, largest * 1.18)
ax.set_yticks(range(len(rows)), [r[0] for r in rows])
ax.invert_yaxis()
ax.set_xlabel("pooled RMSE, group means (ETDRS letters; 5 letters = 0.10 logMAR)")
series = summary["next_observation_group_means"][PRIMARY]["series"]
initial = summary["initial_state"]["group_mean"]
coverage = analysis["H1_calibration_next_observation_group_means"]
ax.set_title(
    f"Sealed test: {series} group-mean series, all four checks fail\n"
    f"initial state {initial['population_initial_state']['pooled_rmse_pp']:.2f} vs pooled "
    f"training curve {initial['pooled_training_curve']['pooled_rmse_pp']:.2f} letters; "
    f"80% coverage {coverage['80']['coverage']:.2f}",
    fontsize=10,
)
ax.legend(loc="upper right")

ax = axes[0, 1]
fit = report["studies"][0]["population_fit"]
mean = fit["families"]["human/all"]["mean"]["parameters"]
t1 = fit["reference_days"] * DAY_MONTHS
tq = fit["transient_days"] * DAY_MONTHS
tau = math.exp(mean["log_tau_months"])
months = [t1 * (1.0 + i * 0.25) for i in range(0, 4)] + [0.05 * 1.08**i for i in range(0, 80)]
months = sorted(m for m in months if t1 <= m <= 12.0)


def letters(t: float, transient: bool = True) -> float:
    log_f = mean["level"] + mean["excess_at_t1"] * math.exp(-(t - t1) / tau)
    if transient:
        log_f += mean["transient_at_t1"] * math.exp(-(t - t1) / tq)
    return -50.0 * log_f  # letters gained = -(logMAR change) / 0.02


ax.plot(months, [letters(t) for t in months], color="#d62728", label="fitted law (mean)")
ax.plot(
    months,
    [letters(t, transient=False) for t in months],
    color="#1f77b4",
    linestyle="--",
    label="the same law without its early transient term",
)
ax.axvspan(EARLIEST_TRAINING_MONTH, 12.0, color="#2ca02c", alpha=0.08)
ax.text(
    1.1,
    -60,
    "every training visit\n(30 days to 12+ months)",
    fontsize=8,
    color="#2ca02c",
)
ax.axvline(t1, color="black", linewidth=1, linestyle=":")
ax.text(t1 * 1.05, -85, "day-1 visit of one\nsealed study", fontsize=8)
ax.axhline(0.0, color="grey", linewidth=0.8)
ax.set_xscale("log")
ax.set_xlim(t1 * 0.8, 12.0)
ax.set_xlabel("months after treatment (log scale)")
ax.set_ylabel("population mean acuity change (ETDRS letters; up is a gain)")
ax.set_title(
    "Root cause: the early transient was never constrained\n"
    f"(fitted transient +{mean['transient_at_t1']:.2f} logMAR at day 1, a "
    f"{50 * mean['transient_at_t1']:.0f}-letter loss;\n"
    "no training visit before day 30)",
    fontsize=10,
)
ax.legend(fontsize=8, loc="lower right")


def series_panel(ax: plt.Axes, study_id: str, title: str) -> None:
    colours = ("black", "#d62728", "#2ca02c", "#9467bd")
    study = studies[study_id]
    handles = []
    last_month = 0.0
    drawn: list[float] = []
    for index, initial_entry in enumerate(study["initial_state"]):
        colour = colours[index % len(colours)]
        logmar0 = initial_entry["reference_value"]
        tag = initial_entry["series_id"].split("/", 1)[1].rsplit("/", 1)[0]
        times = [0.0, *initial_entry["months"]]
        last_month = max(last_month, *times)
        observed = [logmar0, *(logmar0 + 2 * (g - 1) for g in initial_entry["observed"])]
        (line,) = ax.plot(
            times, observed, "o-", color=colour, label=f"{tag} (logMAR0 {logmar0:.2f})"
        )
        handles.append(line)
        prior = [
            logmar0 + 2 * (g - 1) for g in initial_entry["predictions"]["population_initial_state"]
        ]
        ax.plot(initial_entry["months"], prior, "^:", color=colour, alpha=0.7)
        drawn += observed + prior
        landmark = next(
            (e for e in study["landmark"] if e["series_id"] == initial_entry["series_id"]), None
        )
        if landmark is not None and landmark["rows"]:
            targets_ = [r["target_month"] for r in landmark["rows"]]
            ax.plot(
                targets_,
                [logmar0 + 2 * (r["predictions"][PRIMARY] - 1) for r in landmark["rows"]],
                "s--",
                color=colour,
                alpha=0.6,
            )
            low = [logmar0 + 2 * (r["intervals"][PRIMARY]["0.05"] - 1) for r in landmark["rows"]]
            high = [logmar0 + 2 * (r["intervals"][PRIMARY]["0.95"] - 1) for r in landmark["rows"]]
            if len(targets_) > 1:
                ax.fill_between(targets_, low, high, color=colour, alpha=0.10)
            else:  # one target: draw the interval as a bar
                ax.vlines(targets_, low, high, color=colour, alpha=0.4, linewidth=6)
            drawn += low + high
    ax.set_xlim(-0.3, last_month * 1.08)
    # Inverted logMAR axis with room below the curves for the legend.
    best, worst = min(drawn), max(drawn)
    ax.set_ylim(worst + 0.45 * (worst - best), best - 0.05 * (worst - best))
    ax.set_xlabel("months after the first treatment")
    ax.set_ylabel("best-corrected acuity (logMAR; up is better)")
    ax.set_title(f"{title}\n(shaded: 90% interval of the landmark forecast)", fontsize=10)
    handles += [
        Line2D([], [], color="grey", marker="o", linestyle="-", label="observed"),
        Line2D(
            [], [], color="grey", marker="^", linestyle=":", label="initial state (logMAR0 only)"
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
