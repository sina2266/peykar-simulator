# Visual-acuity model 1.2 — development log

Training data: the 42 tranche-1 record studies of dataset
[`cornea-acuity-2026-10-29`](../../../data/cornea-acuity-2026-10-29/README.md), the
training data of model 1.1 (the 27 development records and the 15 sealed records
consumed by the one-shot test of model 1.0.0; 87 series). Model 1.1 adopted no change
and froze no protocol, so tranche 2 (27 admitted articles, 11 custodian records, 23
series) is still sealed and unread; it is the sealed test set of model 1.2. No value of
it is read before the 1.2 protocol is frozen. Leave-one-study-out (LOSO) over the 42
studies uses the development clusters of
[`tranche-2/author-overlap-1.0.0.json`](../../../data/cornea-acuity-2026-10-29/tranche-2/author-overlap-1.0.0.json)
(one cluster of two studies). Operator `scripts/evaluate_acuity_population_1_2.py`;
candidates `peykar.model_learning.acuity_population_1_2`. Each round declares its
change and its adoption rule before its results are seen.

## Why stacking (training data only)

Changes to the hierarchy did not help: model 1.1 tried correlated random effects (b1)
and a cross-linking slope (b2), and neither gained 0.10 letters; model 1.0 tried an
indication level (a1) and per-family hierarchies (a2), which gained in the forecast
tasks and lost about 3 letters in the initial state. The simple comparators stay close
to a0 on the 42 studies (model 1.1 round 0, human group means): landmark 7.136 letters
for a0's primary against 7.167 for the pooled-curve increment, initial state 13.006
against 14.135 for the pooled training curve, next observation 5.412 against 5.863 for
persistence. In the three sealed tests of retinal acuity the same comparators beat the
population model.

When forecasts of similar accuracy make different errors, a weighted average of them
has a smaller error than each one. Stacked regressions (Breiman 1996) choose such
weights from out-of-sample forecasts, non-negative and summing to one, by least
squares. This round tests that idea without changing the population model: the fit,
the law and the series stay a0's.

## Round 0 — reference, declared before results (2026-10-29)

- **a0**: the frozen model 1.0.0 configuration, rerun on the 42 studies by additive
  evaluation 4 with harness 6 (`eye_recovery_evaluation6`). Harness 6 is harness 2
  plus the predictive quantiles of every initial state, which the stacked initial
  states need for their intervals. Every other number must equal model 1.1's round-0
  report (`.peykar/acuity-1.1-dev/round0-a0/report.json`, harness 2); if any differs,
  round 1 waits until the cause is found.

## Round 1 — declared before results (2026-10-29)

- **k1**: a0 with stacked forecasts (`peykar.model_learning.eye_stacking` 1.0.0,
  `peykar.validation.eye_stacking_evaluation` 1.0.0):
  - forecasts from follow-up (next observation and landmark): a convex combination of
    a0's primary `population_assimilation_robust`, persistence (the last observation)
    and the pooled-curve increment (`pooled_curve_ratio`: the last observation moved by
    the change of the pooled training curve from the last visit to the target month);
    the interval is the robust variant's, moved to the stacked point with its width
    unchanged;
  - initial states: a convex combination of `population_initial_state` and the pooled
    training curve; the interval is `population_initial_state`'s, moved likewise;
  - a member missing for a target (no pooled curve) drops out and the other weights
    are renormalized;
  - weights: non-negative, summing to one, minimizing the pooled squared error over
    the group means of human series (both roles; every forecast row of the two forecast
    tasks once, a row shared by both counting once; for the initial state, every
    follow-up of a human control group mean);
  - development estimate: nested LOSO. Each held-out study (with its linked study) is
    stacked with weights fitted only on the other studies' LOSO forecasts. The weights
    fitted on all 42 studies are the ones a protocol would freeze.
- **Adoption rule**, against a0's primaries (robust variant for next observation and
  landmark, `population_initial_state` for the initial state), pooled RMSE in letters on
  human group means: a gain of at least 0.10 letters in at least one task, a loss of at
  most 0.05 letters in every task, and the coverage of the stacked next-observation
  intervals inside both H1 windows ([0.70, 0.90] at 80%, [0.80, 0.97] at 90%).
- If k1 is adopted, model 1.2.0 is a0 with the frozen weights, and its protocol is
  frozen before tranche 2 is read. Hypotheses on the human group means of tranche 2:
  H1, coverage of the stacked next-observation intervals inside both windows; H2 and
  H3, stacked next-observation and landmark RMSE lower than every comparator of model
  1.0's protocol on common targets; H4, stacked initial state lower than the pooled
  training curve; H5, against model 1.0.0 with the same fit (a0's own forecasts of
  tranche 2 in the same run), landmark and initial-state RMSE lower and next-observation
  RMSE at most 0.05 letters higher. If k1 is rejected, there is no 1.2 protocol and
  tranche 2 stays sealed.
