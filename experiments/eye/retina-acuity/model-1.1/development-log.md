# Retinal visual-acuity model 1.1 — development log

Development data: the 24 tranche-1 record studies of dataset
[`retina-acuity-2026-10-29`](../../../data/retina-acuity-2026-10-29/README.md), listed as
development in
[`tranche-2/split-reservation-1.0.0.json`](../../../data/retina-acuity-2026-10-29/tranche-2/split-reservation-1.0.0.json):
the 15 development records of model 1.0 (13 public, 2 private) and the 9 sealed records
consumed by its one-shot test (private). Together 63 series: 29 control group means, 6
treated group means and 28 per-eye series of one treated study. The sealed test set of
1.1 is tranche 2 (29 custodian records, 81 series), reserved before triage and unread;
no tranche-2 value is read before the 1.1 protocol is frozen. The tranche-2
author-overlap file links no two training studies and no training study to a sealed
one, so leave-one-study-out (LOSO) holds out single studies. Each round declares its
change and its adoption rule before its LOSO results are seen.

## Starting point (model 1.0.0 and its sealed failure)

Model 1.0.0 (candidate `v3`) failed all four hypotheses of its sealed test
([README](../model-1.0/README.md)). After unsealing, two causes were found:

1. **An unidentified early transient.** The law `change(t) = ell + e r(t) + s q(t)`
   kept the IOP law's transient `q(t) = exp(-(t - 1 day)/7 days)`. No development
   series had a visit before day 30, so the fit left `s` at about +2 logMAR at day 1.
   The one sealed study with a day-1 visit (now training study `rac-pmc5377272`; its two
   series are the only training series with a visit before month 1) had contexts with
   no training study, and its day-1 visit was read through that term.
2. **Steroid implant wear-off.** One dexamethasone implant's gain waned between months 2
   and 6, while the law kept it.

Structure of the 24 training studies (counts only): control contexts dme/anti_vegf 7
studies, dme/steroid_implant 4, rvo/anti_vegf 2, and 1 each for dme/steroid_injection,
namd/anti_vegf, other/laser, rvo/combination, rvo/steroid_implant and
uveitic/steroid_implant; three control series are single dexamethasone implants under
the retinal-thickness dose rule.

## Model

Generic eye model version 2 (`eye_recovery_population2`, unchanged) with the
configurations of `peykar.model_learning.retina_acuity_population_1_1`; series
`peykar.validation.eye_retina_acuity_data` (and, for `w3`, the same series with the
declared single-dose effect duration); scoring `peykar.validation.eye_additive_evaluation2`
in ETDRS letters (unchanged); operator `scripts/evaluate_retina_acuity_population_1_1.py`.

## Round 0 and round 1 — declared before results (2026-10-29)

- **w0**: model 1.0.0 itself (the frozen `v3` configuration), the reference. Its LOSO on
  the 24 studies is round 0.
- **w1**: `w0` without the early transient (`transient_months = None`): the law is
  `change(t) = ell + e r(t)`, so the early course is the recovery term's extrapolation,
  identified by the visits from month 1 on. Penalty modes and robust SDs lose the
  transient's entry (0.1, 0.1, 0.5 and 0.3, 0.3, 2.0).
- **w2**: `w0` with `treatment_fallback`: a context without training series takes the
  pooled level of the trained contexts of its treatment before the species level.
- **w3**: `w0` with wear-off of one dexamethasone implant: the retinal-thickness 1.1 dose
  rule (duration 3.5 months, logistic width 0.5 months) and 80% of the effect lost once
  the implant has worn off (`wear_off_loss = 0.8`), the retinal-thickness 1.1.0 setting.

Primary method of a candidate (the model 1.0 rule): among the population variants whose
next-observation coverage of group means lies inside both H1 windows ([0.70, 0.90] at
80%, [0.80, 0.97] at 90%), the lowest pooled next-observation RMSE; a variant within
0.05 letters of that minimum that comes earlier in the order Gaussian, t4, robust,
anchored, anchored t4 is preferred. If no variant lies inside both windows, the robust
variant is the primary and the miss is recorded. The same variant is the landmark
primary; the initial state uses `population_initial_state`.

Adoption rule (each of w1, w2, w3 against w0): adopted if its primary improves the
pooled group-mean RMSE by at least 0.10 letters in at least one task (next observation,
landmark, initial state), loses no more than 0.05 letters in any, and lies inside both
H1 windows whenever w0's primary does. If two or more are adopted, round 2 runs one
candidate that combines all adopted changes, adopted by the same rule against the
adopted single candidate with the largest summed gain over the three tasks (ties within
0.05 letters: the order w1, w3, w2). If none is adopted, w0 stays the model and no 1.1
protocol is frozen.

The protocol of a 1.1 version keeps H1–H4 of model 1.0 and adds H5 against model
1.0.0 trained on the same records and scored in the same sealed run (the IOP 1.3 form):
over common human group-mean targets, the 1.1 primary's pooled landmark RMSE and
initial-state RMSE (control group means) each lower than 1.0.0's, and its pooled
next-observation RMSE at most 0.05 letters higher.
