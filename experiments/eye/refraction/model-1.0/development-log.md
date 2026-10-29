# Refraction model 1.0 — development log

Development data: the 24 development records (43 series, all group means) of dataset
[`cornea-refraction-2026-10-29`](../../../data/cornea-refraction-2026-10-29/README.md):
13 CC BY records in `development/records` and 11 non-commercial or publisher-licensed
records kept private in `.peykar/refraction-development-private/records`. Seven of
the 31 reserved development studies have no record (non-plano targets, outcome- or
attendance-selected analysis sets, medians). No sealed value is read before the 1.0
protocol is frozen. Studies of one author-overlap cluster (`author-overlap-1.0.0.json`,
2 development clusters) leave training together in leave-one-study-out (LOSO). Each
round declares its change and its adoption rule before its LOSO results are seen.

## Model

Generic eye model version 2 (`eye_recovery_population2`, unchanged) with the
configurations of `peykar.model_learning.refraction_population`; series
`peykar.validation.cornea_refraction_data`; scoring
`peykar.validation.eye_additive_evaluation` 1.0.0 on harness 2; operator
`scripts/evaluate_refraction_population.py`.

The model works on the change of the spherical equivalent from its preoperative value
in units of 5 D, `change(t)/5 D = ell + e r(t) + s q(t)` with `r = exp(-(t - 1 day)/tau)`
and `q = exp(-(t - 1 day)/7 days)`, and `ell = context level + b SE0/5 D` (full
correction of the attempted refraction is `b = -1`). The series give the change as the
exact pseudo-fraction `exp(change/5 D)` and SE0 as `exp(SE0/5 D)`, so every part of the
generic model and harness applies unchanged and every comparator becomes its additive
analogue. The additive evaluation rewrites forecasts, observations and intervals in
tenths of a dioptre and recomputes the scores: one reported pp is 0.1 D, so the
per-series criteria read RMSE ≤ 0.5 D, maximum ≤ 1.0 D and, for forecasts, at least
0.05 D below persistence.

## Round 1 — declared before results (2026-10-29)

- **f0**: contexts `species/procedure` (surface ablation, LASIK, lenticule
  extraction, other), grouping `("context",)`, a study effect per procedure arm, the
  generic constants otherwise.
- **f1**: f0 with a refractive-error level: contexts
  `species/<refractive error>/procedure`, grouping `("treatment", "context")`, so the
  hierarchy is species → procedure → myopia, hyperopia or mixed astigmatism (5
  hyperopic LASIK and 1 hyperopic surface-ablation series among the training
  controls; hyperopic corrections are known to regress more).

Adoption rule (as for the other components, in reported pp): f1 replaces f0 only if,
on human group means, it improves the pooled RMSE of its primary method by at least
0.10 pp in at least one task (initial state, next observation, landmark) and loses no
more than 0.05 pp in any. The primary method is the lowest pooled group-mean
next-observation RMSE among the population variants whose group-mean coverage lies
inside both H1 windows ([0.70, 0.90] and [0.80, 0.97]); ties within 0.05 pp go to the
simpler variant (Gaussian). If no population variant lies inside both windows, the
Gaussian variant is the primary and the miss is recorded.

## Round 1 — results (2026-10-29)

LOSO over the 24 training studies (98 next-observation and landmark targets, 124
initial-state targets of control group means). RMSE in dioptres; signed error =
forecast − observed (`.peykar/refraction_compare.py`). By the selection rule the
primary of both candidates is the robust variant `population_assimilation_robust`
(every variant lies inside both H1 windows); the Gaussian variant is far worse
(0.27 D), so a few series deviate from the population curve much more than a
Gaussian prior allows.

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|
| f0 | 0.153 (bias −0.03) | 0.326 (−0.04) | 0.552 (+0.09) | 0.806 / 0.898 |
| f1 (refractive-error level) | 0.157 (−0.03) | 0.322 (−0.02) | **0.538** (+0.09) | 0.806 / 0.908 |
| persistence | 0.163 | 0.348 | — | — |
| plano target (full correction) | 0.616 | 0.616 | 0.575 (+0.21) | — |

- **f1 adopted.** In reported pp (0.1 D) f1 changes next observation by +0.043,
  landmark by −0.040 and the initial state by −0.145 pp: one gain beyond the 0.10 pp
  threshold and no loss beyond 0.05 pp.
- The fitted level slope is b = −0.90 (SD 0.03): eyes keep on average 90% of the
  attempted correction. The fast settling time is about 7.5 days. Between studies the
  long-term change varies with an SD of about 0.48 D, which bounds the initial-state
  accuracy.
- The gains over the simple baselines are small: persistence is only 0.01 D worse for
  the next observation and 0.02 D worse for the landmark task, and the population
  initial state is 0.04 D better than assuming full correction (the plano target).
  Group-mean refraction after surgery is close to plano and stable after the first
  weeks.
- After round 1 the additive evaluation gained the declared comparator `target_level`
  (the plano target, change = −SE0; `AdditiveScale.target = 0 D`). f0 and f1 were
  rerun with it (`round1t-*`); their model forecasts and intervals are identical to
  the first runs. The rerun reports are the development evidence.

## Freeze decision (2026-10-29, before any sealed value is read)

No further round; the training set is small (24 studies) and further rounds would
mostly fit noise. Model 1.0.0 is candidate **f1** with primaries
`population_assimilation_robust` (U2, U3) and `population_initial_state` (U1). The
reference is **f0**, trained on the same records and scored in the same sealed run.
The tested hypotheses are those of the other components with two changes declared
here: H4 compares the initial state with the plano target (the clinical default),
not with the pooled training curve, which ignores the attempted correction (34.8 pp
in development); H5 tests the adopted change: over common human group-mean targets,
the initial-state RMSE of f1 lower than f0's, with next observation and landmark
each at most 0.05 pp (0.005 D) higher.
