"""Plot retained metrics of the 2.0.0 sealed test (no model is run).

Inputs are the public reports beside this script and the 2026-10-28 development
analysis. Both sealed studies are CC BY 4.0: IL-10 mice (Cell Reports 2026,
Mendeley Data 10.17632/kwhys2zfwd.1) and i-PRF patients (Arq Bras Oftalmol 2025).
"""

import json
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

base = Path(__file__).resolve().parent
sealed = json.loads((base / "report-public.json").read_text())
analysis = json.loads((base / "primary-analysis.json").read_text())
development = json.loads(
    (base.parent / "development-loso-2026-10-28/development-analysis.json").read_text()
)

fig, axes = plt.subplots(1, 3, figsize=(15.5, 5.4), gridspec_kw={"width_ratios": [1.0, 1.15, 1.15]})
fig.subplots_adjust(left=0.06, right=0.98, top=0.84, bottom=0.14, wspace=0.32)

# Panel 1: cohort interval coverage, development vs sealed, both versions.
labels = ["80% interval", "90% interval"]
x = [0, 1]
for shift, (name, source, version, colour, marker) in enumerate(
    [
        ("2.0.0 development", development, "H1_calibration_v2_cohort", "#147d83", "o"),
        ("1.0.0 development", development["H3_v2_vs_v1_cohort"], "v1_calibration", "#c57b32", "o"),
        ("2.0.0 sealed", analysis, "H1_calibration_v2_cohort", "#147d83", "s"),
        ("1.0.0 sealed", analysis["H3_v2_vs_v1_cohort"], "v1_calibration", "#c57b32", "s"),
    ]
):
    values = [100 * source[version][level]["coverage"] for level in ("80", "90")]
    filled = "full" if "sealed" in name else "none"
    axes[0].plot(
        [v + 0.08 * (shift - 1.5) for v in x],
        values,
        marker=marker,
        linestyle="",
        color=colour,
        fillstyle=filled,
        markersize=9,
        label=name,
    )
for xi, (low, high) in zip(x, [(70, 90), (80, 97)], strict=True):
    axes[0].fill_between([xi - 0.3, xi + 0.3], low, high, color="#dddddd", zorder=0)
axes[0].set_xticks(x, labels)
axes[0].set_ylim(60, 100)
axes[0].set_ylabel("Observed coverage (%)")
axes[0].set_title("Cohort-assimilation intervals\n(grey = frozen acceptance window)", fontsize=9)
axes[0].legend(frameon=False, fontsize=8, loc="lower right")

# Panel 2: IL-10 controls (mouse, 2.5 mm), initial-state predictions.
for ax, study_id, title, xlabel in [
    (
        axes[1],
        "il10-c-mast-cell-rep-2026",
        "IL-10 study: PBS control mice (14), initial state only",
        "Hours after 2.5 mm debridement (mouse)",
    ),
    (
        axes[2],
        "iprf-cxl-human-arq-bras-oftalmol-2025",
        "i-PRF study: standard-care patients (25), initial state only",
        "Hours after 8 mm epithelial removal + cross-linking (human)",
    ),
]:
    study = next(s for s in sealed["version2"]["studies"] if s["study_id"] == study_id)
    target = study["initial_state"][0]
    v1 = next(s for s in sealed["version1_reference"]["studies"] if s["study_id"] == study_id)
    hours = [0.0, *target["hours"]]
    ax.plot(hours, [100.0, *[100 * v for v in target["observed"]]], "ko", label="Observed mean")
    for key, label, colour, style in [
        ("population_initial_state", "Model 2.0.0", "#147d83", "-"),
        ("crosson1986_fixed", "Crosson 1986 fixed", "#85939c", "-"),
        ("pooled_training_curve", "Pooled training curve", "#c57b32", ":"),
    ]:
        ax.plot(
            hours,
            [100.0, *[100 * v for v in target["predictions"][key]]],
            style,
            color=colour,
            label=label,
        )
    ax.plot(
        hours,
        [
            100.0,
            *[100 * v for v in v1["initial_state"][0]["predictions"]["population_initial_state"]],
        ],
        "--",
        color="#147d83",
        alpha=0.6,
        label="Model 1.0.0",
    )
    score = target["scores"]["population_initial_state"]
    ax.set_title(
        f"{title}\nmodel 2.0.0 RMSE {score['rmse_pp']:.1f} pp, max {score['max_abs_pp']:.1f} pp",
        fontsize=9,
    )
    ax.set_xlabel(xlabel)
    ax.set_ylabel("Open wound area (% of hour 0)")
    ax.set_ylim(-5, 105)
    ax.legend(frameon=False, fontsize=8)
for ax in axes:
    ax.spines[["top", "right"]].set_visible(False)
fig.suptitle(
    "Corneal closure-tail model 2.0.0: one-shot sealed test (IL-10 mice, i-PRF patients)",
    fontsize=12,
)
fig.savefig(base / "sealed-results.png", dpi=160)
fig.savefig(base / "sealed-results.pdf")
