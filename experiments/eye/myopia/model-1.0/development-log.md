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

## Round 2 — results (2026-10-30)

The m1 rerun through `eye_additive_evaluation5` (`.peykar/myopia-1.0-dev/round2-m1-rerun`)
equals its round-1 report apart from the three evaluation-version fields. LOSO over the
same 22 studies (`.peykar/myopia-1.0-dev/round2-m3`, `round2-m4`,
`.peykar/myopia-1.0-dev/round2-compare.txt`; each run under a minute):

| candidate | primary | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|---|
| m1 (adopted in round 1) | anchored Student-t | 15.143 (+2.65) | 27.320 (+3.29) | 35.480 (−24.94) | 0.838 / 0.912 |
| m3 (zero start, rate) | robust | 8.975 (−2.90) | 17.066 (−6.47) | 15.314 (+2.98) | 0.794 / 0.912 |
| m4 (zero start, level) | Gaussian | 9.722 (−3.80) | 15.663 (−7.27) | 14.026 (−2.02) | 0.735 / 0.853 |

- **Both meet the rule** against m1 (m3 gains 20.166, 6.168 and 10.254 pp in the
  initial state, next observation and landmark, summed 36.588; m4 21.454, 5.421 and
  11.657, summed 38.532). **m4 is adopted** by the declared tie-break (larger summed
  gain).
- The zero start removes the shortening at day 1: under m4 the species law gives a
  change of 0.22 mm at 12 months (median over the LOSO fits), a time constant of 38
  months (31 to 46) and a long-term change of 0.82 mm at a baseline of 24.5 mm; under m3
  an initial rate of 0.38 mm per year and a time constant of 22 months. The initial state
  of m4 is better than the pooled training curve for single vision (20.7 against 35.3
  pp), orthokeratology (11.7 against 21.5) and `other` (8.1 against 17.0), and about equal
  for atropine (14.8 against 14.3), combinations (8.2 against 8.8), defocus spectacles
  (12.0 against 11.8) and the red-light series (13.4 against 13.5).
- On the 64 targets with an analog forecast, next observation: m4 8.57 and m3 8.47 pp
  against analog kNN 9.14; landmark: m4 10.11 and m3 9.81 against analog kNN 9.92. The
  difference between m3 and m4 in the pooled landmark RMSE comes from the 84- and
  132-month visits of the 11-year study (m3's shorter time constant levels off earlier).

## Freeze decision (2026-10-30)

Model 1.0.0 is **m4** (generic eye model 5 with the zero-start law in the long-term
change, `change(t)/1 mm = ell (1 − r(t))`, `ell = context level + b (AL0 − 24.5 mm)/1 mm`;
hierarchy species → treatment family → treatment; a study effect per family; forecast-only
pooled noise), as the round-2 rule decided. Primaries: `population_assimilation` for the
next observation (U2) and the landmark task (U3), `population_initial_state` for the
initial state (U1). The development analysis by the frozen publisher
(`development-loso-2026-10-30/`, `scripts/publish_myopia_population.py --development`) is
the development reference of the protocol:

- H1 passes: 50/68 = 0.735 of next-observation targets inside the 80% interval (window
  0.70–0.90) and 58/68 = 0.853 inside the 90% interval (window 0.80–0.97); both are near
  the lower edges.
- H2 passes: 9.72 pp against persistence 18.53, the last-two trend 15.75 and the
  pooled-curve increment 18.33 on 68 targets, and 8.57 against analog kNN 9.14 on 64.
- **H3 fails**: 15.66 pp against persistence 34.83 and the pooled-curve increment 27.04
  on 68 targets, but 10.11 against analog kNN 9.92 on 64.
- H4 passes: 14.03 pp against the pooled training curve's 21.72 over 127 targets of 59
  control group means; 19/59 groups within 5/10 pp (pooled curve 14/59).
- Reported only: landmark coverage 0.735/0.809; initial-state coverage 0.795/0.890; per
  series 23/40 within 5/10 pp for the next observation and 21/40 for the landmark task.

The protocol keeps H1–H4 with the comparators of the other eye components; the H3
failure on development is recorded as its development reference, as acuity 1.0 recorded
its H2 and H3 failures. There is no earlier myopia version, hence no H5.
`.peykar/check_myopia_redaction.py development` found no value of the 10 private
studies in the public development files. The sealed set (structure seen through the
custody verifier only, as disclosed above) has 23 series, 18 with forecast targets:
atropine 6, single vision 5, orthokeratology 4, combinations 2, multifocal contact
lenses 2 (an untrained treatment, which uses the optical family), red light 2,
defocus spectacles 1 and `other` 1.
