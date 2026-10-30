# Retinal-thickness model 1.4 — development log

Development data: the 195 record studies of tranches 1–3 of dataset
`retina-cst-2026-10-28` (46 tranche-1 development records, 19 tranche-1 sealed records
consumed by the test of model 1.0.0, 63 tranche-2 sealed records consumed by the test of
model 1.1.0, 67 tranche-3 sealed records consumed by the test of model 1.3.0; 449
series, all human). The tranche-4 reservation (`tranche-4/split-reservation-1.0.0.json`)
lists them as development and the 22 admitted tranche-4 articles as the sealed test of
model 1.4; custodians hold 17 tranche-4 records (26 series) in
`.peykar/retina-tranche4-sealed-2026-10-30/`. No tranche-4 value is read before the
1.4 protocol is frozen. Studies of one author-overlap cluster
(`tranche-4/author-overlap-1.0.0.json`, 21 development clusters) leave training
together in leave-one-study-out (LOSO). Operator
`scripts/evaluate_retina_population_1_4.py`; candidates
`peykar.model_learning.retina_thickness_population_1_4`. Each round declares its change
and its adoption rule before its results are seen.

## Why history-only stacking with a scaled interval (consumed data only)

Stacking 1.0.0 (`eye_stacking`) was tested once on two sealed sets: visual acuity 1.2
on acuity tranche 2 and retinal thickness 1.3 on retina tranche 3
(`../model-1.3/README.md`). In both tests the stacked forecasts from follow-up beat the
model's own: thickness 5.70 against 6.08 pp at the next observation and 7.72 against
7.98 pp at the landmark; acuity 3.72 against 4.06 and 5.74 against 6.66 letters. Both
failed elsewhere:

- stacked initial states were not better than the model's own (thickness 11.62
  against 11.61 pp, acuity 9.83 against 9.01 letters): the pooled training curve
  ignores the baseline value, so averaging it in does not correct a study's offset;
- moved intervals (the model's width at the stacked point) covered too much: thickness
  0.911 at 80%, just above the window [0.70, 0.90].

Version 2.0.0 (`eye_stacking2`) therefore stacks only forecasts from follow-up and
leaves initial states to the model. A stacked forecast is closer to the observations
than the model's own, so the model's interval, which was calibrated for the model's
own errors, is too wide around it. Version 2.0.0 keeps the shape of the model's
interval (its quantiles measured from the model's point) and multiplies it by one
factor `c` chosen on development forecasts.

## Round 0 — reference, declared before results (2026-10-30)

- **e0**: model 1.1.0 (candidate `s5` of `retina_thickness_population_1_1`, its series
  and dose rule), rerun on the 195 studies by harness 6 (`eye_recovery_evaluation6`) in
  16 shards (`--shard i/16`) joined by `--merge`. It is the base of round 1 and the
  reference of the declared H5.
- The stacking code (`peykar.model_learning.eye_stacking2` 2.0.0,
  `peykar.validation.eye_stacking_evaluation2` 2.0.0, unit tests
  `tests/unit/test_eye_stacking2.py`) is committed with this declaration, before any
  shard is started; the merge is the first time a LOSO number over the 195 studies is
  seen.

## Round 1 — declared before results (2026-10-30)

- **k2**: e0 with stacked forecasts from follow-up (`eye_stacking2` 2.0.0):
  - forecasts from follow-up (next observation and landmark): the convex combination of
    stacking 1.0.0: e0's primary `population_assimilation` (Gaussian), persistence (the
    last observation) and the pooled-curve increment (`pooled_curve_ratio`); a member
    missing for a target drops out and the other weights are renormalized;
  - weights: non-negative, summing to one, minimizing the pooled squared error over the
    forecast rows of human group means (every row of the two forecast tasks once, a row
    shared by both counting once, only rows where every member exists); this is
    stacking 1.0.0's history fit unchanged;
  - interval: e0's Gaussian quantiles q with point m (its median) become
    `q' = s + c (q - m)` at the stacked point s. The factor c is chosen on the same
    studies with those weights, from the grid 0.30, 0.31, …, 3.00: the factor whose
    pooled coverage of the 80% next-observation intervals of human group means is
    closest to 0.80; ties go to the factor closest to 1, then to the smaller one. The
    landmark intervals use the same factor;
  - initial states: e0's own `population_initial_state` and its interval (not stacked);
  - development estimate: nested LOSO. Each held-out study (with its linked studies)
    is stacked with weights and a factor fitted only on the other studies' LOSO
    forecasts (`scripts/evaluate_retina_population_1_4.py --stack`). The weights and
    the factor fitted on all 195 studies are the ones a protocol would freeze.
- **Adoption rule**, against e0's primary `population_assimilation`, pooled RMSE in pp
  of the baseline thickness on the forecast rows of human group means: a gain of at
  least 0.10 pp in at least one of the two forecast tasks, a loss of at most 0.05 pp in
  both, and the coverage of the stacked next-observation intervals inside both H1
  windows ([0.70, 0.90] at 80%, [0.80, 0.97] at 90%). The 90% coverage is not used to
  choose c, so the 90% window is a check.
- Reported, not part of the rule: the coverage of the same stacked forecasts with the
  interval only moved (c = 1, stacking 1.0.0's rule), and the landmark coverage.
- If k2 is adopted, model 1.4.0 is e0 with the frozen weights and factor, and its
  protocol is frozen before tranche 4 is read. Hypotheses on the human group means of
  tranche 4:
  - H1: coverage of the stacked next-observation intervals inside both windows;
  - H2: stacked next-observation RMSE lower than each of persistence,
    `last_two_log_linear`, `pooled_curve_ratio` and `analog_knn` on common targets;
  - H3: stacked landmark RMSE lower than each of persistence, `pooled_curve_ratio` and
    `analog_knn` on common targets;
  - H4: `population_initial_state` RMSE over control group means lower than the pooled
    training curve (e0's own initial states, so this is model 1.1.0's H4);
  - H5: against model 1.1.0 with the same fit (e0's own forecasts of tranche 4 in the
    same run), the pooled landmark RMSE lower and the pooled next-observation RMSE at
    most 0.05 pp higher. Initial states equal model 1.1.0's by construction (reported).
  - If k2 is rejected, there is no 1.4 protocol and tranche 4 stays sealed. After
    tranche 4 no unread article of this dataset remains.

## Round 0 — results (2026-10-30)

e0 rerun by harness 6 in 16 shards (`.peykar/retina-1.4-dev/round0-e0`, joined by
`--merge`; 4,131–4,706 s per shard; report SHA-256 `e4152c3a…`). The stacking code had
been committed (`6ef76603`) before the shards started. e0's primary
`population_assimilation` on the forecast rows of human group means (750 targets per
task): next observation 6.752 pp, landmark 8.850 pp (persistence 7.724 and 11.337);
initial state of control group means 12.199 pp (875 targets; pooled training curve
14.810). Coverage of e0's own intervals: next observation 0.861 at 80% and 0.931 at
90%, landmark 0.805 and 0.903.

## Round 1 — results (2026-10-30)

Nested LOSO of k2 on e0's round-0 report (`.peykar/retina-1.4-dev/round1-k2`;
`scripts/evaluate_retina_population_1_4.py --stack`). Pooled RMSE in pp of the baseline
thickness on the forecast rows of human group means:

| task | targets | e0 primary | k2 stacked | gain |
|---|---|---|---|---|
| next observation | 750 | 6.752 | 6.623 | +0.128 |
| landmark | 750 | 8.850 | 8.673 | +0.176 |

- Coverage of the stacked next-observation intervals: 598/750 = 0.797 at 80% and
  658/750 = 0.877 at 90%, inside both windows.
- **k2 meets the declared adoption rule**: both forecast tasks gain more than 0.10 pp.
- Fitted on all 195 studies (1,211 forecast rows): weights 0.729 model, 0.212
  persistence, 0.058 pooled-curve increment; interval scale c = 0.79 (pooled 80%
  next-observation coverage 0.803 with it, 0.864 with c = 1). Over the 195 nested fits
  the scale ranged 0.74–0.83 (median 0.79), the model's weight 0.68–0.76, persistence
  0.17–0.23 and the increment 0.04–0.09.
- Reported, not part of the rule. With the interval only moved (c = 1, stacking
  1.0.0), next-observation coverage would be 0.863/0.929. **The landmark intervals
  under-cover with k2's scale: 519/750 = 0.692 at 80% (below the window's 0.70) and
  0.807 at 90%**; moved without scaling they cover 0.792/0.900.

Reading: the scale is not mainly an effect of stacking (the stacked points are only 2%
closer to the observations than e0's). It recalibrates e0's own next-observation
intervals, which were too wide (0.861 at 80%), while e0's landmark intervals were
already close to nominal (0.805). One factor for both tasks therefore narrows landmark
intervals that did not need it.

## Round 2 — declared before results (2026-10-30)

Before any freeze, one more development round fixes the landmark intervals. k2 is not
changed; its code path and description stay as they were (checked by rerunning k2 with
the round-2 code: its report must be identical).

- **k3**: k2 with one interval scale per forecast task. The weights are k2's (the same
  fit). The next-observation scale is k2's (chosen on the 80% next-observation
  intervals); the landmark scale is chosen the same way on the 80% landmark intervals
  of human group means (grid 0.30–3.00 by 0.01, closest to 0.80, ties closest to 1,
  then smaller). Point forecasts and next-observation intervals are k2's by
  construction; only the landmark intervals change. Nested LOSO as in round 1.
- **Adoption rule**: k3 replaces k2 if its nested landmark coverage of human group
  means is inside both windows ([0.70, 0.90] at 80%, [0.80, 0.97] at 90%), and its
  point forecasts and next-observation intervals equal k2's. Otherwise k2 is frozen as
  declared in round 1.
- The declared hypotheses are unchanged: H1 tests next-observation coverage only;
  landmark coverage stays reported, not tested, as in every earlier protocol.
