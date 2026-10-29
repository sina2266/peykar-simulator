# Visual-acuity model 1.1 — development log

Training data: the 42 tranche-1 record studies of dataset
[`cornea-acuity-2026-10-29`](../../../data/cornea-acuity-2026-10-29/README.md), as
listed in [`tranche-2/split-reservation-1.0.0.json`](../../../data/cornea-acuity-2026-10-29/tranche-2/split-reservation-1.0.0.json):
the 27 development records (62 series) and the 15 sealed records (25 series) consumed
by the one-shot test of model 1.0.0, 87 series in all. The sealed test set of model 1.1
is tranche 2 (27 admitted articles, 11 records, 23 series, all held by custodians; no
value of them is read before the 1.1 protocol is frozen). Leave-one-study-out (LOSO)
over the 42 studies uses the development clusters of
[`tranche-2/author-overlap-1.0.0.json`](../../../data/cornea-acuity-2026-10-29/tranche-2/author-overlap-1.0.0.json)
(one cluster of two studies). Operator `scripts/evaluate_acuity_population_1_1.py`;
configurations `peykar.model_learning.acuity_population_1_1`. Each round declares its
change and its adoption rule before its LOSO results are seen.

## Round 0 — reference, declared before results (2026-10-29)

- **a0**: the frozen model 1.0.0 configuration, unchanged, on the 42 training studies.
  It is the reference of every 1.1 candidate and the reference candidate of the 1.1
  sealed test (hypothesis H5). Its diagnostics (where it loses to the pooled-curve
  increment, by context and horizon) are read before the round-1 candidates are
  declared; round 1 does not change a0.

## Round 0 — results (2026-10-29)

LOSO of a0 over the 42 studies (`.peykar/acuity-1.1-dev/round0-a0`, 252 s; 159
next-observation and 159 landmark targets, 214 initial-state targets of control group
means; `.peykar/acuity_compare.py` and `.peykar/acuity11_breakdown.py`; RMSE in ETDRS
letters, signed error = forecast − observed, negative = forecast better acuity than
observed):

| method | next observation | landmark | initial state |
|---|---|---|---|
| a0 robust (`population_assimilation_robust`) | 5.412 (bias −1.34), cov 0.767/0.830 | 7.136 (−2.45) | — |
| a0 anchored t4 | 5.377 (−1.51), cov 0.792/0.843 | 7.244 (−2.64) | — |
| a0 initial state | — | — | 13.006 (−0.84) |
| persistence | 5.863 | 9.846 | — |
| pooled-curve increment | 6.017 | 7.167 | — |
| pooled training curve | — | — | 14.135 |

With the 15 consumed studies, a0 beats the pooled-curve increment for the next
observation; for the landmark task it is level with it. The breakdown shows where it
still loses:

- **Cross-linking** (68 next-observation, 68 landmark and 90 initial-state targets):
  next observation 4.10 letters against 3.97 (pooled-curve increment) and 3.86
  (persistence); landmark 5.72 (bias −4.11) against 4.32 and 4.75; initial state 6.63
  against the pooled curve's 4.47. After cross-linking the model expects more
  improvement than happens: its one level slope on the preoperative acuity (fitted
  across keratoplasty and cross-linking) moves a cross-linking group with poor
  preoperative acuity towards the large gains of keratoplasty. In model 1.0 round 1,
  separate hierarchies (a2) gave cross-linking its own slope (−0.10 against −0.77) and
  fixed this, but separate variances and noise made keratoplasty worse.
- **Keratoplasty**: the model is better than the comparator after DMEK (next
  observation 6.05 against 11.10) and level with it after DSAEK (6.96 against 6.26)
  and PK (7.32 against 6.32, 9 targets), where groups that recover poorly stay poor.
- The model is optimistic overall (bias −1.3 letters at the next observation and −2.5
  at the landmark), mostly from cross-linking and PK.

## Round 1 — declared before results (2026-10-29)

Both candidates are a0 in generic model 4 (`eye_recovery_population4`, new; with its
options at their defaults it is model 2 bit for bit), scored by harness 4
(`eye_recovery_evaluation4`) through `eye_additive_evaluation2`. Model 4 adds two
options to the hierarchy:

- **b1**: `random_correlation = "level_excess"`. The study level and the two series
  levels share one correlation `rho` between the long-term level `ell` and the
  transient amplitude `e`, fitted in the same penalized REML as the variances
  (`rho = 0.95 tanh(phi)`, standard normal penalty on `phi`, start `phi = 0`). An early
  deviation and the long-term deviation then have covariance
  `var_ell + r(t) rho sd_ell sd_e`, so the data decide how much of an early shortfall
  is expected to persist; a0 fixes `rho = 0`. Aimed at the poorly recovering
  keratoplasty groups of the model 1.0 diagnosis.
- **b2**: `slope_groups = (("crosslinking", ("cxl", "other")),)`. Cross-linking gets
  its own slope of the long-term level on the preoperative logMAR and keratoplasty
  keeps one shared slope, inside the single hierarchy, noise model and variances of
  a0. Every `other` group of the training records (8 groups) and of the tranche-2
  structure is cross-linking combined with a ring segment or an ablation for
  keratoconus. Aimed at the cross-linking errors above.

Primary method of a candidate: among the population variants whose next-observation
coverage of group means lies inside both H1 windows ([0.70, 0.90] at 80%, [0.80, 0.97]
at 90%), the lowest pooled next-observation RMSE; a variant within 0.05 letters of that
minimum that comes earlier in the order Gaussian, t4, robust, anchored, anchored t4 is
preferred (the earliest one). The same variant is the primary of the landmark task; the
initial state uses `population_initial_state`. By this rule the primary of a0 is the
robust variant (5.412; anchored t4 5.377 lies within 0.05 and comes later).

Adoption rule, against a0 (5.412 / 7.136 / 13.006): a candidate is adopted only if it
improves the pooled RMSE of its primary by at least 0.10 letters in at least one task
(next observation, landmark, initial state) and loses no more than 0.05 letters in any.
If both are adopted, round 2 runs **b3** (b1 and b2 together) against the one with the
larger summed gain over the three tasks, under the same rule. If neither is adopted,
there is no 1.1 protocol and tranche 2 stays sealed and unread.
