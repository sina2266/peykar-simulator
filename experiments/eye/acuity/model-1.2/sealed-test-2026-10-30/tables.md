# Visual-acuity model 1.2.0-k1 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_landmark_and_initial_state_lower_next_observation_within_0_05_pp: FAIL

- H1 next observation 80% interval: 59/66 = 0.894 (window 0.70–0.90)
- H1 next observation 90% interval: 61/66 = 0.924 (window 0.80–0.97)
- H2 next observation vs persistence: 3.72 against 5.03 pp on 66 common targets
- H2 next observation vs last_two_log_linear: 3.72 against 18.59 pp on 66 common targets
- H2 next observation vs pooled_curve_ratio: 3.72 against 5.05 pp on 66 common targets
- H2 next observation vs analog_knn: 3.72 against 5.95 pp on 66 common targets
- H3 landmark vs persistence: 5.74 against 11.58 pp on 66 common targets
- H3 landmark vs pooled_curve_ratio: 5.74 against 7.01 pp on 66 common targets
- H3 landmark vs analog_knn: 5.74 against 16.25 pp on 66 common targets
- landmark 80% interval (reported): 55/66 = 0.833
- landmark 90% interval (reported): 58/66 = 0.879
- H4 initial state stacked_initial_state: 9.83 pp, 8/22 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 14.59 pp, 10/22 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 74/85 = 0.871
- H4 initial state 90% interval (reported): 79/85 = 0.929

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| stacked | 23 | 19 | 8 | 3.72 | 13.69 | 66 |
| population_assimilation_robust | 23 | 16 | 8 | 4.06 | 14.27 | 66 |
| population_assimilation_t4 | 23 | 16 | 6 | 4.08 | 14.30 | 66 |
| population_assimilation | 23 | 16 | 8 | 4.24 | 14.27 | 66 |
| population_anchored_t4 | 23 | 14 | 8 | 4.62 | 13.29 | 66 |
| population_anchored | 23 | 13 | 8 | 4.82 | 13.22 | 66 |
| persistence | 23 | 16 | 0 | 5.03 | 15.00 | 66 |
| pooled_curve_ratio | 23 | 14 | 5 | 5.05 | 14.64 | 66 |
| weak_prior | 23 | 13 | 1 | 5.18 | 15.91 | 66 |
| analog_knn | 23 | 13 | 6 | 5.95 | 18.06 | 66 |
| last_two_log_linear | 23 | 4 | 0 | 18.59 | 51.00 | 66 |
| reference_to_last_log_linear | 23 | 3 | 0 | 25.18 | 81.00 | 66 |
| baseline_level | 23 | 9 | 4 | 28.54 | 81.00 | 66 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| stacked | 23 | 12 | 6 | 5.74 | 15.61 | 66 |
| population_assimilation_robust | 23 | 10 | 7 | 6.66 | 14.27 | 66 |
| population_assimilation_t4 | 23 | 9 | 6 | 6.67 | 14.30 | 66 |
| population_assimilation | 23 | 9 | 6 | 6.86 | 14.27 | 66 |
| pooled_curve_ratio | 23 | 11 | 5 | 7.01 | 19.54 | 66 |
| population_anchored_t4 | 23 | 8 | 5 | 9.65 | 27.37 | 66 |
| population_anchored | 23 | 8 | 5 | 9.85 | 27.71 | 66 |
| weak_prior | 23 | 8 | 0 | 11.41 | 31.50 | 66 |
| persistence | 23 | 9 | 0 | 11.58 | 31.50 | 66 |
| analog_knn | 23 | 12 | 9 | 16.25 | 50.18 | 66 |
| baseline_level | 23 | 9 | 4 | 28.54 | 81.00 | 66 |
| last_two_log_linear | 23 | 0 | 0 | 118.03 | 615.00 | 66 |
| reference_to_last_log_linear | 23 | 0 | 0 | 118.03 | 615.00 | 66 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| stacked | 23 | 19 | 8 | 3.72 | 13.69 | 66 |
| population_assimilation_robust | 23 | 16 | 8 | 4.06 | 14.27 | 66 |
| population_assimilation_t4 | 23 | 16 | 6 | 4.08 | 14.30 | 66 |
| population_assimilation | 23 | 16 | 8 | 4.24 | 14.27 | 66 |
| population_anchored_t4 | 23 | 14 | 8 | 4.62 | 13.29 | 66 |
| population_anchored | 23 | 13 | 8 | 4.82 | 13.22 | 66 |
| persistence | 23 | 16 | 0 | 5.03 | 15.00 | 66 |
| pooled_curve_ratio | 23 | 14 | 5 | 5.05 | 14.64 | 66 |
| weak_prior | 23 | 13 | 1 | 5.18 | 15.91 | 66 |
| analog_knn | 23 | 13 | 6 | 5.95 | 18.06 | 66 |
| last_two_log_linear | 23 | 4 | 0 | 18.59 | 51.00 | 66 |
| reference_to_last_log_linear | 23 | 3 | 0 | 25.18 | 81.00 | 66 |
| baseline_level | 23 | 9 | 4 | 28.54 | 81.00 | 66 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| stacked | 23 | 12 | 6 | 5.74 | 15.61 | 66 |
| population_assimilation_robust | 23 | 10 | 7 | 6.66 | 14.27 | 66 |
| population_assimilation_t4 | 23 | 9 | 6 | 6.67 | 14.30 | 66 |
| population_assimilation | 23 | 9 | 6 | 6.86 | 14.27 | 66 |
| pooled_curve_ratio | 23 | 11 | 5 | 7.01 | 19.54 | 66 |
| population_anchored_t4 | 23 | 8 | 5 | 9.65 | 27.37 | 66 |
| population_anchored | 23 | 8 | 5 | 9.85 | 27.71 | 66 |
| weak_prior | 23 | 8 | 0 | 11.41 | 31.50 | 66 |
| persistence | 23 | 9 | 0 | 11.58 | 31.50 | 66 |
| analog_knn | 23 | 12 | 9 | 16.25 | 50.18 | 66 |
| baseline_level | 23 | 9 | 4 | 28.54 | 81.00 | 66 |
| last_two_log_linear | 23 | 0 | 0 | 118.03 | 615.00 | 66 |
| reference_to_last_log_linear | 23 | 0 | 0 | 118.03 | 615.00 | 66 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 22 | 8 | 9.00 | 31.60 | 85 |
| population_initial_state | 22 | 8 | 9.01 | 31.79 | 85 |
| stacked_initial_state | 22 | 8 | 9.83 | 34.96 | 85 |
| pooled_training_curve | 22 | 10 | 14.59 | 52.28 | 85 |

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

- human/cxl: next_observation 10/10 within, 2.36 (3.16) pp; landmark 7/10 within, 4.14 (6.04) pp; initial_state_group_mean 7/9 within, 6.09 (3.22) pp
- human/dmek: next_observation 5/9 within, 5.71 (6.97) pp; landmark 2/9 within, 8.25 (14.46) pp; initial_state_group_mean 0/9 within, 14.03 (21.00) pp
- human/dsaek: next_observation 3/3 within, 2.12 (5.04) pp; landmark 2/3 within, 4.05 (15.60) pp; initial_state_group_mean 1/3 within, 6.90 (15.99) pp
- human/other: next_observation 1/1 within, 1.79 (0.87) pp; landmark 1/1 within, 2.90 (0.67) pp; initial_state_group_mean 0/1 within, 8.21 (6.68) pp

## H5: model 1.2 against the frozen model 1.0.0, same fit

- next_observation: 1.2 3.72 against 1.0 4.06 letters on 66 common targets
- landmark: 1.2 5.74 against 1.0 6.66 letters on 66 common targets
- initial_state: 1.2 9.83 against 1.0 9.01 letters on 85 common targets

## Model 1.0.0 on the same studies, its own H1-H4 (reported, not tested)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H1 next observation 80% interval: 57/66 = 0.864
- H1 next observation 90% interval: 63/66 = 0.955
- H2 next observation vs persistence: 4.06 against 5.03 letters on 66 common targets
- H2 next observation vs last_two_log_linear: 4.06 against 18.59 letters on 66 common targets
- H2 next observation vs pooled_curve_ratio: 4.06 against 5.05 letters on 66 common targets
- H2 next observation vs analog_knn: 4.06 against 5.95 letters on 66 common targets
- H3 landmark vs persistence: 6.66 against 11.58 letters on 66 common targets
- H3 landmark vs pooled_curve_ratio: 6.66 against 7.01 letters on 66 common targets
- H3 landmark vs analog_knn: 6.66 against 16.25 letters on 66 common targets
- H4 initial state population_initial_state: 9.01 letters
- H4 initial state pooled_training_curve: 14.59 letters

Units: 1 pp = 1 ETDRS letter (0.02 logMAR) of corrected distance acuity change from the preoperative acuity.
