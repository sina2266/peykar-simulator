# IOP-after-surgery model 1.1.0 — development results

## Declared checks (group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_model_1_1_better_than_1_0_on_group_means: pass

- H1 next observation 80% interval: 393/521 = 0.754 (window 0.70–0.90)
- H1 next observation 90% interval: 451/521 = 0.866 (window 0.80–0.97)
- H2 next observation vs persistence: 6.77 against 8.05 pp on 521 common targets
- H2 next observation vs last_two_log_linear: 6.77 against 38.81 pp on 521 common targets
- H2 next observation vs pooled_curve_ratio: 6.77 against 10.19 pp on 521 common targets
- H2 next observation vs analog_knn: 6.78 against 9.65 pp on 517 common targets
- H3 landmark vs persistence: 8.56 against 13.98 pp on 521 common targets
- H3 landmark vs pooled_curve_ratio: 8.56 against 16.78 pp on 521 common targets
- H3 landmark vs analog_knn: 8.52 against 14.78 pp on 517 common targets
- landmark 80% interval (reported): 354/521 = 0.679
- landmark 90% interval (reported): 432/521 = 0.829
- H4 initial state population_initial_state: 11.46 pp, 31/117 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 19.30 pp, 8/117 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 451/595 = 0.758
- H4 initial state 90% interval (reported): 502/595 = 0.844

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 123 | 68 | 35 | 6.77 | 37.99 | 521 |
| population_assimilation_t4 | 123 | 66 | 34 | 6.79 | 38.05 | 521 |
| population_assimilation_robust | 123 | 66 | 33 | 6.83 | 37.89 | 521 |
| persistence | 123 | 50 | 0 | 8.05 | 41.57 | 521 |
| population_anchored | 123 | 55 | 20 | 8.21 | 45.62 | 521 |
| population_anchored_t4 | 123 | 55 | 20 | 8.38 | 48.78 | 521 |
| weak_prior | 123 | 45 | 13 | 8.99 | 58.68 | 521 |
| analog_knn | 123 | 34 | 14 | 9.65 | 62.35 | 517 |
| pooled_curve_ratio | 123 | 35 | 14 | 10.19 | 95.34 | 521 |
| reference_to_last_log_linear | 123 | 0 | 0 | 34.95 | 201.05 | 521 |
| last_two_log_linear | 123 | 0 | 0 | 38.81 | 226.54 | 521 |
| preoperative_level | 123 | 0 | 0 | 41.75 | 75.91 | 521 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 123 | 46 | 29 | 8.40 | 38.05 | 521 |
| population_assimilation_robust | 123 | 48 | 31 | 8.45 | 37.89 | 521 |
| population_assimilation | 123 | 46 | 29 | 8.56 | 37.99 | 521 |
| population_anchored_t4 | 123 | 43 | 16 | 13.38 | 39.19 | 521 |
| population_anchored | 123 | 43 | 16 | 13.39 | 39.20 | 521 |
| persistence | 123 | 37 | 0 | 13.98 | 46.99 | 521 |
| analog_knn | 123 | 17 | 11 | 14.78 | 62.35 | 517 |
| pooled_curve_ratio | 123 | 26 | 16 | 16.78 | 55.23 | 521 |
| weak_prior | 123 | 36 | 16 | 17.50 | 51.04 | 521 |
| preoperative_level | 123 | 0 | 0 | 41.75 | 75.91 | 521 |
| last_two_log_linear | 123 | 0 | 0 | 68.75 | 232.72 | 521 |
| reference_to_last_log_linear | 123 | 0 | 0 | 68.75 | 232.72 | 521 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 151 | 69 | 34 | 8.75 | 49.80 | 608 |
| population_assimilation_t4 | 151 | 69 | 35 | 8.79 | 49.86 | 608 |
| population_assimilation | 151 | 72 | 37 | 9.40 | 60.74 | 608 |
| persistence | 151 | 55 | 0 | 9.83 | 57.14 | 608 |
| population_anchored | 151 | 59 | 21 | 10.34 | 74.91 | 608 |
| population_anchored_t4 | 151 | 59 | 21 | 10.53 | 80.73 | 608 |
| weak_prior | 151 | 49 | 14 | 10.97 | 83.49 | 608 |
| analog_knn | 151 | 38 | 15 | 11.19 | 62.35 | 604 |
| pooled_curve_ratio | 151 | 39 | 14 | 11.51 | 95.34 | 608 |
| reference_to_last_log_linear | 151 | 1 | 1 | 35.11 | 201.05 | 608 |
| last_two_log_linear | 151 | 1 | 1 | 40.44 | 252.38 | 608 |
| preoperative_level | 151 | 0 | 0 | 43.28 | 77.78 | 608 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 151 | 51 | 32 | 12.03 | 84.07 | 608 |
| population_assimilation_t4 | 151 | 49 | 30 | 12.07 | 85.21 | 608 |
| population_assimilation | 151 | 49 | 30 | 12.17 | 86.25 | 608 |
| population_anchored | 151 | 47 | 16 | 14.73 | 56.71 | 608 |
| population_anchored_t4 | 151 | 47 | 16 | 14.73 | 56.74 | 608 |
| persistence | 151 | 41 | 0 | 15.12 | 57.14 | 608 |
| analog_knn | 151 | 21 | 12 | 16.53 | 71.32 | 604 |
| weak_prior | 151 | 40 | 17 | 18.15 | 55.24 | 608 |
| pooled_curve_ratio | 151 | 30 | 17 | 18.36 | 69.02 | 608 |
| preoperative_level | 151 | 0 | 0 | 43.28 | 77.78 | 608 |
| last_two_log_linear | 151 | 1 | 1 | 66.92 | 232.72 | 608 |
| reference_to_last_log_linear | 151 | 1 | 1 | 66.92 | 232.72 | 608 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 117 | 31 | 11.46 | 54.37 | 595 |
| population_initial_state | 117 | 31 | 11.46 | 54.47 | 595 |
| pooled_training_curve | 117 | 8 | 19.30 | 80.18 | 595 |

## single eyes (reported, not tested)

- next_observation vs persistence: 18.51 against 16.95 pp on 87 targets
- next_observation vs last_two_log_linear: 18.51 against 49.10 pp on 87 targets
- next_observation vs reference_to_last_log_linear: 18.51 against 36.05 pp on 87 targets
- next_observation vs pooled_curve_ratio: 18.51 against 17.45 pp on 87 targets
- next_observation vs analog_knn: 18.51 against 17.77 pp on 87 targets
- next_observation vs preoperative_level: 18.51 against 51.45 pp on 87 targets
- landmark vs persistence: 24.43 against 20.67 pp on 87 targets
- landmark vs last_two_log_linear: 24.43 against 54.67 pp on 87 targets
- landmark vs reference_to_last_log_linear: 24.43 against 54.67 pp on 87 targets
- landmark vs pooled_curve_ratio: 24.43 against 25.89 pp on 87 targets
- landmark vs analog_knn: 24.43 against 24.48 pp on 87 targets
- landmark vs preoperative_level: 24.43 against 51.45 pp on 87 targets
- next_observation 80% interval: 56/87 = 0.644
- next_observation 90% interval: 62/87 = 0.713
- landmark 80% interval: 53/87 = 0.609
- landmark 90% interval: 64/87 = 0.736

## by procedure context (primary methods; comparator in parentheses)

- human/angle_mims: next_observation 32/63 within, 10.46 (9.33) pp; landmark 22/63 within, 14.21 (16.28) pp; initial_state_group_mean 22/57 within, 9.23 (16.54) pp
- human/cataract: next_observation 5/8 within, 6.46 (8.11) pp; landmark 6/8 within, 5.78 (5.56) pp; initial_state_group_mean 0/7 within, 9.77 (40.16) pp
- human/cyclodestruction: next_observation 5/21 within, 9.59 (10.95) pp; landmark 2/21 within, 11.38 (14.72) pp; initial_state_group_mean 0/9 within, 20.00 (17.41) pp
- human/laser_trabeculoplasty: next_observation 0/2 within, 11.61 (8.28) pp; landmark 0/2 within, 22.69 (17.45) pp; initial_state_group_mean 0/2 within, 25.80 (22.07) pp
- human/nonpenetrating: next_observation 4/7 within, 6.75 (7.41) pp; landmark 3/7 within, 8.45 (13.90) pp; initial_state_group_mean 1/6 within, 9.80 (23.99) pp
- human/other: next_observation 1/6 within, 21.14 (25.96) pp; landmark 1/6 within, 21.14 (25.96) pp
- human/subconjunctival_mims: next_observation 6/14 within, 6.18 (10.91) pp; landmark 4/14 within, 7.31 (20.03) pp; initial_state_group_mean 2/13 within, 9.37 (21.31) pp
- human/trabeculectomy: next_observation 13/22 within, 8.97 (10.43) pp; landmark 7/22 within, 10.86 (12.71) pp; initial_state_group_mean 3/15 within, 13.40 (16.61) pp
- human/tube: next_observation 6/8 within, 4.41 (5.38) pp; landmark 4/8 within, 5.03 (8.23) pp; initial_state_group_mean 3/8 within, 6.33 (20.49) pp

## H5: model 1.1 against model 1.0.0 trained on the same records

- next_observation: 1.1 6.77 against 1.0 6.85 pp on 521 common targets
- landmark: 1.1 8.56 against 1.0 8.76 pp on 521 common targets
