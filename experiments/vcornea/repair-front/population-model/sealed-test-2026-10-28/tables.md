# Sealed test of model 2.0.0, 2026-10-28 (2 studies, 57 series)

One-shot result under the frozen protocol 2.0.0 (SHA 68ef4771...). Both versions were trained on the same 16 development studies. Both sealed studies are CC BY 4.0.

## Model 2.0.0 (closure tail)

### Next observation: RMSE (pp) per study

| Method | il10-c-mast-cell-rep-2026 | iprf-cxl-human-arq-bras-oftalmol-2025 | Series passing |
|---|---|---|---|
| cohort_assimilation | 13.10 | — | 3/55 |
| population_assimilation_t4 | 11.41 | 3.24 | 10/57 |
| population_assimilation | 11.55 | 2.57 | 11/57 |
| radius_lag_flat_prior | 28.15 | 11.61 | 0/57 |
| analog_knn | 13.12 | 2.95 | 6/57 |
| last_two_radius_trend | 16.27 | 2.56 | 6/57 |
| first_to_current_trend | 13.71 | 2.51 | 6/57 |
| crosson1986_fixed | 37.72 | 4.50 | 1/57 |
| persistence | 16.32 | 24.77 | 0/57 |

In-sample radius–lag fit within 5/10 pp (diagnostic floor): il10-c-mast-cell-rep-2026 30/55, iprf-cxl-human-arq-bras-oftalmol-2025 2/2

### Interval coverage (pooled next-observation targets)

- 80%: cohort_assimilation 116/165 (70%), population_assimilation 158/169 (93%), population_assimilation_t4 161/169 (95%), radius_lag_flat_prior 141/169 (83%); initial-state individual 56/56
- 90%: cohort_assimilation 136/165 (82%), population_assimilation 168/169 (99%), population_assimilation_t4 168/169 (99%), radius_lag_flat_prior 164/169 (97%); initial-state individual 56/56

### Initial state only: control-group mean curves

| Study / group | Animals | Population model | Crosson 1986 | Pooled curve |
|---|---:|---|---|---|
| il10-c-mast-cell-rep-2026 / PBS (-) | 14 | 18.0 / 24.6 | 40.6 / 47.0 | 19.8 / 21.5 |
| iprf-cxl-human-arq-bras-oftalmol-2025 / i-prf-nonapplied | 25 | 25.0 / 41.7 | 5.3 / 7.7 | 19.7 / 23.3 |

Cells are RMSE / maximum absolute error in percentage points; ✓ meets 5/10.

## Model 1.0.0 (reference, same data)

### Next observation: RMSE (pp) per study

| Method | il10-c-mast-cell-rep-2026 | iprf-cxl-human-arq-bras-oftalmol-2025 | Series passing |
|---|---|---|---|
| cohort_assimilation | 13.01 | — | 4/55 |
| population_assimilation_t4 | 11.43 | 4.50 | 10/57 |
| population_assimilation | 11.49 | 3.45 | 12/57 |
| radius_lag_flat_prior | 28.18 | 12.92 | 0/57 |
| analog_knn | 13.12 | 2.95 | 6/57 |
| last_two_radius_trend | 16.27 | 2.56 | 6/57 |
| first_to_current_trend | 13.71 | 2.51 | 6/57 |
| crosson1986_fixed | 37.72 | 4.50 | 1/57 |
| persistence | 16.32 | 24.77 | 0/57 |

In-sample radius–lag fit within 5/10 pp (diagnostic floor): il10-c-mast-cell-rep-2026 30/55, iprf-cxl-human-arq-bras-oftalmol-2025 2/2

### Interval coverage (pooled next-observation targets)

- 80%: cohort_assimilation 115/165 (70%), population_assimilation 157/169 (93%), population_assimilation_t4 160/169 (95%), radius_lag_flat_prior 147/169 (87%); initial-state individual 56/56
- 90%: cohort_assimilation 139/165 (84%), population_assimilation 168/169 (99%), population_assimilation_t4 168/169 (99%), radius_lag_flat_prior 166/169 (98%); initial-state individual 56/56

### Initial state only: control-group mean curves

| Study / group | Animals | Population model | Crosson 1986 | Pooled curve |
|---|---:|---|---|---|
| il10-c-mast-cell-rep-2026 / PBS (-) | 14 | 15.8 / 22.7 | 40.6 / 47.0 | 19.8 / 21.5 |
| iprf-cxl-human-arq-bras-oftalmol-2025 / i-prf-nonapplied | 25 | 25.1 / 42.3 | 5.3 / 7.7 | 19.7 / 23.3 |

Cells are RMSE / maximum absolute error in percentage points; ✓ meets 5/10.
