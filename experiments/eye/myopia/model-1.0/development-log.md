# Axial length in childhood myopia, model 1.0 — development log

Development data: the 22 development records (59 series) of dataset
[`myopia-axial-2026-10-30`](../../../data/myopia-axial-2026-10-30/README.md): 12 CC BY
records in `development/records` and 10 non-commercial or no-derivatives records kept
private in `.peykar/myopia-development-private/records`. Twelve of the 34 reserved
development studies have no record (nine complete-case sets, two with model-adjusted
estimates only and one analysis set from which a child was removed for fast progression;
see the dataset README). All 59 series are group means (43 absolute and 16 change
series) and have the protocol role `control` (the extraction keeps raw means only). Contexts: defocus spectacles 15 series,
orthokeratology 13, single-vision spectacles 13, atropine 8 (0.01% in 10 dose entries,
one each at 0.02%, 0.05% and 1%), atropine combined with an optical treatment 6, `other`
3 and repeated low-level red light 1; no development series has multifocal contact
lenses or an outdoor programme. 47 series are from East Asia, 8 from Europe and 4 from
South or Southeast Asia. Baseline axial length ranges from 23.11 to 25.60 mm (median
24.48 mm); mean age is reported for 43 series (8.5 to 13.0 years).

Nineteen series have one follow-up visit (they enter the initial-state task only) and 40
have two or more. First follow-ups are at 1 month (5 series), 3 (9), about 4 to 6 (23)
and 12 months (22); 46 series end at 12 months, 4 at 24, 2 at 18 and 2 at 132 months
(one study followed to 11 years). Five series have a visit at 1 month, none earlier.
Median 12-month elongation (first visit between 10 and 14 months, scaled to a year):
single vision 0.34 mm, `other` 0.31, atropine 0.29, orthokeratology 0.20, combination
0.14 and defocus spectacles 0.13 mm; the red-light series shortens by 0.05 to 0.07 mm
during its 6 months. (These are descriptive development facts, not criteria.)

Studies of one author-overlap cluster (`author-overlap-1.0.0.json`, 2 development
clusters) leave training together in leave-one-study-out (LOSO). Each round declares
its change and its adoption rule before its LOSO results are seen. No sealed value is
read before the 1.0 protocol is frozen. Disclosure: before round 1 was declared, the
developer saw the custody check of the sealed records (`.peykar/myopia-custody-check.json`:
counts by context, region, device, value kind and number of visits, no values). It
shows 23 sealed series, two of them with multifocal contact lenses, a context with no
development series.

## Model

Generic eye model version 2 (`eye_recovery_population2`, unchanged) with the
configurations of `peykar.model_learning.myopia_population`; series
`peykar.validation.myopia_axial_data`; scoring
`peykar.validation.eye_additive_evaluation4` 4.0.0 (version-2 configurations on harness
6, which is harness 2 with initial-state intervals); operator
`scripts/evaluate_myopia_population.py`.

The model works on the change of axial length from its baseline value in millimetres,
`change(t)/1 mm = ell + e r(t)` with `r = exp(-(t - 1 day)/tau)` and
`ell = context level + b (AL0 - 24.5 mm)/1 mm`: the level covariate is centred at a
baseline axial length of 24.5 mm (the development median is 24.48 mm). With a long
`tau` the eye elongates almost linearly for a few years and more slowly later; the
`tau` grid of the generic model reaches 120 months. The series give the change as the
exact pseudo-fraction `exp(change/1 mm)` and AL0 as `exp(AL0/1 mm)`, so the generic
model and harness apply unchanged and every comparator becomes its additive analogue.
The additive evaluation rewrites forecasts, observations and intervals in hundredths of
a millimetre and recomputes the scores: one reported pp is 0.01 mm, so the per-series
criteria read RMSE ≤ 0.05 mm, maximum ≤ 0.10 mm and, for forecasts, at least 0.005 mm
below persistence. The no-change comparator (`baseline_level`, the baseline axial
length) is reported only; the initial state is compared with the pooled training curve.
Group means whose group size the article does not give get the noise of a group of the
reference size.

Known course, from the clinical literature and not from these data: untreated myopic
children of school age elongate by roughly 0.3 to 0.4 mm a year, faster when younger;
the elongation slows with age. Atropine slows it with a dose response (0.01% least);
orthokeratology, defocus spectacles and multifocal contact lenses slow it by an optical
defocus; red light can shorten the eye for months through choroidal thickening; some
treatments are followed by a rebound when stopped. The law is monotone from day 1, so a
shortening followed by growth cannot be followed. Age is not a covariate: the generic
model has one level covariate (here the baseline axial length).

## Round 1 — declared before results (2026-10-30)

The base candidate carries two lessons of the Kmax model 1.0 development, decided before
any myopia run: no 7-day early transient (no development series has a visit before month
1, so the term would be read from no data) and forecast-only pooled noise (the median of
the studies' residual SDs gave too narrow intervals when most studies have few residual
degrees of freedom).

- **m0**: contexts `human/treatment` (single_vision, atropine, orthokeratology,
  defocus_spectacles, multifocal_contacts, red_light, combination, outdoor, other),
  grouping `("context",)`, a study effect per treatment arm (context), no early
  transient (`transient_months = None`, penalty modes (0.1, 0.1, 0.5) and robust SDs
  (0.3, 0.3, 2.0) for level, excess and ln tau), forecast-only pooled noise
  (`forecast_noise_pooling = "pooled"`), the level covariate centred at 24.5 mm, the
  generic constants otherwise.
- **m1**: m0 with a treatment-family level: contexts `human/<treatment>/<family>` and
  grouping `("treatment", "context")`, so the hierarchy is species → family → treatment.
  Families: control (single_vision, other), optical (orthokeratology,
  defocus_spectacles, multifocal_contacts), atropine (atropine, combination) and light
  (red_light, outdoor). An untrained treatment (multifocal contact lenses here) uses its
  family mean instead of the species mean. With the per-group study effect of the
  generic model the study effect is then per family, so arms of one study in the same
  family share it.
- **m2**: m0 with the median noise centre in forecasts too (`forecast_noise_pooling =
  None`): this tests whether the Kmax lesson on interval width transfers.

Adoption rule (as for the other components, in reported pp, here 0.01 mm): a candidate
replaces m0 only if, on human group means, it improves the pooled RMSE of its primary
method by at least 0.10 pp in at least one task (initial state, next observation,
landmark), loses no more than 0.05 pp in any, and its primary's next-observation
coverage lies inside both H1 windows. The primary method is the lowest pooled
group-mean next-observation RMSE among the five population variants whose group-mean
coverage lies inside both H1 windows ([0.70, 0.90] and [0.80, 0.97]); ties within
0.05 pp go to the simpler variant (Gaussian). If no population variant lies inside both
windows, the Gaussian variant is the primary and the miss is recorded. The landmark
primary is the next-observation primary; the initial-state primary is
`population_initial_state`. If both candidates meet the rule, the one with the larger
summed gain over the three tasks is adopted, and round 2 tests m1 combined with m2 (the
two changes are compatible) against the adopted one under the same rule. If no candidate
meets the rule, m0 is the model 1.0 candidate, provided its own primary lies inside both
windows; otherwise round 2 targets its coverage. Each candidate runs as one process over
the 22 studies; the comparison is `.peykar/myopia_compare.py`.
