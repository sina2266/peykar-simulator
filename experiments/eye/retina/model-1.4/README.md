# Central retinal thickness after treatment — model 1.4.0

Version 1.4 of the fifth eye component. Model 1.3.0 (stacking 1.0.0) improved the
forecasts from follow-up on its sealed test but failed H1 (its moved intervals covered
too much) and H5 (its stacked initial state was not better). Model 1.4.0 keeps model
1.1.0's fit and uses **stacking 2.0.0** (`peykar.model_learning.eye_stacking2`): only
the forecasts from follow-up are stacked, initial states stay model 1.1.0's own, and
each stacked interval is model 1.1.0's interval scaled by a factor chosen on
development forecasts. It was developed on the 195 record studies of tranches 1–3 and
tested once on tranche 4 of dataset `retina-cst-2026-10-28` (17 records with 26
series, reserved before triage and extracted by three custodians). **Model 1.4.0
passed H1, H2 and H4 and failed H3 and H5.** Its next-observation intervals were well
calibrated (0.793 at 80%), but its landmark forecasts were worse than model 1.1.0's
(10.58 against 9.99 pp) and worse than the pooled-curve increment (9.80). Model 1.1.0
stays the component of `peykar eye`. Nothing here is a clinical or biological
validation claim.

## Model

- Population fit: model 1.1.0 (`retina_thickness_population_1_1`, candidate `s5`, its
  series and dose rule), unchanged, evaluated by harness 6.
- Stacking (`peykar.validation.eye_stacking_evaluation2`, candidate `k3`):
  - forecasts from follow-up: 0.729 × model 1.1.0's Gaussian forecast + 0.212 × the
    last observation (persistence) + 0.058 × the pooled-curve increment; weights
    non-negative, summing to one, minimizing the squared error of the 195 studies'
    leave-one-study-out forecasts of human group means;
  - intervals: model 1.1.0's quantiles q with point m become `s + c (q − m)` at the
    stacked point s, with c = 0.79 for next-observation forecasts and c = 1.00 for
    landmark forecasts. Each c was chosen on the 0.30–3.00 grid so that the pooled 80%
    coverage of that task's development intervals is closest to 0.80;
  - initial states: model 1.1.0's own (not stacked).
- Operator `scripts/evaluate_retina_population_1_4.py`, publisher
  `scripts/publish_retina_population_1_4.py`. Values are percentage points (pp) of each
  series' baseline thickness.

## Development (195 training studies)

The [development log](development-log.md) declared each round before its run. Model
1.1.0 was rerun on the 195 studies by harness 6 in 16 shards; the stacking code was
committed before the shards started. Nested leave-one-study-out (each study stacked
with weights and scales fitted without it), human group means, 750 targets per task:

| task | model 1.1.0 | stacked |
|---|---|---|
| next observation | 6.752 | 6.623 |
| landmark | 8.850 | 8.673 |

- Round 1, k2 (one scale, 0.79, for both tasks): met the declared adoption rule, but
  its landmark intervals covered only 0.692 at 80%. The scale mainly corrected model
  1.1.0's own next-observation intervals, which were too wide (0.861 at 80%), while
  its landmark intervals were already near nominal (0.805).
- Round 2, k3 (one scale per task; declared after round 1, before its run): the same
  points and next-observation intervals; landmark scale 1.00 and landmark coverage
  0.791/0.897. Adopted. On development it passed H1–H5: coverage 0.797/0.877; next
  observation 6.62 against persistence 7.74; landmark 8.67 against 11.36; H5 6.62 and
  8.67 against model 1.1.0's 6.75 and 8.85.

## One-shot sealed test (2026-10-30)

The test ran once after freeze commit `c1dbd266`, with protocol
[`protocol-1.4.0.json`](protocol-1.4.0.json) (SHA `ea054039…`; all 81 pinned files and
the 195 training and 17 sealed record hashes matched). Exit 0, 1,760 s; report
`dd3803cc…`, model 1.1.0's unstacked report from the same fit `e3a2baae…`. Public
artifacts are in [`sealed-test-2026-10-30/`](sealed-test-2026-10-30/tables.md) with a
[figure](sealed-test-2026-10-30/sealed-results.png). Values of 11 of the 17 studies are
redacted; `.peykar/check_stacking_redaction.py` found no value of a redacted study in
the public files (22 study reports and 48 per-series rows checked, 0 leaks).

Declared checks, human group means (24 forecast series, 87 next-observation and 87
landmark targets; 20 control group means with 85 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | **pass**: 69/87 = 0.793 at 80% (window 0.70–0.90); 73/87 = 0.839 at 90% (window 0.80–0.97) |
| H2 accuracy, next observation | **pass**: 5.84 pp against persistence 7.00, pooled-curve increment 6.65, analog kNN 6.58 and the last-two trend 13.78 |
| H3 landmark | **fail**: 10.58 pp against persistence 16.13 and analog kNN 11.65 (lower), but the pooled-curve increment 9.80 (higher) |
| H4 initial state | **pass**: 10.98 pp (model 1.1.0's own) against the pooled training curve 15.14 |
| H5 against model 1.1.0, same fit | **fail**: landmark 10.58 against 9.99 (must be lower); next observation 5.84 against 5.83 (allowed: +0.05) |

Reported, not tested:

- **Model 1.1.0's own checks on the same studies.** H1 passes (0.839/0.920), H2 and H4
  pass, and H3 fails as well: 9.99 pp against the pooled-curve increment's 9.80.
- **Landmark coverage** was low for both models: 0.690/0.782 for model 1.4.0 and
  0.713/0.793 for model 1.1.0 (windows 0.70–0.90 and 0.80–0.97).
- Per series, 12 of 24 group means were within 5/10 pp for the next observation (model
  1.1.0: 13) and 10 for the landmark task (11); 16 of 24 had a larger landmark RMSE with
  stacking.

## What the test shows (reported after unsealing, not tested)

Commands: `.peykar/stacking_sealed_breakdown.py`, `.peykar/stacking_drop_one.py` and
`.peykar/posthoc_stacking_14.py` on the private reports.

- **On tranche 4 the persistence weight hurt the landmark forecasts.** Here the
  thickness kept changing after the first post-treatment visit: persistence from that
  visit had a landmark RMSE of 16.13 pp, 1.61 times model 1.1.0's 9.99 (on development
  11.34 against 8.85, 1.28 times). The losses were largest after combination
  treatments (landmark 7.28 against 4.81 pp after nAMD combinations) and in treated
  groups (5.57 against 3.99; persistence 17.98). By treatment (model 1.1.0 minus
  stacked), next observation −0.25 pp after anti-VEGF (44 targets), +0.56 after steroid
  implants (16), −0.22 after combinations (21) and +0.06 for other treatments (6);
  landmark −0.60, −0.29, −1.64 and +0.71.
- **It was not one study.** Removing any single sealed study leaves H5 failed.
- **Other weights would not have helped.** Post hoc, weights fitted on development
  landmark rows only (0.73 model, 0.22 persistence, 0.05 increment) give 10.63 pp on
  tranche 4, and weights fitted by landmark horizon band are no better than one set of
  weights; model 1.1.0 alone gives 9.99.
- **The scaled next-observation interval worked.** The scale 0.79 moved 80% coverage
  from model 1.1.0's 0.839 to 0.793, close to nominal; the 90% interval covered 0.839,
  near the lower edge of its window.

Across the three sealed tests of stacking, the forecasts from follow-up gained in two
and lost in one (visual acuity 1.2: next observation +0.34 and landmark +0.92 letters;
retinal thickness 1.3: +0.38 and +0.26 pp; retinal thickness 1.4: −0.01 and −0.59 pp).
A fixed persistence weight learned on development data helps when the new studies'
trajectories are as flat after the first visit as the development studies' were, and
hurts when they change more.

## Limits and next steps

- Tranche 4 is consumed; no unread article of this dataset remains. A later retina
  version needs a new literature screen whose test part is reserved before triage.
- Treated groups use the control prior of their context; retreatment schedules, fluid
  and drug levels are not modelled.
