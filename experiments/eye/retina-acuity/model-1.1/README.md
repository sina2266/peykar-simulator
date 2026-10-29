# Best-corrected visual acuity after treatment of macular disease — model 1.1.0

Version 1.1 of the eighth eye component. Model 1.0.0 failed its sealed test because
its 7-day early transient had no training visit before day 30
([model 1.0](../model-1.0/README.md)). Model 1.1.0 removes that term. It was developed
on the 24 tranche-1 record studies (the 15 development records of 1.0 and the 9
records consumed by its sealed test) and tested once on tranche 2 of dataset
[`retina-acuity-2026-10-29`](../../../data/retina-acuity-2026-10-29/README.md): 29
records with 81 series, reserved before triage and extracted by eight custodians. **Model
1.1.0 passed H1 and failed H2, H3, H4 and H5.** Model 1.0.0, scored in the same run,
failed all four of its own checks on the same studies. Neither version is offered by
`peykar eye`. Nothing here is a clinical or biological validation claim.

## Model

- Generic eye model 2 with the configurations of
  `peykar.model_learning.retina_acuity_population_1_1`, scored in ETDRS letters by
  `peykar.validation.eye_additive_evaluation2` (one reported pp is one letter, 0.02
  logMAR), as in model 1.0.
- **Law of 1.1.0 (candidate w1):** `change(t) = ell + e r(t)` (logMAR), with
  `r = exp(−(t − 1 day)/tau)` and `ell = context level + b · logMAR0`; contexts
  `human/<indication>/<treatment>`, a study effect per context arm, forecast intervals
  from the pooled study noise. Every other setting is that of 1.0.0.
- Primaries: `population_anchored_t4` for the next observation and the landmark task
  (the level is estimated from the series' own visits with a flat prior, the curve's
  shape comes from the population), `population_initial_state` for the initial state.
- Operator `scripts/evaluate_retina_acuity_population_1_1.py`, publisher
  `scripts/publish_retina_acuity_population_1_1.py` (loads the frozen 1.0 publisher;
  adds H5 and 1.0.0's own H1–H4, reported only).

## Development (leave-one-study-out over the 24 training studies)

The [development log](development-log.md) declared each candidate and the adoption
rule before its run. Human group means: 100 next-observation and landmark targets, 117
initial-state targets. Pooled RMSE in letters:

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|
| w0 = 1.0.0 | 4.327 | 5.827 | 7.615 | 0.770 / 0.830 |
| **w1 no early transient (adopted)** | 3.460 | 3.965 | 4.873 | 0.840 / 0.910 |
| w2 treatment fallback | 4.327 | 5.814 | 7.613 | 0.770 / 0.830 |
| w3 dexamethasone wear-off | 4.515 | 6.516 | 8.790 | 0.770 / 0.850 |
| persistence | 3.468 | 4.463 | — | |
| pooled-curve increment | 3.805 | 5.781 | — | |
| pooled training curve | — | — | 5.524 | |

Almost all of w1's gain came from the one training study with day-1 visits. On development
w1 passed H1–H5, but H2 against persistence only by 0.01 letters (3.46 against 3.47).

## One-shot sealed test (2026-10-29)

The test ran once after freeze commit `7fce2b4f`, with protocol
[`protocol-1.1.0.json`](protocol-1.1.0.json) (SHA `c0c37c7e…`; all 87 pinned files and
the 24 training and 29 sealed record hashes matched). Exit 0, 169.7 s, report
`eb8110f1…`, reference (1.0.0) `bd3cb6b6…`. Public artifacts are in
[`sealed-test-2026-10-29/`](sealed-test-2026-10-29/tables.md): both public reports,
the declared analysis `sealed-analysis.json`, figure `sealed-results.png` and its script
`plot-results.py`. Values of 14 studies are redacted (licences without republication
rights); `.peykar/check_retina_acuity_redaction_1_1.py` found no value of a redacted
study in the public files (28 study reports and 152 per-series rows checked, 0 leaks).

Declared checks, human group means (50 forecast series, 106 targets; 45 control group
means with 134 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | **pass**: 77/106 = 0.726 and 93/106 = 0.877 (windows 0.70–0.90 and 0.80–0.97) |
| H2 accuracy, next observation | **fail**: 3.95 letters against persistence 3.00 and the pooled-curve increment 2.80 (better than analog kNN 5.42 and the last-two trend 14.61) |
| H3 landmark | **fail**: 6.24 letters against persistence 3.56 and the pooled-curve increment 3.06 (better than analog kNN 10.12) |
| H4 initial state | **fail**: 6.65 letters against the pooled training curve 5.87 |
| H5 against 1.0.0, same training | **fail**: next observation 3.95 against 3.61, landmark 6.24 against 5.01, initial state 6.65 against 6.52 |

Reported, not tested: model 1.0.0's own H1–H4 all fail on the same studies (coverage
0.585 / 0.736; next observation 3.61 against persistence 3.00; landmark 5.01 against
3.56; initial state 6.52 against 5.87). Per series, 43 of 50 group means were within
5/10 letters for the next observation and none of them was also 0.5 letter better than
persistence; for the landmark task 39 and 4; for the initial state 27 of 45 control
groups (the pooled training curve 31). Landmark coverage 0.764 / 0.934 and
initial-state coverage 0.604 / 0.739. The three assimilation variants had lower
landmark errors on this set than the anchored primary (4.72–5.27 letters against 6.24)
and, except the Gaussian one, lower next-observation errors (3.60 and 3.69 against
3.95); all were still worse than persistence and the pooled-curve increment.

## Why it failed (reported after unsealing, not tested)

Commands: `.peykar/retina_acuity11_sealed_breakdown.py` and inline reads of the private
reports. None of this changes a verdict, and the subsets below were chosen after the
results were seen.

- **The same identification problem, through another term.** Without the transient,
  the recovery term `e r(t)` took its place. In the fitted population (published in
  `report-public.json`, `population_fit`) its time constant is about 7.5–13 days in
  every context (species median 9.2 days), so by month 1 the term has almost vanished
  and the law is a constant level after that; its size in the first weeks is set by the
  priors and the few visits near month 1. The long-term levels of the nine trained
  contexts lie between 5.3 and 6.4 letters of gain at month 6, so after month 1 the
  contexts barely differ. In the DME steroid-implant context the mean
  parameters give a gain of about 32 letters at day 1 and 21 at week 1, falling to 7 at
  month 1 and 6 at month 6. The decline after month 1 matches the waning of a
  dexamethasone implant, but that context's earliest training visit is at month 2, so
  nothing constrains the first weeks. In DME with anti-VEGF the same term gives a loss of
  about 16 letters at day 1 (earliest training visit at month 1).
- **Two sealed studies had visits on day 3.** Both are DME groups after one
  dexamethasone implant, under licences that do not permit republishing their values,
  and they are an author-linked pair (same design and visit times). The anchored
  forecast takes the day-3 visit as the series' own level, so the difference between
  that visit and the population curve's large day-3 gain is carried into every later
  forecast. Their series RMSEs (per-series metrics, public): landmark 19.45 and 18.85
  letters (1.0.0: 4.08 and 2.72; persistence 5.90 and 5.08), next observation 9.74 and
  8.68 (1.0.0: 3.62 and 3.08), initial state 11.75 and 11.38 (1.0.0: 4.28 and 2.71;
  pooled training curve 2.87 and 2.61). The figure (upper right) shows them as the two
  red points far above the diagonal.
- **Training time support.** Splitting the targets by whether the forecast origin is
  before the earliest training control visit of the series' context (the support that
  `peykar eye` now reports): from origins inside the support, 1.1 has 2.78 letters for
  the next observation (77 targets; persistence 2.60, pooled-curve increment 2.48,
  1.0.0 3.25) and 3.02 for the landmark task (61 targets; 3.14, 2.91, 3.64); from
  origins before it, 6.04 (29 targets; 3.86, 3.52, 4.42) and 8.90 (45 targets; 4.07,
  3.26, 6.42). Initial states inside the support: 5.94 against the pooled curve 5.83
  (104 targets); before it 8.67 against 5.99 (30).
- **Sensitivity.** Without the linked dexamethasone pair, H5 would pass (3.15 against
  3.63, 3.48 against 5.12, 6.08 against 6.70) and H1 would still pass, but H2–H4 would
  still fail (next observation 3.15 against persistence 2.87 and the increment 2.77;
  landmark 3.48 against 3.36 and 3.08; initial state 6.08 against 6.05). Removing any
  single study changes no H1–H4 verdict.
- **Where 1.1 helps.** A public nAMD group switched to brolucizumab has visits at day 1,
  week 1, week 2 and month 1 (figure, lower left). From the day-1 visit, 1.1 forecasts the
  later visits within about 2 letters (landmark RMSE 1.42; persistence 1.94), while
  1.0.0 is 6–7 letters off (6.80), the failure mode of 1.0. Its initial state is still
  6–7.5 letters too optimistic for these 12 previously treated eyes.

## What this shows

- On this set, BCVA group means move little after the first months, and the two
  simplest comparators, persistence and the pooled-curve increment (the pooled training
  curve's change added to the last visit), beat both model versions. Context curves
  estimated from one to seven training studies did not beat one pooled curve: the
  pooled training curve was better for the initial state overall (5.87 against 6.65
  letters) and level with it inside the time support (5.83 against 5.94).
- Removing one unidentified term is not enough: any fast term can be fitted where the
  training data have no visits. A law needs a check before the freeze that the training
  visits identify each term in every context it is used for, or a rule for times before
  a context's earliest training visit (for example, holding the curve's shape to the
  pooled curve there, or interpolating from the baseline to the first supported month).
- A flat-level (anchored) forecast trusts the first visit completely. When that visit
  lies outside the support, its error persists in every later forecast.

## Limits and next steps

- Tranche 2 is now consumed. Tranches 3–4 of the dataset (138 articles) are untriaged and
  unread and can serve as the sealed set of a later version, reserved before triage.
- Candidates worth declaring for that version: the support rule above; context curves
  shrunk toward the pooled curve when a context has few training studies; the pooled-
  curve increment as a forecaster in its own right; steroid-implant wear-off with
  early-visit training data.
- Treated groups use the control prior of their context; per-eye series, regimens,
  injection counts, switches and retinal anatomy are not modelled.
- The custody verifier counted 13 public and 16 private records because one
  custodian wrote the licence class under the field name `license_class`; the frozen
  publisher's licence rule (the one that decides republication) classes 15 records as
  public, and both custodian fields agree with it.
