# Retinal-thickness model 1.4.0-k3 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: FAIL
- H4_initial_state_control_group_means: pass
- H5_landmark_lower_next_observation_within_0_05_pp: FAIL

- H1 next observation 80% interval: 69/87 = 0.793 (window 0.70–0.90)
- H1 next observation 90% interval: 73/87 = 0.839 (window 0.80–0.97)
- H2 next observation vs persistence: 5.84 against 7.00 pp on 87 common targets
- H2 next observation vs last_two_log_linear: 5.84 against 13.78 pp on 87 common targets
- H2 next observation vs pooled_curve_ratio: 5.84 against 6.65 pp on 87 common targets
- H2 next observation vs analog_knn: 5.84 against 6.58 pp on 87 common targets
- H3 landmark vs persistence: 10.58 against 16.13 pp on 87 common targets
- H3 landmark vs pooled_curve_ratio: 10.58 against 9.80 pp on 87 common targets
- H3 landmark vs analog_knn: 10.58 against 11.65 pp on 87 common targets
- landmark 80% interval (reported): 60/87 = 0.690
- landmark 90% interval (reported): 68/87 = 0.782
- H4 initial state population_initial_state: 10.98 pp, 7/20 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 15.14 pp, 4/20 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 70/85 = 0.824
- H4 initial state 90% interval (reported): 72/85 = 0.847

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 24 | 13 | 8 | 5.67 | 19.62 | 87 |
| population_anchored_t4 | 24 | 12 | 7 | 5.69 | 18.63 | 87 |
| population_assimilation_robust | 24 | 13 | 8 | 5.73 | 19.47 | 87 |
| population_anchored | 24 | 13 | 8 | 5.73 | 18.51 | 87 |
| population_assimilation | 24 | 13 | 8 | 5.83 | 19.51 | 87 |
| stacked | 24 | 12 | 8 | 5.84 | 19.64 | 87 |
| analog_knn | 24 | 11 | 6 | 6.58 | 23.54 | 87 |
| pooled_curve_ratio | 24 | 13 | 10 | 6.65 | 25.21 | 87 |
| weak_prior | 24 | 10 | 1 | 6.96 | 19.67 | 87 |
| persistence | 24 | 10 | 0 | 7.00 | 21.43 | 87 |
| last_two_log_linear | 24 | 1 | 0 | 13.78 | 40.23 | 87 |
| reference_to_last_log_linear | 24 | 1 | 0 | 18.18 | 40.23 | 87 |
| baseline_level | 24 | 1 | 0 | 36.79 | 60.95 | 87 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| pooled_curve_ratio | 24 | 12 | 8 | 9.80 | 27.61 | 87 |
| population_assimilation | 24 | 11 | 8 | 9.99 | 26.12 | 87 |
| population_assimilation_t4 | 24 | 11 | 7 | 10.00 | 26.22 | 87 |
| population_assimilation_robust | 24 | 11 | 8 | 10.05 | 26.00 | 87 |
| population_anchored | 24 | 10 | 8 | 10.50 | 26.20 | 87 |
| population_anchored_t4 | 24 | 10 | 8 | 10.54 | 26.03 | 87 |
| stacked | 24 | 10 | 7 | 10.58 | 25.08 | 87 |
| analog_knn | 24 | 4 | 2 | 11.65 | 28.75 | 87 |
| persistence | 24 | 5 | 0 | 16.13 | 34.12 | 87 |
| weak_prior | 24 | 4 | 1 | 18.33 | 40.10 | 87 |
| baseline_level | 24 | 1 | 0 | 36.79 | 60.95 | 87 |
| last_two_log_linear | 24 | 1 | 0 | 44.60 | 88.01 | 87 |
| reference_to_last_log_linear | 24 | 1 | 0 | 44.60 | 88.01 | 87 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 26 | 13 | 8 | 6.27 | 23.02 | 92 |
| population_assimilation_robust | 26 | 13 | 8 | 6.32 | 23.07 | 92 |
| population_assimilation | 26 | 13 | 8 | 6.39 | 22.96 | 92 |
| stacked | 26 | 12 | 8 | 6.41 | 22.99 | 92 |
| population_anchored_t4 | 26 | 12 | 7 | 6.60 | 24.50 | 92 |
| population_anchored | 26 | 13 | 8 | 6.63 | 24.79 | 92 |
| analog_knn | 26 | 11 | 6 | 6.97 | 23.54 | 92 |
| pooled_curve_ratio | 26 | 13 | 10 | 7.23 | 25.21 | 92 |
| weak_prior | 26 | 10 | 1 | 7.45 | 21.94 | 92 |
| persistence | 26 | 10 | 0 | 7.52 | 22.74 | 92 |
| last_two_log_linear | 26 | 1 | 0 | 15.93 | 67.66 | 92 |
| reference_to_last_log_linear | 26 | 1 | 0 | 20.32 | 67.66 | 92 |
| baseline_level | 26 | 1 | 0 | 36.78 | 60.95 | 92 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 26 | 11 | 8 | 10.55 | 27.31 | 92 |
| population_assimilation_t4 | 26 | 11 | 7 | 10.58 | 27.61 | 92 |
| population_assimilation_robust | 26 | 11 | 8 | 10.64 | 27.82 | 92 |
| pooled_curve_ratio | 26 | 12 | 8 | 10.78 | 32.50 | 92 |
| stacked | 26 | 10 | 7 | 11.15 | 28.26 | 92 |
| population_anchored | 26 | 10 | 8 | 11.62 | 33.96 | 92 |
| population_anchored_t4 | 26 | 10 | 8 | 11.65 | 33.82 | 92 |
| analog_knn | 26 | 4 | 2 | 12.66 | 32.60 | 92 |
| persistence | 26 | 5 | 0 | 16.38 | 34.12 | 92 |
| weak_prior | 26 | 4 | 1 | 18.28 | 40.10 | 92 |
| baseline_level | 26 | 1 | 0 | 36.78 | 60.95 | 92 |
| last_two_log_linear | 26 | 1 | 0 | 45.79 | 88.01 | 92 |
| reference_to_last_log_linear | 26 | 1 | 0 | 45.79 | 88.01 | 92 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 20 | 7 | 10.98 | 28.04 | 85 |
| population_initial_state | 20 | 7 | 10.98 | 28.05 | 85 |
| pooled_training_curve | 20 | 4 | 15.14 | 38.72 | 85 |

## single eyes (reported, not tested)

- next_observation vs persistence: 12.80 against 13.73 pp on 5 targets
- next_observation vs last_two_log_linear: 12.80 against 36.92 pp on 5 targets
- next_observation vs reference_to_last_log_linear: 12.80 against 42.94 pp on 5 targets
- next_observation vs pooled_curve_ratio: 12.80 against 13.86 pp on 5 targets
- next_observation vs analog_knn: 12.80 against 11.78 pp on 5 targets
- next_observation vs baseline_level: 12.80 against 36.61 pp on 5 targets
- landmark vs persistence: 18.38 against 20.36 pp on 5 targets
- landmark vs last_two_log_linear: 18.38 against 63.07 pp on 5 targets
- landmark vs reference_to_last_log_linear: 18.38 against 63.07 pp on 5 targets
- landmark vs pooled_curve_ratio: 18.38 against 21.59 pp on 5 targets
- landmark vs analog_knn: 18.38 against 24.23 pp on 5 targets
- landmark vs baseline_level: 18.38 against 36.61 pp on 5 targets
- next_observation 80% interval: 3/5 = 0.600
- next_observation 90% interval: 4/5 = 0.800
- landmark 80% interval: 3/5 = 0.600
- landmark 90% interval: 3/5 = 0.600

## by context (primary methods; comparator in parentheses)

- human/dme/anti_vegf: next_observation 2/5 within, 7.84 (9.80) pp; landmark 2/5 within, 11.17 (15.41) pp; initial_state_group_mean 2/5 within, 7.25 (10.32) pp
- human/dme/combination: next_observation 0/1 within, 5.30 (7.24) pp; landmark 1/1 within, 4.86 (15.48) pp
- human/dme/steroid_implant: next_observation 0/5 within, 9.47 (9.65) pp; landmark 0/5 within, 15.86 (19.75) pp; initial_state_group_mean 0/3 within, 14.83 (13.47) pp
- human/namd/anti_vegf: next_observation 5/7 within, 4.48 (5.50) pp; landmark 5/7 within, 10.39 (13.05) pp; initial_state_group_mean 2/6 within, 10.65 (15.92) pp
- human/namd/combination: next_observation 2/3 within, 4.48 (7.12) pp; landmark 0/3 within, 7.28 (22.39) pp; initial_state_group_mean 2/2 within, 4.43 (10.41) pp
- human/other/other: next_observation 1/1 within, 2.43 (0.26) pp; landmark 1/1 within, 3.55 (0.35) pp; initial_state_group_mean 0/1 within, 8.74 (21.74) pp
- human/other/vitrectomy: next_observation 1/2 within, 3.53 (4.87) pp; landmark 1/2 within, 3.54 (5.01) pp; initial_state_group_mean 1/2 within, 13.75 (27.69) pp
- human/rvo/anti_vegf: next_observation 0/1 within, 6.63 (9.11) pp; landmark 0/1 within, 13.72 (18.76) pp; initial_state_group_mean 0/1 within, 9.29 (5.26) pp
- human/rvo/combination: next_observation 1/1 within, 2.82 (3.74) pp; landmark 0/1 within, 6.34 (8.09) pp

## H5: model 1.4 against the frozen model 1.1.0, same fit

- next_observation: 1.4 5.84 against 1.1 5.83 pp on 87 common targets
- landmark: 1.4 10.58 against 1.1 9.99 pp on 87 common targets
- initial_state: 1.4 10.98 against 1.1 10.98 pp on 85 common targets (not tested: 1.1.0's own initial states)

## Model 1.1.0 on the same studies, its own H1-H4 (reported, not tested)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: FAIL
- H4_initial_state_control_group_means: pass
- H1 next observation 80% interval: 73/87 = 0.839
- H1 next observation 90% interval: 80/87 = 0.920
- H2 next observation vs persistence: 5.83 against 7.00 pp on 87 common targets
- H2 next observation vs last_two_log_linear: 5.83 against 13.78 pp on 87 common targets
- H2 next observation vs pooled_curve_ratio: 5.83 against 6.65 pp on 87 common targets
- H2 next observation vs analog_knn: 5.83 against 6.58 pp on 87 common targets
- H3 landmark vs persistence: 9.99 against 16.13 pp on 87 common targets
- H3 landmark vs pooled_curve_ratio: 9.99 against 9.80 pp on 87 common targets
- H3 landmark vs analog_knn: 9.99 against 11.65 pp on 87 common targets
- H4 initial state population_initial_state: 10.98 pp
- H4 initial state pooled_training_curve: 15.14 pp

Units: pp of each series' baseline central retinal thickness.
