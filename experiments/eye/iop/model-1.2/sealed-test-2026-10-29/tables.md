# IOP-after-surgery model 1.2.0-c1 — sealed results

## Declared checks (group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_first_month_coverage_closer_without_rmse_loss: pass

- H1 next observation 80% interval: 435/521 = 0.835 (window 0.70–0.90)
- H1 next observation 90% interval: 485/521 = 0.931 (window 0.80–0.97)
- H2 next observation vs persistence: 5.89 against 7.21 pp on 521 common targets
- H2 next observation vs last_two_log_linear: 5.89 against 33.28 pp on 521 common targets
- H2 next observation vs pooled_curve_ratio: 5.89 against 8.67 pp on 521 common targets
- H2 next observation vs analog_knn: 5.89 against 9.53 pp on 521 common targets
- H3 landmark vs persistence: 7.76 against 11.89 pp on 521 common targets
- H3 landmark vs pooled_curve_ratio: 7.76 against 15.80 pp on 521 common targets
- H3 landmark vs analog_knn: 7.76 against 15.71 pp on 521 common targets
- landmark 80% interval (reported): 414/521 = 0.795
- landmark 90% interval (reported): 474/521 = 0.910
- H4 initial state population_initial_state: 10.17 pp, 29/118 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 18.06 pp, 12/118 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 499/589 = 0.847
- H4 initial state 90% interval (reported): 556/589 = 0.944

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 130 | 80 | 38 | 5.89 | 38.04 | 521 |
| population_assimilation_t4 | 130 | 80 | 36 | 5.96 | 38.04 | 521 |
| population_assimilation_robust | 130 | 79 | 35 | 6.00 | 38.07 | 521 |
| persistence | 132 | 57 | 0 | 7.54 | 36.60 | 531 |
| population_anchored | 130 | 61 | 18 | 8.18 | 49.44 | 521 |
| population_anchored_t4 | 130 | 61 | 17 | 8.21 | 49.48 | 521 |
| weak_prior | 132 | 45 | 9 | 8.30 | 42.10 | 531 |
| pooled_curve_ratio | 130 | 52 | 22 | 8.67 | 79.16 | 521 |
| analog_knn | 130 | 45 | 20 | 9.53 | 56.16 | 521 |
| reference_to_last_log_linear | 132 | 4 | 3 | 34.05 | 209.09 | 531 |
| last_two_log_linear | 132 | 2 | 2 | 34.96 | 231.15 | 531 |
| baseline_level | 132 | 2 | 2 | 38.70 | 78.24 | 531 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 130 | 54 | 33 | 7.76 | 38.04 | 521 |
| population_assimilation_t4 | 130 | 55 | 34 | 7.78 | 38.04 | 521 |
| population_assimilation_robust | 130 | 55 | 32 | 7.80 | 38.07 | 521 |
| persistence | 132 | 43 | 0 | 12.39 | 41.88 | 531 |
| population_anchored | 130 | 41 | 12 | 14.07 | 63.25 | 521 |
| population_anchored_t4 | 130 | 41 | 10 | 14.08 | 63.22 | 521 |
| weak_prior | 132 | 29 | 3 | 14.81 | 42.91 | 531 |
| analog_knn | 130 | 27 | 17 | 15.71 | 56.16 | 521 |
| pooled_curve_ratio | 130 | 33 | 19 | 15.80 | 65.38 | 521 |
| baseline_level | 132 | 2 | 0 | 38.70 | 78.24 | 531 |
| last_two_log_linear | 132 | 4 | 2 | 69.80 | 228.75 | 531 |
| reference_to_last_log_linear | 132 | 4 | 2 | 69.80 | 228.75 | 531 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 183 | 87 | 41 | 7.66 | 39.54 | 731 |
| population_assimilation_t4 | 183 | 86 | 37 | 7.68 | 38.04 | 731 |
| population_assimilation_robust | 183 | 85 | 36 | 7.82 | 41.53 | 731 |
| persistence | 185 | 71 | 0 | 9.47 | 44.44 | 741 |
| population_anchored_t4 | 183 | 73 | 20 | 9.71 | 49.48 | 731 |
| weak_prior | 185 | 58 | 17 | 9.78 | 46.36 | 741 |
| population_anchored | 183 | 73 | 21 | 9.98 | 59.86 | 731 |
| pooled_curve_ratio | 183 | 55 | 23 | 10.64 | 79.16 | 731 |
| analog_knn | 183 | 53 | 24 | 10.95 | 56.16 | 731 |
| reference_to_last_log_linear | 185 | 4 | 3 | 33.42 | 209.09 | 741 |
| baseline_level | 185 | 2 | 2 | 42.18 | 88.89 | 741 |
| last_two_log_linear | 185 | 2 | 2 | 44.04 | 282.22 | 741 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 183 | 59 | 35 | 9.23 | 41.07 | 731 |
| population_assimilation_t4 | 183 | 59 | 35 | 9.46 | 52.46 | 731 |
| population_assimilation_robust | 183 | 59 | 33 | 9.61 | 61.41 | 731 |
| persistence | 185 | 55 | 0 | 13.93 | 90.00 | 741 |
| population_anchored | 183 | 54 | 15 | 15.23 | 89.52 | 731 |
| population_anchored_t4 | 183 | 54 | 13 | 15.23 | 89.50 | 731 |
| analog_knn | 183 | 35 | 22 | 16.10 | 78.51 | 731 |
| weak_prior | 185 | 41 | 10 | 16.21 | 79.60 | 741 |
| pooled_curve_ratio | 183 | 38 | 23 | 17.05 | 92.11 | 731 |
| baseline_level | 185 | 2 | 0 | 42.18 | 88.89 | 741 |
| last_two_log_linear | 185 | 4 | 2 | 65.72 | 228.75 | 741 |
| reference_to_last_log_linear | 185 | 4 | 2 | 65.72 | 228.75 | 741 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 118 | 29 | 10.17 | 55.12 | 589 |
| population_initial_state | 118 | 29 | 10.17 | 55.12 | 589 |
| pooled_training_curve | 118 | 12 | 18.06 | 62.80 | 589 |

## single eyes (reported, not tested)

- next_observation vs persistence: 10.89 against 13.14 pp on 210 targets
- next_observation vs last_two_log_linear: 10.89 against 61.27 pp on 210 targets
- next_observation vs reference_to_last_log_linear: 10.89 against 31.79 pp on 210 targets
- next_observation vs pooled_curve_ratio: 10.89 against 14.42 pp on 210 targets
- next_observation vs analog_knn: 10.89 against 13.87 pp on 210 targets
- next_observation vs preoperative_level: n/a against n/a pp on 0 targets
- landmark vs persistence: 12.15 against 17.24 pp on 210 targets
- landmark vs last_two_log_linear: 12.15 against 54.06 pp on 210 targets
- landmark vs reference_to_last_log_linear: 12.15 against 54.06 pp on 210 targets
- landmark vs pooled_curve_ratio: 12.15 against 19.81 pp on 210 targets
- landmark vs analog_knn: 12.15 against 17.02 pp on 210 targets
- landmark vs preoperative_level: n/a against n/a pp on 0 targets
- next_observation 80% interval: 179/210 = 0.852
- next_observation 90% interval: 193/210 = 0.919
- landmark 80% interval: 177/210 = 0.843
- landmark 90% interval: 195/210 = 0.929

## by procedure context (primary methods; comparator in parentheses)

- human/angle_mims: next_observation 39/79 within, 7.27 (8.67) pp; landmark 33/79 within, 8.23 (12.54) pp; initial_state_group_mean 18/58 within, 8.84 (16.44) pp
- human/cataract: next_observation 6/8 within, 6.70 (9.40) pp; landmark 3/8 within, 10.83 (12.40) pp; initial_state_group_mean 1/8 within, 14.58 (14.73) pp
- human/cyclodestruction: next_observation 9/14 within, 6.54 (8.27) pp; landmark 4/14 within, 9.26 (12.98) pp; initial_state_group_mean 1/14 within, 11.72 (15.20) pp
- human/nonpenetrating: next_observation 6/11 within, 5.34 (5.66) pp; landmark 3/11 within, 9.70 (11.19) pp; initial_state_group_mean 1/9 within, 10.57 (22.06) pp
- human/other: next_observation 8/18 within, 5.80 (6.32) pp; landmark 4/18 within, 7.91 (8.92) pp; initial_state_group_mean 0/3 within, 9.32 (38.99) pp
- human/subconjunctival_mims: next_observation 1/10 within, 9.35 (11.68) pp; landmark 1/10 within, 11.33 (16.96) pp; initial_state_group_mean 1/3 within, 10.80 (18.42) pp
- human/trabeculectomy: next_observation 11/17 within, 4.69 (6.25) pp; landmark 8/17 within, 7.00 (11.43) pp; initial_state_group_mean 4/14 within, 12.44 (14.60) pp
- human/tube: next_observation 7/26 within, 11.89 (14.63) pp; landmark 3/26 within, 13.00 (20.39) pp; initial_state_group_mean 3/9 within, 8.30 (17.86) pp

## H5: model 1.2 against the frozen model 1.1.0, same training

- next_observation: 1.2 5.89 against 1.1 5.88 pp on 521 common targets
- landmark: 1.2 7.76 against 1.1 7.76 pp on 521 common targets
- initial_state: 1.2 10.17 against 1.1 10.18 pp on 589 common targets
- first-month next-observation 80% coverage (131 targets): 1.2 0.82 against 1.1 0.68
- first-month next-observation 90% coverage (131 targets): 1.2 0.94 against 1.1 0.82
