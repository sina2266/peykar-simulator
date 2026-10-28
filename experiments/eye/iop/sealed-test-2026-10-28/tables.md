# IOP-after-surgery model 1.0.0 — sealed results

## Declared checks (group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass

- H1 next observation 80% interval: 151/198 = 0.763 (window 0.70–0.90)
- H1 next observation 90% interval: 177/198 = 0.894 (window 0.80–0.97)
- H2 next observation vs persistence: 7.74 against 8.64 pp on 198 common targets
- H2 next observation vs last_two_log_linear: 7.74 against 39.98 pp on 198 common targets
- H2 next observation vs pooled_curve_ratio: 7.74 against 8.98 pp on 198 common targets
- H2 next observation vs analog_knn: 7.74 against 9.77 pp on 198 common targets
- H3 landmark vs persistence: 9.92 against 12.89 pp on 198 common targets
- H3 landmark vs pooled_curve_ratio: 9.92 against 14.24 pp on 198 common targets
- H3 landmark vs analog_knn: 9.92 against 14.82 pp on 198 common targets
- landmark 80% interval (reported): 129/198 = 0.652
- landmark 90% interval (reported): 163/198 = 0.823
- H4 initial state population_initial_state: 10.96 pp, 10/40 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 17.99 pp, 4/40 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 170/222 = 0.766
- H4 initial state 90% interval (reported): 189/222 = 0.851

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 43 | 21 | 11 | 7.65 | 34.59 | 198 |
| population_assimilation | 43 | 21 | 11 | 7.71 | 34.69 | 198 |
| population_assimilation_robust | 43 | 21 | 11 | 7.74 | 34.45 | 198 |
| persistence | 43 | 18 | 0 | 8.64 | 41.57 | 198 |
| population_anchored_t4 | 43 | 19 | 7 | 8.89 | 38.56 | 198 |
| population_anchored | 43 | 19 | 7 | 8.90 | 35.68 | 198 |
| pooled_curve_ratio | 43 | 12 | 4 | 8.98 | 36.33 | 198 |
| weak_prior | 43 | 18 | 0 | 9.17 | 48.81 | 198 |
| analog_knn | 43 | 13 | 3 | 9.77 | 51.52 | 198 |
| reference_to_last_log_linear | 43 | 0 | 0 | 32.03 | 130.55 | 198 |
| last_two_log_linear | 43 | 0 | 0 | 39.98 | 226.54 | 198 |
| preoperative_level | 43 | 0 | 0 | 43.53 | 75.91 | 198 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 43 | 19 | 11 | 9.57 | 34.59 | 198 |
| population_assimilation | 43 | 18 | 10 | 9.57 | 34.69 | 198 |
| population_assimilation_robust | 43 | 17 | 10 | 9.92 | 34.45 | 198 |
| persistence | 43 | 14 | 0 | 12.89 | 35.55 | 198 |
| weak_prior | 43 | 13 | 6 | 13.82 | 34.63 | 198 |
| pooled_curve_ratio | 43 | 7 | 2 | 14.24 | 46.81 | 198 |
| analog_knn | 43 | 6 | 5 | 14.82 | 51.52 | 198 |
| population_anchored_t4 | 43 | 15 | 6 | 15.29 | 39.21 | 198 |
| population_anchored | 43 | 16 | 7 | 15.41 | 39.73 | 198 |
| preoperative_level | 43 | 0 | 0 | 43.53 | 75.91 | 198 |
| last_two_log_linear | 43 | 0 | 0 | 70.95 | 232.72 | 198 |
| reference_to_last_log_linear | 43 | 0 | 0 | 70.95 | 232.72 | 198 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 54 | 24 | 14 | 7.83 | 34.69 | 231 |
| population_assimilation_t4 | 54 | 23 | 13 | 7.86 | 34.59 | 231 |
| population_assimilation_robust | 54 | 23 | 13 | 7.92 | 34.45 | 231 |
| persistence | 54 | 20 | 0 | 9.28 | 41.57 | 231 |
| population_anchored_t4 | 54 | 21 | 7 | 9.46 | 38.56 | 231 |
| population_anchored | 54 | 21 | 7 | 9.48 | 35.68 | 231 |
| pooled_curve_ratio | 54 | 14 | 4 | 9.52 | 36.33 | 231 |
| weak_prior | 54 | 19 | 1 | 9.87 | 48.81 | 231 |
| analog_knn | 54 | 14 | 4 | 10.21 | 51.52 | 231 |
| reference_to_last_log_linear | 54 | 0 | 0 | 30.72 | 130.55 | 231 |
| last_two_log_linear | 54 | 0 | 0 | 37.73 | 226.54 | 231 |
| preoperative_level | 54 | 0 | 0 | 46.79 | 76.36 | 231 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 54 | 20 | 12 | 9.46 | 34.69 | 231 |
| population_assimilation_t4 | 54 | 20 | 12 | 9.57 | 34.59 | 231 |
| population_assimilation_robust | 54 | 18 | 11 | 9.86 | 34.45 | 231 |
| persistence | 54 | 15 | 0 | 13.77 | 36.96 | 231 |
| pooled_curve_ratio | 54 | 7 | 2 | 14.73 | 46.81 | 231 |
| weak_prior | 54 | 14 | 7 | 14.89 | 39.06 | 231 |
| analog_knn | 54 | 7 | 6 | 15.41 | 51.52 | 231 |
| population_anchored_t4 | 54 | 15 | 6 | 15.59 | 39.21 | 231 |
| population_anchored | 54 | 16 | 7 | 15.68 | 39.73 | 231 |
| preoperative_level | 54 | 0 | 0 | 46.79 | 76.36 | 231 |
| last_two_log_linear | 54 | 0 | 0 | 66.79 | 232.72 | 231 |
| reference_to_last_log_linear | 54 | 0 | 0 | 66.79 | 232.72 | 231 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 40 | 10 | 10.94 | 43.79 | 222 |
| population_initial_state | 40 | 10 | 10.96 | 43.83 | 222 |
| pooled_training_curve | 40 | 4 | 17.99 | 70.23 | 222 |

## single eyes (reported, not tested)

- next_observation vs persistence: 8.92 against 12.48 pp on 33 targets
- next_observation vs last_two_log_linear: 8.92 against 19.38 pp on 33 targets
- next_observation vs reference_to_last_log_linear: 8.92 against 21.23 pp on 33 targets
- next_observation vs pooled_curve_ratio: 8.92 against 12.28 pp on 33 targets
- next_observation vs analog_knn: 8.92 against 12.51 pp on 33 targets
- next_observation vs preoperative_level: 8.92 against 62.89 pp on 33 targets
- landmark vs persistence: 9.52 against 18.22 pp on 33 targets
- landmark vs last_two_log_linear: 9.52 against 31.93 pp on 33 targets
- landmark vs reference_to_last_log_linear: 9.52 against 31.93 pp on 33 targets
- landmark vs pooled_curve_ratio: 9.52 against 17.34 pp on 33 targets
- landmark vs analog_knn: 9.52 against 18.53 pp on 33 targets
- landmark vs preoperative_level: 9.52 against 62.89 pp on 33 targets
- next_observation 80% interval: 29/33 = 0.879
- next_observation 90% interval: 30/33 = 0.909
- landmark 80% interval: 28/33 = 0.848
- landmark 90% interval: 31/33 = 0.939

## by procedure context (primary methods; comparator in parentheses)

- human/angle_mims: next_observation 7/17 within, 8.57 (10.37) pp; landmark 7/17 within, 10.34 (15.91) pp; initial_state_group_mean 4/18 within, 9.19 (14.16) pp
- human/cataract: next_observation 3/3 within, 2.01 (1.80) pp; landmark 3/3 within, 2.90 (2.50) pp; initial_state_group_mean 1/2 within, 5.87 (49.54) pp
- human/cyclodestruction: next_observation 3/17 within, 10.07 (11.80) pp; landmark 2/17 within, 11.69 (15.84) pp; initial_state_group_mean 0/5 within, 21.49 (17.27) pp
- human/nonpenetrating: next_observation 2/2 within, 4.18 (2.59) pp; landmark 1/2 within, 6.28 (7.38) pp; initial_state_group_mean 1/2 within, 6.02 (8.90) pp
- human/subconjunctival_mims: next_observation 2/2 within, 4.00 (4.40) pp; landmark 2/2 within, 3.95 (3.15) pp; initial_state_group_mean 1/2 within, 7.07 (25.79) pp
- human/trabeculectomy: next_observation 4/9 within, 5.70 (6.07) pp; landmark 1/9 within, 9.39 (10.74) pp; initial_state_group_mean 2/9 within, 9.66 (14.85) pp
- human/tube: next_observation 2/4 within, 4.98 (5.62) pp; landmark 2/4 within, 6.51 (8.52) pp; initial_state_group_mean 1/2 within, 4.38 (22.43) pp
