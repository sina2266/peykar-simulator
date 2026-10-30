# Kmax model 1.0.0-k6 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: FAIL
- H4_initial_state_control_group_means: FAIL

- H1 next observation 80% interval: 41/55 = 0.745 (window 0.70–0.90)
- H1 next observation 90% interval: 47/55 = 0.855 (window 0.80–0.97)
- H2 next observation vs persistence: 10.20 against 11.91 pp on 55 common targets
- H2 next observation vs last_two_log_linear: 10.20 against 27.40 pp on 55 common targets
- H2 next observation vs pooled_curve_ratio: 10.20 against 10.73 pp on 55 common targets
- H2 next observation vs analog_knn: 10.20 against 12.07 pp on 55 common targets
- H3 landmark vs persistence: 12.60 against 16.65 pp on 55 common targets
- H3 landmark vs pooled_curve_ratio: 12.60 against 12.35 pp on 55 common targets
- H3 landmark vs analog_knn: 12.60 against 14.28 pp on 55 common targets
- landmark 80% interval (reported): 39/55 = 0.709
- landmark 90% interval (reported): 46/55 = 0.836
- H4 initial state population_initial_state: 14.61 pp, 5/33 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 14.17 pp, 5/33 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 72/88 = 0.818
- H4 initial state 90% interval (reported): 77/88 = 0.875

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 20 | 9 | 3 | 10.16 | 46.92 | 55 |
| population_assimilation_t4 | 20 | 9 | 3 | 10.20 | 47.04 | 55 |
| population_anchored | 20 | 9 | 3 | 10.38 | 41.97 | 55 |
| population_anchored_t4 | 20 | 8 | 2 | 10.47 | 42.03 | 55 |
| population_assimilation | 20 | 9 | 3 | 10.64 | 47.34 | 55 |
| pooled_curve_ratio | 20 | 6 | 2 | 10.73 | 36.13 | 55 |
| persistence | 20 | 8 | 0 | 11.91 | 35.30 | 55 |
| weak_prior | 20 | 8 | 0 | 12.03 | 41.00 | 55 |
| analog_knn | 20 | 3 | 2 | 12.07 | 35.04 | 55 |
| reference_to_last_log_linear | 20 | 2 | 2 | 23.38 | 67.93 | 55 |
| baseline_level | 20 | 2 | 1 | 23.54 | 58.00 | 55 |
| last_two_log_linear | 20 | 2 | 2 | 27.40 | 70.82 | 55 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_anchored | 20 | 6 | 3 | 12.23 | 30.39 | 55 |
| population_anchored_t4 | 20 | 6 | 3 | 12.32 | 30.80 | 55 |
| pooled_curve_ratio | 20 | 4 | 4 | 12.35 | 36.41 | 55 |
| population_assimilation_robust | 20 | 9 | 4 | 12.59 | 35.55 | 55 |
| population_assimilation | 20 | 9 | 4 | 12.60 | 36.26 | 55 |
| population_assimilation_t4 | 20 | 9 | 4 | 12.60 | 35.64 | 55 |
| analog_knn | 20 | 2 | 2 | 14.28 | 39.11 | 55 |
| weak_prior | 20 | 8 | 0 | 16.58 | 43.57 | 55 |
| persistence | 20 | 8 | 0 | 16.65 | 44.30 | 55 |
| baseline_level | 20 | 2 | 1 | 23.54 | 58.00 | 55 |
| last_two_log_linear | 20 | 2 | 2 | 75.73 | 361.00 | 55 |
| reference_to_last_log_linear | 20 | 2 | 2 | 75.73 | 361.00 | 55 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 20 | 9 | 3 | 10.16 | 46.92 | 55 |
| population_assimilation_t4 | 20 | 9 | 3 | 10.20 | 47.04 | 55 |
| population_anchored | 20 | 9 | 3 | 10.38 | 41.97 | 55 |
| population_anchored_t4 | 20 | 8 | 2 | 10.47 | 42.03 | 55 |
| population_assimilation | 20 | 9 | 3 | 10.64 | 47.34 | 55 |
| pooled_curve_ratio | 20 | 6 | 2 | 10.73 | 36.13 | 55 |
| persistence | 20 | 8 | 0 | 11.91 | 35.30 | 55 |
| weak_prior | 20 | 8 | 0 | 12.03 | 41.00 | 55 |
| analog_knn | 20 | 3 | 2 | 12.07 | 35.04 | 55 |
| reference_to_last_log_linear | 20 | 2 | 2 | 23.38 | 67.93 | 55 |
| baseline_level | 20 | 2 | 1 | 23.54 | 58.00 | 55 |
| last_two_log_linear | 20 | 2 | 2 | 27.40 | 70.82 | 55 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_anchored | 20 | 6 | 3 | 12.23 | 30.39 | 55 |
| population_anchored_t4 | 20 | 6 | 3 | 12.32 | 30.80 | 55 |
| pooled_curve_ratio | 20 | 4 | 4 | 12.35 | 36.41 | 55 |
| population_assimilation_robust | 20 | 9 | 4 | 12.59 | 35.55 | 55 |
| population_assimilation | 20 | 9 | 4 | 12.60 | 36.26 | 55 |
| population_assimilation_t4 | 20 | 9 | 4 | 12.60 | 35.64 | 55 |
| analog_knn | 20 | 2 | 2 | 14.28 | 39.11 | 55 |
| weak_prior | 20 | 8 | 0 | 16.58 | 43.57 | 55 |
| persistence | 20 | 8 | 0 | 16.65 | 44.30 | 55 |
| baseline_level | 20 | 2 | 1 | 23.54 | 58.00 | 55 |
| last_two_log_linear | 20 | 2 | 2 | 75.73 | 361.00 | 55 |
| reference_to_last_log_linear | 20 | 2 | 2 | 75.73 | 361.00 | 55 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| pooled_training_curve | 33 | 5 | 14.17 | 50.20 | 88 |
| population_initial_state | 33 | 5 | 14.61 | 52.42 | 88 |
| population_initial_state_t4 | 33 | 5 | 14.61 | 52.47 | 88 |

## single eyes (reported, not tested)

- next_observation vs persistence: n/a against n/a pp on 0 targets
- next_observation vs last_two_log_linear: n/a against n/a pp on 0 targets
- next_observation vs reference_to_last_log_linear: n/a against n/a pp on 0 targets
- next_observation vs pooled_curve_ratio: n/a against n/a pp on 0 targets
- next_observation vs analog_knn: n/a against n/a pp on 0 targets
- next_observation vs baseline_level: n/a against n/a pp on 0 targets
- landmark vs persistence: n/a against n/a pp on 0 targets
- landmark vs last_two_log_linear: n/a against n/a pp on 0 targets
- landmark vs reference_to_last_log_linear: n/a against n/a pp on 0 targets
- landmark vs pooled_curve_ratio: n/a against n/a pp on 0 targets
- landmark vs analog_knn: n/a against n/a pp on 0 targets
- landmark vs baseline_level: n/a against n/a pp on 0 targets
- next_observation 80% interval: 0/0 = n/a
- next_observation 90% interval: 0/0 = n/a
- landmark 80% interval: 0/0 = n/a
- landmark 90% interval: 0/0 = n/a

## by procedure context (primary methods; comparator in parentheses)

- human/cxl_accelerated: next_observation 3/6 within, 13.87 (15.79) pp; landmark 2/6 within, 12.39 (17.34) pp; initial_state_group_mean 2/11 within, 13.50 (14.49) pp
- human/cxl_combined: next_observation 1/1 within, 1.61 (0.82) pp; landmark 1/1 within, 3.03 (1.29) pp; initial_state_group_mean 0/1 within, 27.22 (27.75) pp
- human/cxl_epi_off: next_observation 4/9 within, 7.94 (9.86) pp; landmark 4/9 within, 14.64 (18.34) pp; initial_state_group_mean 3/15 within, 15.20 (13.69) pp
- human/cxl_epi_on: next_observation 1/4 within, 10.41 (11.83) pp; landmark 2/4 within, 8.69 (13.42) pp; initial_state_group_mean 0/4 within, 9.39 (9.00) pp
- human/observation: initial_state_group_mean 0/1 within, 7.71 (13.00) pp
- human/other: initial_state_group_mean 0/1 within, 14.95 (7.13) pp

Units: 1 pp = 0.1 D of maximum keratometry (Kmax) change from the baseline Kmax.
