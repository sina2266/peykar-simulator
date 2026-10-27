"""Plot retained development and sealed-test metrics only (no model is run).

Inputs are the public reports beside this script. The Zeb1 2023 curve (CC BY 4.0,
Zhang et al., Communications Biology) is drawn from its unredacted public rows;
the CC BY-NC-ND LSC study contributes metrics only.
"""

import json
import math
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

base = Path(__file__).resolve().parent
sealed = json.loads((base / "report-public.json").read_text())
development = json.loads((base.parent / "development-loso-2026-10-27/report.json").read_text())
methods = [
    ("cohort_assimilation", "Cohort assimilation (primary)", "#147d83"),
    ("population_assimilation_t4", "Population prior, own prefix", "#3f8fd2"),
    ("analog_knn", "Nearest trajectories (ML)", "#6c64a7"),
    ("last_two_radius_trend", "Last-two radius trend", "#c57b32"),
    ("first_to_current_trend", "First-to-current trend", "#a2a83a"),
    ("crosson1986_fixed", "Crosson 1986 fixed", "#85939c"),
    ("persistence", "Persistence", "#b13c3c"),
]
fig, axes = plt.subplots(1, 3, figsize=(15, 5.6), gridspec_kw={"width_ratios": [1.25, 1.1, 1.2]})
fig.subplots_adjust(left=0.2, right=0.98, top=0.86, bottom=0.14, wspace=0.42)
y = list(range(len(methods)))
for offset, (report, label, alpha) in enumerate(
    [(development, "development (seen, LOSO)", 0.45), (sealed, "sealed test (once)", 1.0)]
):
    summary = report["summary"]["next_observation"]
    shares = [
        100 * summary[m]["series_meeting_criteria"] / summary[m]["series"] if m in summary else 0
        for m, _, _ in methods
    ]
    axes[0].barh(
        [v + 0.38 * offset - 0.19 for v in y],
        shares,
        height=0.36,
        color=[c for _, _, c in methods],
        alpha=alpha,
        label=label,
    )
    rmse = [summary[m]["pooled_rmse_pp"] if m in summary else math.nan for m, _, _ in methods]
    axes[1].scatter(
        rmse, y, s=60, color=[c for _, _, c in methods], alpha=alpha, marker="os"[offset]
    )
axes[0].set_yticks(y, [name for _, name, _ in methods])
axes[0].invert_yaxis()
axes[0].set_xlabel("Series meeting 5/10 pp + persistence gain (%)")
axes[0].legend(frameon=False, fontsize=8, loc="lower right")
axes[1].set_yticks(y, [])
axes[1].invert_yaxis()
axes[1].axvline(5, color="#b13c3c", linestyle="--", linewidth=1)
axes[1].set_xlabel("Pooled next-observation RMSE (pp)\ncircle = development, square = sealed")
zeb1 = next(s for s in sealed["studies"] if s["study_id"] == "zeb1-commun-biol-2023")
target = zeb1["initial_state"][0]
hours = [0.0, *target["hours"]]
observed = [100.0, *[100 * v for v in target["observed"]]]
axes[2].plot(hours, observed, "ko", label="Observed Zeb1 wt mean (8 eyes)")
for key, label, color in [
    ("population_initial_state", "Population model, initial state only", "#147d83"),
    ("crosson1986_fixed", "Crosson 1986 fixed", "#85939c"),
    ("pooled_training_curve", "Pooled training curve", "#c57b32"),
]:
    axes[2].plot(
        hours,
        [100.0, *[100 * v for v in target["predictions"][key]]],
        "-",
        color=color,
        label=label,
    )
axes[2].set_xlabel("Hours after 2 mm debridement (mouse)")
axes[2].set_ylabel("Open wound area (% of hour 0)")
axes[2].legend(frameon=False, fontsize=8)
lsc = next(s for s in sealed["studies"] if s["study_id"].startswith("lsc"))
lsc_score = lsc["initial_state"][0]["scores"]["population_initial_state"]
axes[2].set_title(
    "Sealed initial-state check (Zeb1 2023 controls)\n"
    f"LSC 2026 native controls: RMSE {lsc_score['rmse_pp']:.1f} pp, "
    f"max {lsc_score['max_abs_pp']:.1f} pp (metrics only)",
    fontsize=8,
)
for ax in axes:
    ax.spines[["top", "right"]].set_visible(False)
fig.suptitle(
    "Corneal wound-closure population model: development vs one-shot sealed test", fontsize=12
)
fig.savefig(base / "sealed-results.png", dpi=160)
fig.savefig(base / "sealed-results.pdf")
