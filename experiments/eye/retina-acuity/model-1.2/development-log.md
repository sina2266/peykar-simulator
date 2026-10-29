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
