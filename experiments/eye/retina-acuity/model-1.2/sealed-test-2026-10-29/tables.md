# Retinal visual-acuity model 1.2.0-x3 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: FAIL
- H2_accuracy_next_observation_group_means: FAIL
- H3_landmark_group_means: FAIL
- H4_initial_state_control_group_means: FAIL
- H5_landmark_and_initial_state_lower_next_observation_within_0_05_pp: FAIL

- H1 next observation 80% interval: 42/71 = 0.592 (window 0.70–0.90)
- H1 next observation 90% interval: 59/71 = 0.831 (window 0.80–0.97)
- H2 next observation vs persistence: 4.51 against 4.06 pp on 71 common targets
- H2 next observation vs last_two_log_linear: 4.51 against 11.17 pp on 71 common targets
- H2 next observation vs pooled_curve_ratio: 4.51 against 3.82 pp on 71 common targets
- H2 next observation vs analog_knn: 4.53 against 4.63 pp on 70 common targets
- H3 landmark vs persistence: 5.85 against 5.91 pp on 71 common targets
- H3 landmark vs pooled_curve_ratio: 5.85 against 4.52 pp on 71 common targets
- H3 landmark vs analog_knn: 5.87 against 6.34 pp on 70 common targets
- landmark 80% interval (reported): 40/71 = 0.563
- landmark 90% interval (reported): 53/71 = 0.746
- H4 initial state population_initial_state: 8.82 pp, 13/27 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 8.70 pp, 17/27 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 42/86 = 0.488
- H4 initial state 90% interval (reported): 50/86 = 0.581

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| pooled_curve_ratio | 34 | 30 | 12 | 3.82 | 15.23 | 71 |
| persistence | 34 | 29 | 0 | 4.06 | 16.00 | 71 |
| weak_prior | 34 | 29 | 0 | 4.31 | 16.19 | 71 |
| population_anchored_t4 | 34 | 26 | 2 | 4.47 | 16.51 | 71 |
| population_anchored | 34 | 26 | 2 | 4.51 | 16.47 | 71 |
| analog_knn | 34 | 27 | 11 | 4.63 | 15.50 | 70 |
| population_assimilation_robust | 34 | 25 | 5 | 4.96 | 17.88 | 71 |
| population_assimilation_t4 | 34 | 23 | 5 | 4.99 | 17.85 | 71 |
| population_assimilation | 34 | 23 | 6 | 6.05 | 18.00 | 71 |
| last_two_log_linear | 34 | 10 | 2 | 11.17 | 45.00 | 71 |
| reference_to_last_log_linear | 34 | 9 | 2 | 13.02 | 45.00 | 71 |
| baseline_level | 34 | 13 | 3 | 15.11 | 35.00 | 71 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| pooled_curve_ratio | 34 | 26 | 12 | 4.52 | 15.23 | 71 |
| population_anchored_t4 | 34 | 21 | 3 | 5.84 | 17.41 | 71 |
| population_anchored | 34 | 21 | 3 | 5.85 | 17.41 | 71 |
| persistence | 34 | 21 | 0 | 5.91 | 17.50 | 71 |
| weak_prior | 34 | 22 | 0 | 6.02 | 17.61 | 71 |
| population_assimilation_robust | 34 | 21 | 6 | 6.33 | 17.88 | 71 |
| analog_knn | 34 | 21 | 11 | 6.34 | 15.50 | 70 |
| population_assimilation_t4 | 34 | 20 | 5 | 6.53 | 17.85 | 71 |
| population_assimilation | 34 | 20 | 6 | 7.35 | 18.00 | 71 |
| baseline_level | 34 | 13 | 3 | 15.11 | 35.00 | 71 |
| last_two_log_linear | 34 | 7 | 3 | 70.02 | 265.00 | 71 |
| reference_to_last_log_linear | 34 | 7 | 3 | 70.02 | 265.00 | 71 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| weak_prior | 112 | 74 | 6 | 5.95 | 26.74 | 204 |
| persistence | 112 | 70 | 0 | 5.96 | 27.00 | 204 |
| population_anchored_t4 | 112 | 75 | 12 | 6.02 | 27.15 | 204 |
| population_anchored | 112 | 76 | 13 | 6.07 | 27.15 | 204 |
| pooled_curve_ratio | 112 | 67 | 26 | 6.15 | 32.03 | 204 |
| population_assimilation_robust | 112 | 75 | 19 | 6.21 | 26.75 | 204 |
| population_assimilation_t4 | 112 | 73 | 19 | 6.21 | 26.68 | 204 |
| population_assimilation | 112 | 72 | 21 | 6.57 | 26.72 | 204 |
| analog_knn | 112 | 63 | 21 | 6.63 | 29.72 | 203 |
| last_two_log_linear | 112 | 24 | 7 | 14.31 | 54.50 | 204 |
| baseline_level | 112 | 33 | 13 | 14.55 | 45.00 | 204 |
| reference_to_last_log_linear | 112 | 26 | 7 | 14.96 | 47.00 | 204 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| pooled_curve_ratio | 112 | 61 | 29 | 6.97 | 32.03 | 204 |
| population_anchored | 112 | 66 | 6 | 7.23 | 34.69 | 204 |
| population_anchored_t4 | 112 | 66 | 6 | 7.23 | 34.71 | 204 |
| population_assimilation_robust | 112 | 63 | 16 | 7.28 | 32.42 | 204 |
| persistence | 112 | 63 | 0 | 7.31 | 35.00 | 204 |
| population_assimilation_t4 | 112 | 62 | 15 | 7.34 | 32.20 | 204 |
| weak_prior | 112 | 63 | 0 | 7.35 | 35.00 | 204 |
| population_assimilation | 112 | 63 | 17 | 7.63 | 32.27 | 204 |
| analog_knn | 112 | 56 | 21 | 7.77 | 35.00 | 203 |
| baseline_level | 112 | 33 | 10 | 14.55 | 45.00 | 204 |
| last_two_log_linear | 112 | 23 | 8 | 47.00 | 265.00 | 204 |
| reference_to_last_log_linear | 112 | 23 | 8 | 47.00 | 265.00 | 204 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| pooled_training_curve | 27 | 17 | 8.70 | 23.16 | 86 |
| population_initial_state_t4 | 27 | 13 | 8.81 | 23.33 | 86 |
| population_initial_state | 27 | 13 | 8.82 | 23.35 | 86 |

## single eyes (reported, not tested)

- next_observation vs persistence: 6.75 against 6.76 pp on 133 targets
- next_observation vs last_two_log_linear: 6.75 against 15.74 pp on 133 targets
- next_observation vs reference_to_last_log_linear: 6.75 against 15.90 pp on 133 targets
- next_observation vs pooled_curve_ratio: 6.75 against 7.08 pp on 133 targets
- next_observation vs analog_knn: 6.75 against 7.47 pp on 133 targets
- next_observation vs baseline_level: 6.75 against 14.24 pp on 133 targets
- landmark vs persistence: 7.87 against 7.96 pp on 133 targets
- landmark vs last_two_log_linear: 7.87 against 27.78 pp on 133 targets
- landmark vs reference_to_last_log_linear: 7.87 against 27.78 pp on 133 targets
- landmark vs pooled_curve_ratio: 7.87 against 7.98 pp on 133 targets
- landmark vs analog_knn: 7.87 against 8.43 pp on 133 targets
- landmark vs baseline_level: 7.87 against 14.24 pp on 133 targets
- next_observation 80% interval: 119/133 = 0.895
- next_observation 90% interval: 130/133 = 0.977
- landmark 80% interval: 115/133 = 0.865
- landmark 90% interval: 128/133 = 0.962

## by procedure context (primary methods; comparator in parentheses)

- human/dme/anti_vegf: next_observation 13/24 within, 6.74 (6.81) pp; landmark 12/24 within, 6.84 (7.04) pp; initial_state_group_mean 3/7 within, 6.45 (6.94) pp
- human/dme/combination: next_observation 2/3 within, 5.40 (5.18) pp; landmark 1/3 within, 8.54 (8.62) pp; initial_state_group_mean 0/2 within, 10.95 (10.22) pp
- human/dme/steroid_implant: next_observation 10/17 within, 9.27 (9.23) pp; landmark 10/17 within, 9.27 (9.23) pp
- human/dme/vitrectomy: next_observation 1/1 within, 4.12 (3.48) pp; landmark 0/1 within, 5.55 (5.59) pp; initial_state_group_mean 1/1 within, 2.69 (2.15) pp
- human/myopic_cnv/anti_vegf: next_observation 0/1 within, 16.47 (16.00) pp; landmark 0/1 within, 16.47 (16.00) pp; initial_state_group_mean 0/1 within, 14.72 (14.98) pp
- human/namd/anti_vegf: next_observation 17/22 within, 4.86 (5.13) pp; landmark 13/22 within, 5.66 (5.86) pp; initial_state_group_mean 5/6 within, 3.54 (2.84) pp
- human/namd/combination: next_observation 15/21 within, 5.07 (4.95) pp; landmark 15/21 within, 6.19 (6.19) pp
- human/other/anti_vegf: next_observation 4/5 within, 7.93 (7.31) pp; landmark 2/5 within, 10.43 (10.51) pp
- human/other/combination: next_observation 3/5 within, 8.99 (8.22) pp; landmark 3/5 within, 13.15 (13.23) pp
- human/rvo/anti_vegf: next_observation 7/8 within, 3.45 (2.89) pp; landmark 6/8 within, 4.79 (4.80) pp; initial_state_group_mean 3/7 within, 10.34 (10.00) pp
- human/rvo/combination: next_observation 2/2 within, 1.44 (1.35) pp; landmark 2/2 within, 1.69 (1.68) pp
- human/rvo/steroid_implant: next_observation 1/2 within, 4.71 (3.33) pp; landmark 1/2 within, 6.67 (6.67) pp; initial_state_group_mean 0/2 within, 15.85 (16.45) pp
- human/rvo/steroid_injection: next_observation 1/1 within, 2.62 (2.92) pp; landmark 1/1 within, 2.26 (2.24) pp; initial_state_group_mean 1/1 within, 4.24 (3.67) pp

## H5: model 1.2 against the frozen model 1.1.0, same training

- next_observation: 1.2 4.51 against 1.1 4.42 pp on 71 common targets
- landmark: 1.2 5.85 against 1.1 5.90 pp on 71 common targets
- initial_state: 1.2 8.82 against 1.1 8.88 pp on 86 common targets

## Model 1.1.0 on the same studies, its own H1-H4 (reported, not tested)

- H1_calibration_next_observation_group_means: FAIL
- H2_accuracy_next_observation_group_means: FAIL
- H3_landmark_group_means: FAIL
- H4_initial_state_control_group_means: FAIL
- H1 next observation 80% interval: 48/71 = 0.676
- H1 next observation 90% interval: 65/71 = 0.915
- H2 next observation vs persistence: 4.42 against 4.06 pp on 71 common targets
- H2 next observation vs last_two_log_linear: 4.42 against 11.17 pp on 71 common targets
- H2 next observation vs pooled_curve_ratio: 4.42 against 3.82 pp on 71 common targets
- H2 next observation vs analog_knn: 4.43 against 4.63 pp on 70 common targets
- H3 landmark vs persistence: 5.90 against 5.91 pp on 71 common targets
- H3 landmark vs pooled_curve_ratio: 5.90 against 4.52 pp on 71 common targets
- H3 landmark vs analog_knn: 5.92 against 6.34 pp on 70 common targets
- H4 initial state population_initial_state: 8.88 pp
- H4 initial state pooled_training_curve: 8.70 pp

Units: 1 pp = 1 ETDRS letter (0.02 logMAR) of best-corrected acuity change from the pretreatment acuity.
