# Retinal-thickness model 1.1 — development log

Development data: the 65 tranche-1 record studies of dataset `retina-cst-2026-10-28`
(46 development records and the 19 tranche-1 sealed records consumed by the one-shot
test of model 1.0.0; 180 series). The tranche-2 reservation lists them as development
([`tranche-2/split-reservation-1.0.0.json`](../../../data/retina-cst-2026-10-28/tranche-2/split-reservation-1.0.0.json)).
No tranche-2 value is read before the 1.1 protocol is frozen. Studies of one
author-overlap cluster (`tranche-2/author-overlap-1.0.0.json`, 3 development clusters)
leave training together in leave-one-study-out (LOSO). Each round declares its change
and its adoption rule before its LOSO results are seen.

## Starting point and targets

Model 1.0.0 is candidate `r3` of `retina_thickness_population` (generic model 1, the
1.0.0 harness). Its sealed test passed all five hypotheses but showed two failures,
which version 1.1 addresses:

1. **Wear-off of one dexamethasone implant.** Thickness falls for about two months and
   rises again from month 3 as the implant is used up. The kinetic law of model 1.0.0
   is monotone after the first weeks, so it missed the rise by 15–30 pp, and implant
   series were forecast worse than by persistence.
2. **Contexts without training series** fell back to the species mean, which is
   dominated by anti-VEGF series (initial-state error 17.17 pp against 10.25 pp in
   trained contexts).

## Model

`peykar.model_learning.eye_recovery_population2` (generic version 2) adds three
optional extensions to generic model 1, each off by default; with defaults it
reproduces model 1 bit for bit (`tests/unit/test_eye_recovery_population2.py`, which
also checks the new likelihoods against the exact joint Gaussian):

- **Wear-off.** A series whose group received one dose with a declared effect duration
  `D` follows `ln f(t) = L(t) (1 - rho W(t))`, `W(t) = 1 / (1 + exp(-(t - D) / w))`,
  where `L` is model 1's law. Given `ln tau` this stays linear, so posteriors stay
  exact Gaussian mixtures; the dimension of the parameters does not change.
- **Treatment fallback.** A context without training series takes the mean location of
  the trained contexts of the same treatment, plus the between-context variance.
  Trained contexts are unchanged.
- **Early noise** (for IOP 1.2; not used here).

The harness `peykar.validation.eye_recovery_evaluation2` runs the 1.0.0 tasks and
baselines unchanged; it differs only in calling generic version 2 and in passing the
declared duration to prior-only (initial-state) forecasts. The configurations and the
dose rule are in `peykar.model_learning.retina_thickness_population_1_1`; the series
adapter is `peykar.validation.eye_retina_data_1_1`; the operator is
`scripts/evaluate_retina_population_1_1.py`.

## Round 0 — in-sample look at the training implant series (2026-10-28)

The training records hold nine single-dose dexamethasone implant series (eight
studies, one of them a treated arm) and three single fluocinolone implant series
(two studies). Every training group names its agent (115 of 115 groups).

- Dexamethasone series with visits after month 3 rise again between months 2 and 6:
  to 0.97, 0.89, 0.95 and 1.00 of the baseline at month 6, after a low near 0.56–0.73
  at months 1–2; one uveitic series stays low until month 4.4 and rises by month 5.9.
- Fluocinolone series stay low through month 12 (no wear-off within follow-up).
- The fraction of the treatment effect (in `ln f`) lost by month 6 is about 0.6–1.0,
  mean about 0.8.

**Declared dose rule** (fixed before any LOSO run of version 1.1; the constants come
from the implant's known course and these training series, not from tranche 2): a
group that received one dexamethasone implant (regimen `single`; treatment
`steroid_implant` unless the agent names fluocinolone, or treatment `combination` when
the agent names a dexamethasone implant) has an effect duration `D = 3.5` months.
Wear-off width `w = 0.5` months; lost fraction `rho = 0.8`. All other groups have no
duration.

Note on outside sources: on 2026-10-28, one web search about the implant's time course
returned search-result snippets. The only snippet with tabulated thickness values was
a training study (ret-pmc11943906, the same article). One result listed a tranche-4
article (PMC12744596) by title only, without any value. The two other snippets with
values belong to articles outside the screened set. No tranche-2 article appeared. No
further searches on outcomes of this kind are made before the freeze.

## Round 1 — declared before results (2026-10-28)

Candidates (`retina_thickness_population_1_1.DEVELOPMENT_CANDIDATES`), each run by
LOSO over the 65 training studies:

- **s0**: model 1.0.0 (`r3`) in generic version 2. Check: its LOSO report must equal
  that of model 1.0.0 run with the 1.0.0 harness (`--model 1.0.0`) on the same data.
- **s1**: s0 with the treatment fallback.
- **s2**: s0 with the dose rule's wear-off (`rho = 0.8`).
- **s3**: s1 and s2 together.

Adoption rule (as for model 1.0): a candidate replaces the incumbent only if, on group
means, it improves the pooled RMSE of its primary method by at least 0.10 pp in at
least one task (initial state, next observation, landmark) and loses no more than
0.05 pp in any. The primary method of a candidate is the probabilistic variant with
the lowest pooled next-observation RMSE on group means; variants within 0.05 pp count
as tied and the Gaussian one is preferred. Procedure: s1 and s2 are each compared with
s0. If both are adopted, s3 is adopted when it meets the rule against s0 and loses no
more than 0.05 pp in any task against either s1 or s2; otherwise the better of s1 and
s2 (larger summed gain) is kept. If only one is adopted, it is the incumbent.

### Round 1 results (LOSO, 65 studies, group means; primary = lowest next-observation variant)

| task | model 1.0.0 (1.0.0 harness) | s0 (robust) | s1 (robust) | s2 (Gaussian) | s3 (Gaussian) | persistence | pooled curve |
|---|---|---|---|---|---|---|---|
| next observation, 243 targets | 7.204 | 7.204 | 7.225 | **6.375** | 6.381 | 7.817 | — |
| landmark, 243 targets | 9.047 | 9.047 | 9.056 | **7.945** | 7.950 | 11.846 | — |
| initial state, 290 targets | 13.285 | 13.285 | 13.302 | **12.163** | 12.155 | — | 17.350 |
| next-obs coverage 80% / 90% (primary) | 0.704 / 0.807 | 0.704 / 0.807 | 0.704 / 0.811 | 0.671 / 0.782 | 0.671 / 0.786 | — | — |

- **Check passed**: s0's report equals that of model 1.0.0 with its own harness in every
  score of every study.
- **s1 is not adopted**: no task gains 0.10 pp (it loses 0.02, 0.01 and 0.02 pp). The
  treatment fallback did not help the 26 initial-state targets of untrained contexts
  either (16.92 against 16.77 pp).
- **s2 is adopted**: it gains 0.83 pp (next observation), 1.10 pp (landmark) and 1.12 pp
  (initial state) and loses nothing. On the implant contexts (25 next-observation and
  landmark targets, 33 initial-state targets) the errors fall from 12.08 to 6.05,
  14.91 to 6.25 and 18.16 to 9.49 pp; persistence is 11.17 and 13.64 pp. These implant
  gains are optimistic: the dose-rule constants were set after looking at the same
  training implant series. Only one candidate of s1 and s2 is adopted, so s3 is not
  considered. **Incumbent after round 1: s2.**

Calibration problem found in round 1 (it is not caused by the wear-off): the
next-observation coverage of the Gaussian variant is 0.671 / 0.782 for s2 and also for
model 1.0.0 on these 65 studies (it was 0.738 / 0.834 on the 46 development studies in
model 1.0 development). Both are below the H1 windows ([0.70, 0.90] and [0.80, 0.97]).
The shortfall is uniform over group sizes, origins and history lengths (RMS of
standardized log errors 1.3–1.8 instead of 1). Cause: the noise centre is the median
of the few study noise estimates (studies with at least five follow-ups), each with
one or two degrees of freedom. It fell from 0.082 (seven estimates) to 0.045 (nine
estimates) because two added estimates sit at the 0.01 floor.

## Round 2 — declared before results (2026-10-28)

**s4**: s2 with a pooled noise centre (`noise_pooling = "pooled"` in generic version 2):
`sqrt(sum(dof * sd^2) / sum(dof))` over the same study estimates instead of their
median, so that estimates with one degree of freedom no longer decide the centre. With
the median the population fit is unchanged, so s4 changes only the noise level of
fits, posteriors and forecasts.

Adoption rule for this calibration round: s4 replaces s2 if its primary's pooled 80%
and 90% next-observation coverage on group means both move closer to 0.80 and 0.90 and
both lie inside the H1 windows, and no task's pooled RMSE rises by more than 0.05 pp.
The primary variant is chosen as in round 1.

### Round 2 results (LOSO, 65 studies, group means)

| task | s2 (Gaussian) | s4 (Gaussian) |
|---|---|---|
| next observation, 243 targets | 6.375 | 6.473 |
| landmark, 243 targets | 7.945 | 8.003 |
| initial state, 290 targets | 12.163 | 11.755 |
| next-obs coverage 80% / 90% | 0.671 / 0.782 | **0.765 / 0.864** |

The pooled noise centre is 0.075 (median 0.045). s4 brings both coverages inside the
H1 windows and improves the initial state by 0.41 pp, but next observation rises by
0.098 pp and landmark by 0.058 pp, more than the declared 0.05 pp: **s4 is not
adopted; s2 stays.** The larger noise also changes the posteriors (forecasts follow
a series' own observations less), which costs point accuracy.

## Round 3 — declared before results (2026-10-28); last structural round

**s5**: s2 with the pooled noise centre used for forecast intervals only
(`forecast_noise_pooling = "pooled"` in generic version 2). Likelihoods, posteriors
and the population fit keep s2's median centre; each forecast adds independent noise
with variance `sigma^2 ((c_pooled / c_median)^2 - 1)`, where both centres come from the
same fit (no new constant), so that the forecast noise matches the pooled centre.
Point forecasts stay those of s2 apart from small shifts of mixture medians.
Adoption rule as in round 2 (against s2). After round 3 only already declared
candidates may be chosen, by the same rules.

### Round 3 results (LOSO, 65 studies, group means)

| | s2 (Gaussian) | s5 (Gaussian) |
|---|---|---|
| next observation, 243 targets | 6.375 | 6.368 |
| landmark, 243 targets | 7.945 | 7.952 |
| initial state, 290 targets | 12.163 | 12.153 |
| next-obs coverage 80% / 90% | 0.671 / 0.782 | **0.848 / 0.930** |
| landmark coverage 80% / 90% (reported) | 0.667 / 0.798 | 0.802 / 0.905 |
| initial-state coverage 80% / 90% (reported) | 0.724 / 0.872 | 0.793 / 0.917 |

Both next-observation coverages move into the H1 windows, and no task's RMSE rises by
more than 0.01 pp: **s5 is adopted.** A rerun of s2 with the final code (after the
generic module gained the forecast-pooling and study-effect options) reproduced its
round-1 report in every score (`final-code-s2`).

## Freeze decision (2026-10-28)

- Primary candidate **s5** (generic version 2; s0's law plus the declared wear-off of
  single dexamethasone implants and forecast noise from the pooled centre). U1 primary
  `population_initial_state`; U2 and U3 primary `population_assimilation` (Gaussian;
  the robust variant is within 0.05 pp).
- Reference: the frozen model 1.0.0 (`r3`, its own harness), scored in the same sealed
  run. H5 tests all three tasks against it, because the adopted changes target both
  forecasts (wear-off, intervals) and the initial state (wear-off).
- Expected, from development: the gain over 1.0.0 comes mostly from implant series,
  and these constants were set with the training implant series in view; the sealed
  set has 20 series marked by the dose rule (4 group means and 15 single eyes with
  treatment `steroid_implant`, 1 combination group).
- No further development rounds; thresholds and windows as in protocol 1.0.0.
