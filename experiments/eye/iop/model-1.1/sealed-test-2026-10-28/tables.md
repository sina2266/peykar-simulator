# IOP-after-surgery model 1.1.0 — sealed results

## Declared checks (group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_model_1_1_better_than_1_0_on_group_means: pass

- H1 next observation 80% interval: 341/425 = 0.802 (window 0.70–0.90)
- H1 next observation 90% interval: 381/425 = 0.896 (window 0.80–0.97)
- H2 next observation vs persistence: 6.69 against 8.06 pp on 425 common targets
- H2 next observation vs last_two_log_linear: 6.69 against 44.56 pp on 425 common targets
- H2 next observation vs pooled_curve_ratio: 6.69 against 8.60 pp on 425 common targets
- H2 next observation vs analog_knn: 6.69 against 9.90 pp on 425 common targets
- H3 landmark vs persistence: 8.95 against 15.10 pp on 425 common targets
- H3 landmark vs pooled_curve_ratio: 8.95 against 15.44 pp on 425 common targets
- H3 landmark vs analog_knn: 8.95 against 17.60 pp on 425 common targets
- landmark 80% interval (reported): 285/425 = 0.671
- landmark 90% interval (reported): 345/425 = 0.812
- H4 initial state population_initial_state: 9.71 pp, 27/99 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 17.88 pp, 15/102 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 355/467 = 0.760
- H4 initial state 90% interval (reported): 399/467 = 0.854

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 113 | 60 | 26 | 6.66 | 40.43 | 425 |
| population_assimilation | 113 | 58 | 25 | 6.69 | 40.30 | 425 |
| population_assimilation_robust | 113 | 56 | 23 | 6.79 | 40.66 | 425 |
| persistence | 116 | 54 | 0 | 7.98 | 47.13 | 443 |
| pooled_curve_ratio | 116 | 43 | 18 | 8.50 | 49.20 | 443 |
| weak_prior | 116 | 49 | 18 | 8.85 | 59.68 | 443 |
| population_anchored_t4 | 113 | 53 | 16 | 9.58 | 53.31 | 425 |
| population_anchored | 113 | 53 | 16 | 9.66 | 53.50 | 425 |
| analog_knn | 116 | 45 | 21 | 10.23 | 64.50 | 443 |
| reference_to_last_log_linear | 116 | 1 | 1 | 33.87 | 202.13 | 443 |
| preoperative_level | 116 | 2 | 2 | 41.02 | 79.66 | 443 |
| last_two_log_linear | 116 | 1 | 1 | 43.82 | 260.35 | 443 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 113 | 42 | 21 | 8.95 | 40.30 | 425 |
| population_assimilation_t4 | 113 | 42 | 20 | 9.28 | 40.43 | 425 |
| population_assimilation_robust | 113 | 38 | 16 | 9.66 | 40.66 | 425 |
| persistence | 116 | 36 | 0 | 15.01 | 47.13 | 443 |
| pooled_curve_ratio | 116 | 28 | 19 | 15.25 | 49.20 | 443 |
| weak_prior | 116 | 36 | 15 | 17.58 | 47.01 | 443 |
| analog_knn | 116 | 25 | 10 | 18.25 | 64.50 | 443 |
| population_anchored_t4 | 113 | 37 | 11 | 19.48 | 86.28 | 425 |
| population_anchored | 113 | 38 | 11 | 19.51 | 86.43 | 425 |
| preoperative_level | 116 | 2 | 2 | 41.02 | 79.66 | 443 |
| last_two_log_linear | 116 | 1 | 1 | 62.04 | 224.53 | 443 |
| reference_to_last_log_linear | 116 | 1 | 1 | 62.04 | 224.53 | 443 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 154 | 66 | 28 | 10.91 | 83.32 | 591 |
| population_assimilation | 154 | 64 | 27 | 10.98 | 84.80 | 591 |
| population_assimilation_robust | 154 | 62 | 25 | 11.03 | 82.95 | 591 |
| pooled_curve_ratio | 157 | 46 | 20 | 14.59 | 121.52 | 609 |
| population_anchored_t4 | 154 | 60 | 16 | 15.26 | 123.17 | 591 |
| analog_knn | 157 | 51 | 23 | 15.40 | 143.58 | 609 |
| population_anchored | 154 | 60 | 16 | 15.63 | 123.17 | 591 |
| weak_prior | 175 | 55 | 18 | 19.09 | 168.83 | 661 |
| persistence | 175 | 62 | 0 | 20.50 | 166.67 | 661 |
| preoperative_level | 175 | 2 | 2 | 42.25 | 183.33 | 661 |
| reference_to_last_log_linear | 175 | 1 | 1 | 43.54 | 246.67 | 661 |
| last_two_log_linear | 175 | 1 | 1 | 57.11 | 263.89 | 661 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 154 | 48 | 23 | 12.95 | 92.68 | 591 |
| population_assimilation_t4 | 154 | 48 | 22 | 13.23 | 92.65 | 591 |
| population_assimilation_robust | 154 | 44 | 18 | 13.55 | 92.80 | 591 |
| pooled_curve_ratio | 157 | 31 | 21 | 26.03 | 121.52 | 609 |
| analog_knn | 157 | 31 | 12 | 26.64 | 143.58 | 609 |
| weak_prior | 175 | 42 | 15 | 26.97 | 213.53 | 661 |
| population_anchored_t4 | 154 | 44 | 11 | 30.45 | 149.82 | 591 |
| population_anchored | 154 | 45 | 11 | 30.50 | 149.85 | 591 |
| persistence | 175 | 43 | 0 | 31.84 | 220.00 | 661 |
| preoperative_level | 175 | 2 | 2 | 42.25 | 183.33 | 661 |
| last_two_log_linear | 175 | 1 | 1 | 77.30 | 253.60 | 661 |
| reference_to_last_log_linear | 175 | 1 | 1 | 77.30 | 253.60 | 661 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 99 | 27 | 9.71 | 54.67 | 467 |
| population_initial_state | 99 | 27 | 9.71 | 54.67 | 467 |
| pooled_training_curve | 102 | 15 | 17.88 | 58.93 | 488 |

## single eyes (reported, not tested)

- next_observation vs persistence: 17.73 against 23.82 pp on 166 targets
- next_observation vs last_two_log_linear: 17.73 against 78.97 pp on 166 targets
- next_observation vs reference_to_last_log_linear: 17.73 against 55.75 pp on 166 targets
- next_observation vs pooled_curve_ratio: 17.73 against 24.25 pp on 166 targets
- next_observation vs analog_knn: 17.73 against 24.30 pp on 166 targets
- next_observation vs preoperative_level: 17.73 against 37.64 pp on 166 targets
- landmark vs persistence: 19.79 against 46.00 pp on 166 targets
- landmark vs last_two_log_linear: 19.79 against 106.86 pp on 166 targets
- landmark vs reference_to_last_log_linear: 19.79 against 106.86 pp on 166 targets
- landmark vs pooled_curve_ratio: 19.79 against 43.18 pp on 166 targets
- landmark vs analog_knn: 19.79 against 41.41 pp on 166 targets
- landmark vs preoperative_level: 19.79 against 37.64 pp on 166 targets
- next_observation 80% interval: 138/166 = 0.831
- next_observation 90% interval: 152/166 = 0.916
- landmark 80% interval: 132/166 = 0.795
- landmark 90% interval: 151/166 = 0.910

## by procedure context (primary methods; comparator in parentheses)

- human/angle_mims: next_observation 29/58 within, 12.40 (17.97) pp; landmark 22/58 within, 14.18 (34.72) pp; initial_state_group_mean 18/47 within, 7.84 (17.36) pp
- human/cataract: next_observation 1/5 within, 8.52 (16.02) pp; landmark 2/5 within, 8.97 (22.53) pp; initial_state_group_mean 0/3 within, 24.29 (26.22) pp
- human/cyclodestruction: next_observation 6/14 within, 7.67 (9.44) pp; landmark 5/14 within, 10.79 (16.76) pp; initial_state_group_mean 3/15 within, 10.35 (16.60) pp
- human/nonpenetrating: next_observation 4/7 within, 4.22 (7.78) pp; landmark 5/7 within, 4.20 (19.73) pp; initial_state_group_mean 3/6 within, 12.71 (24.26) pp
- human/other: next_observation 1/1 within, 3.42 (3.54) pp; landmark 0/1 within, 6.06 (7.59) pp; initial_state_group_mean 1/1 within, 4.16 (37.33) pp
- human/subconjunctival_mims: next_observation 4/31 within, 13.15 (12.82) pp; landmark 2/31 within, 15.79 (24.11) pp; initial_state_group_mean 1/6 within, 8.54 (20.30) pp
- human/trabeculectomy: next_observation 8/17 within, 5.37 (6.80) pp; landmark 3/17 within, 7.35 (14.87) pp; initial_state_group_mean 0/13 within, 9.83 (10.71) pp
- human/tube: next_observation 11/21 within, 7.07 (6.85) pp; landmark 9/21 within, 8.01 (10.39) pp; initial_state_group_mean 1/8 within, 12.47 (19.48) pp

## H5: model 1.1 against model 1.0.0 trained on the same records

- next_observation: 1.1 6.69 against 1.0 6.92 pp on 425 common targets
- landmark: 1.1 8.95 against 1.0 9.40 pp on 425 common targets
