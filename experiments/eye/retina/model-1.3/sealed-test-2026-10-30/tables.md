# Retinal-thickness model 1.3.0-k1 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: FAIL
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_landmark_and_initial_state_lower_next_observation_within_0_05_pp: FAIL

- H1 next observation 80% interval: 234/257 = 0.911 (window 0.70–0.90)
- H1 next observation 90% interval: 249/257 = 0.969 (window 0.80–0.97)
- H2 next observation vs persistence: 5.70 against 6.43 pp on 257 common targets
- H2 next observation vs last_two_log_linear: 5.70 against 19.00 pp on 257 common targets
- H2 next observation vs pooled_curve_ratio: 5.70 against 7.89 pp on 257 common targets
- H2 next observation vs analog_knn: 5.70 against 7.87 pp on 251 common targets
- H3 landmark vs persistence: 7.72 against 10.67 pp on 257 common targets
- H3 landmark vs pooled_curve_ratio: 7.72 against 10.41 pp on 257 common targets
- H3 landmark vs analog_knn: 7.57 against 11.88 pp on 251 common targets
- landmark 80% interval (reported): 223/257 = 0.868
- landmark 90% interval (reported): 242/257 = 0.942
- H4 initial state stacked_initial_state: 11.62 pp, 14/90 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 15.44 pp, 13/91 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 286/310 = 0.923
- H4 initial state 90% interval (reported): 300/310 = 0.968

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| stacked | 96 | 55 | 29 | 5.70 | 19.71 | 257 |
| population_assimilation | 96 | 49 | 28 | 6.08 | 21.46 | 257 |
| population_assimilation_robust | 96 | 51 | 28 | 6.12 | 21.46 | 257 |
| population_assimilation_t4 | 96 | 51 | 28 | 6.18 | 21.44 | 257 |
| persistence | 99 | 58 | 0 | 6.40 | 25.58 | 260 |
| weak_prior | 99 | 57 | 8 | 6.43 | 23.70 | 260 |
| population_anchored | 96 | 50 | 27 | 6.56 | 21.58 | 257 |
| population_anchored_t4 | 96 | 50 | 28 | 6.65 | 22.64 | 257 |
| analog_knn | 99 | 39 | 17 | 7.84 | 28.97 | 254 |
| pooled_curve_ratio | 99 | 42 | 24 | 8.06 | 29.77 | 260 |
| last_two_log_linear | 99 | 14 | 8 | 18.97 | 70.11 | 260 |
| reference_to_last_log_linear | 99 | 11 | 5 | 20.36 | 70.11 | 260 |
| baseline_level | 99 | 6 | 3 | 32.11 | 60.55 | 260 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| stacked | 96 | 34 | 24 | 7.72 | 22.28 | 257 |
| population_assimilation_robust | 96 | 32 | 22 | 7.96 | 26.33 | 257 |
| population_assimilation | 96 | 33 | 24 | 7.98 | 27.27 | 257 |
| population_assimilation_t4 | 96 | 32 | 23 | 8.01 | 27.02 | 257 |
| population_anchored_t4 | 96 | 34 | 25 | 8.53 | 22.24 | 257 |
| population_anchored | 96 | 34 | 25 | 8.54 | 22.35 | 257 |
| weak_prior | 99 | 37 | 8 | 10.17 | 30.19 | 260 |
| pooled_curve_ratio | 99 | 35 | 24 | 10.51 | 50.60 | 260 |
| persistence | 99 | 43 | 0 | 10.62 | 46.25 | 260 |
| analog_knn | 99 | 30 | 18 | 11.82 | 49.12 | 254 |
| baseline_level | 99 | 6 | 3 | 32.11 | 60.55 | 260 |
| last_two_log_linear | 99 | 8 | 6 | 41.10 | 182.83 | 260 |
| reference_to_last_log_linear | 99 | 8 | 6 | 41.10 | 182.83 | 260 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| stacked | 122 | 59 | 29 | 9.03 | 58.53 | 315 |
| persistence | 125 | 66 | 0 | 9.25 | 69.19 | 318 |
| population_assimilation | 122 | 53 | 28 | 9.78 | 60.15 | 315 |
| pooled_curve_ratio | 125 | 52 | 29 | 9.91 | 58.16 | 318 |
| analog_knn | 125 | 45 | 19 | 9.91 | 70.32 | 312 |
| population_assimilation_robust | 122 | 55 | 28 | 9.93 | 66.14 | 315 |
| population_assimilation_t4 | 122 | 55 | 28 | 9.95 | 66.56 | 315 |
| weak_prior | 125 | 61 | 8 | 10.18 | 69.22 | 318 |
| population_anchored_t4 | 122 | 54 | 29 | 10.24 | 70.63 | 315 |
| population_anchored | 122 | 54 | 28 | 10.37 | 72.05 | 315 |
| last_two_log_linear | 125 | 14 | 8 | 21.09 | 100.84 | 318 |
| reference_to_last_log_linear | 125 | 11 | 5 | 23.20 | 148.53 | 318 |
| baseline_level | 125 | 6 | 3 | 34.53 | 69.48 | 318 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| stacked | 122 | 38 | 24 | 10.94 | 57.47 | 315 |
| population_assimilation_robust | 122 | 36 | 22 | 11.56 | 64.85 | 315 |
| population_assimilation_t4 | 122 | 36 | 23 | 11.58 | 64.65 | 315 |
| population_assimilation | 122 | 37 | 24 | 11.64 | 68.25 | 315 |
| population_anchored | 122 | 38 | 26 | 11.89 | 51.19 | 315 |
| population_anchored_t4 | 122 | 38 | 26 | 11.90 | 51.55 | 315 |
| pooled_curve_ratio | 125 | 43 | 28 | 12.86 | 54.46 | 318 |
| weak_prior | 125 | 41 | 8 | 13.44 | 66.49 | 318 |
| persistence | 125 | 50 | 0 | 13.58 | 69.89 | 318 |
| analog_knn | 125 | 34 | 20 | 14.54 | 83.72 | 312 |
| baseline_level | 125 | 6 | 3 | 34.53 | 69.48 | 318 |
| last_two_log_linear | 125 | 8 | 6 | 45.97 | 242.43 | 318 |
| reference_to_last_log_linear | 125 | 8 | 6 | 45.97 | 242.43 | 318 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state | 90 | 12 | 11.61 | 37.37 | 310 |
| population_initial_state_t4 | 90 | 12 | 11.62 | 37.40 | 310 |
| stacked_initial_state | 90 | 14 | 11.62 | 39.19 | 310 |
| pooled_training_curve | 91 | 13 | 15.44 | 53.31 | 312 |

## single eyes (reported, not tested)

- next_observation vs persistence: 17.28 against 16.90 pp on 58 targets
- next_observation vs last_two_log_linear: 17.28 against 28.72 pp on 58 targets
- next_observation vs reference_to_last_log_linear: 17.28 against 33.06 pp on 58 targets
- next_observation vs pooled_curve_ratio: 17.28 against 15.72 pp on 58 targets
- next_observation vs analog_knn: 17.28 against 16.08 pp on 58 targets
- next_observation vs baseline_level: 17.28 against 43.73 pp on 58 targets
- landmark vs persistence: 19.66 against 22.50 pp on 58 targets
- landmark vs last_two_log_linear: 19.66 against 63.35 pp on 58 targets
- landmark vs reference_to_last_log_linear: 19.66 against 63.35 pp on 58 targets
- landmark vs pooled_curve_ratio: 19.66 against 20.30 pp on 58 targets
- landmark vs analog_knn: 19.66 against 22.92 pp on 58 targets
- landmark vs baseline_level: 19.66 against 43.73 pp on 58 targets
- next_observation 80% interval: 31/58 = 0.534
- next_observation 90% interval: 43/58 = 0.741
- landmark 80% interval: 24/58 = 0.414
- landmark 90% interval: 31/58 = 0.534

## by context (primary methods; comparator in parentheses)

- human/cme_postoperative/combination: next_observation 0/1 within, 11.02 (2.83) pp; landmark 0/1 within, 10.29 (4.51) pp; initial_state_group_mean 0/1 within, 13.32 (9.31) pp
- human/cme_postoperative/steroid_implant: next_observation 0/11 within, 15.68 (10.30) pp; landmark 0/11 within, 20.67 (13.78) pp
- human/csc/steroid_implant: next_observation 1/1 within, 0.50 (2.34) pp; landmark 1/1 within, 0.50 (2.34) pp
- human/dme/anti_vegf: next_observation 16/22 within, 5.05 (5.34) pp; landmark 10/22 within, 7.10 (11.70) pp; initial_state_group_mean 6/22 within, 10.63 (11.28) pp
- human/dme/combination: next_observation 6/10 within, 5.44 (4.26) pp; landmark 2/10 within, 8.05 (5.31) pp; initial_state_group_mean 0/5 within, 15.43 (18.71) pp
- human/dme/laser: next_observation 3/4 within, 4.31 (5.24) pp; landmark 3/4 within, 3.72 (6.20) pp; initial_state_group_mean 0/4 within, 16.83 (20.48) pp
- human/dme/observation: next_observation 1/2 within, 6.13 (4.41) pp; landmark 1/2 within, 6.55 (5.35) pp; initial_state_group_mean 1/3 within, 9.74 (14.19) pp
- human/dme/other: next_observation 1/1 within, 2.87 (1.73) pp; landmark 1/1 within, 4.57 (2.78) pp
- human/dme/steroid_implant: next_observation 3/17 within, 9.79 (13.32) pp; landmark 2/17 within, 11.33 (18.86) pp; initial_state_group_mean 0/12 within, 11.80 (13.23) pp
- human/dme/steroid_injection: next_observation 3/9 within, 6.19 (6.65) pp; landmark 1/9 within, 9.02 (6.27) pp; initial_state_group_mean 1/9 within, 16.02 (20.85) pp
- human/namd/anti_vegf: next_observation 4/8 within, 4.40 (5.65) pp; landmark 1/8 within, 7.11 (8.34) pp; initial_state_group_mean 2/9 within, 10.42 (13.40) pp
- human/namd/combination: next_observation 5/11 within, 12.69 (13.19) pp; landmark 6/11 within, 12.53 (13.04) pp; initial_state_group_mean 1/3 within, 5.75 (18.28) pp
- human/namd/pdt: next_observation 1/1 within, 1.30 (1.92) pp; landmark 1/1 within, 0.90 (2.12) pp; initial_state_group_mean 0/1 within, 7.25 (16.69) pp
- human/other/anti_vegf: next_observation 2/2 within, 2.51 (3.62) pp; landmark 1/2 within, 6.00 (7.93) pp; initial_state_group_mean 0/2 within, 6.36 (22.15) pp
- human/other/combination: next_observation 1/2 within, 31.67 (30.44) pp; landmark 0/2 within, 31.55 (35.70) pp; initial_state_group_mean 0/1 within, 10.69 (4.56) pp
- human/other/observation: next_observation 1/1 within, 4.88 (3.72) pp; landmark 0/1 within, 6.34 (2.28) pp; initial_state_group_mean 0/1 within, 10.55 (32.40) pp
- human/other/steroid_implant: next_observation 1/1 within, 2.68 (3.18) pp; landmark 1/1 within, 2.75 (3.25) pp
- human/other/vitrectomy: next_observation 1/2 within, 5.57 (5.42) pp; landmark 1/2 within, 6.44 (13.04) pp; initial_state_group_mean 0/1 within, 8.68 (2.03) pp
- human/rvo/anti_vegf: next_observation 7/9 within, 4.54 (5.30) pp; landmark 6/9 within, 7.17 (8.32) pp; initial_state_group_mean 3/11 within, 10.04 (17.01) pp
- human/rvo/combination: next_observation 2/4 within, 5.09 (5.17) pp; landmark 0/4 within, 8.44 (10.99) pp; initial_state_group_mean 0/2 within, 10.13 (12.66) pp
- human/rvo/steroid_implant: next_observation 0/3 within, 9.76 (12.35) pp; landmark 0/3 within, 13.33 (25.21) pp; initial_state_group_mean 0/3 within, 9.19 (16.99) pp

## H5: model 1.3 against the frozen model 1.1.0, same fit

- next_observation: 1.3 5.70 against 1.1 6.08 pp on 257 common targets
- landmark: 1.3 7.72 against 1.1 7.98 pp on 257 common targets
- initial_state: 1.3 11.62 against 1.1 11.61 pp on 310 common targets

## Model 1.1.0 on the same studies, its own H1-H4 (reported, not tested)

- H1_calibration_next_observation_group_means: FAIL
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H1 next observation 80% interval: 232/257 = 0.903
- H1 next observation 90% interval: 248/257 = 0.965
- H2 next observation vs persistence: 6.08 against 6.43 pp on 257 common targets
- H2 next observation vs last_two_log_linear: 6.08 against 19.00 pp on 257 common targets
- H2 next observation vs pooled_curve_ratio: 6.08 against 7.89 pp on 257 common targets
- H2 next observation vs analog_knn: 6.07 against 7.87 pp on 251 common targets
- H3 landmark vs persistence: 7.98 against 10.67 pp on 257 common targets
- H3 landmark vs pooled_curve_ratio: 7.98 against 10.41 pp on 257 common targets
- H3 landmark vs analog_knn: 7.88 against 11.88 pp on 251 common targets
- H4 initial state population_initial_state: 11.61 pp
- H4 initial state pooled_training_curve: 15.44 pp

Units: pp of each series' baseline central retinal thickness.
