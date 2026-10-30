# Myopia axial-length model 1.0.0-m4 — development results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: FAIL
- H4_initial_state_control_group_means: pass

- H1 next observation 80% interval: 50/68 = 0.735 (window 0.70–0.90)
- H1 next observation 90% interval: 58/68 = 0.853 (window 0.80–0.97)
- H2 next observation vs persistence: 9.72 against 18.53 pp on 68 common targets
- H2 next observation vs last_two_log_linear: 9.72 against 15.75 pp on 68 common targets
- H2 next observation vs pooled_curve_ratio: 9.72 against 18.33 pp on 68 common targets
- H2 next observation vs analog_knn: 8.57 against 9.14 pp on 64 common targets
- H3 landmark vs persistence: 15.66 against 34.83 pp on 68 common targets
- H3 landmark vs pooled_curve_ratio: 15.66 against 27.04 pp on 68 common targets
- H3 landmark vs analog_knn: 10.11 against 9.92 pp on 64 common targets
- landmark 80% interval (reported): 50/68 = 0.735
- landmark 90% interval (reported): 55/68 = 0.809
- H4 initial state population_initial_state: 14.03 pp, 19/59 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 21.72 pp, 14/59 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 101/127 = 0.795
- H4 initial state 90% interval (reported): 113/127 = 0.890

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| analog_knn | 40 | 27 | 24 | 9.14 | 39.20 | 64 |
| population_assimilation | 40 | 23 | 23 | 9.72 | 35.47 | 68 |
| population_assimilation_t4 | 40 | 23 | 23 | 9.75 | 35.72 | 68 |
| population_assimilation_robust | 40 | 23 | 23 | 9.76 | 35.74 | 68 |
| population_anchored_t4 | 40 | 17 | 17 | 11.05 | 43.71 | 68 |
| population_anchored | 40 | 17 | 17 | 11.21 | 43.60 | 68 |
| weak_prior | 40 | 8 | 6 | 12.61 | 42.28 | 68 |
| reference_to_last_log_linear | 40 | 23 | 23 | 14.72 | 70.14 | 68 |
| last_two_log_linear | 40 | 24 | 24 | 15.75 | 80.86 | 68 |
| pooled_curve_ratio | 40 | 20 | 20 | 18.33 | 113.90 | 68 |
| persistence | 40 | 3 | 0 | 18.53 | 66.00 | 68 |
| baseline_level | 40 | 2 | 1 | 44.24 | 180.00 | 68 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| analog_knn | 40 | 22 | 21 | 9.92 | 39.20 | 64 |
| population_assimilation_t4 | 40 | 21 | 21 | 15.48 | 78.30 | 68 |
| population_assimilation_robust | 40 | 21 | 21 | 15.52 | 78.82 | 68 |
| population_assimilation | 40 | 21 | 21 | 15.66 | 78.81 | 68 |
| last_two_log_linear | 40 | 20 | 19 | 20.06 | 79.86 | 68 |
| reference_to_last_log_linear | 40 | 20 | 19 | 20.06 | 79.86 | 68 |
| population_anchored_t4 | 40 | 15 | 14 | 22.00 | 82.90 | 68 |
| population_anchored | 40 | 15 | 14 | 22.26 | 85.22 | 68 |
| pooled_curve_ratio | 40 | 19 | 19 | 27.04 | 142.20 | 68 |
| weak_prior | 40 | 8 | 6 | 30.11 | 149.58 | 68 |
| persistence | 40 | 2 | 0 | 34.83 | 163.00 | 68 |
| baseline_level | 40 | 2 | 1 | 44.24 | 180.00 | 68 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| analog_knn | 40 | 27 | 24 | 9.14 | 39.20 | 64 |
| population_assimilation | 40 | 23 | 23 | 9.72 | 35.47 | 68 |
| population_assimilation_t4 | 40 | 23 | 23 | 9.75 | 35.72 | 68 |
| population_assimilation_robust | 40 | 23 | 23 | 9.76 | 35.74 | 68 |
| population_anchored_t4 | 40 | 17 | 17 | 11.05 | 43.71 | 68 |
| population_anchored | 40 | 17 | 17 | 11.21 | 43.60 | 68 |
| weak_prior | 40 | 8 | 6 | 12.61 | 42.28 | 68 |
| reference_to_last_log_linear | 40 | 23 | 23 | 14.72 | 70.14 | 68 |
| last_two_log_linear | 40 | 24 | 24 | 15.75 | 80.86 | 68 |
| pooled_curve_ratio | 40 | 20 | 20 | 18.33 | 113.90 | 68 |
| persistence | 40 | 3 | 0 | 18.53 | 66.00 | 68 |
| baseline_level | 40 | 2 | 1 | 44.24 | 180.00 | 68 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| analog_knn | 40 | 22 | 21 | 9.92 | 39.20 | 64 |
| population_assimilation_t4 | 40 | 21 | 21 | 15.48 | 78.30 | 68 |
| population_assimilation_robust | 40 | 21 | 21 | 15.52 | 78.82 | 68 |
| population_assimilation | 40 | 21 | 21 | 15.66 | 78.81 | 68 |
| last_two_log_linear | 40 | 20 | 19 | 20.06 | 79.86 | 68 |
| reference_to_last_log_linear | 40 | 20 | 19 | 20.06 | 79.86 | 68 |
| population_anchored_t4 | 40 | 15 | 14 | 22.00 | 82.90 | 68 |
| population_anchored | 40 | 15 | 14 | 22.26 | 85.22 | 68 |
| pooled_curve_ratio | 40 | 19 | 19 | 27.04 | 142.20 | 68 |
| weak_prior | 40 | 8 | 6 | 30.11 | 149.58 | 68 |
| persistence | 40 | 2 | 0 | 34.83 | 163.00 | 68 |
| baseline_level | 40 | 2 | 1 | 44.24 | 180.00 | 68 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state | 59 | 19 | 14.03 | 72.07 | 127 |
| population_initial_state_t4 | 59 | 19 | 14.05 | 72.33 | 127 |
| pooled_training_curve | 59 | 14 | 21.72 | 139.90 | 127 |

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

- human/atropine/atropine: next_observation 2/5 within, 18.21 (29.43) pp; landmark 2/5 within, 16.83 (30.27) pp; initial_state_group_mean 3/8 within, 14.77 (14.31) pp
- human/combination/atropine: next_observation 5/6 within, 5.72 (7.37) pp; landmark 4/6 within, 5.63 (11.01) pp; initial_state_group_mean 2/6 within, 8.22 (8.84) pp
- human/defocus_spectacles/optical: next_observation 7/8 within, 6.10 (11.49) pp; landmark 7/8 within, 9.67 (15.18) pp; initial_state_group_mean 4/15 within, 12.03 (11.82) pp
- human/orthokeratology/optical: next_observation 4/10 within, 6.29 (13.93) pp; landmark 4/10 within, 12.35 (30.96) pp; initial_state_group_mean 6/13 within, 11.70 (21.48) pp
- human/other/control: next_observation 2/2 within, 3.39 (9.47) pp; landmark 1/2 within, 5.96 (14.12) pp; initial_state_group_mean 1/3 within, 8.08 (17.05) pp
- human/red_light/light: next_observation 0/1 within, 10.66 (1.58) pp; landmark 0/1 within, 13.07 (1.58) pp; initial_state_group_mean 0/1 within, 13.37 (13.54) pp
- human/single_vision/control: next_observation 3/8 within, 13.62 (28.94) pp; landmark 3/8 within, 26.67 (60.76) pp; initial_state_group_mean 3/13 within, 20.66 (35.28) pp

Units: 1 pp = 0.01 mm of axial-length change from the baseline axial length.
