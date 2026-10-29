# Visual-acuity model 1.0.0-a0 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass

- H1 next observation 80% interval: 34/41 = 0.829 (window 0.70–0.90)
- H1 next observation 90% interval: 38/41 = 0.927 (window 0.80–0.97)
- H2 next observation vs persistence: 5.00 against 5.06 pp on 41 common targets
- H2 next observation vs last_two_log_linear: 5.00 against 75.41 pp on 41 common targets
- H2 next observation vs pooled_curve_ratio: 5.00 against 6.07 pp on 41 common targets
- H2 next observation vs analog_knn: 5.00 against 7.78 pp on 41 common targets
- H3 landmark vs persistence: 5.24 against 7.27 pp on 41 common targets
- H3 landmark vs pooled_curve_ratio: 5.24 against 7.25 pp on 41 common targets
- H3 landmark vs analog_knn: 5.24 against 11.85 pp on 41 common targets
- landmark 80% interval (reported): 37/41 = 0.902
- landmark 90% interval (reported): 39/41 = 0.951
- H4 initial state population_initial_state: 9.04 pp, 8/20 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 9.79 pp, 11/20 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 55/59 = 0.932
- H4 initial state 90% interval (reported): 58/59 = 0.983

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 21 | 16 | 7 | 5.00 | 14.98 | 41 |
| population_assimilation_t4 | 21 | 16 | 7 | 5.02 | 14.88 | 41 |
| persistence | 21 | 14 | 0 | 5.06 | 18.50 | 41 |
| population_assimilation | 21 | 16 | 7 | 5.12 | 14.83 | 41 |
| weak_prior | 21 | 13 | 2 | 5.24 | 17.25 | 41 |
| population_anchored_t4 | 21 | 12 | 5 | 5.56 | 15.60 | 41 |
| population_anchored | 21 | 10 | 5 | 5.72 | 15.46 | 41 |
| pooled_curve_ratio | 21 | 13 | 7 | 6.07 | 18.78 | 41 |
| analog_knn | 21 | 12 | 5 | 7.78 | 30.47 | 41 |
| baseline_level | 21 | 8 | 3 | 22.33 | 50.00 | 41 |
| last_two_log_linear | 21 | 7 | 1 | 75.41 | 410.80 | 41 |
| reference_to_last_log_linear | 21 | 7 | 1 | 86.78 | 410.80 | 41 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 21 | 13 | 7 | 5.24 | 13.00 | 41 |
| population_assimilation_t4 | 21 | 13 | 6 | 5.26 | 13.05 | 41 |
| population_assimilation | 21 | 13 | 6 | 5.37 | 13.49 | 41 |
| population_anchored_t4 | 21 | 10 | 5 | 6.36 | 11.98 | 41 |
| population_anchored | 21 | 10 | 5 | 6.60 | 12.10 | 41 |
| pooled_curve_ratio | 21 | 13 | 7 | 7.25 | 20.00 | 41 |
| persistence | 21 | 12 | 0 | 7.27 | 20.00 | 41 |
| weak_prior | 21 | 12 | 2 | 7.79 | 22.10 | 41 |
| analog_knn | 21 | 9 | 3 | 11.85 | 30.47 | 41 |
| baseline_level | 21 | 8 | 3 | 22.33 | 50.00 | 41 |
| last_two_log_linear | 21 | 5 | 1 | 482.51 | 2236.50 | 41 |
| reference_to_last_log_linear | 21 | 5 | 1 | 482.51 | 2236.50 | 41 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| persistence | 25 | 17 | 0 | 5.53 | 21.30 | 53 |
| population_assimilation_t4 | 25 | 19 | 7 | 5.55 | 20.05 | 53 |
| population_assimilation_robust | 25 | 19 | 7 | 5.61 | 20.00 | 53 |
| population_anchored_t4 | 25 | 15 | 6 | 5.75 | 20.13 | 53 |
| weak_prior | 25 | 15 | 2 | 5.77 | 22.08 | 53 |
| population_assimilation | 25 | 19 | 7 | 5.78 | 20.02 | 53 |
| population_anchored | 25 | 13 | 6 | 6.10 | 20.11 | 53 |
| analog_knn | 25 | 14 | 5 | 7.39 | 30.47 | 53 |
| pooled_curve_ratio | 25 | 13 | 7 | 7.74 | 24.30 | 53 |
| baseline_level | 25 | 8 | 3 | 40.98 | 106.20 | 53 |
| last_two_log_linear | 25 | 7 | 1 | 68.72 | 410.80 | 53 |
| reference_to_last_log_linear | 25 | 7 | 1 | 81.42 | 410.80 | 53 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 25 | 15 | 7 | 7.86 | 28.65 | 53 |
| population_assimilation_t4 | 25 | 15 | 6 | 7.89 | 28.74 | 53 |
| population_assimilation | 25 | 15 | 6 | 7.94 | 28.70 | 53 |
| population_anchored_t4 | 25 | 12 | 5 | 8.48 | 28.81 | 53 |
| population_anchored | 25 | 13 | 6 | 8.61 | 28.80 | 53 |
| persistence | 25 | 14 | 0 | 9.27 | 30.10 | 53 |
| weak_prior | 25 | 14 | 2 | 9.91 | 31.16 | 53 |
| pooled_curve_ratio | 25 | 13 | 7 | 10.02 | 35.10 | 53 |
| analog_knn | 25 | 9 | 3 | 11.88 | 30.47 | 53 |
| baseline_level | 25 | 8 | 3 | 40.98 | 106.20 | 53 |
| last_two_log_linear | 25 | 5 | 1 | 437.80 | 2236.50 | 53 |
| reference_to_last_log_linear | 25 | 5 | 1 | 437.80 | 2236.50 | 53 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 20 | 8 | 9.02 | 28.25 | 59 |
| population_initial_state | 20 | 8 | 9.04 | 28.39 | 59 |
| pooled_training_curve | 20 | 11 | 9.79 | 34.00 | 59 |

## single eyes (reported, not tested)

- next_observation vs persistence: 7.31 against 6.89 pp on 12 targets
- next_observation vs last_two_log_linear: 7.31 against 37.84 pp on 12 targets
- next_observation vs reference_to_last_log_linear: 7.31 against 59.57 pp on 12 targets
- next_observation vs pooled_curve_ratio: 7.31 against 11.78 pp on 12 targets
- next_observation vs analog_knn: 7.31 against 5.83 pp on 12 targets
- next_observation vs baseline_level: 7.31 against 75.58 pp on 12 targets
- landmark vs persistence: 13.38 against 14.09 pp on 12 targets
- landmark vs last_two_log_linear: 13.38 against 226.05 pp on 12 targets
- landmark vs reference_to_last_log_linear: 13.38 against 226.05 pp on 12 targets
- landmark vs pooled_curve_ratio: 13.38 against 16.26 pp on 12 targets
- landmark vs analog_knn: 13.38 against 12.00 pp on 12 targets
- landmark vs baseline_level: 13.38 against 75.58 pp on 12 targets
- next_observation 80% interval: 10/12 = 0.833
- next_observation 90% interval: 11/12 = 0.917
- landmark 80% interval: 9/12 = 0.750
- landmark 90% interval: 9/12 = 0.750

## by procedure context (primary methods; comparator in parentheses)

- human/cxl: next_observation 10/12 within, 4.87 (4.93) pp; landmark 8/12 within, 4.68 (4.45) pp; initial_state_group_mean 5/11 within, 6.64 (5.50) pp
- human/dalk: next_observation 1/1 within, 4.24 (5.32) pp; landmark 0/1 within, 5.69 (14.11) pp; initial_state_group_mean 0/1 within, 6.81 (25.54) pp
- human/dmek: next_observation 6/8 within, 6.42 (6.33) pp; landmark 5/8 within, 10.86 (12.03) pp; initial_state_group_mean 0/4 within, 12.90 (10.11) pp
- human/dsaek: next_observation 0/1 within, 13.00 (5.00) pp; landmark 0/1 within, 13.00 (5.00) pp; initial_state_group_mean 0/1 within, 24.04 (7.70) pp
- human/other: next_observation 2/3 within, 3.62 (4.90) pp; landmark 2/3 within, 4.74 (8.19) pp; initial_state_group_mean 3/3 within, 2.75 (3.50) pp

Units: 1 pp = 1 ETDRS letter (0.02 logMAR) of corrected distance acuity change from the preoperative acuity.
