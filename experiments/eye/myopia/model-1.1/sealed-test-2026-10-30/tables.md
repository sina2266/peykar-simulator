# Myopia axial-length model 1.1.0-n3 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: FAIL
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: FAIL
- H5_next_observation_and_landmark_within_0_05_pp_of_model_1_0: FAIL

- H1 next observation 80% interval: 68/75 = 0.907 (window 0.70–0.90)
- H1 next observation 90% interval: 73/75 = 0.973 (window 0.80–0.97)
- H2 next observation vs persistence: 6.80 against 12.44 pp on 75 common targets
- H2 next observation vs last_two_log_linear: 6.80 against 18.16 pp on 75 common targets
- H2 next observation vs pooled_curve_ratio: 6.80 against 8.19 pp on 75 common targets
- H2 next observation vs analog_knn: 6.80 against 8.28 pp on 75 common targets
- H3 landmark vs persistence: 8.93 against 20.68 pp on 75 common targets
- H3 landmark vs pooled_curve_ratio: 8.93 against 10.76 pp on 75 common targets
- H3 landmark vs analog_knn: 8.93 against 11.43 pp on 75 common targets
- landmark 80% interval (reported): 66/75 = 0.880
- landmark 90% interval (reported): 72/75 = 0.960
- H4 initial state population_initial_state: 11.31 pp, 28/56 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 11.10 pp, 28/56 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 117/131 = 0.893
- H4 initial state 90% interval (reported): 122/131 = 0.931

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 42 | 31 | 30 | 6.80 | 32.72 | 75 |
| population_assimilation_t4 | 42 | 31 | 30 | 7.06 | 34.70 | 75 |
| population_assimilation_robust | 42 | 31 | 30 | 7.94 | 36.54 | 75 |
| pooled_curve_ratio | 42 | 27 | 26 | 8.19 | 41.30 | 75 |
| analog_knn | 42 | 24 | 24 | 8.28 | 44.88 | 75 |
| weak_prior | 42 | 14 | 12 | 10.20 | 55.48 | 75 |
| persistence | 42 | 5 | 0 | 12.44 | 45.00 | 75 |
| reference_to_last_log_linear | 42 | 24 | 24 | 16.16 | 101.00 | 75 |
| population_anchored_t4 | 42 | 26 | 26 | 16.70 | 101.44 | 75 |
| population_anchored | 42 | 26 | 26 | 16.72 | 101.48 | 75 |
| last_two_log_linear | 42 | 24 | 24 | 18.16 | 101.00 | 75 |
| baseline_level | 42 | 3 | 2 | 30.06 | 90.00 | 75 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 42 | 25 | 25 | 8.93 | 32.72 | 75 |
| population_assimilation_t4 | 42 | 25 | 25 | 9.26 | 34.70 | 75 |
| pooled_curve_ratio | 42 | 23 | 23 | 10.76 | 41.30 | 75 |
| population_assimilation_robust | 42 | 25 | 25 | 10.91 | 42.18 | 75 |
| analog_knn | 42 | 23 | 22 | 11.43 | 44.88 | 75 |
| weak_prior | 42 | 9 | 7 | 18.34 | 59.26 | 75 |
| persistence | 42 | 3 | 0 | 20.68 | 53.00 | 75 |
| baseline_level | 42 | 3 | 1 | 30.06 | 90.00 | 75 |
| last_two_log_linear | 42 | 19 | 19 | 33.28 | 186.00 | 75 |
| reference_to_last_log_linear | 42 | 19 | 19 | 33.28 | 186.00 | 75 |
| population_anchored_t4 | 42 | 20 | 20 | 36.15 | 183.56 | 75 |
| population_anchored | 42 | 20 | 20 | 36.19 | 183.74 | 75 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 42 | 31 | 30 | 6.80 | 32.72 | 75 |
| population_assimilation_t4 | 42 | 31 | 30 | 7.06 | 34.70 | 75 |
| population_assimilation_robust | 42 | 31 | 30 | 7.94 | 36.54 | 75 |
| pooled_curve_ratio | 42 | 27 | 26 | 8.19 | 41.30 | 75 |
| analog_knn | 42 | 24 | 24 | 8.28 | 44.88 | 75 |
| weak_prior | 42 | 14 | 12 | 10.20 | 55.48 | 75 |
| persistence | 42 | 5 | 0 | 12.44 | 45.00 | 75 |
| reference_to_last_log_linear | 42 | 24 | 24 | 16.16 | 101.00 | 75 |
| population_anchored_t4 | 42 | 26 | 26 | 16.70 | 101.44 | 75 |
| population_anchored | 42 | 26 | 26 | 16.72 | 101.48 | 75 |
| last_two_log_linear | 42 | 24 | 24 | 18.16 | 101.00 | 75 |
| baseline_level | 42 | 3 | 2 | 30.06 | 90.00 | 75 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 42 | 25 | 25 | 8.93 | 32.72 | 75 |
| population_assimilation_t4 | 42 | 25 | 25 | 9.26 | 34.70 | 75 |
| pooled_curve_ratio | 42 | 23 | 23 | 10.76 | 41.30 | 75 |
| population_assimilation_robust | 42 | 25 | 25 | 10.91 | 42.18 | 75 |
| analog_knn | 42 | 23 | 22 | 11.43 | 44.88 | 75 |
| weak_prior | 42 | 9 | 7 | 18.34 | 59.26 | 75 |
| persistence | 42 | 3 | 0 | 20.68 | 53.00 | 75 |
| baseline_level | 42 | 3 | 1 | 30.06 | 90.00 | 75 |
| last_two_log_linear | 42 | 19 | 19 | 33.28 | 186.00 | 75 |
| reference_to_last_log_linear | 42 | 19 | 19 | 33.28 | 186.00 | 75 |
| population_anchored_t4 | 42 | 20 | 20 | 36.15 | 183.56 | 75 |
| population_anchored | 42 | 20 | 20 | 36.19 | 183.74 | 75 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| pooled_training_curve | 56 | 28 | 11.10 | 40.09 | 131 |
| population_initial_state_t4 | 56 | 27 | 11.24 | 36.68 | 131 |
| population_initial_state | 56 | 28 | 11.31 | 36.49 | 131 |

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

- human/atropine/atropine: next_observation 2/3 within, 4.48 (7.93) pp; landmark 1/3 within, 7.48 (12.19) pp; initial_state_group_mean 4/7 within, 7.49 (7.90) pp
- human/combination/atropine: next_observation 1/1 within, 1.71 (6.08) pp; landmark 1/1 within, 1.90 (9.82) pp; initial_state_group_mean 1/1 within, 1.32 (1.22) pp
- human/defocus_spectacles/optical: next_observation 10/11 within, 3.09 (7.96) pp; landmark 10/11 within, 3.27 (8.53) pp; initial_state_group_mean 10/14 within, 5.41 (6.18) pp
- human/multifocal_contacts/optical: next_observation 4/6 within, 6.59 (13.96) pp; landmark 2/6 within, 11.55 (25.56) pp; initial_state_group_mean 2/6 within, 12.51 (12.73) pp
- human/orthokeratology/optical: next_observation 5/11 within, 11.04 (14.08) pp; landmark 3/11 within, 12.42 (19.32) pp; initial_state_group_mean 5/15 within, 14.99 (15.12) pp
- human/other/control: next_observation 1/1 within, 1.34 (9.00) pp; landmark 1/1 within, 1.34 (9.00) pp; initial_state_group_mean 0/1 within, 7.01 (13.54) pp
- human/single_vision/control: next_observation 8/9 within, 3.00 (13.67) pp; landmark 7/9 within, 5.39 (26.51) pp; initial_state_group_mean 6/12 within, 11.57 (9.33) pp

## H5: model 1.1 against the frozen model 1.0.0, same training

- next_observation: 1.1 6.80 against 1.0 6.88 pp on 75 common targets (within 0.05 pp)
- landmark: 1.1 8.93 against 1.0 8.66 pp on 75 common targets (NOT within 0.05 pp)

Units: 1 pp = 0.01 mm of axial-length change from the baseline axial length.
