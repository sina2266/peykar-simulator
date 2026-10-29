# Retinal visual-acuity model 1.1.0-w1 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: FAIL
- H3_landmark_group_means: FAIL
- H4_initial_state_control_group_means: FAIL
- H5_landmark_and_initial_state_lower_next_observation_within_0_05_pp: FAIL

- H1 next observation 80% interval: 77/106 = 0.726 (window 0.70–0.90)
- H1 next observation 90% interval: 93/106 = 0.877 (window 0.80–0.97)
- H2 next observation vs persistence: 3.95 against 3.00 pp on 106 common targets
- H2 next observation vs last_two_log_linear: 3.95 against 14.61 pp on 106 common targets
- H2 next observation vs pooled_curve_ratio: 3.95 against 2.80 pp on 106 common targets
- H2 next observation vs analog_knn: 3.95 against 5.42 pp on 106 common targets
- H3 landmark vs persistence: 6.24 against 3.56 pp on 106 common targets
- H3 landmark vs pooled_curve_ratio: 6.24 against 3.06 pp on 106 common targets
- H3 landmark vs analog_knn: 6.24 against 10.12 pp on 106 common targets
- landmark 80% interval (reported): 81/106 = 0.764
- landmark 90% interval (reported): 99/106 = 0.934
- H4 initial state population_initial_state: 6.65 pp, 27/45 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 5.87 pp, 31/45 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 81/134 = 0.604
- H4 initial state 90% interval (reported): 99/134 = 0.739

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| pooled_curve_ratio | 50 | 48 | 18 | 2.80 | 8.46 | 106 |
| persistence | 50 | 47 | 0 | 3.00 | 8.00 | 106 |
| weak_prior | 50 | 46 | 0 | 3.13 | 8.00 | 106 |
| population_assimilation_robust | 50 | 42 | 6 | 3.60 | 10.16 | 106 |
| population_assimilation_t4 | 50 | 41 | 7 | 3.69 | 10.67 | 106 |
| population_anchored_t4 | 50 | 43 | 0 | 3.95 | 18.22 | 106 |
| population_assimilation | 50 | 40 | 8 | 4.02 | 11.55 | 106 |
| population_anchored | 50 | 43 | 0 | 4.03 | 18.87 | 106 |
| analog_knn | 50 | 40 | 10 | 5.42 | 25.41 | 106 |
| baseline_level | 50 | 16 | 6 | 11.40 | 30.00 | 106 |
| last_two_log_linear | 50 | 17 | 6 | 14.61 | 99.00 | 106 |
| reference_to_last_log_linear | 50 | 18 | 5 | 14.87 | 99.00 | 106 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| pooled_curve_ratio | 50 | 49 | 22 | 3.06 | 8.46 | 106 |
| persistence | 50 | 42 | 0 | 3.56 | 8.00 | 106 |
| weak_prior | 50 | 42 | 0 | 3.65 | 8.04 | 106 |
| population_assimilation_robust | 50 | 35 | 5 | 4.72 | 14.48 | 106 |
| population_assimilation_t4 | 50 | 36 | 6 | 4.83 | 15.60 | 106 |
| population_assimilation | 50 | 36 | 6 | 5.27 | 17.72 | 106 |
| population_anchored_t4 | 50 | 39 | 4 | 6.24 | 22.15 | 106 |
| population_anchored | 50 | 39 | 4 | 6.41 | 22.86 | 106 |
| analog_knn | 50 | 34 | 13 | 10.12 | 48.39 | 106 |
| baseline_level | 50 | 16 | 7 | 11.40 | 30.00 | 106 |
| last_two_log_linear | 50 | 6 | 5 | 34.37 | 123.46 | 106 |
| reference_to_last_log_linear | 50 | 6 | 5 | 34.37 | 123.46 | 106 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| persistence | 81 | 65 | 0 | 5.07 | 30.00 | 264 |
| pooled_curve_ratio | 81 | 67 | 21 | 5.17 | 30.82 | 264 |
| weak_prior | 81 | 64 | 1 | 5.63 | 33.11 | 264 |
| population_anchored_t4 | 81 | 61 | 1 | 5.89 | 33.74 | 264 |
| population_anchored | 81 | 61 | 1 | 5.93 | 33.81 | 264 |
| population_assimilation_robust | 81 | 59 | 7 | 6.01 | 33.19 | 264 |
| population_assimilation_t4 | 81 | 59 | 8 | 6.07 | 33.16 | 264 |
| population_assimilation | 81 | 57 | 9 | 6.60 | 33.02 | 264 |
| analog_knn | 81 | 52 | 13 | 7.94 | 35.31 | 264 |
| baseline_level | 81 | 23 | 7 | 16.93 | 76.50 | 264 |
| last_two_log_linear | 81 | 24 | 6 | 21.71 | 235.47 | 264 |
| reference_to_last_log_linear | 81 | 27 | 6 | 26.25 | 235.47 | 264 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| weak_prior | 81 | 56 | 2 | 8.49 | 33.94 | 264 |
| persistence | 81 | 58 | 0 | 8.49 | 35.00 | 264 |
| pooled_curve_ratio | 81 | 64 | 27 | 8.55 | 37.48 | 264 |
| population_anchored_t4 | 81 | 52 | 4 | 9.16 | 34.69 | 264 |
| population_anchored | 81 | 52 | 4 | 9.21 | 34.68 | 264 |
| population_assimilation_robust | 81 | 49 | 8 | 9.24 | 30.60 | 264 |
| population_assimilation_t4 | 81 | 49 | 8 | 9.50 | 30.88 | 264 |
| population_assimilation | 81 | 49 | 8 | 10.10 | 35.87 | 264 |
| analog_knn | 81 | 44 | 19 | 16.67 | 53.51 | 264 |
| baseline_level | 81 | 23 | 7 | 16.93 | 76.50 | 264 |
| last_two_log_linear | 81 | 11 | 5 | 191.31 | 1800.33 | 264 |
| reference_to_last_log_linear | 81 | 11 | 5 | 191.31 | 1800.33 | 264 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| pooled_training_curve | 45 | 31 | 5.87 | 18.07 | 134 |
| population_initial_state | 45 | 27 | 6.65 | 24.96 | 134 |
| population_initial_state_t4 | 45 | 27 | 6.65 | 24.88 | 134 |

## single eyes (reported, not tested)

- next_observation vs persistence: 6.90 against 6.08 pp on 158 targets
- next_observation vs last_two_log_linear: 6.90 against 25.39 pp on 158 targets
- next_observation vs reference_to_last_log_linear: 6.90 against 31.67 pp on 158 targets
- next_observation vs pooled_curve_ratio: 6.90 against 6.28 pp on 158 targets
- next_observation vs analog_knn: 6.90 against 9.26 pp on 158 targets
- next_observation vs baseline_level: 6.90 against 19.80 pp on 158 targets
- landmark vs persistence: 10.68 against 10.58 pp on 158 targets
- landmark vs last_two_log_linear: 10.68 against 245.69 pp on 158 targets
- landmark vs reference_to_last_log_linear: 10.68 against 245.69 pp on 158 targets
- landmark vs pooled_curve_ratio: 10.68 against 10.76 pp on 158 targets
- landmark vs analog_knn: 10.68 against 19.88 pp on 158 targets
- landmark vs baseline_level: 10.68 against 19.80 pp on 158 targets
- next_observation 80% interval: 149/158 = 0.943
- next_observation 90% interval: 152/158 = 0.962
- landmark 80% interval: 138/158 = 0.873
- landmark 90% interval: 143/158 = 0.905

## by procedure context (primary methods; comparator in parentheses)

- human/dme/anti_vegf: next_observation 5/6 within, 2.79 (2.59) pp; landmark 5/6 within, 2.66 (2.68) pp; initial_state_group_mean 4/5 within, 4.51 (3.67) pp
- human/dme/combination: next_observation 2/2 within, 3.44 (3.11) pp; landmark 2/2 within, 4.17 (4.04) pp; initial_state_group_mean 2/2 within, 3.61 (4.26) pp
- human/dme/steroid_implant: next_observation 3/6 within, 6.83 (3.37) pp; landmark 3/6 within, 13.42 (4.24) pp; initial_state_group_mean 3/6 within, 8.44 (3.02) pp
- human/dme/vitrectomy: next_observation 3/3 within, 3.30 (3.09) pp; landmark 2/3 within, 4.05 (3.98) pp; initial_state_group_mean 1/3 within, 10.56 (11.56) pp
- human/myopic_cnv/anti_vegf: next_observation 17/21 within, 6.06 (5.11) pp; landmark 12/21 within, 9.96 (9.97) pp; initial_state_group_mean 2/2 within, 2.98 (3.81) pp
- human/namd/anti_vegf: next_observation 17/20 within, 3.27 (3.10) pp; landmark 15/20 within, 3.42 (3.44) pp; initial_state_group_mean 11/18 within, 5.00 (4.20) pp
- human/namd/combination: next_observation 3/3 within, 2.01 (2.19) pp; landmark 3/3 within, 2.11 (2.15) pp; initial_state_group_mean 2/2 within, 2.35 (1.80) pp
- human/rvo/anti_vegf: next_observation 11/20 within, 7.43 (6.96) pp; landmark 10/20 within, 10.19 (9.84) pp; initial_state_group_mean 2/7 within, 8.86 (9.78) pp

## H5: model 1.1 against the frozen model 1.0.0, same training

- next_observation: 1.1 3.95 against 1.0 3.61 pp on 106 common targets
- landmark: 1.1 6.24 against 1.0 5.01 pp on 106 common targets
- initial_state: 1.1 6.65 against 1.0 6.52 pp on 134 common targets

## Model 1.0.0 on the same studies, its own H1-H4 (reported, not tested)

- H1_calibration_next_observation_group_means: FAIL
- H2_accuracy_next_observation_group_means: FAIL
- H3_landmark_group_means: FAIL
- H4_initial_state_control_group_means: FAIL
- H1 next observation 80% interval: 62/106 = 0.585
- H1 next observation 90% interval: 78/106 = 0.736
- H2 next observation vs persistence: 3.61 against 3.00 pp on 106 common targets
- H2 next observation vs last_two_log_linear: 3.61 against 14.61 pp on 106 common targets
- H2 next observation vs pooled_curve_ratio: 3.61 against 2.80 pp on 106 common targets
- H2 next observation vs analog_knn: 3.61 against 5.42 pp on 106 common targets
- H3 landmark vs persistence: 5.01 against 3.56 pp on 106 common targets
- H3 landmark vs pooled_curve_ratio: 5.01 against 3.06 pp on 106 common targets
- H3 landmark vs analog_knn: 5.01 against 10.12 pp on 106 common targets
- H4 initial state population_initial_state: 6.52 pp
- H4 initial state pooled_training_curve: 5.87 pp

Units: 1 pp = 1 ETDRS letter (0.02 logMAR) of best-corrected acuity change from the pretreatment acuity.
