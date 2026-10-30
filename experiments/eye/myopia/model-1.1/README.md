# Axial length in childhood myopia — model 1.1.0

Second version of the tenth eye component: the ocular axial length (AL, millimetres) of a
group of myopic children or adolescents under single-vision correction or a
myopia-control treatment, forecast from the baseline axial length alone (initial state),
from the first follow-up visit (landmark) or one visit ahead (next observation). It was
developed on the 33 tranche-1 records of dataset
[`myopia-axial-2026-10-30`](../../../data/myopia-axial-2026-10-30/README.md) (the 22
development records of model 1.0 and the 11 records its test consumed; 82 series) and
tested once on tranche 2 (26 records, 60 series), which was reserved before triage and
extracted by six custodians. **It passed H2 and H3 and failed H1, H4 and H5, so it is not
offered by `peykar eye`.** Nothing here is a clinical or biological validation claim.

## Model

Model 1.0.0 (candidate m4: generic eye model 5 with the zero-start law
`change(t) = ell (1 − r(t))` in mm, `r = exp(−(t − 1 day)/tau)`,
`ell = context level + b (AL0 − 24.5 mm)/1 mm`; species → treatment family → treatment;
a study effect per family arm; forecast intervals from the pooled residual variance of
the training studies) with one change, **persistent deviations in forecasting only**:
0.8 of each series' residual variance follows an Ornstein–Uhlenbeck process with a
12-month correlation time, so a forecast carries the series' current deviation from its
fitted law forward; the population is fitted under working independence. Initial states
have no observation and equal those of model 1.0.0. Configuration
`peykar.model_learning.myopia_population_1_1` (candidate n3), operator
`scripts/evaluate_myopia_population_1_1.py`, publisher
`scripts/publish_myopia_population_1_1.py`. One reported pp is 0.01 mm.

## Development (leave-one-study-out over the 33 studies)

The [development log](development-log.md) records each declaration before its run. Round
0 refitted model 1.0.0 (n0); round 1 tested three single changes against it, with the
1.0 RMSE rule or a new interval-score clause (Gneiting and Raftery's score of the 80% and
90% intervals at least 1% lower). Pooled RMSE in pp, human group means (115 forecast
targets, 197 initial-state targets):

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% | interval score |
|---|---|---|---|---|---|
| n0 model 1.0.0 refitted | 10.035 | 13.217 | 13.982 | 0.800 / 0.904 | 85.17 |
| n1 group-mean noise not scaled by group size | 10.096 | 13.436 | 14.068 | 0.861 / 0.922 | 86.88 |
| n2 zero-start law written in its initial rate | 9.673 | 14.697 | 14.368 | 0.826 / 0.922 | 84.47 |
| **n3** forecast-only persistent deviations (adopted) | **9.389** | **12.865** | **13.982** | **0.843 / 0.922** | **77.11** |
| analog kNN (111 targets; n3 9.065 and 12.607 on them) | 9.803 | 15.183 | — | — | — |
| pooled-curve increment | 14.329 | 21.747 | — | — | — |
| pooled training curve | — | — | 18.885 | — | — |

On development n3 passed H1–H5. Round 0 had shown that groups of 150 or more children
(12 targets, all in two consumed sealed studies) were undercovered (0.50/0.67) because
the noise is scaled down with the group size; n1 removed the scaling but widened the
middle groups' intervals more than it helped, and was rejected. Public development
artifacts are in [`development-loso-2026-10-30/`](development-loso-2026-10-30/tables.md)
(values of 16 studies redacted).

## One-shot sealed test (2026-10-30)

The test was run once, after freeze commit `b5195458`, with protocol
[`protocol-1.1.0.json`](protocol-1.1.0.json) (SHA `857d4c4a…`), all 117 pinned files
and the 33 training and 26 sealed record hashes matching. Exit 0, 66.5 s, report
`89d55464…`; the frozen model 1.0.0 was scored in the same run on the same training
(reference report `7a926c57…`). Public artifacts are in
[`sealed-test-2026-10-30/`](sealed-test-2026-10-30/tables.md), with figure
`sealed-results.png`, its script `plot-results.py`, the declared analysis
`sealed-analysis.json` and both public reports; values of 15 of the 26 studies are
redacted, and `.peykar/check_myopia11_redaction.py` found no value of a redacted study in
the public files.

Declared checks, human group means (42 forecast series, 75 targets; 56 control series
with 131 initial-state targets; the 4 per-child series have one visit each and are
reported only):

| check | result |
|---|---|
| H1 calibration, next observation | **FAIL**: 68/75 = 0.907 and 73/75 = 0.973, both just above their windows (0.70–0.90, 0.80–0.97): the intervals were too wide |
| H2 accuracy, next observation | pass: 6.80 pp against persistence 12.44, the last-two trend 18.16, the pooled-curve increment 8.19 and analog kNN 8.28 |
| H3 landmark | pass: 8.93 pp against persistence 20.68, the pooled-curve increment 10.76 and analog kNN 11.43 |
| H4 initial state | **FAIL**: 11.31 pp against the pooled training curve 11.10 on the same 131 targets |
| H5 against model 1.0.0, same training | **FAIL**: next observation 6.80 against 6.88 (within 0.05 pp), landmark 8.93 against 8.66 (0.27 pp higher) |

Per series, 31 of 42 group means were within 5/10 pp (0.05/0.10 mm) for the next
observation (30 also at least 0.5 pp better than persistence) and 25 for the landmark
task. Landmark coverage 0.880/0.960 and initial-state coverage 0.893/0.931 (reported, not
tested). Model 1.0.0's own results on this set (reported; it failed its own test and is
not tested again): coverage 0.920/0.973, next observation 6.88, landmark 8.66, initial
state 11.31 against 11.10.

Reported, not tested (post hoc, `.peykar/myopia11_sealed_breakdown.py`; RMSE in pp):

- **The forecasts generalise; the intervals do not.** Both forecasting hypotheses pass
  with margins of 1.4 to 1.8 pp over the best comparator, and no single study changes
  either verdict. The intervals were too wide in both size classes present (50–149
  children: 0.97/1.00 over 29 targets; under 50: 0.87/0.96 over 46); the sealed set has no
  group of 150 or more children with a forecast target. The forecast noise is the pooled
  residual variance of the training studies, and with all 33 records it is 0.075 mm at 40
  children against a median of 0.045 mm: the retrospective cohort with heavy attrition
  (myo-pmc7447800) carries 46% of it and the four-year trial (myo-pmc6990884, whose
  residual is scaled up to 40 children) 22%. These two were the 1.0 failures that became
  training data. Without one trial of extended-depth-of-focus contact lenses against the
  fellow eye (myo-pmc9253228, 10 forecast targets) H1 would pass (0.892/0.969).
- **H4 is lost by 0.21 pp** and decided by one study of two groups of 32 children (single
  vision and an `other` treatment) measured by ultrasound for up to 24 months
  (myo-pmc6296196); without it 10.44 against 10.71. Single-vision groups: 11.57 against the pooled curve's 9.33 (bias −4.84 pp,
  too short); the model wins for defocus spectacles (5.41 against 6.18), atropine (7.49
  against 7.90) and `other` (7.01 against 13.54).
- **H5 is lost on the landmark task through one study.** In an orthokeratology study with
  four subgroups of 13 to 48 children seen at 1, 3 and 6 months (myo-pmc11253518),
  carrying the first visit's deviation forward added 742 pp² of squared error; without it
  model 1.1 would be lower than 1.0.0 (7.50 against 7.87). The next-observation part
  passes but would fail without either of two other studies.
- **Weakest treatments:** orthokeratology (next observation 11.04, landmark 12.42) and
  multifocal contact lenses (landmark 11.55, forecasts too short by 8.9 pp on average).

## Limits and next steps

- The intervals depend on a pooled noise that one irregular cohort can dominate: the
  development rule tested the 1.0 lesson (pooled rather than median noise) only through
  coverage on the training studies themselves. A next version should calibrate the
  forecast noise on leave-one-study-out residuals (or use a robust pooling) and test
  calibration by group size.
- Group means whose membership changes between visits still enter as ordinary series; the
  tranche-2 records carry the per-visit group size (`n_by_visit`) for 19 series, which a
  later analysis can use.
- Tranche 2 is now consumed and the dataset has no later tranche: a version after 1.1
  needs a new screen whose test part is reserved before triage.
- Age is not a covariate; treatment effects against a control, single eyes, rebound after
  stopping a treatment, refraction and choroidal thickness are out of scope.
