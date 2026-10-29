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

## Round 0 — results (2026-10-30)

a0 rerun by additive evaluation 4 and harness 6 (`.peykar/acuity-1.2-dev/round0-a0`,
375 s): every number equals model 1.1's round-0 report (harness 2); the only additions
are the initial-state intervals of the 84 initial-state entries with a population
forecast and the evaluation versions (`.peykar/compare_harness6.py`, 0 differences).
a0's primaries: next observation 5.412 letters, landmark 7.136, initial state 13.006
(human group means; coverage of the robust variant 0.767/0.830).

## Round 1 — results (2026-10-30)

Nested LOSO of k1 on a0's round-0 report (`.peykar/acuity-1.2-dev/round1-k1`;
`scripts/evaluate_acuity_population_1_2.py --stack`). Pooled RMSE in letters on human
group means:

| task | targets | a0 primary | k1 stacked | gain |
|---|---|---|---|---|
| next observation | 159 | 5.412 | 5.114 | +0.298 |
| landmark | 159 | 7.136 | 6.862 | +0.274 |
| initial state | 214 | 13.006 | 12.919 | +0.086 |

- Coverage of the stacked next-observation intervals: 122/159 = 0.767 at 80% and
  130/159 = 0.818 at 90%, inside both windows (a0 robust: 0.767 and 0.830). The 90%
  coverage is 0.018 above its lower limit.
- **k1 adopted**: it gains 0.30 and 0.27 letters (above 0.10) and loses nothing.
- Weights fitted on all 42 studies (259 forecast rows, 214 initial-state targets):
  forecasts from follow-up 0.544 model, 0.233 persistence, 0.223 pooled-curve
  increment; initial states 0.582 model, 0.418 pooled training curve. Over the 42
  nested fits the model's weight ranged 0.48–0.59 (forecasts) and 0.47–0.82 (initial
  states).
- The development analysis of the declared hypotheses
  (`scripts/publish_acuity_population_1_2.py --development` with a0's round-0 report as
  reference; `.peykar/acuity-1.2-dev/round1-k1-analysis`) passes H1–H5: H2 5.11 letters
  against persistence 5.86, pooled-curve increment 6.02, last-two trend 43.85 and analog
  9.52; H3 6.86 against 9.85, 7.17 and 15.59; H4 12.92 against the pooled training curve
  14.14; H5 5.11/6.86/12.92 against a0's 5.41/7.14/13.01. Reported, not tested: landmark
  coverage 0.824/0.874; stacked initial-state coverage 0.771/0.860 (a0's own initial
  states 0.850 at 80%), so the moved initial-state intervals are narrower than nominal
  at 90%.

## Freeze decision (2026-10-30)

Model 1.2.0 is k1: a0's configuration and fit with the weights above frozen. The
protocol is `protocol-1.2.0.json`; it pins the weights, the code, the training records
and the tranche-2 records (structure only seen). Its one sealed run fits a0 on the 42
training studies (minus studies linked to a sealed study), scores tranche 2 once, and
writes a0's unstacked report (the H5 reference) and the stacked report of model 1.2.0
from the same fit.
