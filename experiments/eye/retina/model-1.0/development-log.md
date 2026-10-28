# Retinal-thickness model 1.0 — development log

Development data: the 46 development records (98 series) of dataset
`retina-cst-2026-10-28`. No sealed record is read. Each round declares its change and
its adoption rule before its leave-one-study-out (LOSO) results are seen.

## Model

`peykar.model_learning.eye_recovery_population` is the generic form of IOP model
1.1.0 written for reuse: same kinetic law, hierarchy, exact mixture posteriors and
noise model, with the component's constants in a `RecoveryConfig`. On synthetic
IOP-like data its IOP configuration reproduces `iop_population_1_1` (fixed effects,
variance components, REML objective, priors and predictive quantiles agree to 1e-9;
`tests/unit/test_eye_recovery_population.py`). The harness
`peykar.validation.eye_recovery_evaluation` runs the same tasks and baselines as the
IOP harnesses. The retinal configurations are in
`peykar.model_learning.retina_thickness_population`.

## Round 0 — in-sample diagnostics (2026-10-28)

`diagnostics_round0.py`, output in `round0-diagnostics.txt`:

- Residual SD of best fits over the ten control group means with at least five
  follow-ups: 0.1139 without a transient, 0.1038 / 0.1031 / 0.1036 with a fixed
  transient of 7 / 14 / 30 days. The time constant is barely identified (few early
  visits); **14 days** is kept, a plausible onset time of intravitreal drugs.
- The reference time (1 day or 1 week) does not change best fits; 1 day is kept as
  in IOP 1.1.
- The log fraction at the last follow-up falls with `ln(CST0 / 300 um)` (slope
  about -0.62 over 79 controls, -0.51 for anti-VEGF): regression toward a normal
  thickness, so the level covariate is kept with a 300 um reference.
- The largest residuals come from steroid implants with retreatment and PRN
  regimens (rebound, then a new dip), which a smooth law cannot follow.
- Few series have five or more follow-ups (11), so few studies give a noise estimate;
  single eyes have none with the four-parameter law.

## Round 1 — declared before results (2026-10-28)

Candidates, both run by LOSO over all 46 development studies with the development
author-overlap clusters held out together:

- **r0**: IOP 1.1 structure, grouping `("context",)`, 14-day transient.
- **r1**: grouping `("treatment", "context")`: contexts nested in their treatment,
  so a new indication of a known treatment (for example uveitic edema treated with a
  steroid implant) borrows that treatment's kinetics instead of the species mean.

Adoption rule (as for IOP 1.1): r1 replaces r0 only if, on group means, it improves
the pooled RMSE of the primary method by at least 0.10 pp in at least one task
(initial state, next observation, landmark) and loses no more than 0.05 pp in any.
The primary method of a candidate is the probabilistic variant with the lowest
pooled next-observation RMSE on group means; variants within 0.05 pp count as tied
and the Gaussian one is preferred.

### Round 1 results (LOSO, 46 studies, group means; primary = lowest next-observation variant)

| task | r0 (robust) | r1 (robust) | persistence | pooled curve |
|---|---|---|---|---|
| next observation, 145 targets | 8.76 | 8.65 | 8.67 | 9.86 |
| landmark, 145 targets | 10.35 | 10.05 | 13.20 | 13.96 |
| initial state, 178 targets (52 groups) | 13.89 | 13.96 | — | 17.28 / 17.04 |

r1 gains 0.11 pp (next observation) and 0.30 pp (landmark) but loses 0.07 pp in the
initial state, more than the declared 0.05 pp: **r1 is not adopted; r0 stays.**

Findings that shape round 2 (development data only):

- Neither candidate beats persistence for the next observation. Consecutive
  out-of-sample next-observation errors of the same series are correlated (lag-1
  correlation of log errors 0.42 for r0): the model does not follow a series'
  current deviation from the smooth law. An equal blend of r0 and persistence would
  give 8.07 pp (in-sample diagnostic, not a candidate).
- The largest errors are steroid-implant rebounds after the drug wears off (3–4
  months) and week-1 to month-1 changes after a single anti-VEGF injection.
- Single-eye initial-state intervals are too narrow (80% coverage 0.49).

## Round 2 — declared before results (2026-10-28)

Persistent deviations: a fraction `phi` of each series' residual variance follows an
Ornstein–Uhlenbeck process with correlation time `ell`; the rest stays independent.
Given `ln tau` the model stays linear-Gaussian: the likelihood is whitened by the
residual correlation `(1 - phi) I + phi exp(-|t - t'| / ell)`, and forecasts condition
the target's deviation on the observed residuals (exactly; checked against the joint
Gaussian in `tests/unit/test_eye_recovery_population.py`). With `phi = 0` it is model
r0; prior-only (initial-state) forecasts do not depend on `phi`.

Candidates on top of r0: **r2a** (phi 0.5, ell 3 months), **r2b** (0.5, 12),
**r2c** (0.8, 3), **r2d** (0.8, 12). The candidate with the lowest group-mean
next-observation RMSE of its primary variant is compared with r0 under the round-1
adoption rule.

### Round 2 results (LOSO, group means)

| candidate (phi, ell) | primary | next observation | landmark | initial state | 80%/90% next-obs coverage |
|---|---|---|---|---|---|
| r0 (0, —) | robust | 8.76 | 10.35 | 13.89 | 0.793 / 0.876 (Gaussian) |
| r2a (0.5, 3 mo) | robust | 8.40 | 10.23 | 13.81 | 0.786 / 0.883 |
| r2b (0.5, 12 mo) | robust | 8.40 | 10.23 | 14.04 | 0.779 / 0.855 |
| r2c (0.8, 3 mo) | robust | 8.24 | 10.22 | 13.83 | 0.786 / 0.876 |
| r2d (0.8, 12 mo) | Gaussian (tied with robust 8.18) | 8.20 | 10.11 | 14.25 | 0.724 / 0.848 |

Every deviation candidate beats persistence (8.67 pp) and r0 for the next
observation, and the lag-1 correlation of consecutive next-observation errors falls
from 0.42 (r0) to 0.34, 0.31, 0.26 and 0.11. By the declared procedure r2d is the
round's pick (lowest next observation), and it loses 0.36 pp in the initial state, so
**r2d is not adopted and r0 stays after round 2.** (r2c would have met the adoption
rule; it was not the declared pick, so it is not adopted either.)

Diagnosis: prior-only forecasts do not depend on `phi`, so the initial-state loss
comes from the population fit. With long-range correlated residuals, each series'
likelihood summary carries less information about its level, which changes the
fitted context and study variances.

## Round 3 — declared before results (2026-10-28)

**r3**: the deviation process acts in forecasting only. The population (series
summaries, REML, noise centres) is fitted under working independence exactly as in
r0, so the initial state equals r0's; forecasts use `phi = 0.8`, `ell = 12 months`
(the best forecasting setting of round 2). Working-independence estimation keeps the
mean structure of the population consistent when residuals are correlated; the
forecasting posterior then conditions on the observed deviations. Adoption rule as
before (against r0). This is the third and last structural round before the freeze;
remaining rounds may only choose among already declared candidates by the same rule.

### Round 3 results (LOSO, group means)

| task | r0 (robust) | r3 (Gaussian, tied with robust 8.21) | persistence | pooled curve |
|---|---|---|---|---|
| next observation, 145 targets | 8.76 | 8.23 | 8.67 | 9.86 |
| landmark, 145 targets | 10.35 | 10.31 | 13.20 | 13.96 |
| initial state, 178 targets | 13.89 | 13.89 (identical by construction) | — | 17.28 |

r3 gains 0.53 pp for the next observation and loses nothing: **r3 is adopted** as
model 1.0.0. Group-mean next-observation coverage of the Gaussian variant is 0.738
(80%) and 0.834 (90%), inside the declared windows but near their lower edges (r0:
0.793 / 0.876); single-eye intervals stay too narrow (initial-state 80% coverage
0.49). The lag-1 correlation of consecutive next-observation errors falls from 0.42
to 0.20. Public development artifacts (licence-redacted):
`development-loso-2026-10-28/` (`scripts/publish_retina_population.py --development`
with r0 as reference).

## Freeze decision (2026-10-28)

- Primary candidate **r3**; U1 primary `population_initial_state`; U2 and U3 primary
  `population_assimilation` (the Gaussian variant, tied within 0.05 pp with the
  robust variant).
- Reference candidate **r0** (robust variant), scored in the same sealed run; H5
  tests only what the adopted change targets: next-observation forecasts on group
  means.
- No further development rounds; thresholds and windows as in the IOP protocols.
