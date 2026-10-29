# IOP model 1.3 — development log

Development data: the 229 record studies of tranches 1–3 of dataset
`iop-glaucoma-surgery-2026-10-28` (47 tranche-1 development records, 25 tranche-1
sealed records consumed by the test of model 1.0.0, 74 tranche-2 sealed records
consumed by the test of model 1.1.0 and 83 tranche-3 sealed records consumed by the
test of model 1.2.0; 557 series). The tranche-4 reservation lists them as development
([`tranche-4/split-reservation-1.0.0.json`](../../../data/iop-glaucoma-surgery-2026-10-28/tranche-4/split-reservation-1.0.0.json)).
No tranche-4 value is read before the 1.3 protocol is frozen. Studies of one
author-overlap cluster (`tranche-4/author-overlap-1.0.0.json`, 35 development
clusters) leave training together in leave-one-study-out (LOSO). Each round declares
its change and its adoption rule before its LOSO results are seen.

## Starting point and diagnostics (training data only)

Model 1.2.0 passed all five hypotheses of its tranche-3 sealed test. Its forecasts are
those of 1.1.0; it widened only the first-month intervals. Out-of-sample errors of 1.2
on all 229 training studies (its development LOSO on tranches 1–2 plus its sealed
report on tranche 3; `.peykar/iop13_diag_oos.py`, `.peykar/iop13_diag_subprocedure.py`;
human group means; signed error = forecast − observed, pp of the preoperative IOP):

- Overall the intervals are calibrated (next observation 0.81/0.90), but not per
  procedure: 80% coverage 0.70 after cyclodestruction, 0.73 after tubes and 0.77 after
  non-penetrating surgery, against 0.83 after angle MIGS.
- The initial state (no follow-up seen) is biased by device or technique within one
  procedure context. After cyclodestruction it was −9.0 pp in the first month and
  −10.6 pp later for micropulse laser (104 targets, 11 studies) but +2.8/+7.0 for
  continuous-wave and −1.0/+7.4 for endoscopic treatment. After subconjunctival MIGS
  it was +10.7/+6.5 for PreserFlo (5 studies) and +1.3/−2.1 for XEN (13). After angle
  MIGS it was −7.4 pp in the first month for stents and −4.7 for goniotomy, against
  −2.3 for trabeculotomy. Valved and non-valved tubes did not differ in bias.
- The same pattern is weaker once follow-ups are seen (next observation), where the
  series' own visits dominate.

## Model

Generic eye model version 2 (`eye_recovery_population2`, unchanged) with the
configurations of `peykar.model_learning.iop_population_1_3`; series adapters
`peykar.validation.eye_iop_data_1_2` (model 1.2's series) and
`peykar.validation.eye_iop_data_1_3` (the same series with a sub-procedure context);
operator `scripts/evaluate_iop_population_1_3.py`.

`eye_iop_data_1_3` reads the sub-procedure of each arm from its own group description
and name with declared keyword rules (checked in order; the record title only when
they match nothing): tubes valved or non-valved; angle MIGS stent, goniotomy,
trabeculotomy or canaloplasty; cyclodestruction micropulse, endoscopic, ultrasound or
continuous; subconjunctival MIGS XEN or PreserFlo; other arms of these procedures
`other`, and the remaining procedures `all`. On the training records the rules give
21 contexts (for example 64 trabeculotomy, 50 goniotomy and 46 stent control series
among angle MIGS; 22 micropulse, 8 continuous, 4 endoscopic and 4 ultrasound among
cyclodestruction).

## Round 1 — declared before results (2026-10-29)

- **d0**: model 1.2.0 (the frozen candidate `c1`, its series), the reference.
- **d1**: the sub-procedure hierarchy: contexts `species/<sub>/procedure` and grouping
  `("treatment", "context")`, so the hierarchy is species → procedure → sub-procedure;
  otherwise `c1` (forecast-only early noise `k = 8`, `T = 1 month`, per-procedure
  study effects).

- **d2** (added at 04:09, before any round-1 result): `c1` on generic eye model
  version 3 (`eye_recovery_population3`, harness `eye_recovery_evaluation3`) with the
  point rule `lognormal_mean`: every population point forecast is the mean of the
  lognormal matched to the predictive median and 5%/95% quantiles, instead of the
  median. Reason: the initial-state forecasts of 1.2 were 1.62 pp too low on average
  over the 229 studies, and the pooled RMSE is minimized by the predictive mean, not
  the median. An approximation on the existing reports (the ratio from one fit on all
  training studies applied to the out-of-sample forecasts,
  `.peykar/diag_lognormal_mean.py`) gave initial-state RMSE 10.35 against 10.44 pp and
  bias −0.31 against −1.62 pp; d2's LOSO run is the test. The exact mixture mean is
  not used: in retinal-thickness fits it is unbounded for a few series. Harness 3 with
  the point rule `median` reproduces harness 2 exactly
  (`tests/unit/test_eye_recovery_version3.py`).
- If both d1 and d2 are adopted against d0, round 2 runs **d3** = d1 with d2's point
  rule, adopted if it meets the same rule against the better of d1 and d2.

Adoption rule (as for models 1.1 and 1.2): a candidate replaces the incumbent only if,
on human group means, it improves the pooled RMSE of its primary method by at least
0.10 pp in at least one task (initial state, next observation, landmark) and loses no
more than 0.05 pp in any. The primary method is the lowest pooled group-mean
next-observation RMSE among the population variants whose group-mean coverage lies
inside both H1 windows ([0.70, 0.90] and [0.80, 0.97]); ties within 0.05 pp go to the
simpler variant (Gaussian). Each run is split into four shards (`--shard i/4`) and
joined by `--merge`.

## Round 1 — results (2026-10-29)

LOSO over the 229 training studies, human group means; the selection rule gives the
Gaussian variant `population_assimilation` as the U2/U3 primary of every candidate
(all five population variants lie inside both H1 windows), and U1 is
`population_initial_state`. Pooled RMSE in pp of the preoperative IOP; signed error =
forecast − observed (`.peykar/iop13_round1_breakdown.py`):

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% | first-month next-obs coverage 80/90% |
|---|---|---|---|---|---|
| d0 (1.2.0) | 6.464 (bias +0.00) | 8.348 (+0.03) | 10.209 (−1.29) | 0.791 / 0.893 | 0.820 / 0.909 |
| d1 (sub-procedure) | **6.382** (−0.01) | **8.061** (−0.02) | **10.066** (−1.26) | 0.793 / 0.898 | 0.826 / 0.914 |
| d2 (lognormal mean) | 6.528 (+0.33) | 8.421 (+0.53) | 10.215 (+0.05) | 0.791 / 0.893 | 0.820 / 0.909 |

Targets: 1,467 next-observation and landmark targets (350 in the first month) and
1,651 initial-state targets. Comparators: persistence 7.748 (next observation) and
13.614 (landmark); pooled training curve 17.659 (d0, d2) and 16.913 (d1, whose curve
is pooled within the sub-procedure).

- **d1 adopted.** It improves all three tasks: next observation −0.082, landmark
  −0.287 and initial state −0.143 pp (two tasks beyond the 0.10 pp threshold), and
  loses nowhere. The gains sit in the procedures that have sub-procedure rules
  (initial state / landmark, d1 − d0): cyclodestruction −0.91 / −0.64 pp,
  subconjunctival MIGS −0.36 / −0.63, tubes −0.17 / −0.57 and angle MIGS +0.03 /
  −0.26. Procedures without rules change by at most 0.06 pp in the initial state;
  the largest such landmark change is cataract surgery, +0.28 pp on 45 targets.
- **d2 rejected.** It removes the initial-state bias (−1.29 → +0.05 pp) but does not
  lower the initial-state RMSE (+0.006 pp), and it shifts the forecasts upwards where
  the series' own visits had already removed the bias: next observation +0.064 and
  landmark +0.073 pp, both beyond the 0.05 pp loss limit. The bias of 1.2's initial
  state is not a point-rule effect that the lognormal mean can correct without cost.
- d3 is not run (it was declared only for the case that both d1 and d2 are adopted).

## Freeze decision (2026-10-29, before any tranche-4 value is read)

No further round. Model 1.3.0 is candidate **d1**: primaries `population_assimilation`
(U2, U3) and `population_initial_state` (U1); the reference is **d0**, the frozen
model 1.2.0 trained on the same records and scored in the same sealed run. Its H5:
over common human group-mean targets, the pooled RMSE of 1.3 is lower than 1.2.0's in
the landmark task (U3) and in the initial state (U1, control group means), and its
pooled next-observation RMSE (U2) is at most 0.05 pp higher than 1.2.0's. H1–H4 are
the hypotheses of model 1.2 with the 1.3 primaries.

The publisher `scripts/publish_iop_population_1_3.py` removes non-human series
before the tested analysis (the tested subset is human, as declared since 1.1; one
rabbit training study cannot fit a rabbit population, so tranche 4's four rabbit
series get no population prior and are reported per series only), and it publishes
redacted studies with an allow-list of initial-state fields (the 1.2 publisher had
left `reference_value` in them; see the model-1.2 README).
