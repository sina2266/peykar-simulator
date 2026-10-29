# Refraction after corneal laser refractive surgery — model 1.0.0

Sixth eye component and fourth corneal one. It forecasts the spherical equivalent
(SE, dioptres) of a group of eyes after photorefractive keratectomy or another surface
ablation, LASIK, or SMILE and other lenticule extraction: from the preoperative SE
alone (initial state), from the first postoperative visit (landmark), or one visit
ahead (next observation). It was developed on the 24 development records of dataset
[`cornea-refraction-2026-10-29`](../../../data/cornea-refraction-2026-10-29/README.md)
(43 group-mean series) and tested once on 8 sealed records (14 series), reserved by a
salted hash before any value was read and extracted by two custodians. Nothing here
is a clinical or biological validation claim.

## Model

- **Additive outcome.** Refraction changes in dioptres, not as a fraction of its
  baseline like the five earlier components. The series of
  `peykar.validation.cornea_refraction_data` give the generic eye model the change
  from the preoperative SE as the exact pseudo-fraction `exp(change / 5 D)` and the
  preoperative SE as the pseudo-reference `exp(SE0 / 5 D)`. Generic eye model 2 and
  its harness therefore apply unchanged, and every comparator becomes its additive
  analogue. `peykar.validation.eye_additive_evaluation` 1.0.0 rewrites forecasts,
  observations and intervals in tenths of a dioptre and recomputes all scores:
  **one reported pp is 0.1 D**, so the per-series criteria read RMSE ≤ 0.5 D, maximum
  ≤ 1.0 D and, for forecasts, at least 0.05 D better than persistence.
- **Law.** `change(t) / 5 D = ell + e r(t) + s q(t)` with `r = exp(−(t − 1 day)/tau)`
  and `q = exp(−(t − 1 day)/7 days)`, and `ell = context level + b · SE0 / 5 D`. The
  fitted slope is b = −0.90 (SD 0.03): eyes keep on average 90% of the attempted
  correction, and the early postoperative offset settles within about a week.
- **Hierarchy (candidate f1, adopted).** Species → procedure (surface ablation, LASIK,
  lenticule extraction, other) → refractive error (myopia, hyperopia, mixed
  astigmatism), with a study effect per procedure arm. Configuration
  `peykar.model_learning.refraction_population`, operator
  `scripts/evaluate_refraction_population.py`.
- **Plano-target comparator.** The additive scale declares the target the operation
  aims at (plano, 0 D), so every initial-state entry and forecast row also carries
  `target_level`: full correction, the surgeon's planning assumption.

## Development (leave-one-study-out over the 24 studies)

The [development log](development-log.md) records each declaration before its run;
results are in [`development-loso-2026-10-29/`](development-loso-2026-10-29/tables.md)
(values of 11 studies redacted). Pooled RMSE in dioptres, group means:

| | f1 (adopted) | f0 (procedure only) | persistence | plano target |
|---|---|---|---|---|
| next observation (98 targets) | 0.157 | 0.153 | 0.163 | 0.616 |
| landmark (98 targets) | 0.322 | 0.326 | 0.348 | 0.616 |
| initial state (124 targets) | **0.538** | 0.552 | — | 0.575 |

The primary of both candidates is the robust variant (the Gaussian variant scored
0.27 D for the next observation). f1 was adopted by the declared rule: initial state
−0.145 pp, no task worse by more than 0.05 pp. The development gains over persistence
and over full correction were small: group-mean refraction after surgery is close to
plano and stable after the first weeks.

## One-shot sealed test (2026-10-29)

The test was run once, after freeze commit `a2eccf2c`, with protocol
[`protocol-1.0.0.json`](protocol-1.0.0.json) (SHA `8634f5a2…`), all 58 pinned files and
the 24 training and 8 sealed record hashes matching. The operator scored each sealed
study with f1 and, in the same run, with f0 as the declared reference; no sealed study
had author-linked training studies. Exit 0, 39.9 s, reports `4c3ca270…` (f1) and
`a05e719a…` (f0). Public artifacts are in
[`sealed-test-2026-10-29/`](sealed-test-2026-10-29/tables.md), with figure
`sealed-results.png`, its script `plot-results.py` and the declared analysis
`sealed-analysis.json`; values of 4 studies are redacted, and a check that loads the
publisher's licence classes found no value of a redacted study in the public files.

Declared checks, human group means (13 forecast series, 28 targets; 12 control series
with 34 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | 23/28 = 0.821 and 24/28 = 0.857: **both inside** (windows 0.70–0.90, 0.80–0.97) |
| H2 accuracy, next observation | **pass**: 0.293 D against persistence 0.370, plano target 0.524, analog kNN 0.800, pooled-curve offset 1.663 and last-two trend 1.897 |
| H3 landmark | **pass**: 0.468 D against persistence 0.780, plano target 0.524, analog kNN 2.448 and pooled-curve offset 3.205 |
| H4 initial state against the plano target | **pass**: 0.397 D against 0.501 on the same 34 targets; 11/12 groups within 0.5/1.0 D |
| H5 against f0 trained on the same studies | **pass**: initial state 0.397 against 0.413 D; next observation 0.293 against 0.290 (allowed +0.005 D); landmark 0.468 against 0.476 |

Per series, 12/13 group means were within 0.5/1.0 D for next observation and 3 of
them were also at least 0.05 D better than persistence; for landmark the counts were
12 and 3.

Reported, not tested (`.peykar/refraction10_sealed_breakdown.py`; RMSE and mean signed
error in D, signed error = forecast − observed):

- **Late regression is where the model helps.** After month 6 (7 targets) the plano
  target missed by 0.75 D (observed SE on average 0.44 D more myopic than plano). The
  model missed by 0.20 D one visit ahead, 0.63 D from the first visit (landmark) and
  0.51 D from the preoperative SE alone (6 targets). In months 1–6 the landmark error
  was 0.42 D against persistence 0.66 and plano 0.46.
- **The model expects only part of the regression after severe myopia.** A public LASIK
  group with preoperative SE −7.95 D (figure, lower left) regressed to −1.77 D at month
  6 and −1.88 D at month 12. Its landmark forecast from day 3 was −0.21 and −0.29 D, and
  the observed values lie below the 90% intervals (lower bounds −1.04 and −1.10 D). The
  initial state, from the preoperative SE alone, forecast −0.66 and −0.71 D.
- **By procedure.** LASIK (19 forecast targets) carried most of the error: next
  observation 0.35 D against persistence 0.45 and plano 0.63. After lenticule
  extraction (7 targets) persistence was slightly better (0.08 against 0.10 D), and in
  the initial state full correction was better (0.14 against 0.25 D on 6 targets).
  Surface ablation had only 2 forecast targets.
- **Bias.** Initial-state forecasts were 0.13 D too myopic on average; the plano
  target was 0.09 D too hyperopic.
- **Coverage.** Landmark coverage 0.821/0.857; initial-state coverage 32/34 at both
  levels (wide intervals).

## Limits

- The sealed test is small: 8 studies and 13 forecast series. Its verdicts are on
  pooled targets and weigh LASIK most.
- Every sealed series is myopic; hyperopic and mixed-astigmatism corrections had six
  training series and were not tested.
- Treated groups (prophylactic cross-linking, goggles, a new technique) use the
  control prior of their context; no treatment effect is modelled.
- The model knows the procedure, the refractive error and the preoperative SE, not the
  laser, nomogram, optical zone, ablation depth or the surgeon. Between studies the
  long-term change varies with an SD of about 0.48 D, which limits the initial state.
- Single eyes, non-plano targets, enhancement decisions, visual acuity and
  astigmatism are out of scope.
