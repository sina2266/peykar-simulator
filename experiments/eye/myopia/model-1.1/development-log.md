# Axial length in childhood myopia, model 1.1 — development log

Development data: the 33 tranche-1 records (82 series) of dataset
[`myopia-axial-2026-10-30`](../../../data/myopia-axial-2026-10-30/README.md): the 22
development records of model 1.0 (59 series; 12 CC BY records in
`development/records` and 10 private records in
`.peykar/myopia-development-private/records`) and the 11 tranche-1 sealed records (23
series, private in `.peykar/myopia-sealed-2026-10-30/records`) that the one-shot test of
model 1.0.0 consumed. The sealed test of model 1.1 is tranche 2
([`tranche-2/README.md`](../../../data/myopia-axial-2026-10-30/tranche-2/README.md)):
the 42 articles admitted by masked triage after they were reserved, of which custodians
extracted 26 records with 60 series. No tranche-2 value is read before the 1.1 protocol
is frozen.

Facts (descriptive, development only, `.peykar/myopia11_dev_summary.py`): all 82 series
are group means with a reported group size (2 to 308 children, median 46.5) and role
`control`. Contexts: single vision 18 series, orthokeratology 17, defocus spectacles 16,
atropine 14, combinations 8, `other` 4, red light 3, multifocal contact lenses 2. East
Asia 63, Europe 14, Southeast Asia 3, South Asia 2. Twenty-four series have one
follow-up visit and 58 have two or more (22 of them with groups under 50 children, 33
with 50 to 149 and 3 with 150 or more). First follow-ups are at 1 month (8 series), 3
(9), 4 to 6 (36), 12 (28) and 39 months (1); 59 series end at 12 months, 9 at 24, 3 at
36, 1 at 48 and 2 at 132. Baseline axial length 23.11 to 25.60 mm (median 24.51); mean
age reported for 60 series (8.5 to 14.9 years). Median 12-month elongation: single
vision 0.35 mm, `other` 0.31, atropine 0.27, multifocal contact lenses 0.23,
orthokeratology 0.20, defocus spectacles 0.13, combinations 0.12, red light 0.11 mm.

What the one-shot test of model 1.0.0 showed on the 11 studies that are now training
data ([`../model-1.0/results.fa.md`](../model-1.0/results.fa.md)): all four hypotheses
failed. A retrospective anisomyopia cohort (myo-pmc7447800) whose atropine groups kept
18 and 13 of 36 children at month 24 decided the calibration and both forecasting
verdicts; the placebo group of about 300 children of a four-year trial
(myo-pmc6990884), growing 0.42 mm a year, decided the initial-state verdict. Model 1.0.0
is not offered by `peykar eye`.

Studies of one author-overlap cluster (`tranche-2/author-overlap-1.0.0.json`, development
clusters myo-pmc10656815 + myo-pmc8592985 + myo-pmc9665032 and myo-pmc6990884 +
myo-pmc9812803) leave training together in leave-one-study-out (LOSO). Each round
declares its change and its adoption rule before its LOSO results are seen.
Disclosure: before round 0 was declared, the developer saw the structural counts of the
tranche-2 custody check (series per context, region, kind, number of visits, licence
class and the number of series with per-visit group sizes; no value) and opened custodian
C4's custody report once (structure, decisions and hashes of seven studies; no
axial-length value), as the tranche-2 README records.

## Model

Generic eye model version 5 (`eye_recovery_population5`, unchanged) with the
configurations of `peykar.model_learning.myopia_population_1_1`; series
`myopia_population.to_axial_series_by_family` (contexts `human/<treatment>/<family>`);
scoring `peykar.validation.eye_additive_evaluation5` 5.0.0; operator
`scripts/evaluate_myopia_population_1_1.py` (training = the 33 records, each checked
against the tranche-1 and tranche-2 reservations); comparison `.peykar/myopia_compare.py`
(unchanged) and diagnostics `.peykar/myopia11_diagnostics.py`. The scale, units,
criteria and comparators are those of model 1.0 (one reported pp is 0.01 mm).

## Hypotheses of the 1.1 protocol (declared before round 0)

H1–H4 as in the 1.0 protocol: H1 next-observation coverage of the primary over human
group means, 80% interval within [0.70, 0.90] and 90% within [0.80, 0.97]; H2 pooled
group-mean next-observation RMSE of the primary below persistence, the last-two trend,
the pooled-curve increment and analog kNN over common targets; H3 pooled group-mean
landmark RMSE below persistence, the pooled-curve increment and analog kNN; H4 pooled
initial-state RMSE of `population_initial_state` below the pooled training curve over
control group means. H5, only if the adopted candidate differs from n0: the 1.1
primary's pooled group-mean next-observation and landmark RMSE are each at most 0.05 pp
above those of model 1.0.0 (m4 with its frozen configuration, fitted on the same training
in the same sealed run) over common targets. `peykar eye` offers myopia 1.1.0 only if
every tested hypothesis passes; model 1.0.0 failed its own test and is not offered,
whatever its reference scores on tranche 2.

## Round 0 — declared before results (2026-10-30)

- **n0**: model 1.0.0 (m4) unchanged apart from its version string (`1.1.0-n0`),
  refitted on the 33 studies. It is the reference of round 1; round 0 makes no adoption
  decision.

The primary is chosen by the 1.0 rule (lowest pooled group-mean next-observation RMSE
among the five population variants whose coverage lies inside both H1 windows; ties
within 0.05 pp to the Gaussian variant; the Gaussian variant if none lies inside). The
diagnostics, declared now and read after the run (`.peykar/myopia11_diagnostics.py`):
(a) next-observation coverage of the primary at 80% and 90% by reported group size
(< 50, 50–149, ≥ 150 children) and by context; (b) pooled RMSE and signed error of the
primary and the comparators by context, target month (≤ 6, 7–12, 13–24, > 24 months) and
horizon; (c) each study's share of the summed squared error in each task; (d) the
initial-state RMSE by context against the pooled training curve; (e) the fitted
group-mean noise exponent and noise centres over the folds; (f) (a), (b) and (d) for the
11 consumed sealed studies alone. Round 1 is declared here after round 0's results and
before its own runs; its candidates come from these diagnostics, and its adoption rule is
the 1.0 rule against n0.
