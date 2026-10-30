# Kmax model 1.0 — development log

Development data: the 25 development records (35 series) of dataset
[`cornea-kmax-2026-10-30`](../../../data/cornea-kmax-2026-10-30/README.md): 12 CC BY
records in `development/records` and 13 non-commercial or share-alike records kept
private in `.peykar/kmax-development-private/records`. Fifteen of the 40 reserved
development studies have no record (mostly complete-case analysis sets; see the dataset
README). The 35 series are 27 group means and 8 per-eye series (two studies), 33
control and 2 treated; contexts epithelium-off cross-linking 11 series (6 studies),
accelerated cross-linking 7 (6), ring segments 7 (6), cross-linking combined with
another treatment 4 (3), transepithelial cross-linking 3 (3) and observation 3 (3); no
development study has the context `other`. Thirteen series have one follow-up visit
(they enter the initial-state task only); 22 have two or more. One series has a visit
before month 1 (one week). No sealed value is read before the 1.0 protocol is frozen.
Studies of one author-overlap cluster (`author-overlap-1.0.0.json`, 2 development
clusters) leave training together in leave-one-study-out (LOSO). Each round declares
its change and its adoption rule before its LOSO results are seen.

## Model

Generic eye model version 2 (`eye_recovery_population2`, unchanged) with the
configurations of `peykar.model_learning.kmax_population`; series
`peykar.validation.cornea_kmax_data`; scoring `peykar.validation.eye_additive_evaluation4`
4.0.0 (version-2 configurations on harness 6, which is harness 2 with initial-state
intervals; the support hold on harness 5); operator `scripts/evaluate_kmax_population.py`.

The model works on the change of Kmax from its baseline value in dioptres,
`change(t)/5 D = ell + e r(t) + s q(t)` with `r = exp(-(t - 1 day)/tau)` and
`q = exp(-(t - 1 day)/7 days)`, and `ell = context level + b (Kmax0 - 55 D)/5 D`: the
level covariate is centred at a baseline Kmax of 55 D (the development baselines range
from 47 to 68 D). The series give the change as the exact pseudo-fraction
`exp(change/5 D)` and Kmax0 as `exp(Kmax0/5 D)`, so the generic model and harness apply
unchanged and every comparator becomes its additive analogue. The additive evaluation
rewrites forecasts, observations and intervals in tenths of a dioptre and recomputes
the scores: one reported pp is 0.1 D, so the per-series criteria read RMSE ≤ 0.5 D,
maximum ≤ 1.0 D and, for forecasts, at least 0.05 D below persistence. There is no
target comparator (no treatment aims at a fixed Kmax); the initial state is compared
with the pooled training curve. Group means whose group size the article does not give
get the noise of a group of the reference size.

Known course, from the clinical literature and not from these data: after
epithelium-off cross-linking Kmax often steepens during the first month and then
flattens slowly for years; ring segments flatten Kmax within weeks and then hold;
untreated progressive keratoconus steepens. After its 7-day transient the law is
monotone, so an early steepening followed by a slow flattening cannot be followed by
`k0`.

## Round 1 — declared before results (2026-10-30)

- **k0**: contexts `human/procedure`, grouping `("context",)`, a study effect per
  procedure arm, the generic constants of the acuity and refraction models otherwise.
- **k1**: k0 without the 7-day early transient (`transient_months = None`), as retinal
  acuity 1.1 adopted after its early term had been read from no data. Only one
  development series has a visit before month 1.
- **k2**: k0 with a treatment-family level: contexts `human/<procedure>/<family>` and
  grouping `("treatment", "context")`, so the hierarchy is species → family
  (cross-linking: epithelium-off, accelerated, transepithelial, combined and `other`;
  ring segments; observation) → procedure. Cross-linking protocols then share a
  family mean instead of the species mean, which also holds ring segments and untreated
  eyes that move in opposite directions.
- **k3**: k0 with the support hold (`eye_support.SupportConfig`): the law is held flat
  before the earliest training visit of a context (of the species for an untrained
  context).
- **k4**: k0 with persistent (Ornstein–Uhlenbeck) deviations from the law in
  forecasting only: 80% of the residual variance as a deviation process with a 12-month
  correlation time, the setting retina model 1.0 adopted. The initial state is k0's.
- **k5**: k0 with a one-month early term instead of the 7-day transient
  (`transient_months = 1`), for the steepening in the first month after cross-linking.

Adoption rule (as for the other components, in reported pp, here 0.1 D): a candidate
replaces k0 only if, on human group means, it improves the pooled RMSE of its primary
method by at least 0.10 pp in at least one task (initial state, next observation,
landmark), loses no more than 0.05 pp in any, and its primary's next-observation
coverage lies inside both H1 windows. The primary method is the lowest pooled
group-mean next-observation RMSE among the five population variants whose group-mean
coverage lies inside both H1 windows ([0.70, 0.90] and [0.80, 0.97]); ties within
0.05 pp go to the simpler variant (Gaussian). If no population variant lies inside both
windows, the Gaussian variant is the primary and the miss is recorded. The landmark
primary is the next-observation primary; the initial-state primary is
`population_initial_state`. If several candidates meet the rule, the one with the
largest summed gain over the three tasks is adopted, and round 2 tests the adopted
candidate combined with the next-best qualifying change, when the two are compatible
(the support hold excludes deviations), against the adopted one under the same rule.
Each candidate runs as one process over the 25 studies; the comparison is
`.peykar/kmax_compare.py`.

## Round 1 — results (2026-10-30)

LOSO over the 25 training studies (39 next-observation and landmark targets of human
group means, 60 initial-state targets of control group means;
`.peykar/kmax_compare.py`, `.peykar/kmax-1.0-dev/round1-compare.txt`; RMSE in pp of
0.1 D, signed error = forecast − observed, positive = forecast steeper than observed).
Each run took about two minutes.

| candidate | primary | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|---|
| k0 | Gaussian (no variant inside) | 7.053 (+0.06) | 11.433 (−0.16) | 15.574 (+2.50) | 0.692 / 0.744 |
| k1 (no transient) | Gaussian (none inside) | 6.518 (−0.09) | 11.078 (−0.55) | 15.406 (+2.45) | 0.667 / 0.718 |
| k2 (family level) | Student-t | 6.763 (+0.05) | 11.599 (−0.91) | 15.513 (+0.79) | 0.718 / 0.821 |
| k3 (support hold) | Gaussian (none inside) | 6.795 (−0.12) | 10.995 (−0.95) | 15.357 (+2.29) | 0.692 / 0.744 |
| k4 (deviations) | Gaussian (none inside) | 6.748 (+0.14) | 11.452 (−0.22) | 15.574 (+2.50) | 0.667 / 0.769 |
| k5 (one-month term) | anchored Student-t | 7.127 (−0.78) | 15.705 (−3.90) | 17.660 (+6.03) | 0.718 / 0.846 |
| persistence | — | 6.592 (+2.15) | 12.364 (+3.64) | — | — |
| pooled-curve increment | — | 7.992 (+0.50) | 12.175 (+0.68) | — | — |
| analog kNN | — | 7.409 (−2.36) | 17.452 (−8.89) | — | — |
| pooled training curve | — | — | — | 13.889 (+1.56) | — |

- **No candidate meets the rule.** k1 gains 0.168, 0.536 and 0.356 pp (initial state,
  next observation, landmark) and k3 0.217, 0.258 and 0.438 pp, but neither has a
  population variant inside both coverage windows. k2 gains 0.061 and 0.291 pp and loses
  0.165 pp in the landmark task; k4 gains 0.305 pp in the next observation only and
  loses 0.018 pp in the landmark task; k5 loses in every task (its one-month term is read
  as a large early flattening).
- The 7-day transient of k0 is fitted from almost no data: at the species level its
  value at day 1 is −0.51 model units (about −2.5 D) with an SD of 0.75. Removing it
  (k1) or holding the law before the earliest training visit (k3) improves all three
  tasks; this is the retinal-acuity failure seen in development before any sealed test.
- **Intervals are too narrow.** Twelve of 39 next-observation targets fall outside k0's
  80% interval: the continued flattening of one study's cross-linking groups from 3 to
  8 years (−3.6 and −3.3 D observed against −2.1 and −1.9 D forecast), the steepening of
  untreated eyes, and the late re-steepening of one transepithelial series. The noise
  centre of group means is the median of six studies' residual SDs (0.040 model units,
  0.20 D); the other studies have no residual degree of freedom.
- **Initial state:** the population law is better than the pooled training curve for
  epithelium-off and accelerated cross-linking (9.63 against 10.66 pp and 10.63 against
  10.85), and worse for transepithelial cross-linking (16.00 against 8.34), observation
  (12.11 against 8.28) and the combined treatments; ring segments have the largest
  errors under both (24.86 and 24.33 pp: segment flattening differs by several dioptres
  between studies). Two of the three transepithelial studies form an author cluster and
  leave training together.
- The population law beats persistence and the pooled-curve increment for
  epithelium-off and accelerated cross-linking, where Kmax keeps flattening, and loses to
  persistence for ring segments, transepithelial cross-linking and untreated eyes, where
  it changes little after the first visit.

## Round 2 — declared before results (2026-10-30)

Both candidates start from k1, the round-1 change with the largest summed gain, and
target its interval coverage:

- **k6**: k1 with forecast-only pooled noise (`forecast_noise_pooling = "pooled"`, as
  retina model 1.1): forecasts add independent noise so that their noise centre is the
  pooled residual variance (summed squares over summed degrees of freedom), which is
  unbiased when each study has few degrees of freedom; likelihoods and posteriors keep
  the median centre, so point forecasts stay close to k1's.
- **k7**: k1 with pooled noise in likelihoods and forecasts (`noise_pooling =
  "pooled"`).

Adoption rule as in round 1, against k0. If both meet it, the one with the larger
summed gain is adopted. If neither meets it, k0 stays the model 1.0 candidate and the
freeze decision records its development failures. No round 3 is planned.
