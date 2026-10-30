# IOP-after-surgery model 1.4.0-d1s — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_initial_state_lower_next_observation_and_landmark_within_0_05_pp: FAIL

- H1 next observation 80% interval: 55/72 = 0.764 (window 0.70–0.90)
- H1 next observation 90% interval: 65/72 = 0.903 (window 0.80–0.97)
- H2 next observation vs persistence: 8.34 against 10.86 pp on 72 common targets
- H2 next observation vs last_two_log_linear: 8.34 against 54.22 pp on 72 common targets
- H2 next observation vs pooled_curve_ratio: 8.34 against 11.57 pp on 72 common targets
- H2 next observation vs analog_knn: 8.34 against 11.06 pp on 72 common targets
- H3 landmark vs persistence: 9.14 against 18.06 pp on 72 common targets
- H3 landmark vs pooled_curve_ratio: 9.14 against 20.20 pp on 72 common targets
- H3 landmark vs analog_knn: 9.14 against 16.26 pp on 72 common targets
- landmark 80% interval (reported): 56/72 = 0.778
- landmark 90% interval (reported): 65/72 = 0.903
- H4 initial state population_initial_state: 9.46 pp, 11/23 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 15.59 pp, 4/23 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 79/87 = 0.908
- H4 initial state 90% interval (reported): 84/87 = 0.966

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 20 | 9 | 3 | 8.34 | 39.14 | 72 |
| population_assimilation_t4 | 20 | 8 | 2 | 8.39 | 39.15 | 72 |
| population_assimilation_robust | 20 | 8 | 2 | 8.43 | 39.17 | 72 |
| population_anchored | 20 | 9 | 2 | 10.37 | 39.55 | 72 |
| population_anchored_t4 | 20 | 9 | 2 | 10.56 | 39.76 | 72 |
| persistence | 20 | 7 | 0 | 10.86 | 40.13 | 72 |
| weak_prior | 20 | 9 | 2 | 11.00 | 41.11 | 72 |
| analog_knn | 20 | 8 | 4 | 11.06 | 45.07 | 72 |
| pooled_curve_ratio | 20 | 6 | 3 | 11.57 | 43.60 | 72 |
| baseline_level | 20 | 0 | 0 | 40.29 | 61.34 | 72 |
| reference_to_last_log_linear | 20 | 0 | 0 | 41.43 | 192.42 | 72 |
| last_two_log_linear | 20 | 0 | 0 | 54.22 | 223.68 | 72 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 20 | 9 | 4 | 9.14 | 40.22 | 72 |
| population_assimilation_t4 | 20 | 8 | 3 | 9.15 | 40.30 | 72 |
| population_assimilation_robust | 20 | 8 | 3 | 9.25 | 40.75 | 72 |
| weak_prior | 20 | 8 | 2 | 15.34 | 42.84 | 72 |
| analog_knn | 20 | 6 | 4 | 16.26 | 57.97 | 72 |
| population_anchored_t4 | 20 | 9 | 3 | 16.96 | 63.07 | 72 |
| population_anchored | 20 | 9 | 3 | 16.96 | 63.08 | 72 |
| persistence | 20 | 7 | 0 | 18.06 | 60.94 | 72 |
| pooled_curve_ratio | 20 | 5 | 2 | 20.20 | 69.45 | 72 |
| baseline_level | 20 | 0 | 0 | 40.29 | 61.34 | 72 |
| last_two_log_linear | 20 | 0 | 0 | 63.06 | 192.42 | 72 |
| reference_to_last_log_linear | 20 | 0 | 0 | 63.06 | 192.42 | 72 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 20 | 9 | 3 | 8.34 | 39.14 | 72 |
| population_assimilation_t4 | 20 | 8 | 2 | 8.39 | 39.15 | 72 |
| population_assimilation_robust | 20 | 8 | 2 | 8.43 | 39.17 | 72 |
| population_anchored | 20 | 9 | 2 | 10.37 | 39.55 | 72 |
| population_anchored_t4 | 20 | 9 | 2 | 10.56 | 39.76 | 72 |
| persistence | 20 | 7 | 0 | 10.86 | 40.13 | 72 |
| weak_prior | 20 | 9 | 2 | 11.00 | 41.11 | 72 |
| analog_knn | 20 | 8 | 4 | 11.06 | 45.07 | 72 |
| pooled_curve_ratio | 20 | 6 | 3 | 11.57 | 43.60 | 72 |
| baseline_level | 20 | 0 | 0 | 40.29 | 61.34 | 72 |
| reference_to_last_log_linear | 20 | 0 | 0 | 41.43 | 192.42 | 72 |
| last_two_log_linear | 20 | 0 | 0 | 54.22 | 223.68 | 72 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 20 | 9 | 4 | 9.14 | 40.22 | 72 |
| population_assimilation_t4 | 20 | 8 | 3 | 9.15 | 40.30 | 72 |
| population_assimilation_robust | 20 | 8 | 3 | 9.25 | 40.75 | 72 |
| weak_prior | 20 | 8 | 2 | 15.34 | 42.84 | 72 |
| analog_knn | 20 | 6 | 4 | 16.26 | 57.97 | 72 |
| population_anchored_t4 | 20 | 9 | 3 | 16.96 | 63.07 | 72 |
| population_anchored | 20 | 9 | 3 | 16.96 | 63.08 | 72 |
| persistence | 20 | 7 | 0 | 18.06 | 60.94 | 72 |
| pooled_curve_ratio | 20 | 5 | 2 | 20.20 | 69.45 | 72 |
| baseline_level | 20 | 0 | 0 | 40.29 | 61.34 | 72 |
| last_two_log_linear | 20 | 0 | 0 | 63.06 | 192.42 | 72 |
| reference_to_last_log_linear | 20 | 0 | 0 | 63.06 | 192.42 | 72 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state | 23 | 11 | 9.46 | 37.35 | 87 |
| population_initial_state_t4 | 23 | 12 | 9.46 | 37.35 | 87 |
| pooled_training_curve | 23 | 4 | 15.59 | 38.59 | 87 |

## single eyes (reported, not tested)

- next_observation vs persistence: n/a against n/a pp on 0 targets
- next_observation vs last_two_log_linear: n/a against n/a pp on 0 targets
- next_observation vs reference_to_last_log_linear: n/a against n/a pp on 0 targets
- next_observation vs pooled_curve_ratio: n/a against n/a pp on 0 targets
- next_observation vs analog_knn: n/a against n/a pp on 0 targets
- next_observation vs preoperative_level: n/a against n/a pp on 0 targets
- landmark vs persistence: n/a against n/a pp on 0 targets
- landmark vs last_two_log_linear: n/a against n/a pp on 0 targets
- landmark vs reference_to_last_log_linear: n/a against n/a pp on 0 targets
- landmark vs pooled_curve_ratio: n/a against n/a pp on 0 targets
- landmark vs analog_knn: n/a against n/a pp on 0 targets
- landmark vs preoperative_level: n/a against n/a pp on 0 targets
- next_observation 80% interval: 0/0 = n/a
- next_observation 90% interval: 0/0 = n/a
- landmark 80% interval: 0/0 = n/a
- landmark 90% interval: 0/0 = n/a

## by procedure context (primary methods; comparator in parentheses)

- human/all/angle_mims: next_observation 1/4 within, 11.87 (15.37) pp; landmark 1/4 within, 12.11 (20.64) pp; initial_state_group_mean 5/8 within, 12.09 (15.44) pp
- human/all/cataract: next_observation 0/2 within, 19.63 (27.83) pp; landmark 0/2 within, 19.63 (27.83) pp; initial_state_group_mean 0/2 within, 13.15 (23.38) pp
- human/all/trabeculectomy: next_observation 5/7 within, 5.15 (6.59) pp; landmark 5/7 within, 5.13 (12.71) pp; initial_state_group_mean 4/6 within, 6.35 (9.92) pp
- human/all/tube: next_observation 1/1 within, 2.87 (2.64) pp; landmark 1/1 within, 2.70 (2.52) pp; initial_state_group_mean 1/1 within, 3.29 (11.98) pp
- human/endoscopic/cyclodestruction: next_observation 0/1 within, 10.60 (8.04) pp; landmark 0/1 within, 11.93 (12.13) pp; initial_state_group_mean 0/1 within, 9.10 (18.81) pp
- human/other/subconjunctival_mims: next_observation 0/2 within, 7.23 (12.42) pp; landmark 0/2 within, 13.21 (24.68) pp; initial_state_group_mean 0/2 within, 14.97 (25.45) pp
- human/preserflo/subconjunctival_mims: next_observation 1/1 within, 3.27 (7.18) pp; landmark 1/1 within, 2.14 (21.95) pp; initial_state_group_mean 0/1 within, 5.01 (15.95) pp
- human/xen/subconjunctival_mims: next_observation 1/2 within, 4.58 (8.00) pp; landmark 1/2 within, 5.97 (22.52) pp; initial_state_group_mean 1/2 within, 4.56 (11.74) pp

Non-human series removed before the tested analysis (reported per series): 0 forecast series, 0 control initial-state series.

## H5: model 1.4 against the frozen model 1.2.0, same training

- next_observation: 1.4 8.34 against 1.2 8.28 pp on 72 common targets
- landmark: 1.4 9.14 against 1.2 8.95 pp on 72 common targets
- initial_state: 1.4 9.46 against 1.2 9.57 pp on 87 common targets
- first-month next-observation 80% coverage, reported (18 targets): 1.4 0.722 against 1.2 0.778
- first-month next-observation 90% coverage, reported (18 targets): 1.4 0.944 against 1.2 1.000
