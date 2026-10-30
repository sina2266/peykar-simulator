# Myopia axial-length model 1.0.0-m4 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: FAIL
- H2_accuracy_next_observation_group_means: FAIL
- H3_landmark_group_means: FAIL
- H4_initial_state_control_group_means: FAIL

- H1 next observation 80% interval: 30/47 = 0.638 (window 0.70–0.90)
- H1 next observation 90% interval: 37/47 = 0.787 (window 0.80–0.97)
- H2 next observation vs persistence: 12.01 against 17.78 pp on 47 common targets
- H2 next observation vs last_two_log_linear: 12.01 against 13.06 pp on 47 common targets
- H2 next observation vs pooled_curve_ratio: 12.01 against 11.14 pp on 47 common targets
- H2 next observation vs analog_knn: 12.01 against 10.20 pp on 47 common targets
- H3 landmark vs persistence: 18.07 against 42.54 pp on 47 common targets
- H3 landmark vs pooled_curve_ratio: 18.07 against 17.11 pp on 47 common targets
- H3 landmark vs analog_knn: 18.07 against 19.71 pp on 47 common targets
- landmark 80% interval (reported): 31/47 = 0.660
- landmark 90% interval (reported): 35/47 = 0.745
- H4 initial state population_initial_state: 19.15 pp, 6/23 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 17.32 pp, 4/23 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 59/70 = 0.843
- H4 initial state 90% interval (reported): 63/70 = 0.900

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| analog_knn | 18 | 9 | 8 | 10.20 | 30.20 | 47 |
| pooled_curve_ratio | 18 | 8 | 6 | 11.14 | 28.40 | 47 |
| population_assimilation_t4 | 18 | 11 | 10 | 11.35 | 36.78 | 47 |
| population_anchored_t4 | 18 | 10 | 9 | 11.74 | 36.70 | 47 |
| population_anchored | 18 | 10 | 9 | 11.85 | 36.71 | 47 |
| population_assimilation_robust | 18 | 10 | 9 | 11.99 | 36.91 | 47 |
| population_assimilation | 18 | 10 | 9 | 12.01 | 36.81 | 47 |
| reference_to_last_log_linear | 18 | 10 | 8 | 13.06 | 58.14 | 47 |
| last_two_log_linear | 18 | 9 | 8 | 13.06 | 58.14 | 47 |
| weak_prior | 18 | 4 | 4 | 14.46 | 40.54 | 47 |
| persistence | 18 | 2 | 0 | 17.78 | 44.00 | 47 |
| baseline_level | 18 | 0 | 0 | 53.93 | 168.00 | 47 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| pooled_curve_ratio | 18 | 5 | 5 | 17.11 | 68.28 | 47 |
| population_assimilation_t4 | 18 | 9 | 9 | 18.06 | 65.63 | 47 |
| population_assimilation | 18 | 9 | 9 | 18.07 | 65.64 | 47 |
| population_assimilation_robust | 18 | 9 | 9 | 18.24 | 66.07 | 47 |
| analog_knn | 18 | 4 | 4 | 19.71 | 80.00 | 47 |
| last_two_log_linear | 18 | 7 | 7 | 26.26 | 130.00 | 47 |
| reference_to_last_log_linear | 18 | 7 | 7 | 26.26 | 130.00 | 47 |
| population_anchored_t4 | 18 | 7 | 7 | 26.41 | 120.73 | 47 |
| population_anchored | 18 | 6 | 6 | 26.50 | 120.74 | 47 |
| weak_prior | 18 | 2 | 2 | 38.36 | 140.22 | 47 |
| persistence | 18 | 0 | 0 | 42.54 | 146.00 | 47 |
| baseline_level | 18 | 0 | 0 | 53.93 | 168.00 | 47 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| analog_knn | 18 | 9 | 8 | 10.20 | 30.20 | 47 |
| pooled_curve_ratio | 18 | 8 | 6 | 11.14 | 28.40 | 47 |
| population_assimilation_t4 | 18 | 11 | 10 | 11.35 | 36.78 | 47 |
| population_anchored_t4 | 18 | 10 | 9 | 11.74 | 36.70 | 47 |
| population_anchored | 18 | 10 | 9 | 11.85 | 36.71 | 47 |
| population_assimilation_robust | 18 | 10 | 9 | 11.99 | 36.91 | 47 |
| population_assimilation | 18 | 10 | 9 | 12.01 | 36.81 | 47 |
| reference_to_last_log_linear | 18 | 10 | 8 | 13.06 | 58.14 | 47 |
| last_two_log_linear | 18 | 9 | 8 | 13.06 | 58.14 | 47 |
| weak_prior | 18 | 4 | 4 | 14.46 | 40.54 | 47 |
| persistence | 18 | 2 | 0 | 17.78 | 44.00 | 47 |
| baseline_level | 18 | 0 | 0 | 53.93 | 168.00 | 47 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| pooled_curve_ratio | 18 | 5 | 5 | 17.11 | 68.28 | 47 |
| population_assimilation_t4 | 18 | 9 | 9 | 18.06 | 65.63 | 47 |
| population_assimilation | 18 | 9 | 9 | 18.07 | 65.64 | 47 |
| population_assimilation_robust | 18 | 9 | 9 | 18.24 | 66.07 | 47 |
| analog_knn | 18 | 4 | 4 | 19.71 | 80.00 | 47 |
| last_two_log_linear | 18 | 7 | 7 | 26.26 | 130.00 | 47 |
| reference_to_last_log_linear | 18 | 7 | 7 | 26.26 | 130.00 | 47 |
| population_anchored_t4 | 18 | 7 | 7 | 26.41 | 120.73 | 47 |
| population_anchored | 18 | 6 | 6 | 26.50 | 120.74 | 47 |
| weak_prior | 18 | 2 | 2 | 38.36 | 140.22 | 47 |
| persistence | 18 | 0 | 0 | 42.54 | 146.00 | 47 |
| baseline_level | 18 | 0 | 0 | 53.93 | 168.00 | 47 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| pooled_training_curve | 23 | 4 | 17.32 | 71.60 | 70 |
| population_initial_state | 23 | 6 | 19.15 | 88.43 | 70 |
| population_initial_state_t4 | 23 | 6 | 19.19 | 88.54 | 70 |

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

- human/atropine/atropine: next_observation 3/6 within, 18.08 (21.74) pp; landmark 3/6 within, 21.94 (36.64) pp; initial_state_group_mean 0/6 within, 15.09 (13.19) pp
- human/combination/atropine: next_observation 1/1 within, 4.28 (6.50) pp; landmark 1/1 within, 4.78 (20.35) pp; initial_state_group_mean 2/2 within, 4.75 (10.94) pp
- human/defocus_spectacles/optical: next_observation 1/1 within, 0.37 (16.00) pp; landmark 1/1 within, 0.37 (16.00) pp; initial_state_group_mean 0/1 within, 13.52 (16.58) pp
- human/multifocal_contacts/optical: next_observation 1/1 within, 3.46 (9.02) pp; landmark 0/1 within, 7.48 (29.73) pp; initial_state_group_mean 0/2 within, 5.98 (5.15) pp
- human/orthokeratology/optical: next_observation 0/2 within, 13.25 (13.27) pp; landmark 0/2 within, 10.83 (16.84) pp; initial_state_group_mean 1/4 within, 10.00 (13.64) pp
- human/other/control: initial_state_group_mean 0/1 within, 28.47 (60.25) pp
- human/red_light/light: next_observation 2/2 within, 3.30 (3.98) pp; landmark 2/2 within, 2.24 (7.26) pp; initial_state_group_mean 2/2 within, 2.57 (11.72) pp
- human/single_vision/control: next_observation 2/5 within, 12.65 (24.05) pp; landmark 2/5 within, 25.93 (67.00) pp; initial_state_group_mean 1/5 within, 31.96 (23.00) pp

Units: 1 pp = 0.01 mm of axial-length change from the baseline axial length.
