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

## Round 0 — results (2026-10-30)

LOSO over the 33 training studies (115 next-observation and landmark targets of human
group means, 197 initial-state targets of 82 control group means;
`.peykar/myopia-1.1-dev/round0-n0`, `round0-compare.txt`, `round0-diagnostics.txt`; 43
s). RMSE in pp of 0.01 mm, mean signed error (forecast − observed) in parentheses.

| method | next observation | landmark | initial state |
|---|---|---|---|
| n0 primary (`population_assimilation`; the t4 variant's 9.993 lies within 0.05 pp) | 10.035 (−3.87), coverage 0.800/0.904 | 13.217 (−6.71), coverage 0.791/0.904 | 13.982 (−3.73) |
| persistence | 18.222 | 38.167 | — |
| last-two trend | 14.712 | 22.798 | — |
| pooled-curve increment | 14.329 | 21.747 | — |
| analog kNN (111 common targets; n0 on them) | 9.803 (n0 9.797) | 15.183 (n0 12.979) | — |
| pooled training curve | — | — | 18.885 |

On these development folds every 1.0 hypothesis would pass, H2 against analog kNN by
0.006 pp. The next-observation interval score of the primary (round 1 below) is 85.167 pp
(80% 36.642, 90% 48.525). The declared diagnostics:

- **(a) Groups of 150 or more children are undercovered.** Coverage 0.50/0.67 over their
  12 targets (the two arms of the four-year trial and one other arm), whose 80%
  intervals are about half as wide (median half-width 5.4 pp) as those of groups of
  50–149 children (0.87/0.96, 10.3 pp) and of fewer than 50 (0.80/0.90, 13.0 pp). By
  context, atropine 0.56/0.81 and single vision 0.71/0.79; every other context covers at
  least 0.80/0.90.
- **(b) Forecasts are too short, more so far ahead.** Mean signed error −3.87 pp (next
  observation) and −6.71 pp (landmark); landmark targets more than 12 months after the
  first visit 22.49 pp (−17.86) on 22 targets, targets after month 24 22.63 pp (−18.67)
  on 13. In 40 of the 57 pairs of consecutive next-observation errors of one series both
  errors have the same sign (correlation of their values 0.05), and 13 of the 30 series
  with two or more targets have only negative errors (2 only positive).
- **(c) A few studies carry most of the squared error:** next observation myo-pmc7447800
  (the attrition cohort) 0.33, myo-pmc7943497 0.20, myo-pmc11175863 0.10,
  myo-pmc12101883 0.09, myo-pmc6990884 (the four-year trial) 0.08; landmark
  myo-pmc7447800 0.26, myo-pmc6990884 0.22, myo-pmc7943497 0.14; initial state
  myo-pmc6990884 0.26, myo-pmc7447800 0.11, myo-pmc12101883 0.10.
- **(d)** The initial state is better than the pooled training curve in every context
  but atropine (16.24 against 12.82 pp over 30 targets).
- **(e) The group-size scaling of the noise is extrapolated.** The fitted exponent ranges
  from 0.218 to 0.500 (its cap) over the folds, the noise centre at 40 children from
  0.039 to 0.047 mm. Among the 13 studies with their own noise estimate (residual SD
  0.013 to 0.156 mm) the slope of ln SD on ln(group size) is −0.40, but the four-year
  trial (285 children on average) has 0.041 mm, the median. If that residual were the
  sampling error of the group mean, the children's SD of the change would be 0.69 mm,
  far larger than the spread of a year's elongation among children (about 0.1 to 0.2 mm
  in the clinical literature): the residual of a group mean around the law is mostly a
  visit-level error that does not shrink with the number of children.
- **(f) The 11 consumed sealed studies alone:** next observation 11.52 pp (−5.60),
  coverage 0.702/0.830; landmark 16.09 pp (−8.15); initial state 16.57 against the pooled
  training curve's 17.38 pp.

## Round 1 — declared before results (2026-10-30)

Each candidate changes one setting of n0 (`peykar.model_learning.myopia_population_1_1`;
the unit tests check that each differs from n0 in that setting only):

- **n1** (from diagnostics a and e): group-mean noise not scaled by group size
  (`max_noise_exponent = 0`), so every group mean gets the noise centre, as a visit-level
  error.
- **n2** (from diagnostic b): the zero-start law written in its initial rate (`start =
  "zero_rate"`, `R = 12` months, the law of 1.0 candidate m3):
  `change(t)/1 mm = v (tau/12)(1 − r(t))`. Given `tau` the two laws are the same curve,
  but the hierarchy, the covariate and the anchored variants act on the elongation per
  year at the start instead of the long-term change, so a series whose first visits grow
  faster than the population is shrunk on its rate, which a year of data identifies,
  and its long-term change follows its time constant.
- **n3** (from diagnostic b, the same-sign errors): persistent deviations in forecasting
  only (`deviation_fraction = 0.8`, `deviation_months = 12`,
  `deviation_in_population_fit = False`, the constants retinal thickness 1.0 adopted), so
  a forecast carries its series' current deviation from the posterior law forward.

Adoption rule against n0 (`.peykar/myopia11_compare.py`, which prints the tables of the
unchanged `.peykar/myopia_compare.py`): a candidate meets the rule if its next-observation
primary's coverage lies inside both H1 windows, it loses no more than 0.05 pp of pooled
RMSE in any task (initial state, next observation, landmark), and either (RMSE clause,
the 1.0 rule) it improves the pooled RMSE of a task by at least 0.10 pp, or (interval
clause, new) it lowers the next-observation primary's summed mean interval score (80%
and 90% intervals) by at least 1%. The interval score of a central (1 − α) interval
[l, u] for an observation y is (u − l) + (2/α)(l − y)·1{y < l} + (2/α)(y − u)·1{y > u}
(Gneiting and Raftery 2007): it rewards narrow intervals that still cover, so it can
show whether intervals are better placed when the pooled coverage is already on target
and the RMSE barely moves, as expected for n1. Primaries are chosen by the 1.0 rule. If
several candidates meet the rule, the one with the larger summed RMSE gain over the three
tasks is adopted (a candidate meeting only the interval clause ranks after those meeting
the RMSE clause, by its interval-score reduction), and round 2 tests the combination of
every candidate that met the rule against the adopted one under the same rule. If none
meets it, n0 is the model 1.1 candidate (model 1.0.0 refitted on the 33 studies) and H5 is
not tested. Each candidate runs as one process over the 33 studies. Reported only:
coverage of the groups of 150 or more children and the diagnostics of round 0.
