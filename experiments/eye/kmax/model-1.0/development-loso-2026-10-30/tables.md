# Kmax model 1.0.0-k6 — development results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: FAIL

- H1 next observation 80% interval: 28/39 = 0.718 (window 0.70–0.90)
- H1 next observation 90% interval: 34/39 = 0.872 (window 0.80–0.97)
- H2 next observation vs persistence: 6.14 against 6.59 pp on 39 common targets
- H2 next observation vs last_two_log_linear: 6.14 against 22.22 pp on 39 common targets
- H2 next observation vs pooled_curve_ratio: 6.14 against 7.99 pp on 39 common targets
- H2 next observation vs analog_knn: 5.57 against 7.41 pp on 37 common targets
- H3 landmark vs persistence: 11.11 against 12.36 pp on 39 common targets
- H3 landmark vs pooled_curve_ratio: 11.11 against 12.18 pp on 39 common targets
- H3 landmark vs analog_knn: 9.10 against 17.45 pp on 37 common targets
- landmark 80% interval (reported): 23/39 = 0.590
- landmark 90% interval (reported): 30/39 = 0.769
- H4 initial state population_initial_state: 15.41 pp, 5/25 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 13.89 pp, 5/25 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 42/60 = 0.700
- H4 initial state 90% interval (reported): 50/60 = 0.833

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 14 | 9 | 3 | 6.14 | 13.87 | 39 |
| weak_prior | 14 | 7 | 0 | 6.20 | 14.00 | 39 |
| population_assimilation_robust | 14 | 9 | 3 | 6.22 | 13.84 | 39 |
| population_anchored_t4 | 14 | 9 | 4 | 6.40 | 15.34 | 39 |
| population_assimilation | 14 | 8 | 3 | 6.50 | 14.00 | 39 |
| persistence | 14 | 7 | 0 | 6.59 | 14.00 | 39 |
| population_anchored | 14 | 9 | 4 | 6.79 | 15.74 | 39 |
| analog_knn | 14 | 4 | 0 | 7.41 | 14.39 | 37 |
| pooled_curve_ratio | 14 | 4 | 1 | 7.99 | 19.65 | 39 |
| baseline_level | 14 | 2 | 0 | 18.13 | 63.80 | 39 |
| reference_to_last_log_linear | 14 | 2 | 0 | 20.59 | 87.70 | 39 |
| last_two_log_linear | 14 | 1 | 0 | 22.22 | 87.70 | 39 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 14 | 8 | 4 | 11.01 | 29.87 | 39 |
| population_assimilation | 14 | 7 | 4 | 11.08 | 29.07 | 39 |
| population_assimilation_t4 | 14 | 8 | 4 | 11.11 | 29.89 | 39 |
| pooled_curve_ratio | 14 | 4 | 1 | 12.18 | 34.93 | 39 |
| population_anchored_t4 | 14 | 7 | 6 | 12.26 | 33.72 | 39 |
| weak_prior | 14 | 7 | 0 | 12.35 | 36.00 | 39 |
| persistence | 14 | 7 | 0 | 12.36 | 36.00 | 39 |
| population_anchored | 14 | 7 | 6 | 12.42 | 33.63 | 39 |
| analog_knn | 14 | 3 | 1 | 17.45 | 41.50 | 37 |
| baseline_level | 14 | 2 | 0 | 18.13 | 63.80 | 39 |
| last_two_log_linear | 14 | 3 | 2 | 37.98 | 87.70 | 39 |
| reference_to_last_log_linear | 14 | 3 | 2 | 37.98 | 87.70 | 39 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| persistence | 22 | 8 | 0 | 8.93 | 33.00 | 55 |
| weak_prior | 22 | 8 | 0 | 8.98 | 35.82 | 55 |
| population_assimilation_t4 | 22 | 11 | 5 | 9.16 | 42.95 | 55 |
| population_assimilation_robust | 22 | 11 | 5 | 9.23 | 42.93 | 55 |
| pooled_curve_ratio | 22 | 5 | 1 | 9.47 | 36.01 | 55 |
| population_assimilation | 22 | 10 | 5 | 9.58 | 43.18 | 55 |
| population_anchored_t4 | 22 | 10 | 5 | 9.70 | 39.40 | 55 |
| population_anchored | 22 | 10 | 5 | 10.14 | 39.34 | 55 |
| analog_knn | 22 | 4 | 0 | 10.84 | 39.59 | 53 |
| baseline_level | 22 | 3 | 1 | 19.82 | 63.80 | 55 |
| reference_to_last_log_linear | 22 | 2 | 0 | 26.75 | 105.00 | 55 |
| last_two_log_linear | 22 | 1 | 0 | 28.32 | 105.00 | 55 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| pooled_curve_ratio | 22 | 5 | 1 | 12.45 | 34.93 | 55 |
| persistence | 22 | 7 | 0 | 13.67 | 36.00 | 55 |
| weak_prior | 22 | 7 | 0 | 13.69 | 36.00 | 55 |
| population_assimilation_robust | 22 | 9 | 5 | 13.73 | 47.44 | 55 |
| population_assimilation_t4 | 22 | 9 | 5 | 13.76 | 47.77 | 55 |
| population_assimilation | 22 | 9 | 6 | 13.89 | 48.23 | 55 |
| population_anchored_t4 | 22 | 8 | 7 | 15.27 | 40.41 | 55 |
| population_anchored | 22 | 8 | 7 | 15.50 | 41.29 | 55 |
| analog_knn | 22 | 3 | 1 | 17.39 | 41.50 | 53 |
| baseline_level | 22 | 3 | 1 | 19.82 | 63.80 | 55 |
| last_two_log_linear | 22 | 3 | 2 | 45.74 | 174.00 | 55 |
| reference_to_last_log_linear | 22 | 3 | 2 | 45.74 | 174.00 | 55 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| pooled_training_curve | 25 | 5 | 13.89 | 37.88 | 60 |
| population_initial_state_t4 | 25 | 5 | 15.36 | 45.25 | 60 |
| population_initial_state | 25 | 5 | 15.41 | 45.33 | 60 |

## single eyes (reported, not tested)

- next_observation vs persistence: 14.02 against 12.96 pp on 16 targets
- next_observation vs last_two_log_linear: 14.02 against 39.43 pp on 16 targets
- next_observation vs reference_to_last_log_linear: 14.02 against 37.76 pp on 16 targets
- next_observation vs pooled_curve_ratio: 14.02 against 12.35 pp on 16 targets
- next_observation vs analog_knn: 14.02 against 16.19 pp on 16 targets
- next_observation vs baseline_level: 14.02 against 23.43 pp on 16 targets
- landmark vs persistence: 18.71 against 16.43 pp on 16 targets
- landmark vs last_two_log_linear: 18.71 against 60.63 pp on 16 targets
- landmark vs reference_to_last_log_linear: 18.71 against 60.63 pp on 16 targets
- landmark vs pooled_curve_ratio: 18.71 against 13.08 pp on 16 targets
- landmark vs analog_knn: 18.71 against 17.25 pp on 16 targets
- landmark vs baseline_level: 18.71 against 23.43 pp on 16 targets
- next_observation 80% interval: 10/16 = 0.625
- next_observation 90% interval: 10/16 = 0.625
- landmark 80% interval: 7/16 = 0.438
- landmark 90% interval: 8/16 = 0.500

## by procedure context (primary methods; comparator in parentheses)

- human/cxl_accelerated: next_observation 0/1 within, 9.79 (11.85) pp; landmark 0/1 within, 20.26 (24.56) pp; initial_state_group_mean 2/6 within, 10.93 (10.85) pp
- human/cxl_combined: next_observation 0/2 within, 9.77 (11.28) pp; landmark 0/2 within, 17.20 (20.82) pp; initial_state_group_mean 0/2 within, 11.87 (9.74) pp
- human/cxl_epi_off: next_observation 5/10 within, 12.15 (11.54) pp; landmark 4/10 within, 16.10 (15.55) pp; initial_state_group_mean 2/5 within, 9.15 (10.66) pp
- human/cxl_epi_on: next_observation 2/3 within, 5.25 (4.64) pp; landmark 2/3 within, 10.14 (6.15) pp; initial_state_group_mean 0/2 within, 14.95 (8.34) pp
- human/icrs: next_observation 3/3 within, 3.17 (1.30) pp; landmark 2/3 within, 5.47 (1.76) pp; initial_state_group_mean 1/7 within, 25.20 (24.33) pp
- human/observation: next_observation 1/3 within, 7.03 (5.55) pp; landmark 1/3 within, 9.61 (8.12) pp; initial_state_group_mean 0/3 within, 11.91 (8.28) pp

Units: 1 pp = 0.1 D of maximum keratometry (Kmax) change from the baseline Kmax.
