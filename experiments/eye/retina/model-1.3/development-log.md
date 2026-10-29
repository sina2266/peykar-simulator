# Retinal-thickness model 1.3 — development log

Development data: the 128 record studies of tranches 1 and 2 of dataset
`retina-cst-2026-10-28`, the development data of model 1.2 (46 tranche-1 development
records, 19 tranche-1 sealed records consumed by the test of model 1.0.0, 63 tranche-2
sealed records consumed by the test of model 1.1.0; 315 series). Model 1.2 adopted no
change and froze no protocol (`../model-1.2/development-log.md`), so tranche 3 (67
custodian records, 134 series) is still sealed and unread; it is the sealed test set of
model 1.3. No tranche-3 value is read before the 1.3 protocol is frozen. Studies of one
author-overlap cluster (`tranche-3/author-overlap-1.0.0.json`, development clusters)
leave training together in leave-one-study-out (LOSO). Operator
`scripts/evaluate_retina_population_1_3.py`; candidates
`peykar.model_learning.retina_thickness_population_1_3`. Each round declares its change
and its adoption rule before its results are seen.

## Why stacking (training data only)

Model 1.2 tried three levels for the out-of-sample biases of model 1.1's initial states
(prior treatment e3, OCT device e4, regimen e6). None gained 0.10 pp: the biases belong
to the held-out studies, not to a level the hierarchy can learn. A different estimate
of the same quantity can still help. The pooled training curve and, for forecasts from
follow-up, persistence and the pooled-curve increment are made without the population
model. When forecasts of similar accuracy make different errors, a weighted average of
them has a smaller error than each one; stacked regressions (Breiman 1996) choose such
weights from out-of-sample forecasts, non-negative and summing to one, by least
squares. On the 128 studies, e0 (model 1.1.0) is clearly better than persistence for
the forecast tasks (7.193 against 8.337 pp at the next observation, 9.376 against
11.699 pp at the landmark; model 1.2 round 1), so the weights may stay close to the
model. This round tests the idea without changing the population model: the fit, the
law, the dose rule and the series stay model 1.1.0's.

## Round 0 — reference, declared before results (2026-10-29)

- **e0**: model 1.1.0 (candidate `s5`, its series and dose rule), rerun on the 128
  studies by harness 6 (`eye_recovery_evaluation6`) in 16 shards joined by `--merge`.
  Harness 6 is harness 2 plus the predictive quantiles of every initial state, which the
  stacked initial states need for their intervals. Every other number must equal model
  1.2's round-1 report of e0 (`.peykar/retina-1.2-dev/round1-e0/report.json`, harness 2);
  if any differs, round 1 waits until the cause is found.

## Round 1 — declared before results (2026-10-29)

- **k1**: e0 with stacked forecasts (`peykar.model_learning.eye_stacking` 1.0.0,
  `peykar.validation.eye_stacking_evaluation` 1.0.0):
  - forecasts from follow-up (next observation and landmark): a convex combination of
    e0's primary `population_assimilation` (Gaussian), persistence (the last
    observation) and the pooled-curve increment (`pooled_curve_ratio`); the interval is
    the Gaussian variant's, moved to the stacked point with its width unchanged;
  - initial states: a convex combination of `population_initial_state` and the pooled
    training curve; the interval is `population_initial_state`'s, moved likewise;
  - a member missing for a target (no pooled curve) drops out and the other weights
    are renormalized;
  - weights: non-negative, summing to one, minimizing the pooled squared error over
    the group means of human series (both roles; every forecast row of the two forecast
    tasks once, a row shared by both counting once; for the initial state, every
    follow-up of a human control group mean);
  - development estimate: nested LOSO. Each held-out study (with its linked studies)
    is stacked with weights fitted only on the other studies' LOSO forecasts. The
    weights fitted on all 128 studies are the ones a protocol would freeze.
- **Adoption rule** (as in model 1.2), against e0's primaries (Gaussian variant for
  next observation and landmark, `population_initial_state` for the initial state),
  pooled RMSE in pp of the baseline thickness on human group means: a gain of at least
  0.10 pp in at least one task, a loss of at most 0.05 pp in every task, and the
  coverage of the stacked next-observation intervals inside both H1 windows
  ([0.70, 0.90] at 80%, [0.80, 0.97] at 90%).
- If k1 is adopted, model 1.3.0 is e0 with the frozen weights, and its protocol is
  frozen before tranche 3 is read. Hypotheses on the human group means of tranche 3:
  H1, coverage of the stacked next-observation intervals inside both windows; H2 and
  H3, stacked next-observation and landmark RMSE lower than every comparator of model
  1.0's protocol on common targets; H4, stacked initial state lower than the pooled
  training curve; H5, against model 1.1.0 with the same fit (e0's own forecasts of
  tranche 3 in the same run), landmark and initial-state RMSE lower and
  next-observation RMSE at most 0.05 pp higher. If k1 is rejected, there is no 1.3
  protocol and tranche 3 stays sealed. Tranche 4 stays unread.
