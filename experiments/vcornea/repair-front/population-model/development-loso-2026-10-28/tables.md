# Development leave-one-study-out, 2026-10-28 (16 studies, 147 series)

Development reference only: every study here had been read by the developer or consumed by an earlier test. Model 2.0.0 (closure tail) and the frozen model 1.0.0 were each trained on the other 15 studies. Metrics only; observed values of CC BY-NC-ND sources are not published.

## Model 2.0.0 (closure tail)

### Next observation: RMSE (pp) per study

| Method | abdel-naby2017 | aloe-vera-rabbit-alkali-jovr-2020 | chung2021 | dio-mouse-plos-one-2020 | epo-rabbit-alkali-jovr-2023 | fl2-rat-alkali-tvst-2021 | insulin-cxl-human-medicina-2025 | ketamine-rat-sci-rep-2022 | lsc-dedifferentiation-nat-commun-2026 | nicergoline-diabetic-rat-bmc-ophthalmol-2021 | pje-bluelight-rat-antioxidants-2023 | pje-upm-rat-antioxidants-2021 | propolis-rat-alkali-bmc-cam-2013 | suarez-barrio2019 | tprk-human-bmc-ophthalmol-2022 | zeb1-commun-biol-2023 | Series passing |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| cohort_assimilation | 9.23 | — | — | 7.55 | — | — | — | 18.31 | 4.47 | — | — | — | — | — | — | 8.28 | 57/104 |
| population_assimilation_t4 | 15.38 | 37.45 | 21.23 | 10.56 | 12.92 | 5.91 | 39.70 | 13.27 | 5.29 | 16.06 | 13.90 | 10.86 | 4.40 | 6.88 | 13.43 | 8.25 | 54/147 |
| population_assimilation | 15.59 | 46.06 | 22.53 | 10.26 | 13.14 | 5.87 | 39.92 | 13.25 | 5.29 | 16.38 | 13.73 | 11.16 | 4.44 | 6.94 | 13.42 | 8.15 | 57/147 |
| radius_lag_flat_prior | 17.87 | 10.73 | 19.11 | 37.42 | 10.76 | 5.21 | 45.18 | 21.13 | 27.80 | 14.84 | 23.82 | 10.87 | 14.66 | 13.55 | 27.26 | 17.67 | 5/147 |
| analog_knn | 5.07 | 10.53 | 15.33 | 9.15 | 7.84 | 5.26 | 38.17 | 14.64 | 4.94 | 19.36 | 10.00 | 15.13 | 12.31 | 8.60 | 18.06 | 8.39 | 67/147 |
| last_two_radius_trend | 5.19 | 22.67 | 15.05 | 10.60 | 10.69 | 6.47 | 47.68 | 19.54 | 5.79 | 11.60 | 15.88 | 12.20 | 9.13 | 8.43 | 15.51 | 9.46 | 55/147 |
| first_to_current_trend | 5.29 | 10.90 | 22.40 | 10.39 | 12.75 | 7.03 | 47.71 | 17.39 | 5.34 | 10.89 | 16.75 | 10.50 | 8.14 | 8.23 | 15.42 | 8.59 | 57/147 |
| crosson1986_fixed | 7.79 | 14.04 | 30.37 | 7.24 | 19.22 | 10.65 | 3.89 | 33.10 | 7.41 | 48.76 | 29.30 | 52.66 | 21.24 | 5.94 | 5.32 | 9.73 | 64/147 |
| persistence | 12.48 | 19.41 | 28.56 | 19.14 | 15.56 | 8.17 | 54.45 | 26.79 | 15.37 | 16.91 | 27.56 | 18.28 | 21.72 | 16.65 | 35.89 | 20.01 | 0/147 |

In-sample radius–lag fit within 5/10 pp (diagnostic floor): abdel-naby2017 5/9, aloe-vera-rabbit-alkali-jovr-2020 0/3, chung2021 3/3, dio-mouse-plos-one-2020 14/16, epo-rabbit-alkali-jovr-2023 0/2, fl2-rat-alkali-tvst-2021 0/8, insulin-cxl-human-medicina-2025 2/2, ketamine-rat-sci-rep-2022 5/15, lsc-dedifferentiation-nat-commun-2026 45/48, nicergoline-diabetic-rat-bmc-ophthalmol-2021 0/2, pje-bluelight-rat-antioxidants-2023 6/6, pje-upm-rat-antioxidants-2021 3/7, propolis-rat-alkali-bmc-cam-2013 1/2, suarez-barrio2019 4/4, tprk-human-bmc-ophthalmol-2022 4/4, zeb1-commun-biol-2023 13/16

### Interval coverage (pooled next-observation targets)

- 80%: cohort_assimilation 243/293 (83%), population_assimilation 383/462 (83%), population_assimilation_t4 390/462 (84%), radius_lag_flat_prior 420/462 (91%); initial-state individual 142/147
- 90%: cohort_assimilation 266/293 (91%), population_assimilation 428/462 (93%), population_assimilation_t4 436/462 (94%), radius_lag_flat_prior 446/462 (97%); initial-state individual 146/147

### Initial state only: control-group mean curves

| Study / group | Animals | Population model | Crosson 1986 | Pooled curve |
|---|---:|---|---|---|
| abdel-naby2017 / Control | 3 | 25.0 / 29.9 | 4.0 / 5.9 ✓ | 30.0 / 40.3 |
| aloe-vera-rabbit-alkali-jovr-2020 / control | 10 | 54.8 / 63.4 | 15.4 / 24.9 | 7.9 / 11.6 |
| chung2021 / Amo | 6 | 26.5 / 35.4 | 24.9 / 34.4 | 6.6 / 8.8 |
| chung2021 / Blade | 6 | 27.8 / 40.4 | 26.4 / 39.4 | 13.8 / 16.8 |
| chung2021 / Ocu | 6 | 27.1 / 36.0 | 24.8 / 32.2 | 9.6 / 14.2 |
| dio-mouse-plos-one-2020 / normal diet | 8 | 13.7 / 18.5 | 11.1 / 19.3 | 10.2 / 14.9 |
| epo-rabbit-alkali-jovr-2023 / saline | 5 | 19.8 / 27.9 | 31.9 / 57.9 | 25.1 / 37.5 |
| fl2-rat-alkali-tvst-2021 / control-np-si-10um | 6 | 13.3 / 26.0 | 21.4 / 48.0 | 16.3 / 31.1 |
| fl2-rat-alkali-tvst-2021 / control-np-si-1um | 6 | 9.8 / 16.5 | 17.3 / 36.5 | 12.8 / 22.5 |
| fl2-rat-alkali-tvst-2021 / control-np-si-20um | 6 | 13.8 / 23.9 | 20.9 / 40.5 | 16.8 / 29.9 |
| fl2-rat-alkali-tvst-2021 / empty-np | 6 | 10.8 / 17.1 | 18.4 / 39.1 | 13.8 / 22.2 |
| insulin-cxl-human-medicina-2025 / control | 4 | 22.7 / 27.3 | 24.4 / 41.6 | 23.4 / 24.9 |
| ketamine-rat-sci-rep-2022 / saline | 7 | 15.4 / 17.1 | 41.1 / 43.7 | 19.7 / 22.2 |
| lsc-dedifferentiation-nat-commun-2026 / Fig.2g Native LSCs | 12 | 4.6 / 9.1 ✓ | 18.9 / 32.1 | 5.0 / 7.1 |
| pje-bluelight-rat-antioxidants-2023 / normal | 10 | 32.9 / 44.3 | 13.1 / 20.8 | 14.3 / 20.5 |
| pje-upm-rat-antioxidants-2021 / normal | 6 | 16.8 / 28.1 | 38.1 / 50.7 | 23.3 / 37.6 |
| propolis-rat-alkali-bmc-cam-2013 / vehicle | 40 | 8.2 / 11.7 | 42.7 / 72.0 | 36.4 / 56.4 |
| tprk-human-bmc-ophthalmol-2022 / arm-a-mprk | 39 | 18.9 / 23.0 | 6.2 / 10.7 | 24.9 / 32.3 |
| tprk-human-bmc-ophthalmol-2022 / arm-a-tprk | 39 | 15.7 / 20.7 | 10.7 / 16.9 | 20.4 / 26.0 |
| tprk-human-bmc-ophthalmol-2022 / arm-b-aaprk | 33 | 15.2 / 20.6 | 9.7 / 14.9 | 21.6 / 26.5 |
| tprk-human-bmc-ophthalmol-2022 / arm-b-tprk | 33 | 14.1 / 18.6 | 11.8 / 17.7 | 18.6 / 24.0 |
| zeb1-commun-biol-2023 / Zeb1 wt | 8 | 3.3 / 5.7 ✓ | 6.4 / 10.7 | 3.3 / 4.9 ✓ |

Cells are RMSE / maximum absolute error in percentage points; ✓ meets 5/10.

## Model 1.0.0 (reference, same data)

### Next observation: RMSE (pp) per study

| Method | abdel-naby2017 | aloe-vera-rabbit-alkali-jovr-2020 | chung2021 | dio-mouse-plos-one-2020 | epo-rabbit-alkali-jovr-2023 | fl2-rat-alkali-tvst-2021 | insulin-cxl-human-medicina-2025 | ketamine-rat-sci-rep-2022 | lsc-dedifferentiation-nat-commun-2026 | nicergoline-diabetic-rat-bmc-ophthalmol-2021 | pje-bluelight-rat-antioxidants-2023 | pje-upm-rat-antioxidants-2021 | propolis-rat-alkali-bmc-cam-2013 | suarez-barrio2019 | tprk-human-bmc-ophthalmol-2022 | zeb1-commun-biol-2023 | Series passing |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| cohort_assimilation | 10.29 | — | — | 6.27 | — | — | — | 21.39 | 4.63 | — | — | — | — | — | — | 8.81 | 62/104 |
| population_assimilation_t4 | 16.68 | 37.63 | 22.93 | 9.38 | 14.78 | 6.79 | 41.11 | 13.27 | 5.06 | 17.02 | 14.03 | 10.71 | 11.10 | 6.79 | 14.89 | 8.41 | 61/147 |
| population_assimilation | 17.26 | 47.50 | 24.29 | 9.00 | 14.90 | 6.72 | 41.84 | 13.30 | 5.34 | 17.57 | 14.16 | 11.00 | 11.41 | 6.83 | 15.01 | 8.58 | 61/147 |
| radius_lag_flat_prior | 18.64 | 9.74 | 19.65 | 42.14 | 12.36 | 6.07 | 43.35 | 21.88 | 28.21 | 14.73 | 23.82 | 10.55 | 12.06 | 14.38 | 26.93 | 16.32 | 4/147 |
| analog_knn | 5.07 | 10.53 | 15.33 | 9.15 | 7.84 | 5.26 | 38.17 | 14.64 | 4.94 | 19.36 | 10.00 | 15.13 | 12.31 | 8.60 | 18.06 | 8.39 | 67/147 |
| last_two_radius_trend | 5.19 | 22.67 | 15.05 | 10.60 | 10.69 | 6.47 | 47.68 | 19.54 | 5.79 | 11.60 | 15.88 | 12.20 | 9.13 | 8.43 | 15.51 | 9.46 | 55/147 |
| first_to_current_trend | 5.29 | 10.90 | 22.40 | 10.39 | 12.75 | 7.03 | 47.71 | 17.39 | 5.34 | 10.89 | 16.75 | 10.50 | 8.14 | 8.23 | 15.42 | 8.59 | 57/147 |
| crosson1986_fixed | 7.79 | 14.04 | 30.37 | 7.24 | 19.22 | 10.65 | 3.89 | 33.10 | 7.41 | 48.76 | 29.30 | 52.66 | 21.24 | 5.94 | 5.32 | 9.73 | 64/147 |
| persistence | 12.48 | 19.41 | 28.56 | 19.14 | 15.56 | 8.17 | 54.45 | 26.79 | 15.37 | 16.91 | 27.56 | 18.28 | 21.72 | 16.65 | 35.89 | 20.01 | 0/147 |

In-sample radius–lag fit within 5/10 pp (diagnostic floor): abdel-naby2017 6/9, aloe-vera-rabbit-alkali-jovr-2020 0/3, chung2021 3/3, dio-mouse-plos-one-2020 14/16, epo-rabbit-alkali-jovr-2023 0/2, fl2-rat-alkali-tvst-2021 0/8, insulin-cxl-human-medicina-2025 2/2, ketamine-rat-sci-rep-2022 6/15, lsc-dedifferentiation-nat-commun-2026 43/48, nicergoline-diabetic-rat-bmc-ophthalmol-2021 0/2, pje-bluelight-rat-antioxidants-2023 6/6, pje-upm-rat-antioxidants-2021 4/7, propolis-rat-alkali-bmc-cam-2013 1/2, suarez-barrio2019 4/4, tprk-human-bmc-ophthalmol-2022 4/4, zeb1-commun-biol-2023 13/16

### Interval coverage (pooled next-observation targets)

- 80%: cohort_assimilation 234/293 (80%), population_assimilation 355/462 (77%), population_assimilation_t4 364/462 (79%), radius_lag_flat_prior 408/462 (88%); initial-state individual 138/147
- 90%: cohort_assimilation 254/293 (87%), population_assimilation 413/462 (89%), population_assimilation_t4 416/462 (90%), radius_lag_flat_prior 436/462 (94%); initial-state individual 140/147

### Initial state only: control-group mean curves

| Study / group | Animals | Population model | Crosson 1986 | Pooled curve |
|---|---:|---|---|---|
| abdel-naby2017 / Control | 3 | 25.1 / 29.4 | 4.0 / 5.9 ✓ | 30.0 / 40.3 |
| aloe-vera-rabbit-alkali-jovr-2020 / control | 10 | 54.9 / 63.9 | 15.4 / 24.9 | 7.9 / 11.6 |
| chung2021 / Amo | 6 | 27.3 / 36.6 | 24.9 / 34.4 | 6.6 / 8.8 |
| chung2021 / Blade | 6 | 28.4 / 41.6 | 26.4 / 39.4 | 13.8 / 16.8 |
| chung2021 / Ocu | 6 | 27.7 / 36.7 | 24.8 / 32.2 | 9.6 / 14.2 |
| dio-mouse-plos-one-2020 / normal diet | 8 | 13.5 / 17.9 | 11.1 / 19.3 | 10.2 / 14.9 |
| epo-rabbit-alkali-jovr-2023 / saline | 5 | 19.2 / 26.4 | 31.9 / 57.9 | 25.1 / 37.5 |
| fl2-rat-alkali-tvst-2021 / control-np-si-10um | 6 | 12.3 / 22.7 | 21.4 / 48.0 | 16.3 / 31.1 |
| fl2-rat-alkali-tvst-2021 / control-np-si-1um | 6 | 9.0 / 15.9 | 17.3 / 36.5 | 12.8 / 22.5 |
| fl2-rat-alkali-tvst-2021 / control-np-si-20um | 6 | 13.1 / 23.3 | 20.9 / 40.5 | 16.8 / 29.9 |
| fl2-rat-alkali-tvst-2021 / empty-np | 6 | 10.0 / 15.2 | 18.4 / 39.1 | 13.8 / 22.2 |
| insulin-cxl-human-medicina-2025 / control | 4 | 26.3 / 31.5 | 24.4 / 41.6 | 23.4 / 24.9 |
| ketamine-rat-sci-rep-2022 / saline | 7 | 11.9 / 13.1 | 41.1 / 43.7 | 19.7 / 22.2 |
| lsc-dedifferentiation-nat-commun-2026 / Fig.2g Native LSCs | 12 | 5.1 / 10.2 | 18.9 / 32.1 | 5.0 / 7.1 |
| pje-bluelight-rat-antioxidants-2023 / normal | 10 | 33.0 / 44.6 | 13.1 / 20.8 | 14.3 / 20.5 |
| pje-upm-rat-antioxidants-2021 / normal | 6 | 17.3 / 29.0 | 38.1 / 50.7 | 23.3 / 37.6 |
| propolis-rat-alkali-bmc-cam-2013 / vehicle | 40 | 18.4 / 30.7 | 42.7 / 72.0 | 36.4 / 56.4 |
| tprk-human-bmc-ophthalmol-2022 / arm-a-mprk | 39 | 27.3 / 33.2 | 6.2 / 10.7 | 24.9 / 32.3 |
| tprk-human-bmc-ophthalmol-2022 / arm-a-tprk | 39 | 23.4 / 29.1 | 10.7 / 16.9 | 20.4 / 26.0 |
| tprk-human-bmc-ophthalmol-2022 / arm-b-aaprk | 33 | 23.6 / 29.9 | 9.7 / 14.9 | 21.6 / 26.5 |
| tprk-human-bmc-ophthalmol-2022 / arm-b-tprk | 33 | 21.7 / 27.0 | 11.8 / 17.7 | 18.6 / 24.0 |
| zeb1-commun-biol-2023 / Zeb1 wt | 8 | 3.0 / 5.0 ✓ | 6.4 / 10.7 | 3.3 / 4.9 ✓ |

Cells are RMSE / maximum absolute error in percentage points; ✓ meets 5/10.
