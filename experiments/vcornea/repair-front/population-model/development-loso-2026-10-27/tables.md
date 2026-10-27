### Next observation: RMSE (pp) per study

| Method | abdel-naby2017 | chung2021 | dio-mouse-plos-one-2020 | ketamine-rat-sci-rep-2022 | suarez-barrio2019 | Series passing |
|---|---|---|---|---|---|---|
| cohort_assimilation | 6.31 | — | 6.46 | 26.18 | — | 9/40 |
| population_assimilation_t4 | 11.27 | 16.41 | 7.18 | 19.46 | 6.91 | 9/47 |
| population_assimilation | 10.86 | 17.92 | 7.21 | 21.20 | 6.96 | 8/47 |
| radius_lag_flat_prior | 15.19 | 17.34 | 13.14 | 14.67 | 7.72 | 5/47 |
| analog_knn | 5.45 | 19.74 | 7.84 | 17.45 | 8.49 | 10/47 |
| last_two_radius_trend | 5.19 | 15.05 | 10.60 | 19.54 | 8.43 | 11/47 |
| first_to_current_trend | 5.29 | 22.40 | 10.39 | 17.39 | 8.23 | 13/47 |
| crosson1986_fixed | 7.79 | 30.37 | 7.24 | 33.10 | 5.94 | 14/47 |
| persistence | 12.48 | 28.56 | 19.14 | 26.79 | 16.65 | 0/47 |

In-sample radius–lag fit within 5/10 pp (diagnostic floor): abdel-naby2017 6/9, chung2021 3/3, dio-mouse-plos-one-2020 14/16, ketamine-rat-sci-rep-2022 6/15, suarez-barrio2019 4/4

### Interval coverage (pooled next-observation targets)

- 80%: cohort_assimilation 102/141 (72%), population_assimilation 130/168 (77%), population_assimilation_t4 130/168 (77%), radius_lag_flat_prior 141/168 (84%); initial-state individual 58/75
- 90%: cohort_assimilation 124/141 (88%), population_assimilation 142/168 (85%), population_assimilation_t4 145/168 (86%), radius_lag_flat_prior 154/168 (92%); initial-state individual 60/75

### Initial state only: control-group mean curves

| Study / group | Animals | Population model | Crosson 1986 | Pooled curve |
|---|---:|---|---|---|
| abdel-naby2017 / Control | 3 | 25.3 / 32.0 | 4.0 / 5.9 ✓ | 31.2 / 41.7 |
| chung2021 / Amo | 6 | 24.8 / 34.8 | 24.9 / 34.4 | 8.8 / 12.0 |
| chung2021 / Blade | 6 | 26.4 / 39.8 | 26.4 / 39.4 | 14.0 / 18.3 |
| chung2021 / Ocu | 6 | 25.4 / 32.3 | 24.8 / 32.2 | 11.1 / 16.3 |
| dio-mouse-plos-one-2020 / normal diet | 8 | 21.8 / 25.7 | 11.1 / 19.3 | 9.3 / 12.5 |
| ketamine-rat-sci-rep-2022 / saline | 7 | 34.2 / 35.9 | 41.1 / 43.7 | 28.5 / 32.2 |

Cells are RMSE / maximum absolute error in percentage points; ✓ meets 5/10.
