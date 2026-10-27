### Next observation: RMSE (pp) per study

| Method | lsc-dedifferentiation-nat-commun-2026 | zeb1-commun-biol-2023 | Series passing |
|---|---|---|---|
| cohort_assimilation | 4.87 | 9.43 | 52/64 |
| population_assimilation_t4 | 5.09 | 9.38 | 52/64 |
| population_assimilation | 5.37 | 9.69 | 52/64 |
| radius_lag_flat_prior | 26.28 | 15.48 | 1/64 |
| analog_knn | 4.79 | 8.77 | 50/64 |
| last_two_radius_trend | 5.79 | 9.46 | 42/64 |
| first_to_current_trend | 5.34 | 8.59 | 42/64 |
| crosson1986_fixed | 7.41 | 9.73 | 46/64 |
| persistence | 15.37 | 20.01 | 0/64 |

In-sample radius–lag fit within 5/10 pp (diagnostic floor): lsc-dedifferentiation-nat-commun-2026 43/48, zeb1-commun-biol-2023 13/16

### Interval coverage (pooled next-observation targets)

- 80%: cohort_assimilation 118/152 (78%), population_assimilation 113/152 (74%), population_assimilation_t4 117/152 (77%), radius_lag_flat_prior 146/152 (96%); initial-state individual 65/72
- 90%: cohort_assimilation 121/152 (80%), population_assimilation 123/152 (81%), population_assimilation_t4 127/152 (84%), radius_lag_flat_prior 147/152 (97%); initial-state individual 65/72

### Initial state only: control-group mean curves

| Study / group | Animals | Population model | Crosson 1986 | Pooled curve |
|---|---:|---|---|---|
| lsc-dedifferentiation-nat-commun-2026 / Fig.2g Native LSCs | 12 | 3.6 / 6.3 ✓ | 18.9 / 32.1 | 5.5 / 8.6 |
| zeb1-commun-biol-2023 / Zeb1 wt | 8 | 2.3 / 2.8 ✓ | 6.4 / 10.7 | 1.7 / 2.9 ✓ |

Cells are RMSE / maximum absolute error in percentage points; ✓ meets 5/10.
