# Retinal visual-acuity model 1.0 — development log

Development data: the 15 development records (52 series) of dataset
[`retina-acuity-2026-10-29`](../../../data/retina-acuity-2026-10-29/README.md): 13 CC BY
records in `development/records` and 2 non-commercial records kept private in
`.peykar/retina-acuity-development-private/records`. Twenty-six of the 41 reserved
development studies have no record (20 complete-case sets, 5 whose acuity is not
established as best-corrected logMAR or letters, 1 selected after baseline). The 52
series are 24 group means and 28 per-eye series (one study of a subretinal graft,
treated); 20 series are control group means in 11 studies: diabetic macular edema
with anti-VEGF 5, steroid implant 3 and steroid injection 2; vein occlusion with
anti-VEGF 4, steroid implant 2 and anti-VEGF plus laser 2; uveitic edema with a
steroid implant 1; neovascular AMD with anti-VEGF 1. No sealed value is read before
the 1.0 protocol is frozen. Studies of one author-overlap cluster
(`author-overlap-1.0.0.json`, 2 development clusters) leave training together in
leave-one-study-out (LOSO). Each round declares its change and its adoption rule
before its LOSO results are seen.

## Model

Generic eye model version 2 (`eye_recovery_population2`, unchanged) with the
configurations of `peykar.model_learning.retina_acuity_population`; series
`peykar.validation.eye_retina_acuity_data`; scoring
`peykar.validation.eye_additive_evaluation2` (version-2 configurations go to the harness
of the corneal acuity component, unchanged); operator
`scripts/evaluate_retina_acuity_population.py`.

The model is that of the corneal acuity component: the change of best-corrected acuity
from its pretreatment value in logMAR, `change(t)/1 logMAR = ell + e r(t) + s q(t)`
with `r = exp(-(t - 1 day)/tau)`, `q = exp(-(t - 1 day)/7 days)` and
`ell = context level + b logMAR0`, given to the generic model as the exact
pseudo-fraction `exp(change/1 logMAR)` with logMAR0 as `exp(logMAR0)`. Scores are in
ETDRS letters (0.02 logMAR): the per-series criteria read RMSE ≤ 5 letters, maximum
≤ 10 letters and, for forecasts, at least 0.5 letter below persistence. Contexts are
`human/<indication>/<treatment>`.

## Round 1 — declared before results (2026-10-29)

- **v0**: one level per context, grouping `("context",)`, a study effect per context
  arm, the generic constants otherwise (the corneal acuity 1.0.0 settings). A context
  without training studies falls back to the species level.
- **v1**: v0 with contexts nested in their treatment, grouping
  `("treatment", "context")`: an indication of a known treatment borrows that
  treatment's level, amplitude and time constant. With 11 control studies spread over 8
  contexts, most contexts have one or two training studies, and a treatment level may
  pool them better than the species level alone.

Primary method of a candidate: among the population variants whose next-observation
coverage of group means lies inside both H1 windows ([0.70, 0.90] at 80%, [0.80, 0.97]
at 90%), the lowest pooled next-observation RMSE; a variant within 0.05 letters of that
minimum that comes earlier in the order Gaussian, t4, robust, anchored, anchored t4 is
preferred (the earliest one). If no variant lies inside both windows, the robust
variant is the primary and the miss is recorded. The same variant is the primary of the
landmark task; the initial state uses `population_initial_state`.

Adoption rule: v1 replaces v0 only if it improves the pooled RMSE of its primary by at
least 0.10 letters in at least one task (next observation, landmark, initial state) and
loses no more than 0.05 letters in any. Otherwise v0 is model 1.0.0. The protocol keeps
the H1–H4 hypotheses and comparators of the other eye components (persistence, last-two
trend, pooled-curve increment and analog kNN for the forecasts; the pooled training
curve for the initial state), whatever development shows.

## Round 1 — results (2026-10-29)

LOSO over the 15 studies (`.peykar/retina-acuity-1.0-dev/round1-v0`, `round1-v1`; 69
next-observation and 69 landmark targets and 83 initial-state targets of group means;
`.peykar/acuity_compare.py`, RMSE in ETDRS letters). **No population variant of either
candidate lies inside both H1 windows**: next-observation coverage of the 80% intervals
is 0.59–0.68 (the 90% intervals 0.74–0.83), so the intervals are too narrow. By the
declared rule the primary of both is the robust variant, and the miss is recorded.

| method | next observation | landmark | initial state |
|---|---|---|---|
| v0 robust | 2.778 (cov 0.623/0.768) | 3.066 | 4.737 |
| v1 robust | 2.772 (cov 0.609/0.783) | 3.031 | 4.763 |
| persistence | 2.654 | 3.148 | — |
| pooled-curve increment | 2.858 | 3.392 | — |
| analog kNN | 3.528 | 4.970 | — |
| pooled training curve | — | — | 5.683 |

**v1 rejected**: gains of 0.006 and 0.035 letters and a loss of 0.026 letters in the
initial state; no gain reaches 0.10. v0 stays the incumbent. Two problems remain: the
intervals are too narrow, and persistence beats the model for the next observation
(2.65 against 2.78 letters). Group means of acuity after these treatments move little
between visits, and a group that is above or below the law at one visit stays there.

## Round 2 — declared before results (2026-10-29)

- **v2**: v0 with persistent (Ornstein–Uhlenbeck) deviations from the law in
  forecasting only, the setting of the retinal-thickness component 1.0 and of corneal
  acuity a4: 80% of the residual variance as a deviation process with a 12-month
  correlation time; the population is fitted as in v0. A group above or below the law
  at its last visit is then expected to stay so for months, and the intervals carry
  that uncertainty.
- **v3**: v0 with forecast intervals from the pooled study noise
  (`forecast_noise_pooling = "pooled"`, the retinal-thickness 1.1 setting), the
  likelihood keeping the median noise centre.

Adoption rule against v0 (robust: 2.778 / 3.066 / 4.737 letters, outside the H1
windows): a candidate is adopted if (a) it meets the round-1 RMSE rule (a gain of at
least 0.10 letters in one task, no loss above 0.05 letters in any), or (b) its primary
lies inside both H1 windows, which v0 misses, while it loses no more than 0.05 letters
in any task. The primary of each candidate is chosen by the round-1 rule. If both are
adopted, the one with the larger summed gain over the three tasks is model 1.0.0
(ties within 0.05 letters: v2, the setting of the anatomical counterpart). If neither
is adopted, v0 is model 1.0.0 and its H1 miss on development is recorded with the
protocol.

## Round 2 — results (2026-10-29)

LOSO over the same 15 studies (`.peykar/retina-acuity-1.0-dev/round2-v2`, `round2-v3`):

| candidate | primary (rule) | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|---|
| v0 | robust (no eligible variant) | 2.778 | 3.066 | 4.737 | 0.623 / 0.768 |
| v2 (forecast-only deviations) | robust (no eligible variant) | 2.737 | 3.066 | 4.737 | 0.551 / 0.667 |
| v3 (pooled forecast noise) | t4 | 2.794 | 3.047 | 4.742 | 0.841 / 0.884 |

- **v2 rejected**: its gains are 0.041, 0.000 and 0.000 letters, and its intervals are
  narrower still (coverage 0.55/0.67): the persistent deviations, once conditioned on
  the last visit, make the next visit look more certain.
- **v3 adopted** under clause (b): every population variant lies inside both H1 windows
  (next-observation coverage 0.81–0.84 and 0.87–0.88). By the declared rule its primary
  is the Student-t variant (`population_assimilation_t4`, 2.794 letters, within 0.05 of
  the robust variant's 2.783 and earlier in the order). Against v0 it loses 0.016
  letters at the next observation and 0.005 in the initial state and gains 0.019 in the
  landmark task, all within the 0.05 limit.

## Freeze decision (2026-10-29)

Model 1.0.0 is **v3** (contexts `human/<indication>/<treatment>`, one context level, a
study effect per context arm, forecast intervals from the pooled study noise), as
declared. Primaries: `population_assimilation_t4` for the next observation (U2) and the
landmark task (U3), `population_initial_state` for the initial state (U1). There is no
reference candidate, so there is no H5.

The declared hypotheses are those of the other eye components, in letters: H1
next-observation coverage of the primary inside [0.70, 0.90] and [0.80, 0.97]; H2
next-observation RMSE lower than persistence, the last-two trend, the pooled-curve
increment and analog kNN; H3 landmark RMSE lower than persistence, the pooled-curve
increment and analog kNN; H4 initial-state RMSE lower than the pooled training curve
over common control group-mean targets. On development v3 passes H1 (0.841/0.884), H3
(3.047 against persistence 3.148, pooled-curve increment 3.392 and analog 4.970) and H4
(4.742 against 5.683), and **fails H2 against persistence** (2.794 against 2.654
letters). The sealed test is run to measure model 1.0 on new studies; the comparators
are not changed after seeing this.
