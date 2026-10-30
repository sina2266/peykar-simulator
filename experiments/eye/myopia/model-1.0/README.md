# Axial length in childhood myopia — model 1.0.0

Tenth eye component. It forecasts the ocular axial length (AL, millimetres) of a group
of myopic children or adolescents under single-vision correction or a myopia-control
treatment (atropine, orthokeratology, myopia-control spectacles, multifocal contact
lenses, red-light therapy, combinations or another non-surgical treatment): from the
baseline axial length alone (initial state), from the first follow-up visit (landmark),
or one visit ahead (next observation). It was developed on the 22 development records of
dataset [`myopia-axial-2026-10-30`](../../../data/myopia-axial-2026-10-30/README.md) (59
series) and tested once on 11 sealed records (23 series), reserved by a salted hash
before any value was read and extracted by three custodians. **It failed all four of its
declared hypotheses and is not offered by `peykar eye`.** Nothing here is a clinical or
biological validation claim.

## Model

- **Additive outcome, scored in hundredths of a millimetre.** The series of
  `peykar.validation.myopia_axial_data` give the model the change from the baseline axial
  length as the exact pseudo-fraction `exp(change / 1 mm)` and the baseline as
  `exp(AL0 / 1 mm)`; `peykar.validation.eye_additive_evaluation5` rescores everything in
  hundredths of a millimetre: **one reported pp is 0.01 mm**, so the per-series criteria
  read RMSE ≤ 0.05 mm, maximum ≤ 0.10 mm and, for forecasts, at least 0.005 mm better
  than persistence.
- **A law that starts at zero (candidate m4).** `change(t) = ell (1 − r(t))` (in mm) with
  `r = exp(−(t − 1 day)/tau)` and `ell = context level + b (AL0 − 24.5 mm)/1 mm`: the eye
  elongates from its baseline towards a long-term change along one exponential. This is
  new generic eye model version 5 (`peykar.model_learning.eye_recovery_population5`,
  harness 7), which adds laws that are unchanged at the reference time to model 2 and
  reproduces model 2 exactly otherwise (unit-tested). Forecast intervals use the pooled
  residual variance of the training studies.
- **Hierarchy.** Species → treatment family (control: single vision and `other`;
  optical: orthokeratology, defocus spectacles, multifocal contact lenses; atropine:
  atropine and combinations; light: red light and outdoor time) → treatment, with a
  study effect per family arm. An untrained treatment uses its family mean.
  Configuration `peykar.model_learning.myopia_population`, operator
  `scripts/evaluate_myopia_population.py`, publisher `scripts/publish_myopia_population.py`.

## Development (leave-one-study-out over the 22 studies)

The [development log](development-log.md) records each declaration before its run.
Pooled RMSE in pp (0.01 mm), human group means (68 forecast targets, 127 initial-state
targets):

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|
| m0 generic model 2, no 7-day transient, forecast-only pooled noise | 15.385 | 27.496 | 35.876 | 0.824 / 0.912 |
| m1 m0 + treatment-family level (adopted in round 1) | 15.143 | 27.320 | 35.480 | 0.838 / 0.912 |
| m2 m0 with the median noise in forecasts | 15.386 | 27.497 | 35.877 | 0.824 / 0.912 |
| m3 m1 with a zero start, written in its initial rate | 8.975 | 17.066 | 15.314 | 0.794 / 0.912 |
| **m4** m1 with a zero start, written in its long-term change (adopted) | **9.722** | **15.663** | **14.026** | **0.735 / 0.853** |
| persistence | 18.525 | 34.828 | — | — |
| pooled-curve increment | 18.327 | 27.039 | — | — |
| analog kNN (64 targets) | 9.138 | 9.924 | — | — |
| pooled training curve | — | — | 21.716 | — |

With the free start of generic model 2, the population read a change of about −0.66 mm
at day 1 from the curvature of visits at 3 to 24 months, a shortening no eye shows;
every context's initial state was 0.07 to 0.36 mm too short. The zero start of round 2
removed it and lowered the errors by 36% (next observation), 43% (landmark) and 60%
(initial state). On development m4 passed H1 (near the lower
edges), H2 and H4, and **failed H3**: on the 64 targets with an analog forecast its
landmark error was 10.11 pp against 9.92 for analog kNN. Public development artifacts
are in [`development-loso-2026-10-30/`](development-loso-2026-10-30/tables.md) (values of
10 studies redacted).

## One-shot sealed test (2026-10-30)

The test was run once, after freeze commit `17ba8feb`, with protocol
[`protocol-1.0.0.json`](protocol-1.0.0.json) (SHA `da2a3a89…`), all 89 pinned files and
the 22 training and 11 sealed record hashes matching. Exit 0, 12.5 s, report
`2976d530…`. Public artifacts are in
[`sealed-test-2026-10-30/`](sealed-test-2026-10-30/tables.md), with figure
`sealed-results.png`, its script `plot-results.py` and the declared analysis
`sealed-analysis.json`; values of 6 of the 11 studies are redacted, and
`.peykar/check_myopia_redaction.py` found no value of a redacted study in the public
files.

Declared checks, human group means (18 forecast series, 47 targets; 23 control series
with 70 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | **FAIL**: 30/47 = 0.638 and 37/47 = 0.787, both below their windows (0.70–0.90, 0.80–0.97) |
| H2 accuracy, next observation | **FAIL**: 12.01 pp against the pooled-curve increment 11.14 and analog kNN 10.20 (it beats persistence 17.78 and the last-two trend 13.06) |
| H3 landmark | **FAIL**: 18.07 pp against the pooled-curve increment 17.11 (it beats persistence 42.54 and analog kNN 19.71) |
| H4 initial state | **FAIL**: 19.15 pp against the pooled training curve 17.32 on the same 70 targets |

Per series, 10 of 18 group means were within 5/10 pp (0.05/0.10 mm) for the next
observation and 9 of them also at least 0.5 pp better than persistence; for the landmark
task 9 and 9. Landmark coverage 0.660 (80%) and 0.745 (90%); initial-state coverage
0.843 and 0.900 (reported, not tested).

Reported, not tested (post hoc, `.peykar/myopia10_sealed_breakdown.py`; RMSE in pp):

- **Two public studies decide every verdict.** A retrospective cohort of anisomyopic
  children treated with orthokeratology or 0.01% or 0.05% atropine
  ([PMC7447800](https://doi.org/10.1038/s41598-020-71142-3)) kept 18 and 13 of 36
  children in its atropine groups at month 24; their group means jumped in the second
  year (0.01% atropine: 0.13 mm at month 12, 0.56 mm at month 18, 0.86 mm at month 24),
  outside the model's intervals (figure, lower right). Without it H1 (0.711/0.816), H2
  (8.72 against 10.75 and 9.39) and H3 (16.48 against 16.80) would pass. In a 3-year
  trial of monthly 1% atropine with progressive lenses against placebo with single-vision
  spectacles ([PMC6990884](https://doi.org/10.7150/ijms.39365)), the placebo group of
  about 300 children grew 0.42 mm a year for four years (1.68 mm at month 48); the law
  levelled off (initial state 0.80 mm at month 48), and the intervals for groups of this
  size were narrow (5 of the study's 11 next-visit targets inside the 80% interval;
  figure, lower left). Without it H4 would pass
  (13.37 against 14.58), but H2 (against the last-two trend) and H3 (against analog kNN)
  would then fail. No other single study changes a verdict.
- **Where the model helps.** Initial states after red light (2 groups: 2.6 pp against
  11.7 for the pooled curve), combinations (4.7 against 10.9), orthokeratology (10.0
  against 13.6) and defocus spectacles (13.5 against 16.6). Multifocal contact lenses,
  a treatment without development data, used the optical family mean: initial state 6.0
  against 5.2, next observation 3.5 pp.
- **Where it does not.** Single-vision controls (initial state 32.0 against 23.0, mean
  signed error −22.5 pp) and atropine (15.1 against 13.2; next observation 18.1 with
  mean signed error −10.9): the model expects less elongation than these groups showed.
  The development intervals had been near the lower edges of their windows (0.735/0.853).

## Limits and next steps

- The law slows down with the time since baseline, at a rate learned mostly from series
  of one or two years and one 11-year study. Children who keep elongating at a steady
  rate for years are forecast too short; the slowing of eye growth depends on age, which
  the model does not know (it knows the treatment, the group size and the baseline axial
  length only).
- Group means of retrospective cohorts whose membership changes between visits (here
  half of a group lost by month 24) are not the trajectory of one group of children. The
  extraction protocol admitted them as ordinary attrition; a next protocol should record
  the number of children at each visit and test such series separately.
- Tranche 2 of the dataset (94 articles that passed the rule) is untriaged and unread. A
  model 1.1 can be developed on the 33 consumed records (22 development and 11 sealed)
  and tested on it once.
- Treatment effects against a control, single eyes, rebound after stopping a treatment,
  refraction and choroidal thickness are out of scope.
