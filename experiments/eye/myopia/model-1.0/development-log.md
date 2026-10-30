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

## Round 1 — results (2026-10-30)

LOSO over the 22 training studies (68 next-observation and landmark targets of human
group means, 127 initial-state targets of 59 control group means;
`.peykar/myopia_compare.py`, `.peykar/myopia-1.0-dev/round1-compare.txt`; RMSE in pp of
0.01 mm, signed error = forecast − observed in parentheses, positive = forecast longer
than observed). Each run took under a minute.

| candidate | primary | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|---|
| m0 | anchored Student-t | 15.385 (+2.90) | 27.496 (+3.43) | 35.876 (−26.56) | 0.824 / 0.912 |
| m1 (family level) | anchored Student-t | 15.143 (+2.65) | 27.320 (+3.29) | 35.480 (−24.94) | 0.838 / 0.912 |
| m2 (median forecast noise) | anchored Student-t | 15.386 (+2.90) | 27.497 (+3.43) | 35.877 (−26.56) | 0.824 / 0.912 |
| persistence | — | 18.525 (−13.29) | 34.828 (−21.60) | — | — |
| last-two trend | — | 15.752 (−0.66) | 20.063 (−1.06) | — | — |
| pooled-curve increment | — | 18.327 (−3.60) | 27.039 (−7.18) | — | — |
| analog kNN (64 targets) | — | 9.138 (−0.35) | 9.924 (−1.04) | — | — |
| pooled training curve | — | — | — | 21.716 (−2.03) | — |

- **m1 meets the rule** against m0: it gains 0.396 pp (initial state), 0.242 pp (next
  observation) and 0.176 pp (landmark), and its anchored Student-t variant lies inside
  both windows. **m2 does not** (−0.001 pp in every task; its primary's intervals equal
  m0's to three decimals). m1 is adopted.
- **The free-start law is wrong for this quantity.** Over the 22 LOSO fits the species
  law of m0 gives a change of −0.66 mm at day 1 (median; range −0.76 to −0.51 mm), and
  m1 −0.64 mm (−0.76 to −0.52): the population reads the value at the reference time from the curvature
  of visits at 3 to 24 months, as the Kmax transient was read from no data. A concave
  exponential that passes through near-linear growth must start below zero. Every
  context's initial state is too short (signed error −7 to −36 pp under m1) and worse
  than the pooled training curve: single vision 48.3 against 35.3 pp, orthokeratology
  37.0 against 21.5, combinations 32.8 against 8.8, atropine 25.9 against 14.3, defocus
  spectacles 21.9 against 11.8.
- **Forecasts follow the population slope rather than the series.** Given a first visit,
  a free start can explain it by the start value instead of the slope, so the next
  forecast keeps the population's growth: after a 6-month change of 0.06 mm the forecast
  of month 12 was 0.33 mm (observed 0.14; the line from the baseline through the first
  visit gives 0.12). On the 64 targets that have an analog forecast, m1's primary has
  12.54 pp against analog kNN 9.14, the pooled-curve increment 9.35 and the line from the
  baseline through the last visit 11.71. The four targets without an analog are the
  84- and 132-month visits of the one 11-year study, which every method misses by 0.3 to
  0.7 mm.

## Round 2 — declared before results (2026-10-30)

Both candidates start from m1 and replace its free start by a law that is unchanged at
the reference time, with new generic eye model version 5
(`peykar.model_learning.eye_recovery_population5`, unit-tested to reproduce version 2
exactly with a free start) and harness 7 (`eye_recovery_evaluation7`) through
`eye_additive_evaluation5` 5.0.0:

- **m3**: m1 with the zero-start law written in its initial rate (`start =
  "zero_rate"`, `R = 12` months): `change(t)/1 mm = v (tau/12)(1 − r(t))`, parameters
  `(v, ln tau)` with `v` the elongation per year at the start and `v tau/12` the
  long-term change. The family hierarchy, the study effects, the baseline covariate
  (`v = context rate + b (AL0 − 24.5 mm)`) and the anchored variants then act on the
  rate, which series of one or two years identify. Penalty modes (0.1, 0.5) and robust
  SDs (0.3, 2.0) for `(v, ln tau)`; everything else as m1.
- **m4**: m1 with the same law written in its long-term change (`start =
  "zero_level"`): `change(t)/1 mm = ell (1 − r(t))`, parameters `(ell, ln tau)`; the
  same constants. It differs from m3 only in which parameter the hierarchy and the
  covariate act on.

The operator now scores every candidate through `eye_additive_evaluation5`, which sends
version-2 configurations (m0–m2) to harness 6 exactly as before; m1 is rerun with it
before the round-2 runs, and its report must equal round 1's apart from the evaluation
versions. Adoption rule as in round 1, against m1. If both meet it, the one with the
larger summed gain is adopted; if neither, m1 stays the model 1.0 candidate and the
freeze decision records its development failures. A round 3, if any, is declared here
before it runs.
