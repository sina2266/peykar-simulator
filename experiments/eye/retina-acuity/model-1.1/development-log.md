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

## Round 1 results (LOSO on the 24 training studies, 2026-10-29)

Human group means: 100 next-observation and landmark targets in 35 series, and 117
initial-state targets in 29 control group means. Pooled RMSE in ETDRS letters
(`.peykar/retina_acuity11_compare.py` on the private reports
`.peykar/retina-acuity-1.1-dev/round1-w{0,1,2,3}/report.json`).

| candidate | primary | next observation | landmark | initial state | next-observation coverage 80% / 90% |
|---|---|---|---|---|---|
| w0 (1.0.0) | t4 | 4.327 | 5.827 | 7.615 | 0.770 / 0.830 |
| w1 | anchored t4 | 3.460 | 3.965 | 4.873 | 0.840 / 0.910 |
| w2 | t4 | 4.327 | 5.814 | 7.613 | 0.770 / 0.830 |
| w3 | t4 | 4.515 | 6.516 | 8.790 | 0.770 / 0.850 |

Comparators on the same targets (next observation / landmark): persistence 3.468 /
4.463, pooled-curve increment 3.805 / 5.781, last-two trend 11.072 / 40.148, analog kNN
7.437 / 11.045. Initial state: pooled training curve 5.524.

- **w1** primary: all five population variants lie inside both H1 windows; the lowest
  next-observation RMSE is anchored t4 (3.460); anchored (3.519) is 0.059 letters above,
  outside the 0.05 margin, so anchored t4 is the primary. Against w0: −0.867, −1.861 and
  −2.742 letters, coverage inside both windows. **Adopted.**
- **w2**: −0.000, −0.012, −0.002 letters, no gain of 0.10. Not adopted.
- **w3**: +0.188, +0.689, +1.175 letters, losses above 0.05. Not adopted.

Only w1 is adopted, so by the declared rule there is no round 2.

Per study (primary against primary): the training study with day-1 visits
(`rac-pmc5377272`) gains most: next observation 11.6 → 4.5, landmark 18.6 → 6.2 and
initial state 33.3 → 12.5 letters. The study whose control series is a single
dexamethasone implant (`rac-pmc11921743`) gets worse: next observation 8.3 → 10.2 and
landmark 13.4 → 15.4 letters. Wear-off is still not modelled; the declared wear-off
change (w3) made the pooled results worse.

## Freeze decision (2026-10-29)

- Model 1.1.0 = **w1**: the law `change(t) = ell + e r(t)`, `r = exp(-(t - 1 day)/tau)`,
  with every other setting of model 1.0.0. Primaries: U2 and U3
  `population_anchored_t4`, U1 `population_initial_state`. The frozen 1.0.0 (`w0`,
  primary `population_assimilation_t4`) is scored in the same sealed run as the H5
  reference.
- H1–H4 are those of model 1.0 with the same thresholds and comparators; H5 is the one
  declared above. Verdicts are computed by `scripts/publish_retina_acuity_population_1_1.py
  --sealed` from the private sealed reports.
- Development status of w1 with that publisher (`--development`, w0 as reference):
  H1 pass (0.840 / 0.910); H2 pass, but against persistence only by 0.01 letters (3.46
  against 3.47 on 100 common targets; last-two trend 11.07, pooled-curve increment 3.80,
  analog kNN 3.37 against 7.44 on 98); H3 pass (3.97 against persistence 4.46,
  pooled-curve increment 5.78, analog kNN 3.86 against 11.04 on 98); H4 pass (4.87
  against 5.52); H5 pass (3.46 against 4.33, 3.97 against 5.83, 4.87 against 7.62).
  H2 against persistence is a near tie on development, so a sealed H2 failure would not
  be surprising. Reported only: landmark coverage 0.890 / 0.930, initial-state coverage
  0.735 / 0.829; on the same LOSO, model 1.0.0's own H1–H4 (its primaries) are pass,
  fail, fail and fail (next observation 4.33 against persistence 3.47; landmark 5.83
  against persistence 4.46; initial state 7.62 against the pooled training curve 5.52).
- No tranche-2 acuity value has been read. Structure seen before the freeze (counts and
  visit times only) is listed in the protocol.
