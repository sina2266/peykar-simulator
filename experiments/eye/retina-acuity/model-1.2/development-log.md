# Retinal visual-acuity model 1.2 — development log

Development data: the 53 record studies of tranches 1 and 2 of dataset
[`retina-acuity-2026-10-29`](../../../data/retina-acuity-2026-10-29/README.md), listed as
development in
[`tranche-3/split-reservation-1.0.0.json`](../../../data/retina-acuity-2026-10-29/tranche-3/split-reservation-1.0.0.json):
the 15 development records of model 1.0, the 9 records consumed by its sealed test and
the 29 tranche-2 records consumed by the sealed test of model 1.1. Together 144 series
(85 group means, 59 per-eye series; 93 control and 51 treated). The sealed test set of
1.2 is tranches 3–4 (70 admitted studies, extraction by custodians under way), reserved
before triage; no tranche-3/4 value is read before the 1.2 protocol is frozen. The
tranche-3 author-overlap file has three development clusters (the linked tranche-2
pairs), which leave leave-one-study-out (LOSO) training together. Each round declares
its change and its adoption rule before its LOSO results are seen.

Control contexts by number of studies (counts only): dme/anti_vegf 12, namd/anti_vegf
10, dme/steroid_implant 9, rvo/anti_vegf 8, dme/vitrectomy 2, dme/combination 2,
myopic_cnv/anti_vegf 2, and 1 each for dme/steroid_injection, rvo/combination,
rvo/steroid_implant, other/laser, uveitic/steroid_implant and namd/combination. 21
series of 7 studies have a visit before month 0.9.

## Starting point

Model 1.1.0 (candidate `w1`, the law `change(t) = ell + e r(t)` without the 7-day
transient) failed H2–H5 of its sealed test ([README](../model-1.1/README.md)). After
unsealing: the recovery term's fitted time constant was 7.5–13 days in every context,
so its size in the first weeks was set by no data before a context's first training
visit; two linked dexamethasone-implant studies with day-3 visits carried most of the
error through anchored forecasts; and inside the training time support 1.1 was only
level with persistence and the pooled-curve increment.

## Model and evaluation

Generic eye model version 2 with the configurations of
`peykar.model_learning.retina_acuity_population_1_2`. The support hold is the generic
option `peykar.model_learning.eye_support.SupportConfig` (`support_hold`), evaluated by
harness 5 (`peykar.validation.eye_recovery_evaluation5`, through
`eye_additive_evaluation3`): before the earliest visit month of the training controls of
the series' context (its species' controls for an untrained context), the law is held
flat, `f(t) = f(max(t, t_s))`, in the likelihood of the series' own visits and in every
forecast and initial state. Training controls never have a visit before their own
context's support, so the population fit is the same as without the hold; comparators
see the true months. Operator `scripts/evaluate_retina_acuity_population_1_2.py`, scores
in ETDRS letters as in models 1.0 and 1.1.

## Round 0 and round 1 — declared before results (2026-10-29)

- **x0**: model 1.1.0 itself (the frozen `w1` configuration object), the reference. Its
  LOSO on the 53 studies is round 0.
- **x1**: `x0` with the support hold.
- **x2**: `x1` with `treatment_fallback`: a context without training series takes the
  pooled level of the trained contexts of its treatment before the species level.
- **x3**: `x1` with wear-off of one dexamethasone implant, the rule of 1.1's `w3`
  (retinal-thickness 1.1 dose rule: duration 3.5 months, logistic width 0.5 months, 80%
  of the effect lost).

Primary method of a candidate (the model 1.0 and 1.1 rule): among the population
variants whose next-observation coverage of human group means lies inside both H1
windows ([0.70, 0.90] at 80%, [0.80, 0.97] at 90%), the lowest pooled next-observation
RMSE; a variant within 0.05 letters of that minimum that comes earlier in the order
Gaussian, t4, robust, anchored, anchored t4 is preferred. If no variant lies inside both
windows, the robust variant is the primary and the miss is recorded. The same variant
is the landmark primary; the initial state uses `population_initial_state`.

Adoption rule (each of x1, x2, x3 against x0): adopted if its primary improves the
pooled group-mean RMSE by at least 0.10 letters in at least one task (next observation,
landmark, initial state), loses no more than 0.05 letters in any, and lies inside both
H1 windows whenever x0's primary does. If two or more are adopted, round 2 runs one
candidate that combines all adopted changes, adopted by the same rule against the
adopted single candidate with the largest summed gain over the three tasks (ties within
0.05 letters: the order x1, x3, x2). If none is adopted, x0 stays the model and no 1.2
protocol is frozen.

The protocol of a 1.2 version keeps H1–H4 of models 1.0 and 1.1 and adds H5 against
model 1.1.0 trained on the same records and scored in the same sealed run: over common
human group-mean targets, the 1.2 primary's pooled landmark RMSE and initial-state RMSE
(control group means) each lower than 1.1.0's, and its pooled next-observation RMSE at
most 0.05 letters higher. Comparisons inside and outside the training time support are
reported, not tested.

## Round 1 results (LOSO on the 53 training studies, 2026-10-29)

Human group means: 206 next-observation and landmark targets, 251 initial-state targets.
Pooled RMSE in ETDRS letters (`.peykar/retina_acuity11_compare.py` on the private
reports `.peykar/retina-acuity-1.2-dev/round1-x{0,1,2,3}/report.json`; EXIT=0 each).

| candidate | primary | next observation | landmark | initial state | next-observation coverage 80% / 90% |
|---|---|---|---|---|---|
| x0 = 1.1.0 | anchored | 3.628 | 4.921 | 5.556 | 0.796 / 0.898 |
| x1 support hold | anchored | 3.477 | 4.366 | 5.429 | 0.796 / 0.903 |
| x2 x1 + treatment fallback | anchored | 3.482 | 4.380 | 5.428 | 0.796 / 0.903 |
| x3 x1 + dexamethasone wear-off | anchored | 3.228 | 3.782 | 5.216 | 0.772 / 0.893 |

Comparators on the same targets (next observation / landmark): persistence 3.234 /
4.026, pooled-curve increment 3.566 / 5.015, analog kNN 5.212 / 8.909, last-two trend
13.013 / 37.287. Initial state: pooled training curve 5.278. In every candidate the
Gaussian anchored variant is the primary: anchored t4 has the lowest next-observation
RMSE, and anchored lies within 0.05 letters of it and comes earlier in the order.

- x1: −0.151, −0.556, −0.127 letters against x0, coverage inside both windows. Adopted.
- x2: −0.146, −0.541, −0.128. Adopted.
- x3: −0.400, −1.139, −0.340. Adopted.

All three are adopted, so by the declared rule round 2 runs the combination.

## Round 2 — declared before results (2026-10-29)

- **x4**: every change adopted in round 1: the support hold, the treatment fallback and
  the dexamethasone wear-off of `x3`.

x4 is compared with x3, the adopted single candidate with the largest summed gain
(−1.879 letters over the three tasks; x1 −0.834, x2 −0.815), by the round-1 adoption
rule. If x4 is adopted it is the model; otherwise x3 is the model.

## Round 2 results (2026-10-29)

x4 (`.peykar/retina-acuity-1.2-dev/round2-x4/report.json`, EXIT=0): primary anchored,
next observation 3.233, landmark 3.806, initial state 5.218 letters, coverage 0.777 /
0.893; against x3 +0.005, +0.024, +0.002. No gain of 0.10: **not adopted**. The
treatment fallback adds nothing once the hold is in place.

## Freeze decision (2026-10-29)

- Model 1.2.0 = **x3**: model 1.1.0's law with the support hold and the wear-off of one
  dexamethasone implant (dose rule of retinal-thickness 1.1). Primaries: U2 and U3
  `population_anchored`, U1 `population_initial_state`. The frozen 1.1.0 (`x0`,
  primaries `population_anchored_t4` and `population_initial_state`) is scored in the
  same sealed run as the H5 reference.
- H1–H4 are those of models 1.0 and 1.1 with the same thresholds and comparators; H5 is
  the one declared in round 1.
- Development status of x3 (LOSO, 53 studies): H1 0.772 / 0.893 inside both windows; H2
  3.228 against persistence 3.234 (by 0.006 letters only), the pooled-curve increment
  3.566, analog kNN 5.212 and the last-two trend 13.013; H3 3.782 against 4.026, 5.015
  and 8.909; H4 5.216 against the pooled training curve 5.278 (by 0.06 letters); H5
  3.228 against 3.628, 3.782 against 4.921 and 5.216 against 5.556. H2 and H4 are near
  ties on development, so sealed failures of either would not be surprising.
- The protocol is written after the custodians finish tranches 3–4; no tranche-3/4
  value is read before it is frozen.

## Custody finished; protocol written (2026-10-29)

The nine custodians built 27 sealed records with 143 series (34 group means); the custody
verifier rebuilt every record byte for byte (tranche-3 README, custody outcome). The
protocol `protocol-1.2.0.json` pins this log, the code, the dataset files, the custody
reports and the record hashes, and records one process note: before the freeze the
developer opened one sealed spec by mistake (no acuity value) and several custody reports.
