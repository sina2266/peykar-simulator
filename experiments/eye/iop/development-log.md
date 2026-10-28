# IOP-after-surgery model — development log (frozen with protocol 1.0.0)

Every declaration below was written before the run it governs. Only development
records were used: 47 records (101 series) of the 54 reserved development studies.

## Round 0 — in-sample comparisons (development only), 2026-10-28

[`round0_law_comparison.py`](round0_law_comparison.py) fits three-parameter laws
`ln f = ell + (s - ell) h(t)` by least squares over a 400-point tau grid to every
development series with at least four follow-ups. Dof-adjusted residual SD of `ln f`:

| law | group means (169 dof) | single eyes (28 dof) |
|---|---|---|
| exponential | **0.0682** | **0.1851** |
| hyperbolic (p = 1) | 0.0716 | 0.1884 |
| power p = 0.5 | 0.0760 | 0.1913 |
| power p = 2 | 0.0698 | 0.1865 |

Adopted: one exponential return, parametrized by the day-1 deviation `e` from the
long-term level, `r(t) = exp(-(t - t1)/tau)` with `t1` = 1 day.

Dependence on the preoperative pressure: for control group means, the mean `ln f`
over follow-ups at or after 3 months against `x = ln(IOP0/20 mmHg)` gives
`ln f_late = -0.343 - 0.765 x` (76 series). With context intercepts the slope is
-0.767; per-context slopes range from -0.49 to -1.34. Postoperative pressure
therefore regresses strongly toward a common target: the elasticity of postoperative
on preoperative IOP is about 0.23. Adopted: one shared level slope on `x`, estimated
as a fixed effect inside each REML fit and never fixed from these in-sample numbers.
The early deviation barely depends on `x` (`-0.100 + 0.100 x`, 44 series) and is not
modelled.

Structure: one hierarchy per species with procedure contexts and no mechanism
families. A procedure without training studies falls back to the species mean
instead of an assigned family. This follows the corneal thickness model's sealed
failure, where laser capsulotomy assigned to the graft family was badly predicted.

Smoke fit on all development controls (in-sample, 2.0 s): slope -0.738 (SD 0.061);
long-term fraction at 20 mmHg 0.68–0.71 in every context (context SD 0.039).
Contexts differ mainly in the day-1 deviation: subconjunctival MIGS -0.70,
trabeculectomy -0.32, tubes and non-penetrating surgery -0.25, cataract -0.11, SLT
0.00, angle surgery and cyclodestruction +0.10. Noise centres: group means 0.055,
single eyes 0.164.

## Round 1 declaration (before the first LOSO run)

- Model as above. Variants scored: population_assimilation (Gaussian prior), _t4
  (Student-t 4), _robust (0.2 broad mixture), population_anchored (flat level),
  population_anchored_t4 and weak_prior (unpooled). Baselines: persistence,
  preoperative_level, last_two_log_linear, reference_to_last_log_linear,
  pooled_curve_ratio and analog_knn.
- Primary-selection rule for U2/U3 (as for the thickness model): among the five
  population variants, the primary is the one with the lowest pooled development
  next-observation RMSE whose pooled next-observation 80% and 90% coverages lie
  within [0.70, 0.90] and [0.80, 0.97]. Ties within 0.05 pp go to the simpler
  variant, in the order Gaussian, t4, robust, anchored, anchored_t4. U1 primary:
  population_initial_state.
- Any change after round 1 is recorded here, with its reason, before the next run.

## Round 1 result (LOSO, 47 studies, 162.6 s; report SHA 0725e53d…)

Next observation (97 series, 377 targets): robust 9.45 pp (44 series within 5/10 pp,
20 meet all criteria; coverage 0.780/0.865), t4 9.54, Gaussian 10.32, anchored
10.50, anchored_t4 17.75; persistence 10.15, analog 11.05, pooled ratio 12.43. Group
means only (80 series): robust 6.32 against persistence 7.67. Landmark: robust 13.38
against persistence 15.89 (group means 8.66 against 14.61). Initial state, 77 control
group means: 11.90 pp (21 within) against the pooled curve's 21.44 (5 within).
Single eyes fail in every task.

Defect found: point forecasts were posterior means of `exp(ln f)`. Under the
Student-t scale mixture the small-scale nodes inflate the variance of `e` and
`ln tau`, and the mean of a lognormal with that variance explodes (anchored_t4
landmark max 480 pp; t4 single-eye initial state 343 pp). The mean of `exp` of a
Student-t variable does not exist, so this is a definition problem, not noise.

## Round 2 declaration (before running)

Change: every point forecast (all population variants, U1–U3) is the median of the
posterior predictive distribution of the observation, its 0.5 quantile, instead of
the posterior mean of the model curve. For group means the two differ by about
`exp(sigma²/2) - 1` = 0.15%; the median is finite for heavy-tailed priors. Nothing
else changes. Adoption rule: round 2 replaces round 1 if the variant chosen by the
primary-selection rule has pooled next-observation and landmark RMSEs no worse than
round 1's best (9.45 and 13.38 pp) by more than 0.05 pp; otherwise round 1 is kept.
The primary is then selected by the round-1 rule.

## Round 2 result (LOSO, 47 studies, 167.2 s; report SHA d5a39284…)

A repeat run gave a byte-identical report. Public artifacts are in
[`development-loso-2026-10-28/`](development-loso-2026-10-28/tables.md).

- Next observation (97 series, 377 targets): robust 9.455 pp (45 within 5/10 pp, 19
  meet all criteria; coverage 0.780/0.865), t4 9.511, Gaussian 10.260, anchored
  10.273, anchored_t4 10.348 (round 1: 17.75); persistence 10.15.
- Landmark: robust 13.389 (coverage 0.732/0.870), t4 13.522, Gaussian 13.663,
  anchored_t4 14.251 (round 1: 88.11, max 480 pp), anchored 14.309; persistence 15.89.
- Initial state, 77 control group means: Gaussian 12.04 pp and t4 12.03 (20 within
  5/10 pp), against the pooled curve's 21.44 (5 within). The single-eye t4 initial
  state max fell to 101.5 pp (round 1: 343 pp).

The median removed the explosions; the other variants' ranks barely moved.

Adoption rule: robust next observation 9.4549 ≤ 9.4539 + 0.05 and landmark 13.3888 ≤
13.3761 + 0.05, so round 2 is adopted. Primary by the round-1 rule: robust. Its
next-observation RMSE (9.4549 pp) is 0.056 pp below t4's (9.5108), just over the
0.05 pp tie margin, and its coverage lies inside both windows. This margin is too
small to call robust better than t4; the declared rule decides. U1 primary:
population_initial_state. No further model change was made before the freeze.

Development subsets (reference only, from the round-2 report):

- Group means, next observation (80 series, 323 targets): robust 6.34 pp against
  persistence 7.67, last-two trend 38.07, pooled ratio 10.51 and analog 8.64 (319
  common targets); coverage 0.814/0.895.
- Group means, landmark: 8.66 against 14.61, 17.98 and, on 319 common targets,
  8.60 against analog 14.16; coverage 0.762/0.907.
- Single eyes, next observation (17 series, 54 targets): robust 19.58 against
  persistence 19.17, which is worse, with coverage 0.574/0.685 (intervals too
  narrow). Landmark: 28.34 against 22.03.
- Group means by context, next observation: robust is below persistence in every
  context except laser trabeculoplasty (one study, 8 targets, no other training study
  for that context: 10.93 against 8.28).

## Context-of-use declaration before the freeze

Single-eye forecasts were worse than persistence and under-covered in development.
The tested sealed hypotheses are therefore restricted to group means (U2, U3) and
control group means (U1). Single eyes, all-series pooled values, treated groups and
per-context values are reported, not tested. When this was decided, the developer
had seen the custodians' structural summaries but no IOP value: 25 records and 68
series (44 group means and 24 single eyes; 53 control and 15 treated), with contexts
angle MIGS 18, cyclodestruction 17, tube 17, trabeculectomy 9, cataract 3,
non-penetrating 2 and subconjunctival MIGS 2.

Declared hypotheses (group means, common targets):

- H1: pooled next-observation coverage of the primary within [0.70, 0.90] (80%) and
  [0.80, 0.97] (90%).
- H2: pooled next-observation RMSE of the primary lower than each of persistence,
  last_two_log_linear, pooled_curve_ratio and analog_knn.
- H3: pooled landmark RMSE of the primary lower than each of persistence,
  pooled_curve_ratio and analog_knn; landmark coverage reported.
- H4: pooled initial-state RMSE of population_initial_state over control group means
  lower than pooled_training_curve; per-group criterion counts and coverage reported.
