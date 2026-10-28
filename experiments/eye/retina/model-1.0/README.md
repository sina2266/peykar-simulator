# Retinal-thickness model 1.0.0

Population model of central retinal thickness after treatment, as a fraction `f` of
each series' pretreatment value, over months `t`. It is the generic eye recovery
model (`peykar.model_learning.eye_recovery_population`) with the configuration
`r3` of `peykar.model_learning.retina_thickness_population`.

## Model

```
ln f(t) = ell + e r(t) + s q(t) + d(t) + noise
r(t) = exp(-(t - 1 day) / tau),  q(t) = exp(-(t - 1 day) / 14 days)
ell  = context level + b ln(CST0 / 300 um)
d    = Ornstein-Uhlenbeck deviation, 80% of the residual variance, correlation time 12 months
```

- **Law.** A slow return `e r(t)` toward the long-term level `ell` and a fixed
  14-day onset transient `s q(t)`; `ln tau` on an 80-point grid from 0.01 to 120
  months. The level regresses on the baseline thickness (thicker eyes fall further:
  regression toward a normal thickness), with one slope estimated inside REML.
- **Hierarchy.** One per species: species mean, context (`indication/treatment`),
  study and series (group means and single eyes separately), diagonal covariances by
  penalized REML. A context without training studies uses the species mean with the
  context variance added.
- **Persistent deviations.** Series stay above or below the smooth law for months
  (retreatment regimens, implant wear-off). Forecasts condition the target's
  deviation on the observed residuals exactly; the population itself is fitted
  under working independence, so initial-state predictions equal those without the
  deviation term.
- **Noise.** Gaussian in `ln f`, group-mean SD scaled by `(n/40)^-g`; a new study's
  SD is marginalized around the scaled median.
- **Forecasts.** Exact Gaussian mixtures over `tau`, noise levels and prior
  components; the point forecast is the posterior predictive median.

The generic module reproduces IOP model 1.1 when configured like it (fixed effects,
variance components, REML objective, priors and predictive quantiles agree to 1e-9
on synthetic data), and its deviation forecasts match the exact joint Gaussian
(`tests/unit/test_eye_recovery_population.py`).

## Development (46 development studies, leave-one-study-out)

See the [development log](development-log.md). Group means, pooled RMSE in
percentage points of the baseline thickness:

| task | model 1.0.0 (r3) | r0 (no deviations) | persistence | pooled curve | analog kNN |
|---|---|---|---|---|---|
| next observation (145 targets) | 8.23 | 8.76 | 8.67 | 9.86 | 10.20 |
| landmark from first follow-up (145) | 10.31 | 10.35 | 13.20 | 13.96 | 14.72 |
| initial state, controls (178) | 13.89 | 13.89 | — | 17.28 | — |

Next-observation interval coverage on group means: 73.8% (80%) and 83.4% (90%).

Known limits seen in development: steroid-implant rebound after the drug wears off
(3–4 months) and retreatment dips are not part of the law; single-eye intervals are
too narrow; most series have only 2–4 follow-ups. No result is a clinical claim.
The sealed test is defined by [protocol 1.0.0](protocol-1.0.0.json).
