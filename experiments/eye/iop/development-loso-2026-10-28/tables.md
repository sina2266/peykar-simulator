# IOP-after-surgery model 1.0.0 — development results

## Declared checks (group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass

- H1 next observation 80% interval: 263/323 = 0.814 (window 0.70–0.90)
- H1 next observation 90% interval: 289/323 = 0.895 (window 0.80–0.97)
- H2 next observation vs persistence: 6.34 against 7.67 pp on 323 common targets
- H2 next observation vs last_two_log_linear: 6.34 against 38.07 pp on 323 common targets
- H2 next observation vs pooled_curve_ratio: 6.34 against 10.51 pp on 323 common targets
- H2 next observation vs analog_knn: 6.34 against 8.64 pp on 319 common targets
- H3 landmark vs persistence: 8.66 against 14.61 pp on 323 common targets
- H3 landmark vs pooled_curve_ratio: 8.66 against 17.98 pp on 323 common targets
- H3 landmark vs analog_knn: 8.60 against 14.16 pp on 319 common targets
- landmark 80% interval (reported): 246/323 = 0.762
- landmark 90% interval (reported): 293/323 = 0.907
- H4 initial state population_initial_state: 12.04 pp, 20/77 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 21.44 pp, 5/77 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 297/373 = 0.796
- H4 initial state 90% interval (reported): 320/373 = 0.858

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 80 | 42 | 19 | 6.34 | 28.86 | 323 |
| population_assimilation_t4 | 80 | 42 | 20 | 6.35 | 28.99 | 323 |
| population_assimilation | 80 | 41 | 20 | 6.48 | 32.18 | 323 |
| population_anchored | 80 | 35 | 12 | 7.38 | 33.82 | 323 |
| population_anchored_t4 | 80 | 34 | 9 | 7.39 | 33.89 | 323 |
| persistence | 80 | 32 | 0 | 7.67 | 34.99 | 323 |
| weak_prior | 80 | 33 | 9 | 7.87 | 34.93 | 323 |
| analog_knn | 80 | 31 | 11 | 8.64 | 46.47 | 319 |
| pooled_curve_ratio | 80 | 26 | 11 | 10.51 | 71.31 | 323 |
| reference_to_last_log_linear | 80 | 0 | 0 | 36.62 | 201.05 | 323 |
| last_two_log_linear | 80 | 0 | 0 | 38.07 | 201.05 | 323 |
| preoperative_level | 80 | 0 | 0 | 40.63 | 71.29 | 323 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 80 | 36 | 24 | 8.66 | 33.45 | 323 |
| population_assimilation_t4 | 80 | 35 | 25 | 8.67 | 33.83 | 323 |
| population_assimilation | 80 | 35 | 25 | 8.80 | 33.95 | 323 |
| population_anchored_t4 | 80 | 28 | 16 | 12.49 | 39.91 | 323 |
| population_anchored | 80 | 26 | 14 | 12.51 | 39.78 | 323 |
| weak_prior | 80 | 27 | 14 | 13.76 | 47.06 | 323 |
| analog_knn | 80 | 13 | 10 | 14.16 | 49.59 | 319 |
| persistence | 80 | 23 | 0 | 14.61 | 46.99 | 323 |
| pooled_curve_ratio | 80 | 20 | 13 | 17.98 | 55.82 | 323 |
| preoperative_level | 80 | 0 | 0 | 40.63 | 71.29 | 323 |
| last_two_log_linear | 80 | 0 | 0 | 67.36 | 217.92 | 323 |
| reference_to_last_log_linear | 80 | 0 | 0 | 67.36 | 217.92 | 323 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 97 | 45 | 19 | 9.45 | 51.08 | 377 |
| population_assimilation_t4 | 97 | 45 | 20 | 9.51 | 52.71 | 377 |
| persistence | 97 | 35 | 0 | 10.15 | 57.14 | 377 |
| population_assimilation | 97 | 44 | 20 | 10.26 | 57.05 | 377 |
| population_anchored | 97 | 38 | 12 | 10.27 | 68.52 | 377 |
| population_anchored_t4 | 97 | 37 | 9 | 10.35 | 71.99 | 377 |
| weak_prior | 97 | 36 | 9 | 10.37 | 62.53 | 377 |
| analog_knn | 97 | 34 | 11 | 11.05 | 58.47 | 373 |
| pooled_curve_ratio | 97 | 29 | 11 | 12.43 | 71.31 | 377 |
| reference_to_last_log_linear | 97 | 1 | 1 | 37.54 | 201.05 | 377 |
| preoperative_level | 97 | 0 | 0 | 40.97 | 77.78 | 377 |
| last_two_log_linear | 97 | 1 | 1 | 42.02 | 252.38 | 377 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 97 | 39 | 24 | 13.39 | 77.41 | 377 |
| population_assimilation_t4 | 97 | 38 | 25 | 13.52 | 79.02 | 377 |
| population_assimilation | 97 | 38 | 25 | 13.66 | 80.00 | 377 |
| population_anchored_t4 | 97 | 32 | 17 | 14.25 | 54.66 | 377 |
| population_anchored | 97 | 30 | 15 | 14.31 | 55.26 | 377 |
| weak_prior | 97 | 31 | 15 | 14.94 | 54.15 | 377 |
| persistence | 97 | 26 | 0 | 15.89 | 57.14 | 377 |
| analog_knn | 97 | 16 | 10 | 16.43 | 70.43 | 373 |
| pooled_curve_ratio | 97 | 23 | 13 | 19.56 | 66.91 | 377 |
| preoperative_level | 97 | 0 | 0 | 40.97 | 77.78 | 377 |
| last_two_log_linear | 97 | 1 | 1 | 66.99 | 217.92 | 377 |
| reference_to_last_log_linear | 97 | 1 | 1 | 66.99 | 217.92 | 377 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 77 | 20 | 12.03 | 53.09 | 373 |
| population_initial_state | 77 | 20 | 12.04 | 53.29 | 373 |
| pooled_training_curve | 77 | 5 | 21.44 | 68.33 | 373 |

## single eyes (reported, not tested)

- next_observation vs persistence: 19.58 against 19.17 pp on 54 targets
- next_observation vs last_two_log_linear: 19.58 against 60.46 pp on 54 targets
- next_observation vs reference_to_last_log_linear: 19.58 against 42.65 pp on 54 targets
- next_observation vs pooled_curve_ratio: 19.58 against 20.41 pp on 54 targets
- next_observation vs analog_knn: 19.58 against 20.03 pp on 54 targets
- next_observation vs preoperative_level: 19.58 against 43.00 pp on 54 targets
- landmark vs persistence: 28.34 against 22.03 pp on 54 targets
- landmark vs last_two_log_linear: 28.34 against 64.75 pp on 54 targets
- landmark vs reference_to_last_log_linear: 28.34 against 64.75 pp on 54 targets
- landmark vs pooled_curve_ratio: 28.34 against 27.13 pp on 54 targets
- landmark vs analog_knn: 28.34 against 26.07 pp on 54 targets
- landmark vs preoperative_level: 28.34 against 43.00 pp on 54 targets
- next_observation 80% interval: 31/54 = 0.574
- next_observation 90% interval: 37/54 = 0.685
- landmark 80% interval: 30/54 = 0.556
- landmark 90% interval: 35/54 = 0.648

## by procedure context (primary methods; comparator in parentheses)

- human/angle_mims: next_observation 24/46 within, 9.52 (8.75) pp; landmark 20/46 within, 15.72 (16.46) pp; initial_state_group_mean 15/39 within, 9.15 (18.51) pp
- human/cataract: next_observation 3/5 within, 8.52 (10.70) pp; landmark 4/5 within, 7.19 (7.08) pp; initial_state_group_mean 0/5 within, 12.60 (33.20) pp
- human/cyclodestruction: next_observation 2/4 within, 4.83 (6.39) pp; landmark 2/4 within, 7.53 (8.82) pp; initial_state_group_mean 0/4 within, 19.81 (19.09) pp
- human/laser_trabeculoplasty: next_observation 0/2 within, 10.93 (8.28) pp; landmark 0/2 within, 22.20 (17.45) pp; initial_state_group_mean 0/2 within, 26.87 (21.00) pp
- human/nonpenetrating: next_observation 2/5 within, 7.40 (8.26) pp; landmark 2/5 within, 9.13 (15.21) pp; initial_state_group_mean 1/4 within, 12.21 (30.59) pp
- human/other: next_observation 2/6 within, 21.74 (25.96) pp; landmark 2/6 within, 21.74 (25.96) pp
- human/subconjunctival_mims: next_observation 4/12 within, 7.04 (11.37) pp; landmark 5/12 within, 7.99 (21.01) pp; initial_state_group_mean 1/11 within, 8.82 (18.27) pp
- human/trabeculectomy: next_observation 6/13 within, 10.73 (12.18) pp; landmark 3/13 within, 11.59 (13.67) pp; initial_state_group_mean 2/6 within, 17.15 (21.15) pp
- human/tube: next_observation 2/4 within, 4.89 (5.05) pp; landmark 1/4 within, 7.06 (7.84) pp; initial_state_group_mean 1/6 within, 7.17 (29.52) pp
