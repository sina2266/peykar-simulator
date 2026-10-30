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
