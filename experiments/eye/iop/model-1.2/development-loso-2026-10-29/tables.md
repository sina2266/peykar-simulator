# IOP-after-surgery model 1.2.0-c1 — development results

## Declared checks (group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_first_month_coverage_closer_without_rmse_loss: pass

- H1 next observation 80% interval: 747/946 = 0.790 (window 0.70–0.90)
- H1 next observation 90% interval: 842/946 = 0.890 (window 0.80–0.97)
- H2 next observation vs persistence: 6.85 against 8.06 pp on 946 common targets
- H2 next observation vs last_two_log_linear: 6.85 against 41.49 pp on 946 common targets
- H2 next observation vs pooled_curve_ratio: 6.85 against 9.43 pp on 946 common targets
- H2 next observation vs analog_knn: 6.86 against 9.44 pp on 942 common targets
- H3 landmark vs persistence: 8.81 against 14.49 pp on 946 common targets
- H3 landmark vs pooled_curve_ratio: 8.81 against 16.01 pp on 946 common targets
- H3 landmark vs analog_knn: 8.80 against 15.63 pp on 942 common targets
- landmark 80% interval (reported): 721/946 = 0.762
- landmark 90% interval (reported): 827/946 = 0.874
- H4 initial state population_initial_state: 10.58 pp, 58/216 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 18.16 pp, 24/219 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 895/1062 = 0.843
- H4 initial state 90% interval (reported): 969/1062 = 0.912

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 236 | 125 | 58 | 6.83 | 40.64 | 946 |
| population_assimilation | 236 | 124 | 58 | 6.85 | 40.61 | 946 |
| population_assimilation_robust | 236 | 122 | 56 | 6.91 | 40.85 | 946 |
| persistence | 239 | 104 | 0 | 8.02 | 47.13 | 964 |
| weak_prior | 239 | 93 | 30 | 8.99 | 60.96 | 964 |
| population_anchored | 236 | 107 | 36 | 8.99 | 54.65 | 946 |
| population_anchored_t4 | 236 | 104 | 35 | 9.02 | 55.42 | 946 |
| pooled_curve_ratio | 239 | 88 | 37 | 9.38 | 118.65 | 964 |
| analog_knn | 239 | 81 | 36 | 9.61 | 61.61 | 960 |
| reference_to_last_log_linear | 239 | 1 | 1 | 34.46 | 202.13 | 964 |
| last_two_log_linear | 239 | 1 | 1 | 41.19 | 260.35 | 964 |
| baseline_level | 239 | 2 | 2 | 41.42 | 79.66 | 964 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 236 | 93 | 48 | 8.81 | 40.61 | 946 |
| population_assimilation_t4 | 236 | 95 | 52 | 8.81 | 40.64 | 946 |
| population_assimilation_robust | 236 | 92 | 47 | 8.92 | 40.85 | 946 |
| persistence | 239 | 73 | 0 | 14.46 | 47.13 | 964 |
| pooled_curve_ratio | 239 | 63 | 37 | 15.93 | 51.26 | 964 |
| analog_knn | 239 | 43 | 27 | 16.04 | 61.61 | 960 |
| population_anchored | 236 | 83 | 25 | 16.43 | 85.02 | 946 |
| population_anchored_t4 | 236 | 82 | 24 | 16.43 | 85.01 | 946 |
| weak_prior | 239 | 71 | 26 | 17.54 | 51.04 | 964 |
| baseline_level | 239 | 2 | 2 | 41.42 | 79.66 | 964 |
| last_two_log_linear | 239 | 1 | 1 | 65.75 | 232.72 | 964 |
| reference_to_last_log_linear | 239 | 1 | 1 | 65.75 | 232.72 | 964 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 305 | 132 | 61 | 10.30 | 75.65 | 1199 |
| population_assimilation_t4 | 305 | 135 | 63 | 10.42 | 79.78 | 1199 |
| population_assimilation | 305 | 134 | 63 | 10.94 | 88.22 | 1199 |
| analog_knn | 308 | 88 | 38 | 12.67 | 102.44 | 1213 |
| population_anchored | 305 | 119 | 36 | 12.77 | 123.54 | 1199 |
| population_anchored_t4 | 305 | 116 | 35 | 12.90 | 123.57 | 1199 |
| pooled_curve_ratio | 308 | 98 | 40 | 13.04 | 123.72 | 1217 |
| weak_prior | 326 | 103 | 31 | 15.68 | 168.79 | 1269 |
| persistence | 326 | 117 | 0 | 16.29 | 166.67 | 1269 |
| reference_to_last_log_linear | 326 | 2 | 2 | 39.72 | 246.67 | 1269 |
| baseline_level | 326 | 2 | 2 | 42.74 | 183.33 | 1269 |
| last_two_log_linear | 326 | 2 | 2 | 49.83 | 263.89 | 1269 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 305 | 106 | 58 | 12.73 | 93.88 | 1199 |
| population_assimilation | 305 | 104 | 54 | 12.74 | 94.69 | 1199 |
| population_assimilation_robust | 305 | 103 | 53 | 12.76 | 92.09 | 1199 |
| analog_knn | 308 | 50 | 29 | 19.81 | 103.04 | 1213 |
| pooled_curve_ratio | 308 | 72 | 39 | 21.97 | 123.72 | 1217 |
| weak_prior | 326 | 81 | 27 | 23.18 | 213.59 | 1269 |
| population_anchored_t4 | 305 | 92 | 24 | 23.70 | 149.72 | 1199 |
| population_anchored | 305 | 93 | 25 | 23.71 | 149.70 | 1199 |
| persistence | 326 | 84 | 0 | 25.25 | 220.00 | 1269 |
| baseline_level | 326 | 2 | 2 | 42.74 | 183.33 | 1269 |
| last_two_log_linear | 326 | 2 | 2 | 72.51 | 253.60 | 1269 |
| reference_to_last_log_linear | 326 | 2 | 2 | 72.51 | 253.60 | 1269 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 216 | 58 | 10.58 | 52.30 | 1062 |
| population_initial_state | 216 | 58 | 10.58 | 52.27 | 1062 |
| pooled_training_curve | 219 | 24 | 18.16 | 70.29 | 1083 |

## single eyes (reported, not tested)

- next_observation vs persistence: 19.80 against 21.71 pp on 253 targets
- next_observation vs last_two_log_linear: 19.80 against 70.15 pp on 253 targets
- next_observation vs reference_to_last_log_linear: 19.80 against 49.87 pp on 253 targets
- next_observation vs pooled_curve_ratio: 19.80 against 21.98 pp on 253 targets
- next_observation vs analog_knn: 19.80 against 20.46 pp on 253 targets
- next_observation vs preoperative_level: n/a against n/a pp on 0 targets
- landmark vs persistence: 21.90 against 39.18 pp on 253 targets
- landmark vs last_two_log_linear: 21.90 against 92.30 pp on 253 targets
- landmark vs reference_to_last_log_linear: 21.90 against 92.30 pp on 253 targets
- landmark vs pooled_curve_ratio: 21.90 against 36.82 pp on 253 targets
- landmark vs analog_knn: 21.90 against 30.09 pp on 253 targets
- landmark vs preoperative_level: n/a against n/a pp on 0 targets
- next_observation 80% interval: 225/253 = 0.889
- next_observation 90% interval: 241/253 = 0.953
- landmark 80% interval: 214/253 = 0.846
- landmark 90% interval: 236/253 = 0.933

## by procedure context (primary methods; comparator in parentheses)

- human/angle_mims: next_observation 60/121 within, 13.06 (14.35) pp; landmark 45/121 within, 14.40 (27.18) pp; initial_state_group_mean 38/104 within, 8.58 (17.13) pp
- human/cataract: next_observation 6/13 within, 7.94 (11.13) pp; landmark 7/13 within, 7.90 (13.27) pp; initial_state_group_mean 1/10 within, 13.00 (34.92) pp
- human/cyclodestruction: next_observation 11/35 within, 8.39 (10.40) pp; landmark 11/35 within, 10.37 (15.52) pp; initial_state_group_mean 3/24 within, 13.96 (15.83) pp
- human/laser_trabeculoplasty: next_observation 0/2 within, 12.15 (8.28) pp; landmark 0/2 within, 22.76 (17.45) pp; initial_state_group_mean 0/2 within, 25.87 (22.77) pp
- human/nonpenetrating: next_observation 8/14 within, 6.10 (7.56) pp; landmark 8/14 within, 7.66 (16.50) pp; initial_state_group_mean 6/12 within, 11.36 (21.37) pp
- human/other: next_observation 2/7 within, 14.56 (20.23) pp; landmark 1/7 within, 14.82 (20.68) pp; initial_state_group_mean 1/1 within, 4.46 (37.33) pp
- human/subconjunctival_mims: next_observation 11/45 within, 11.75 (12.41) pp; landmark 8/45 within, 14.66 (23.24) pp; initial_state_group_mean 3/19 within, 8.96 (18.16) pp
- human/trabeculectomy: next_observation 20/39 within, 7.82 (9.31) pp; landmark 10/39 within, 10.26 (13.51) pp; initial_state_group_mean 3/28 within, 12.32 (15.49) pp
- human/tube: next_observation 16/29 within, 6.50 (6.23) pp; landmark 14/29 within, 7.10 (9.49) pp; initial_state_group_mean 3/16 within, 10.82 (18.76) pp

## H5: model 1.2 against the frozen model 1.1.0, same training

- next_observation: 1.2 6.85 against 1.1 6.84 pp on 946 common targets
- landmark: 1.2 8.81 against 1.1 8.80 pp on 946 common targets
- initial_state: 1.2 10.58 against 1.1 10.59 pp on 1062 common targets
- first-month next-observation 80% coverage (219 targets): 1.2 0.83 against 1.1 0.64
- first-month next-observation 90% coverage (219 targets): 1.2 0.90 against 1.1 0.82
