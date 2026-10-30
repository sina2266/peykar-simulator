# Central retinal thickness after treatment — model 1.3.0

Version 1.3 of the fifth eye component. Model 1.1.0 passed H1–H4 of its sealed test;
model 1.2 adopted none of its three candidate levels. Model 1.3.0 keeps model 1.1.0's
fit and adds **forecast stacking** (`peykar.model_learning.eye_stacking` 1.0.0): each
forecast is a fixed weighted average of the model's forecast and simple comparators,
with weights fitted once on out-of-sample forecasts of the development studies
(stacked regressions, Breiman 1996). It was developed on the 128 record studies of
tranches 1–2 and tested once on tranche 3 of dataset `retina-cst-2026-10-28` (67
records with 134 series, reserved before triage and extracted by seven custodians).
**Model 1.3.0 passed H2–H4 and failed H1 and H5.** Stacking improved both forecast
tasks, but its intervals covered too much at 80% and its initial state was 0.01 pp
worse than model 1.1.0's from the same fit. Model 1.1.0 stays the component of
`peykar eye`. Nothing here is a clinical or biological validation claim.

## Model

- Population fit: model 1.1.0 (`retina_thickness_population_1_1`, candidate `s5`, its
  series and dose rule), unchanged. Harness 6 (`peykar.validation.eye_recovery_evaluation6`)
  is harness 2 plus the predictive quantiles of every initial state.
- Stacking (`peykar.validation.eye_stacking_evaluation`, candidate `k1`):
  - forecasts from follow-up: 0.742 × model 1.1.0's Gaussian forecast + 0.233 × the
    last observation (persistence) + 0.025 × the pooled-curve increment;
  - initial states: 0.839 × model 1.1.0's initial state + 0.161 × the pooled training
    curve;
  - intervals: the model's quantiles moved to the stacked point, width unchanged.
  The weights are non-negative, sum to one and minimize the squared error of the 128
  studies' leave-one-study-out forecasts of human group means.
- Operator `scripts/evaluate_retina_population_1_3.py`, publisher
  `scripts/publish_retina_population_1_3.py`. Values are percentage points (pp) of each
  series' baseline thickness.

## Development (128 training studies)

The [development log](development-log.md) declared each round before its run. Rerun
by harness 6 in 16 shards, model 1.1.0 reproduced its earlier harness-2 report with 0
differences. In nested leave-one-study-out (each study stacked with weights fitted
without it), human group means:

| task | targets | model 1.1.0 | stacked (k1) |
|---|---|---|---|
| next observation | 493 | 7.193 | 7.154 |
| landmark | 493 | 9.376 | 9.228 |
| initial state | 565 | 12.892 | 12.854 |

Stacked next-observation coverage 0.866/0.937 (80%/90%). k1 passed the declared rule
(landmark gain 0.148 pp, no loss) and H1–H5 on development, with small margins.

## One-shot sealed test (2026-10-30)

The test ran once after freeze commit `fc3590ac`, with protocol
[`protocol-1.3.0.json`](protocol-1.3.0.json) (SHA `9fe7bcff…`; all 75 pinned files and
the 128 training and 67 sealed record hashes matched). Exit 0, 5,504 s; report
`dd6876d9…`, model 1.1.0's unstacked report from the same fit `32f02efc…`. Public
artifacts are in [`sealed-test-2026-10-30/`](sealed-test-2026-10-30/tables.md) with a
[figure](sealed-test-2026-10-30/sealed-results.png). Values of 28 studies are
redacted; `.peykar/check_stacking_redaction.py` found no value of a redacted study in
the public files (56 study reports and 177 per-series rows checked, 0 leaks).

Declared checks, human group means (96 forecast series, 257 next-observation and 257
landmark targets; 90 control group means with 310 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | **fail**: 234/257 = 0.911 at 80% (window 0.70–0.90); 249/257 = 0.969 at 90% (window 0.80–0.97) |
| H2 accuracy, next observation | **pass**: 5.70 pp against persistence 6.43, pooled-curve increment 7.89, analog kNN 7.87 (5.70 on its 251 targets) and the last-two trend 19.00 |
| H3 landmark | **pass**: 7.72 pp against persistence 10.67, pooled-curve increment 10.41 and analog kNN 11.88 (7.57 on its 251 targets) |
| H4 initial state | **pass**: 11.62 pp against the pooled training curve 15.44 |
| H5 against model 1.1.0, same fit | **fail**: next observation 5.70 against 6.08 and landmark 7.72 against 7.98 (both lower), but initial state 11.62 against 11.61 (0.01 pp higher) |

Reported, not tested: model 1.1.0's own checks on the same studies: H1 fails (0.903 at
80%, 0.965 at 90%), H2–H4 pass (next observation 6.08; landmark 7.98; initial state
11.61). Per series, 55 of 96 group means were within 5/10 pp for the next observation
(model 1.1.0: 49) and 34 for the landmark task (33); 38 of 96 had a larger landmark
RMSE with stacking. Landmark coverage 0.868/0.942; stacked initial-state coverage
286/310 = 0.923 and 300/310 = 0.968.

## What the test shows (reported after unsealing, not tested)

Commands: `.peykar/stacking_sealed_breakdown.py` and `.peykar/stacking_drop_one.py` on
the private reports.

- **Stacking the forecasts from follow-up worked on both tasks.** The gain in RMSE by
  treatment (human group means; model 1.1.0 minus stacked): next observation +0.12 pp
  after anti-VEGF (123 targets), +0.25 after steroid implants (39), +0.68 after
  combinations (49), +0.78 after steroid injections (19), +0.90 for other treatments
  (27); landmark −0.30, −0.38, +0.45, +2.16 and +2.06. Where the model already did
  well (anti-VEGF, implants), the persistence weight made long-range forecasts a little
  worse.
- **The initial-state stacking changed nothing.** With a weight of 0.16 on the pooled
  training curve the stacked initial state equals the model's to 0.01 pp; removing any
  one of 15 of the 67 studies would flip H5 to a pass. As for visual acuity, the
  pooled curve does not use the baseline thickness and adds no information.
- **Moving an interval to a better point without narrowing it widens coverage.** Model
  1.1.0's own 80% intervals already covered 0.903 here; centred on the stacked point
  they covered 0.911. A stacked version needs intervals rescaled to its own errors.

## Limits and next steps

- Tranche 3 is consumed; tranche 4 (35 articles) is untriaged and unread and could be
  the sealed set of a version that stacks only the forecasts from follow-up, with
  intervals scaled to the stacked errors on development.
- Treated groups use the control prior of their context; retreatment schedules, fluid
  and drug levels are not modelled.
