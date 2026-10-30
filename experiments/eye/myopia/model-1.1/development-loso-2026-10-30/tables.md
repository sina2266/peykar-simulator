# Myopia axial-length model 1.1.0-n3 — development results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_next_observation_and_landmark_within_0_05_pp_of_model_1_0: pass

- H1 next observation 80% interval: 97/115 = 0.843 (window 0.70–0.90)
- H1 next observation 90% interval: 106/115 = 0.922 (window 0.80–0.97)
- H2 next observation vs persistence: 9.39 against 18.22 pp on 115 common targets
- H2 next observation vs last_two_log_linear: 9.39 against 14.71 pp on 115 common targets
- H2 next observation vs pooled_curve_ratio: 9.39 against 14.33 pp on 115 common targets
- H2 next observation vs analog_knn: 9.07 against 9.80 pp on 111 common targets
- H3 landmark vs persistence: 12.86 against 38.17 pp on 115 common targets
- H3 landmark vs pooled_curve_ratio: 12.86 against 21.75 pp on 115 common targets
- H3 landmark vs analog_knn: 12.61 against 15.18 pp on 111 common targets
- landmark 80% interval (reported): 93/115 = 0.809
- landmark 90% interval (reported): 103/115 = 0.896
- H4 initial state population_initial_state: 13.98 pp, 30/82 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 18.89 pp, 25/82 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 176/197 = 0.893
- H4 initial state 90% interval (reported): 188/197 = 0.954

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 58 | 36 | 36 | 9.39 | 37.76 | 115 |
| population_assimilation_robust | 58 | 36 | 36 | 9.55 | 37.82 | 115 |
| population_assimilation_t4 | 58 | 37 | 36 | 9.57 | 37.81 | 115 |
| analog_knn | 58 | 36 | 33 | 9.80 | 44.90 | 111 |
| population_anchored | 58 | 30 | 28 | 11.61 | 63.13 | 115 |
| population_anchored_t4 | 58 | 30 | 28 | 11.61 | 63.11 | 115 |
| weak_prior | 58 | 14 | 13 | 12.85 | 41.35 | 115 |
| reference_to_last_log_linear | 58 | 33 | 31 | 14.06 | 70.14 | 115 |
| pooled_curve_ratio | 58 | 28 | 25 | 14.33 | 83.55 | 115 |
| last_two_log_linear | 58 | 33 | 32 | 14.71 | 80.86 | 115 |
| persistence | 58 | 5 | 0 | 18.22 | 66.00 | 115 |
| baseline_level | 58 | 2 | 1 | 48.43 | 180.00 | 115 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 58 | 30 | 30 | 12.78 | 53.39 | 115 |
| population_assimilation_robust | 58 | 32 | 32 | 12.86 | 53.79 | 115 |
| population_assimilation | 58 | 30 | 30 | 12.86 | 53.54 | 115 |
| analog_knn | 58 | 30 | 27 | 15.18 | 80.00 | 111 |
| pooled_curve_ratio | 58 | 23 | 23 | 21.75 | 124.82 | 115 |
| population_anchored_t4 | 58 | 24 | 23 | 21.79 | 122.78 | 115 |
| population_anchored | 58 | 24 | 23 | 21.82 | 123.01 | 115 |
| last_two_log_linear | 58 | 27 | 26 | 22.80 | 130.00 | 115 |
| reference_to_last_log_linear | 58 | 27 | 26 | 22.80 | 130.00 | 115 |
| weak_prior | 58 | 11 | 9 | 33.40 | 148.29 | 115 |
| persistence | 58 | 2 | 0 | 38.17 | 163.00 | 115 |
| baseline_level | 58 | 2 | 1 | 48.43 | 180.00 | 115 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 58 | 36 | 36 | 9.39 | 37.76 | 115 |
| population_assimilation_robust | 58 | 36 | 36 | 9.55 | 37.82 | 115 |
| population_assimilation_t4 | 58 | 37 | 36 | 9.57 | 37.81 | 115 |
| analog_knn | 58 | 36 | 33 | 9.80 | 44.90 | 111 |
| population_anchored | 58 | 30 | 28 | 11.61 | 63.13 | 115 |
| population_anchored_t4 | 58 | 30 | 28 | 11.61 | 63.11 | 115 |
| weak_prior | 58 | 14 | 13 | 12.85 | 41.35 | 115 |
| reference_to_last_log_linear | 58 | 33 | 31 | 14.06 | 70.14 | 115 |
| pooled_curve_ratio | 58 | 28 | 25 | 14.33 | 83.55 | 115 |
| last_two_log_linear | 58 | 33 | 32 | 14.71 | 80.86 | 115 |
| persistence | 58 | 5 | 0 | 18.22 | 66.00 | 115 |
| baseline_level | 58 | 2 | 1 | 48.43 | 180.00 | 115 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 58 | 30 | 30 | 12.78 | 53.39 | 115 |
| population_assimilation_robust | 58 | 32 | 32 | 12.86 | 53.79 | 115 |
| population_assimilation | 58 | 30 | 30 | 12.86 | 53.54 | 115 |
| analog_knn | 58 | 30 | 27 | 15.18 | 80.00 | 111 |
| pooled_curve_ratio | 58 | 23 | 23 | 21.75 | 124.82 | 115 |
| population_anchored_t4 | 58 | 24 | 23 | 21.79 | 122.78 | 115 |
| population_anchored | 58 | 24 | 23 | 21.82 | 123.01 | 115 |
| last_two_log_linear | 58 | 27 | 26 | 22.80 | 130.00 | 115 |
| reference_to_last_log_linear | 58 | 27 | 26 | 22.80 | 130.00 | 115 |
| weak_prior | 58 | 11 | 9 | 33.40 | 148.29 | 115 |
| persistence | 58 | 2 | 0 | 38.17 | 163.00 | 115 |
| baseline_level | 58 | 2 | 1 | 48.43 | 180.00 | 115 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 82 | 32 | 13.87 | 71.23 | 197 |
| population_initial_state | 82 | 30 | 13.98 | 71.90 | 197 |
| pooled_training_curve | 82 | 25 | 18.89 | 122.55 | 197 |

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

- human/atropine/atropine: next_observation 5/11 within, 17.79 (24.90) pp; landmark 5/11 within, 20.62 (34.39) pp; initial_state_group_mean 4/14 within, 16.24 (12.82) pp
- human/combination/atropine: next_observation 6/7 within, 3.86 (7.11) pp; landmark 5/7 within, 4.97 (14.58) pp; initial_state_group_mean 6/8 within, 6.91 (9.26) pp
- human/defocus_spectacles/optical: next_observation 8/9 within, 4.96 (12.02) pp; landmark 8/9 within, 8.87 (15.26) pp; initial_state_group_mean 4/16 within, 11.60 (12.26) pp
- human/multifocal_contacts/optical: next_observation 1/1 within, 2.55 (9.02) pp; landmark 0/1 within, 6.32 (29.73) pp; initial_state_group_mean 1/2 within, 4.58 (14.07) pp
- human/orthokeratology/optical: next_observation 5/12 within, 7.53 (13.79) pp; landmark 4/12 within, 9.04 (28.53) pp; initial_state_group_mean 9/17 within, 11.57 (19.44) pp
- human/other/control: next_observation 2/2 within, 2.95 (9.47) pp; landmark 1/2 within, 6.10 (14.12) pp; initial_state_group_mean 0/4 within, 15.78 (24.42) pp
- human/red_light/light: next_observation 2/3 within, 3.49 (3.54) pp; landmark 2/3 within, 4.98 (6.33) pp; initial_state_group_mean 2/3 within, 6.64 (11.11) pp
- human/single_vision/control: next_observation 7/13 within, 10.07 (26.60) pp; landmark 5/13 within, 17.12 (63.96) pp; initial_state_group_mean 4/18 within, 19.40 (27.52) pp

## H5: model 1.1 against the frozen model 1.0.0, same training

- next_observation: 1.1 9.39 against 1.0 10.04 pp on 115 common targets (within 0.05 pp)
- landmark: 1.1 12.86 against 1.0 13.22 pp on 115 common targets (within 0.05 pp)

Units: 1 pp = 0.01 mm of axial-length change from the baseline axial length.
