# Axial length in childhood myopia — tenth eye component

How the ocular axial length (AL, millimetres) of a group of myopic children or
adolescents grows from its baseline value over months to years under single-vision
correction or a myopia-control treatment (atropine eye drops, orthokeratology,
myopia-control spectacle lenses, multifocal soft contact lenses, repeated low-level
red-light therapy, combinations, outdoor time or another non-surgical treatment). Axial
elongation is the main measure of myopia progression and of the slowing these
treatments aim at. It is the first component that is not a response to surgery or to a
single treatment course: the eye keeps growing, so its change from baseline starts at
zero and rises for years.

- Dataset: [`experiments/data/myopia-axial-2026-10-30`](../../data/myopia-axial-2026-10-30/README.md)
  (1,004 screened open-access articles, 214 passed a masked-table rule; tranche 1 of 120
  was triaged blind, 54 admitted, split 34/20 before any value was read; 22 development
  records with 59 series and 11 custodian-extracted sealed records with 23 series). The
  sealed set was consumed by the one-shot test of model 1.0.0 on 2026-10-30. Tranche 2
  (94 articles) was then reserved before triage as the sealed set of model 1.1 (42
  admitted, 26 records with 60 series) and consumed by its test on the same day; the
  dataset has no later tranche.
- Model 1.0: [`model-1.0/`](model-1.0/README.md). With the free start of generic eye
  model 2 the population read a shortening of about 0.66 mm at day 1; new generic eye
  model version 5 (`peykar.model_learning.eye_recovery_population5`) adds laws that are
  unchanged at the start, and the adopted candidate m4 uses one of them with a
  treatment-family hierarchy. Its one-shot sealed test **failed all four hypotheses**:
  coverage 0.638/0.787 below its windows, next observation 12.01 pp against the
  pooled-curve increment 11.14 and analog kNN 10.20, landmark 18.07 against the
  pooled-curve increment 17.11, initial state 19.15 against the pooled training curve
  17.32 (one pp is 0.01 mm; [Persian results](model-1.0/results.fa.md)). A retrospective
  cohort whose atropine groups lost half or more of their children by month 24 decides H1–H3,
  and a 4-year trial whose controls kept growing 0.42 mm a year decides H4. Axial length
  is therefore not offered by `peykar eye`.
- Model 1.1: [`model-1.1/`](model-1.1/README.md), developed on all 33 tranche-1 records.
  It is model 1.0.0 with persistent deviations in forecasting only (an Ornstein–Uhlenbeck
  process carrying 0.8 of each series' residual variance with a 12-month correlation
  time), adopted in round 1 against model 1.0.0 refitted (n0) under the 1.0 RMSE rule and
  a new interval-score clause. Its one-shot sealed test on tranche 2 **passed H2 and H3**
  (next observation 6.80 pp against the pooled-curve increment 8.19 and analog kNN 8.28;
  landmark 8.93 against 10.76 and 11.43) and **failed H1, H4 and H5**: coverage
  0.907/0.973 just above its windows (the intervals were too wide), initial state 11.31
  against the pooled training curve 11.10, and landmark 8.93 against model 1.0.0's 8.66 on
  the same training ([Persian results](model-1.1/results.fa.md)). The pooled forecast
  noise is dominated by the two studies that had failed model 1.0 (46% and 22% of it). Axial
  length is therefore still not offered by `peykar eye`.
