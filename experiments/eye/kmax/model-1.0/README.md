# Maximum keratometry after cross-linking or ring segments — model 1.0.0

Ninth eye component and sixth corneal one. It forecasts the maximum keratometry (Kmax,
the steepest anterior corneal power, in dioptres) of a group of eyes with keratoconus or
corneal ectasia after corneal collagen cross-linking (any protocol), intracorneal ring
segments, both, or under observation: from the baseline Kmax alone (initial state), from
the first post-treatment visit (landmark), or one visit ahead (next observation). It was
developed on the 25 development records of dataset
[`cornea-kmax-2026-10-30`](../../../data/cornea-kmax-2026-10-30/README.md) (35 series)
and tested once on 17 sealed records (33 series), reserved by a salted hash before any
value was read and extracted by three custodians. **It failed two of its four declared
hypotheses (H3 and H4) and is not offered by `peykar eye`.** Nothing here is a clinical
or biological validation claim.

## Model

- **Additive outcome, scored in tenths of a dioptre.** Kmax changes additively. As for
  refraction and acuity, the series of `peykar.validation.cornea_kmax_data` give generic
  eye model 2 the change from the baseline Kmax as the exact pseudo-fraction
  `exp(change / 5 D)` and the baseline as `exp(Kmax0 / 5 D)`, and
  `peykar.validation.eye_additive_evaluation4` rescores everything in tenths of a
  dioptre: **one reported pp is 0.1 D**, so the per-series criteria read RMSE ≤ 0.5 D,
  maximum ≤ 1.0 D and, for forecasts, at least 0.05 D better than persistence.
- **Law (candidate k6).** `change(t) = ell + e r(t)` (in units of 5 D) with
  `r = exp(−(t − 1 day)/tau)` and `ell = context level + b (Kmax0 − 55 D)/5 D`; no early
  transient. Forecast intervals use the pooled residual variance of the training
  studies; likelihoods keep the median noise centre.
- **Hierarchy.** Species → procedure (epithelium-off, accelerated and transepithelial
  cross-linking, cross-linking combined with another treatment, ring segments,
  observation, other), with a study effect per procedure arm. Configuration
  `peykar.model_learning.kmax_population`, operator `scripts/evaluate_kmax_population.py`,
  publisher `scripts/publish_kmax_population.py`.

## Development (leave-one-study-out over the 25 studies)

The [development log](development-log.md) records each declaration before its run.
Pooled RMSE in pp (0.1 D), human group means (39 forecast targets, 60 initial-state
targets):

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|
| k0 generic constants | 7.053 | 11.433 | 15.574 | 0.692 / 0.744 |
| k1 no 7-day transient | 6.518 | 11.078 | 15.406 | 0.667 / 0.718 |
| k2 family level | 6.763 | 11.599 | 15.513 | 0.718 / 0.821 |
| k3 support hold | 6.795 | 10.995 | 15.357 | 0.692 / 0.744 |
| k4 persistent deviations | 6.748 | 11.452 | 15.574 | 0.667 / 0.769 |
| k5 one-month early term | 7.127 | 15.705 | 17.660 | 0.718 / 0.846 |
| **k6** k1 + forecast-only pooled noise (adopted) | **6.145** | **11.113** | **15.412** | **0.718 / 0.872** |
| k7 k1 + pooled noise | 7.723 | 11.548 | 16.183 | 0.692 / 0.744 |
| persistence | 6.592 | 12.364 | — | — |
| pooled-curve increment | 7.992 | 12.175 | — | — |
| pooled training curve | — | — | 13.889 | — |

Round 1 found the generic 7-day transient fitted from almost no data (about −2.5 D at
day 1, with an SD of about 3.8 D) and the forecast intervals too narrow; removing the transient
improved every task but left the coverage below its window. Round 2 adopted k6. On
development k6 passed H1–H3 and **failed H4**: the population initial state was worse
than the pooled training curve (15.41 against 13.89 pp), mainly for transepithelial
cross-linking, untreated eyes and the combined treatments; ring segments had the largest
errors under both. Public development artifacts are in
[`development-loso-2026-10-30/`](development-loso-2026-10-30/tables.md) (values of 13
studies redacted).

## One-shot sealed test (2026-10-30)

The test was run once, after freeze commit `9fe023ae`, with protocol
[`protocol-1.0.0.json`](protocol-1.0.0.json) (SHA `9a20b859…`), all 82 pinned files and
the 25 training and 17 sealed record hashes matching. Exit 0, 43.5 s, report
`13d80e8b…`. Public artifacts are in
[`sealed-test-2026-10-30/`](sealed-test-2026-10-30/tables.md), with figure
`sealed-results.png`, its script `plot-results.py` and the declared analysis
`sealed-analysis.json`; values of 7 studies are redacted, and
`.peykar/check_kmax_redaction.py` found no value of a redacted study in the public
files.

Declared checks, human group means (20 forecast series, 55 targets; 33 control series
with 88 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | 41/55 = 0.745 and 47/55 = 0.855: **both inside** (windows 0.70–0.90, 0.80–0.97) |
| H2 accuracy, next observation | **pass**: 10.20 pp against persistence 11.91, pooled-curve increment 10.73, analog kNN 12.07 and last-two trend 27.40 |
| H3 landmark | **FAIL**: 12.60 pp against the pooled-curve increment 12.35 (it beats persistence 16.65 and analog kNN 14.28) |
| H4 initial state | **FAIL**: 14.61 pp against the pooled training curve 14.17 on the same 88 targets |

Per series, 9 of 20 group means were within 5/10 pp (0.5/1.0 D) for the next
observation and 3 of them also at least 0.5 pp better than persistence; for the landmark
task the counts were 9 and 4. Landmark coverage 0.709 (80%) and 0.836 (90%);
initial-state coverage 0.818 and 0.875 (reported, not tested).

Reported, not tested (post hoc, `.peykar/kmax10_sealed_breakdown.py` and
`.peykar/kmax_diagnostics.py`; RMSE in pp):

- **One study carries both failures.** A CC BY study of 12 eyes treated under 18 years
  (epithelium-off, baseline Kmax 57.1 D) kept flattening for five years, to about
  −5.5 D. The law levelled off after the first year: from the 6-month visit it forecast
  about −2.2 D for every later visit, outside its 90% interval at month 18 and at every
  visit from month 36 (6 of 8 targets; figure, lower right). The pooled-curve increment kept part of the long-term flattening of the
  longest development series. Without this study, H3 would pass (8.94 against 12.64) and
  H4 too (11.89 against 12.85); no other single study changes either verdict.
- **Where the model helps.** Landmark forecasts after accelerated cross-linking 12.39 pp
  against the pooled-curve increment 14.45 and persistence 17.34, after transepithelial
  cross-linking 8.69 against 11.05 and 13.42, after the combined treatment 3.03 against
  6.41. A large public study with conventional and accelerated groups (93 and 76 eyes)
  was followed to five years within its intervals (figure, lower left), where the
  pooled-curve increment expected more flattening than occurred.
- **Where it does not.** After epithelium-off cross-linking the landmark error is 14.64
  pp against 12.08 for the pooled-curve increment, and the initial state 15.20 against
  13.69: the law expects less flattening than observed (bias +0.8 D). The single series
  of the untrained context `other` (an epithelial-island protocol) used the species
  mean and missed by 1.5 D, against 0.7 D for the pooled curve.
- **Sealed and development sets differ.** The sealed set has no ring-segment study, 26
  of its 33 series are epithelium-off or accelerated cross-linking, and 13 series have a
  single follow-up visit (initial state only).

## Limits and next steps

- The law approaches a long-term level along one exponential, with a time constant
  shared through the population. Kmax after cross-linking can keep flattening slowly for
  years, so a single exponential levels off too early for such groups. A next version
  needs a second, slow component (or a law in log time) whose rate is learned from the
  long development series, and a new screen: every article of this dataset is now
  consumed.
- The model knows the treatment context and the baseline Kmax only. It does not know
  age (the failing study treated eyes under 18), stage, cone location, epithelial
  thickness, segment type or the cross-linking dose.
- Treated groups use the control prior of their context; per-eye series were not
  tested (the sealed set has none).
- Progression risk of untreated eyes, treatment failure, visual acuity and refraction
  are out of scope.
