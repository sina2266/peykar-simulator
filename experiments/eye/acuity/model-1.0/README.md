# Corrected visual acuity after keratoplasty and cross-linking — model 1.0.0

Seventh eye component and fifth corneal one. It forecasts the corrected distance
visual acuity (BCVA or CDVA, logMAR) of a group of eyes after penetrating keratoplasty
(PK), deep anterior lamellar keratoplasty (DALK), endothelial keratoplasty (DSAEK,
DMEK) or corneal collagen cross-linking (CXL, alone or combined with a ring segment or
a refractive ablation): from the preoperative acuity alone (initial state), from the
first postoperative visit (landmark), or one visit ahead (next observation). It was
developed on the 27 development records of dataset
[`cornea-acuity-2026-10-29`](../../../data/cornea-acuity-2026-10-29/README.md)
(62 series) and tested once on 15 sealed records (25 series), reserved by a salted
hash before any value was read and extracted by three custodians. Nothing here is a
clinical or biological validation claim.

## Model

- **Additive outcome, scored in letters.** Acuity in logMAR changes additively. As
  for refraction, the series of `peykar.validation.cornea_acuity_data` give generic eye
  model 2 the change from the preoperative acuity as the exact pseudo-fraction
  `exp(change / 1 logMAR)` and the preoperative acuity as `exp(logMAR0)`, and
  `peykar.validation.eye_additive_evaluation` rescores everything in ETDRS letters:
  **one reported pp is one letter (0.02 logMAR)**, so the per-series criteria read
  RMSE ≤ 5 letters (one line), maximum ≤ 10 letters and, for forecasts, at least 0.5
  letter better than persistence. Letter scores in the sources are converted exactly
  (logMAR = (85 − letters)/50).
- **Law.** `change(t) = ell + e r(t) + s q(t)` (logMAR) with
  `r = exp(−(t − 1 day)/tau)` and `q = exp(−(t − 1 day)/7 days)`, and
  `ell = context level + b · logMAR0`. Fitted on all development controls, b = −0.67:
  a group with a worse preoperative acuity is expected to gain more.
- **Hierarchy (candidate a0).** Species → procedure (pk, dalk, dsaek, dmek, cxl,
  other), with a study effect per procedure arm. Configuration
  `peykar.model_learning.acuity_population`, operator
  `scripts/evaluate_acuity_population.py`, publisher
  `scripts/publish_acuity_population.py`.

## Development (leave-one-study-out over the 27 studies)

The [development log](development-log.md) records each declaration before its run.
Pooled RMSE in letters, group means (118 forecast targets, 155 initial-state targets):

| candidate | next observation | landmark | initial state |
|---|---|---|---|
| a0 (adopted) | 5.434 | 7.588 | 15.918 |
| a1 indication level | 5.332 | 7.283 | 16.107 |
| a2 one hierarchy per operation family | 5.320 | 7.346 | 16.213 |
| a4 a0 + forecast-only persistent deviations | 5.387 | 7.569 | 15.918 |
| persistence | 6.119 | 10.594 | — |
| pooled-curve increment | 5.130 | 7.179 | — |
| pooled training curve | — | — | 16.804 |

a1 and a2 gained in both forecast tasks but lost more than 0.05 letters in the initial
state; a4 gained less than 0.10. a2 did fix cross-linking (its own slope −0.10, initial
state 6.68 → 3.55 letters) while making keratoplasty worse. On development a0 **failed
H2 and H3** against the pooled-curve increment (the last observed value plus the mean
training change of its context). The errors there come from keratoplasty groups that
recover far less than the law expects for their preoperative acuity (an nDSAEK cohort
at logMAR 1.8, PK with scleral IOL fixation at logMAR 1.9): the model reads their early
shortfall mostly as a transient, while the comparator keeps it. The between-study SD of
the long-term level is 0.19 logMAR (about 9 letters), which bounds the initial state.
The hypotheses kept the comparators of the other eye components anyway. Public
development artifacts are in
[`development-loso-2026-10-29/`](development-loso-2026-10-29/tables.md) (values of 10
studies redacted).

## One-shot sealed test (2026-10-29)

The test was run once, after freeze commit `baafe4cd`, with protocol
[`protocol-1.0.0.json`](protocol-1.0.0.json) (SHA `0c60925c…`), all 62 pinned files and
the 27 training and 15 sealed record hashes matching. Exit 0, 52.1 s, report
`194c4ee7…`. Public artifacts are in
[`sealed-test-2026-10-29/`](sealed-test-2026-10-29/tables.md), with figure
`sealed-results.png`, its script `plot-results.py` and the declared analysis
`sealed-analysis.json`; values of 6 studies are redacted, and a check that loads the
publisher's licence classes found no value of a redacted study in the public files.

Declared checks, human group means (21 forecast series, 41 targets; 20 control series
with 59 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | 34/41 = 0.829 and 38/41 = 0.927: **both inside** (windows 0.70–0.90, 0.80–0.97) |
| H2 accuracy, next observation | **pass**: 5.00 letters against persistence 5.06, pooled-curve increment 6.07, analog kNN 7.78 and last-two trend 75.41 |
| H3 landmark | **pass**: 5.24 letters against persistence 7.27, pooled-curve increment 7.25 and analog kNN 11.85 |
| H4 initial state | **pass**: 9.04 letters against the pooled training curve 9.79 on the same 59 targets |

Per series, 16/21 group means were within 5/10 letters for the next observation and 7
of them also at least 0.5 letter better than persistence; for the landmark task the
counts were 13 and 7.

Reported, not tested (`.peykar/acuity10_sealed_breakdown.py`; RMSE and mean signed
error in letters, negative = forecast better acuity than observed):

- **The sealed set differs from development.** Of its 25 series, 12 follow
  cross-linking, 8 DMEK (4 of them per-eye), 3 cross-linking combined with a ring
  segment or PRK, and one each DSAEK and DALK; there is no PK. The keratoplasty groups that drove the development failure
  are nearly absent, so the pass does not show that the development weakness is gone.
- **Where the model helps.** Landmark forecasts after DMEK 5.13 letters against
  persistence 7.99 and the pooled-curve increment 9.36; after DALK 5.69 against 14.11
  (the only DALK training series has one follow-up at day 1); combined cross-linking
  4.74 against 8.19 and 6.11; after month 12, 5.34 against 8.63 and 9.64.
- **Where it does not.** After cross-linking alone the three methods are equal for the
  next observation (4.87, 4.93, 4.97) and the model is worse for the landmark task
  (4.68 against persistence 4.45 and the increment 4.08). For the next observation
  overall the gain over persistence is 0.06 letters, too small to matter.
- **The DSAEK miss.** One DSAEK group after glaucoma surgery (mixed indications,
  preoperative logMAR 1.60; figure, lower right) reached logMAR 1.06 at month 3 and
  1.16 at month 12. The initial state forecast 0.69 and 0.59 (23.6 letters too good on
  average) and the landmark forecast 0.90 at month 12, outside its 90% interval: the
  development failure mode.
- **Initial state by procedure.** Cross-linking 6.64 letters against the pooled
  training curve 5.50; DMEK 12.90 against 10.11 (the model expected less recovery than
  observed, bias +8.2 letters); DALK 6.81 against 25.54; DSAEK 24.04 against 7.70.
- **Coverage.** Landmark coverage 0.902/0.951; initial-state coverage 55/59 and 58/59.
- **All series.** With the 4 per-eye series (reported only), next-observation RMSE is
  5.61 letters against persistence 5.53.

## Limits

- The sealed test is small: 15 studies and 21 group-mean series, most after
  cross-linking and DMEK. PK and DSAEK, where the model failed on development, are
  hardly tested.
- The model knows the procedure and the preoperative acuity only. It does not know the
  indication, ocular comorbidity (glaucoma surgery, retinal disease), graft type
  details, suture removal or the age of the eyes; groups with comorbidities recover
  less than the law expects.
- Treated groups use the control prior of their context; per-eye series and treated
  groups were scored but not tested.
- Uncorrected or near acuity, graft survival, rejection, refraction and single-eye
  decisions are out of scope.
- Next version: a forecast that keeps an early shortfall (correlated level and
  transient, or the population curve's increment from the last visit), tested on the
  55 unread articles of tranche 2.
