# Axial length in childhood myopia, model 1.2 — development log

Development data: the 59 consumed records (142 series) of dataset
[`myopia-axial-2026-10-30`](../../../data/myopia-axial-2026-10-30/README.md): the 33
tranche-1 records that model 1.1 was developed on (22 development records of model 1.0
and the 11 records its test consumed) and the 26 tranche-2 records (60 series, private in
`.peykar/myopia-tranche2-sealed-2026-10-30/records`) that the one-shot test of model 1.1
consumed. The sealed test of model 1.2 is screen-2 tranche 1
([`screen-2/README.md`](../../../data/myopia-axial-2026-10-30/screen-2/README.md)),
declared before that screen was run; no screen-2 value is read before the 1.2 protocol is
frozen.

What the one-shot test of model 1.1.0 showed on the 26 tranche-2 studies that are now
training data ([`../model-1.1/README.md`](../model-1.1/README.md)): the forecasts beat
every comparator (H2, H3), but the intervals were too wide (coverage 0.907/0.973, both
just above their windows; model 1.0.0's too), the initial state lost to the pooled
training curve by 0.21 pp (H4) and the landmark forecasts were 0.27 pp worse than model
1.0.0's (H5), through one orthokeratology study. The pooled forecast noise of model 1.1
(0.075 mm at 40 children against a median of 0.045 mm) came mostly from two training
studies (46% and 22%): a pooled residual variance from training studies' own fits does
not measure how wide the intervals must be for a study the model has not seen.

Studies linked by the author-overlap rule leave training together in leave-one-study-out
(LOSO). Until the screen-2 overlap file exists, the clusters are the connected components,
among the 59 studies, of the pairs and development clusters of
`tranche-2/author-overlap-1.0.0.json` (9 clusters of 2 to 4 studies; the operator computes
them and records them in each run). Each round declares its change and its adoption rule
before its LOSO results are seen.

## Model

Generic eye model version 5 with the configurations of
`peykar.model_learning.myopia_population_1_2` (model 1.0.0 = m4 and model 1.1.0 = n3,
imported unchanged); series `myopia_population.to_axial_series_by_family`; scoring
`eye_additive_evaluation5` 5.0.0; operator `scripts/evaluate_myopia_population_1_2.py`;
new generic **model-only interval scaling** `peykar.validation.eye_interval_scaling`
1.0.0 (stacking 2.0.0's scaled intervals `q' = m + c (q − m)` with the model's weight fixed
at one, so the points stay the model's; `c` chosen on development forecasts so that the
pooled coverage of a declared interval is closest to its nominal level; in development
each held-out study is scaled with a factor chosen without it and its linked studies;
unit-tested). One reported pp is 0.01 mm.

## Hypotheses of the 1.2 protocol (declared before round 0)

H1–H4 as in the 1.0 and 1.1 protocols (H1 windows [0.70, 0.90] and [0.80, 0.97] for the
primary's next-observation coverage over human group means; H2 and H3 against
persistence, the last-two trend (H2 only), the pooled-curve increment and analog kNN; H4
against the pooled training curve). **H5**: against model 1.1.0 (n3, its frozen
configuration) fitted on the same training records in the same sealed run, over common
human group-mean targets, the 1.2 primary's summed mean next-observation interval score
(80% and 90% intervals; Gneiting and Raftery 2007, as in the 1.1 development log) is lower
than 1.1.0's, and its pooled next-observation and landmark RMSEs are each at most 0.05 pp
higher. `peykar eye` offers myopia 1.2.0 only if every tested hypothesis passes.

## Round 0 — declared before results (2026-10-30)

- **p0**: model 1.1.0 (n3) unchanged apart from its version string (`1.2.0-p0`),
  refitted on the 59 studies; the reference of round 1.
- **p0m**: model 1.0.0 (m4) likewise (`1.2.0-p0m`), for comparison.

Primaries by the 1.0 rule. Diagnostics, read after the run: the comparison and
interval-score tables of `.peykar/myopia11_compare.py` (p0 then p0m), and the diagnostics
of `.peykar/myopia11_diagnostics.py` adapted to the 59 records
(`.peykar/myopia12_diagnostics.py`: the same tables, with the tranche-2 studies alone in
place of the consumed 1.0 studies), and the noise centres of p0 on all 59 studies with
each study's share of the pooled variance. Round 1 is declared here after round 0's
results and before its own runs; the direction planned from the 1.1 test is interval
scaling of p0 (and of p0m if round 0 favours it) with `eye_interval_scaling`, nested in
LOSO, adopted by the interval clause of the 1.1 rule.

## Round 0 — results (2026-10-30)

LOSO over the 59 studies (190 next-observation and landmark targets of human group means,
328 initial-state targets; `.peykar/myopia-1.2-dev/round0-p0`, `round0-p0m`,
`round0-compare.txt`, `round0-p0-diagnostics.txt`, `round0-p0-noise.txt`; about two
minutes each, in parallel). Both primaries are the Gaussian variant.

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% | landmark coverage | interval score |
|---|---|---|---|---|---|---|
| p0 (1.1.0 refitted) | 8.486 (−2.36) | 12.006 (−5.09) | 13.309 | 0.895 / 0.932 | 0.847 / 0.900 | 70.467 |
| p0m (1.0.0 refitted) | 8.770 (−2.55) | 11.990 (−4.98) | 13.309 | 0.863 / 0.926 | 0.842 / 0.905 | 74.873 |
| pooled-curve increment | 12.715 | 18.499 | — | — | — | — |
| analog kNN (186 targets; p0 8.303 and 11.269 on them) | 9.480 | 13.932 | — | — | — | — |
| pooled training curve | — | — | 16.415 | — | — | — |

- **The intervals are too wide.** p0's 80% next-observation coverage is 0.895, at the
  upper edge of its window; groups of 50–149 children cover 0.94/0.98 and groups under 50
  0.88/0.92, while the 12 targets of groups of 150 or more cover 0.75/0.75 (80% half-width
  5.9 pp against 8.5 and 11.3). On the 26 tranche-2 studies alone, p0 covers 0.933/0.960,
  as model 1.1.0 did on them in its sealed test.
- **Why:** p0's forecast noise is the pooled residual variance, 0.069 mm at 40 children
  on all 59 studies against a median of 0.032 mm; four studies with irregular series carry
  80% of it: the attrition cohort myo-pmc7447800 (35%), the tranche-2 orthokeratology study
  with visits at 1, 3 and 6 months myo-pmc11253518 (23%, its residual SD 0.177 mm is the
  largest of the 22 studies with one), the four-year trial myo-pmc6990884 (12%) and
  myo-pmc7943497 (10%). The noise exponent ranges from 0.16 to 0.50 over the folds.
- Forecasts stay short far ahead (landmark targets more than 12 months after the first
  visit 24.37 pp, −18.00), and on the tranche-2 studies the initial state loses to the
  pooled training curve (11.26 against 10.94 pp), as in the 1.1 sealed test.
- p0 does not meet the 1.1 rule against itself, of course; p0m is not better (−0.284 pp in
  the next observation, interval score 6.25% higher).

## Round 1 — declared before results (2026-10-30)

Interval scaling, planned from the 1.1 test: each candidate keeps a population
candidate's Gaussian-variant points (`population_assimilation`) and scales its intervals
with `eye_interval_scaling` 1.0.0, one factor per forecast task (`scale_tasks =
("next_observation", "landmark")`), each chosen so that the pooled coverage of that task's
80% intervals over human group means is closest to 0.80, on the grid 0.30 to 3.00 by 0.01.
In LOSO each held-out study gets factors chosen without it and its linked studies; the
factors chosen on all 59 studies are the ones a protocol would freeze.

- **p1**: p0 scaled.
- **p2**: p0 with group-mean noise not scaled by group size (`p0g`, `max_noise_exponent =
  0`), scaled: one noise level for every group size, so that the scale fits the total and
  the large groups are no longer narrowed by an exponent read from a few studies.
- **p3**: p0m (model 1.0.0 refitted) scaled.

Adoption rule against p0 (`.peykar/myopia12_compare.py`, the 1.1 rule): coverage of the
candidate's next-observation primary (`stacked`) inside both H1 windows, no task more than
0.05 pp worse, and an RMSE gain of at least 0.10 pp in a task or an interval score at
least 1% lower. Among the candidates that meet it, the one with the largest
interval-score reduction is adopted (scaling does not move the points, so RMSE gains
cannot rank them); candidates within 1% of the best go to the simpler one, in the order
p1, p3, p2. If none meets the rule, p0 is the 1.2 candidate. Reported only: coverage by
group size and on the tranche-2 studies alone, landmark coverage, and the factors chosen
on all 59 studies. A round 2, if any, is declared here before it runs.
