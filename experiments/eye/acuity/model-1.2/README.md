# Corrected visual acuity after keratoplasty and cross-linking — model 1.2.0

Version 1.2 of the seventh eye component. Model 1.0.0 passed its sealed test; changes to
its hierarchy (model 1.1 and model 1.0's own rounds) did not help on development data.
Model 1.2.0 keeps model 1.0.0's fit and adds **forecast stacking**
(`peykar.model_learning.eye_stacking` 1.0.0): each forecast is a fixed weighted average
of the model's forecast and simple comparators, with weights fitted once on
out-of-sample forecasts of the development studies (stacked regressions, Breiman 1996).
It was developed on the 42 tranche-1 record studies and tested once on tranche 2 of
dataset [`cornea-acuity-2026-10-29`](../../../data/cornea-acuity-2026-10-29/README.md):
11 records with 23 group-mean series, reserved before triage and extracted by four
custodians. **Model 1.2.0 passed H1–H4 and failed H5**: stacking improved both
forecast tasks but made the initial state worse than model 1.0.0 from the same fit.
By the version rule, model 1.0.0 (which passed its own H1–H4 on the same studies)
stays the component of `peykar eye`. Nothing here is a clinical or biological
validation claim.

## Model

- Population fit: model 1.0.0 (`acuity_population`, candidate `a0`), unchanged.
  Harness 6 (`peykar.validation.eye_recovery_evaluation6`) is harness 2 plus the
  predictive quantiles of every initial state; `eye_additive_evaluation4` rewrites
  everything in ETDRS letters (one reported pp is one letter, 0.02 logMAR).
- Stacking (`peykar.validation.eye_stacking_evaluation`, candidate `k1`):
  - forecasts from follow-up: 0.544 × model 1.0.0's robust forecast + 0.233 × the last
    observation (persistence) + 0.223 × the pooled-curve increment;
  - initial states: 0.582 × model 1.0.0's initial state + 0.418 × the pooled training
    curve;
  - intervals: the model's quantiles moved to the stacked point, width unchanged.
  The weights are non-negative, sum to one and minimize the squared error of the 42
  studies' leave-one-study-out forecasts of human group means.
- Operator `scripts/evaluate_acuity_population_1_2.py`, publisher
  `scripts/publish_acuity_population_1_2.py`.

## Development (42 training studies)

The [development log](development-log.md) declared each round before its run. Rerun
by harness 6, model 1.0.0 reproduced its earlier harness-2 report with 0 differences.
In nested leave-one-study-out (each study stacked with weights fitted without it),
human group means:

| task | targets | model 1.0.0 | stacked (k1) |
|---|---|---|---|
| next observation | 159 | 5.412 | 5.114 |
| landmark | 159 | 7.136 | 6.862 |
| initial state | 214 | 13.006 | 12.919 |

Stacked next-observation coverage 0.767/0.818 (80%/90%). k1 passed the declared rule
(a gain of at least 0.10 letters in one task, no loss above 0.05) and H1–H5 on
development. The initial-state gain was small and its weight unstable: the model's
initial-state weight ranged 0.47–0.82 across the 42 nested fits, and with the weights
fitted on all studies the in-sample initial-state RMSE was 11.68 letters against 12.92
nested.

## One-shot sealed test (2026-10-30)

The test ran once after freeze commit `fc3590ac`, with protocol
[`protocol-1.2.0.json`](protocol-1.2.0.json) (SHA `484c1694…`; all 86 pinned files and
the 42 training and 11 sealed record hashes matched). Exit 0, 69.5 s; report
`c8c5fff3…`, model 1.0.0's unstacked report from the same fit `77db13c4…`. Public
artifacts are in [`sealed-test-2026-10-30/`](sealed-test-2026-10-30/tables.md) with a
[figure](sealed-test-2026-10-30/sealed-results.png). Values of 4 studies are redacted;
`.peykar/check_stacking_redaction.py` found no value of a redacted study in the public
files (8 study reports and 44 per-series rows checked, 0 leaks).

Declared checks, human group means (23 series, 66 next-observation and 66 landmark
targets; 22 control group means with 85 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | **pass**: 59/66 = 0.894 at 80% (window 0.70–0.90), 61/66 = 0.924 at 90% (window 0.80–0.97) |
| H2 accuracy, next observation | **pass**: 3.72 letters against persistence 5.03, pooled-curve increment 5.05, analog kNN 5.95 and the last-two trend 18.59 |
| H3 landmark | **pass**: 5.74 letters against persistence 11.58, pooled-curve increment 7.01 and analog kNN 16.25 |
| H4 initial state | **pass**: 9.83 letters against the pooled training curve 14.59 |
| H5 against model 1.0.0, same fit | **fail**: next observation 3.72 against 4.06 and landmark 5.74 against 6.66 (both lower), but initial state 9.83 against 9.01 (higher) |

Reported, not tested: model 1.0.0's own H1–H4 all pass on the same studies (coverage
0.864/0.955; next observation 4.06; landmark 6.66; initial state 9.01). Per series,
19 of 23 group means were within 5/10 letters for the next observation (model 1.0.0:
16) and 12 for the landmark task (10). Landmark coverage 0.833/0.879; stacked
initial-state coverage 74/85 = 0.871 and 79/85 = 0.929.

## What the test shows (reported after unsealing, not tested)

Commands: `.peykar/stacking_sealed_breakdown.py` and `.peykar/stacking_drop_one.py` on
the private reports.

- **Stacking the forecasts from follow-up worked.** The gain came from
  cross-linking: next observation 2.36 against 3.58 letters (29 targets), landmark 4.14
  against 6.98. After DMEK and DSAEK the stacked forecasts were about level with the
  model (next observation 5.71 against 5.53 and 2.12 against 2.06).
- **Stacking the initial state did not.** The pooled training curve has no
  preoperative-acuity covariate. For cross-linking, whose groups change little, it was
  the best initial-state forecast (3.22 letters against the model's 9.73), and stacking
  helped (6.09). For keratoplasty, where the gain depends strongly on the starting
  acuity, it was far off (DMEK 21.00, DSAEK 15.99), and stacking moved the model's
  forecasts towards it (DMEK 14.03 against 9.83, DSAEK 6.90 against 5.01). One weight
  for all procedures cannot do both; the development data had already shown this weight
  to be unstable.
- **One study decides H5.** Without `acu-pmc7774118` (8 DMEK groups by indication,
  visits at 6, 12 and 24 months; values redacted), the stacked model is better in all
  three tasks (next
  observation 2.41 against 3.23, landmark 4.03 against 5.94, initial state 6.80
  against 8.20) and H5 would pass; removing any other study leaves H5 failing.

## Limits and next steps

- Tranche 2 is consumed. A next version could stack only the forecasts from follow-up,
  or fit one initial-state weight per procedure family (cross-linking and
  keratoplasty); either needs a new sealed set, so a new literature screen.
- 23 group-mean series of 11 studies is a small test; one study changes the H5 verdict.
- Treated groups use the control prior of their context; indication, comorbidity,
  per-eye series and graft outcomes are not modelled.
