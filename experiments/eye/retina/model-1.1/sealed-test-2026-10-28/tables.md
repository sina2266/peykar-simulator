# Retinal-thickness model 1.1.0-s5 — sealed results

## Declared checks (group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_primary_better_than_model_1_0_0: FAIL

- H1 next observation 80% interval: 211/250 = 0.844 (window 0.70–0.90)
- H1 next observation 90% interval: 224/250 = 0.896 (window 0.80–0.97)
- H2 next observation vs persistence: 7.91 against 8.81 pp on 250 common targets
- H2 next observation vs last_two_log_linear: 7.91 against 26.40 pp on 250 common targets
- H2 next observation vs pooled_curve_ratio: 7.91 against 10.47 pp on 250 common targets
- H2 next observation vs analog_knn: 7.94 against 9.84 pp on 248 common targets
- H3 landmark vs persistence: 9.90 against 11.55 pp on 250 common targets
- H3 landmark vs pooled_curve_ratio: 9.90 against 13.41 pp on 250 common targets
- H3 landmark vs analog_knn: 9.92 against 14.48 pp on 248 common targets
- landmark 80% interval (reported): 186/250 = 0.744
- landmark 90% interval (reported): 201/250 = 0.804
- H4 initial state population_initial_state: 13.00 pp, 11/74 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 18.51 pp, 8/74 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 218/275 = 0.793
- H4 initial state 90% interval (reported): 254/275 = 0.924

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 92 | 46 | 25 | 7.89 | 40.62 | 250 |
| population_assimilation | 92 | 47 | 26 | 7.91 | 40.55 | 250 |
| population_anchored | 92 | 47 | 29 | 8.05 | 41.69 | 250 |
| population_assimilation_t4 | 92 | 47 | 26 | 8.09 | 40.56 | 250 |
| population_anchored_t4 | 92 | 45 | 26 | 8.51 | 49.48 | 250 |
| persistence | 92 | 36 | 0 | 8.81 | 43.03 | 250 |
| weak_prior | 92 | 35 | 3 | 9.59 | 51.19 | 250 |
| analog_knn | 92 | 33 | 24 | 9.84 | 33.99 | 248 |
| pooled_curve_ratio | 92 | 33 | 18 | 10.47 | 40.67 | 250 |
| reference_to_last_log_linear | 92 | 6 | 6 | 21.42 | 64.85 | 250 |
| last_two_log_linear | 92 | 8 | 5 | 26.40 | 186.87 | 250 |
| baseline_level | 92 | 2 | 1 | 35.51 | 62.92 | 250 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 92 | 35 | 25 | 9.84 | 42.73 | 250 |
| population_assimilation_t4 | 92 | 38 | 28 | 9.87 | 42.48 | 250 |
| population_assimilation | 92 | 38 | 28 | 9.90 | 42.27 | 250 |
| population_anchored | 92 | 33 | 23 | 10.20 | 46.90 | 250 |
| population_anchored_t4 | 92 | 34 | 23 | 10.22 | 46.73 | 250 |
| persistence | 92 | 24 | 0 | 11.55 | 43.03 | 250 |
| weak_prior | 92 | 21 | 3 | 12.37 | 43.44 | 250 |
| pooled_curve_ratio | 92 | 27 | 21 | 13.41 | 44.69 | 250 |
| analog_knn | 92 | 27 | 18 | 14.48 | 48.77 | 248 |
| baseline_level | 92 | 2 | 1 | 35.51 | 62.92 | 250 |
| last_two_log_linear | 92 | 7 | 7 | 40.99 | 96.04 | 250 |
| reference_to_last_log_linear | 92 | 7 | 7 | 40.99 | 96.04 | 250 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 134 | 60 | 30 | 9.70 | 48.29 | 346 |
| population_assimilation_robust | 134 | 58 | 28 | 9.73 | 48.41 | 346 |
| population_assimilation_t4 | 134 | 59 | 29 | 9.85 | 48.18 | 346 |
| population_anchored | 134 | 60 | 31 | 10.19 | 48.13 | 346 |
| population_anchored_t4 | 134 | 58 | 28 | 10.49 | 49.48 | 346 |
| persistence | 134 | 51 | 0 | 10.81 | 54.05 | 346 |
| analog_knn | 134 | 44 | 27 | 12.30 | 42.85 | 344 |
| weak_prior | 134 | 48 | 3 | 12.37 | 56.08 | 346 |
| pooled_curve_ratio | 134 | 34 | 18 | 14.56 | 88.71 | 346 |
| reference_to_last_log_linear | 134 | 13 | 11 | 26.07 | 119.16 | 346 |
| last_two_log_linear | 134 | 15 | 10 | 29.49 | 188.63 | 346 |
| baseline_level | 134 | 6 | 2 | 35.69 | 73.46 | 346 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 134 | 51 | 32 | 13.04 | 47.66 | 346 |
| population_assimilation_robust | 134 | 47 | 28 | 13.13 | 47.72 | 346 |
| population_assimilation_t4 | 134 | 50 | 31 | 13.17 | 47.49 | 346 |
| population_anchored | 134 | 45 | 26 | 14.50 | 57.63 | 346 |
| population_anchored_t4 | 134 | 47 | 26 | 14.52 | 57.57 | 346 |
| persistence | 134 | 37 | 0 | 18.30 | 63.88 | 346 |
| pooled_curve_ratio | 134 | 28 | 21 | 18.37 | 88.71 | 346 |
| weak_prior | 134 | 34 | 4 | 19.21 | 70.23 | 346 |
| analog_knn | 134 | 37 | 22 | 20.61 | 91.65 | 344 |
| baseline_level | 134 | 6 | 2 | 35.69 | 73.46 | 346 |
| last_two_log_linear | 134 | 12 | 11 | 58.79 | 254.07 | 346 |
| reference_to_last_log_linear | 134 | 12 | 11 | 58.79 | 254.07 | 346 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 74 | 11 | 13.00 | 35.64 | 275 |
| population_initial_state | 74 | 11 | 13.00 | 35.71 | 275 |
| pooled_training_curve | 74 | 8 | 18.51 | 61.53 | 275 |

## single eyes (reported, not tested)

- next_observation vs persistence: 13.28 against 14.81 pp on 96 targets
- next_observation vs last_two_log_linear: 13.28 against 36.34 pp on 96 targets
- next_observation vs reference_to_last_log_linear: 13.28 against 35.42 pp on 96 targets
- next_observation vs pooled_curve_ratio: 13.28 against 21.87 pp on 96 targets
- next_observation vs analog_knn: 13.28 against 17.09 pp on 96 targets
- next_observation vs baseline_level: 13.28 against 36.15 pp on 96 targets
- landmark vs persistence: 18.92 against 29.31 pp on 96 targets
- landmark vs last_two_log_linear: 18.92 against 89.90 pp on 96 targets
- landmark vs reference_to_last_log_linear: 18.92 against 89.90 pp on 96 targets
- landmark vs pooled_curve_ratio: 18.92 against 27.35 pp on 96 targets
- landmark vs analog_knn: 18.92 against 31.30 pp on 96 targets
- landmark vs baseline_level: 18.92 against 36.15 pp on 96 targets
- next_observation 80% interval: 61/96 = 0.635
- next_observation 90% interval: 73/96 = 0.760
- landmark 80% interval: 51/96 = 0.531
- landmark 90% interval: 63/96 = 0.656

## by context (primary methods; comparator in parentheses)

- human/cme_postoperative/laser: next_observation 1/1 within, 0.54 (2.65) pp; landmark 1/1 within, 0.30 (5.85) pp
- human/csc/laser: next_observation 1/2 within, 5.04 (5.31) pp; landmark 1/2 within, 7.28 (8.52) pp; initial_state_group_mean 0/1 within, 27.72 (20.90) pp
- human/dme/anti_vegf: next_observation 8/15 within, 7.57 (9.22) pp; landmark 8/15 within, 6.93 (8.95) pp; initial_state_group_mean 4/16 within, 11.98 (13.47) pp
- human/dme/combination: next_observation 3/4 within, 4.15 (5.31) pp; landmark 1/4 within, 7.83 (9.70) pp; initial_state_group_mean 0/3 within, 13.80 (16.13) pp
- human/dme/other: next_observation 1/4 within, 11.86 (14.31) pp; landmark 1/4 within, 18.31 (22.99) pp
- human/dme/steroid_implant: next_observation 12/33 within, 10.98 (13.42) pp; landmark 12/33 within, 14.40 (23.53) pp; initial_state_group_mean 2/8 within, 11.61 (12.88) pp
- human/dme/steroid_injection: next_observation 2/3 within, 14.80 (14.80) pp; landmark 1/3 within, 16.49 (17.21) pp; initial_state_group_mean 0/2 within, 19.28 (19.32) pp
- human/dme/vitrectomy: next_observation 1/1 within, 3.08 (2.75) pp; landmark 1/1 within, 2.82 (3.90) pp; initial_state_group_mean 0/1 within, 6.08 (26.41) pp
- human/myopic_cnv/anti_vegf: next_observation 0/1 within, 5.39 (3.49) pp; landmark 0/1 within, 5.39 (3.49) pp; initial_state_group_mean 1/1 within, 4.41 (5.56) pp
- human/namd/anti_vegf: next_observation 7/11 within, 4.05 (5.44) pp; landmark 6/11 within, 5.59 (8.15) pp; initial_state_group_mean 1/10 within, 11.62 (14.34) pp
- human/namd/combination: next_observation 3/4 within, 3.77 (5.53) pp; landmark 2/4 within, 5.49 (10.03) pp; initial_state_group_mean 1/1 within, 2.64 (15.59) pp
- human/other/anti_vegf: next_observation 2/7 within, 12.37 (13.86) pp; landmark 2/7 within, 19.08 (21.34) pp; initial_state_group_mean 0/2 within, 26.88 (24.68) pp
- human/other/combination: next_observation 0/4 within, 15.67 (11.93) pp; landmark 0/4 within, 18.70 (14.48) pp
- human/other/other: next_observation 1/4 within, 13.12 (12.37) pp; landmark 1/4 within, 14.43 (10.77) pp; initial_state_group_mean 0/2 within, 18.20 (18.52) pp
- human/other/steroid_injection: next_observation 0/1 within, 13.36 (19.73) pp; landmark 0/1 within, 13.36 (19.73) pp; initial_state_group_mean 0/1 within, 8.21 (7.97) pp
- human/other/vitrectomy: next_observation 3/6 within, 8.09 (8.51) pp; landmark 1/6 within, 8.51 (11.11) pp; initial_state_group_mean 0/5 within, 15.10 (30.08) pp
- human/rvo/anti_vegf: next_observation 7/11 within, 4.23 (5.35) pp; landmark 7/11 within, 4.52 (6.63) pp; initial_state_group_mean 1/10 within, 12.13 (17.80) pp
- human/rvo/combination: next_observation 4/5 within, 3.26 (3.65) pp; landmark 3/5 within, 10.27 (12.21) pp; initial_state_group_mean 0/3 within, 7.59 (19.48) pp
- human/rvo/laser: next_observation 1/2 within, 6.25 (9.27) pp; landmark 1/2 within, 6.17 (12.74) pp; initial_state_group_mean 0/1 within, 8.47 (22.36) pp
- human/rvo/steroid_implant: next_observation 1/12 within, 14.73 (15.34) pp; landmark 1/12 within, 20.48 (30.70) pp; initial_state_group_mean 1/5 within, 10.34 (14.30) pp
- human/rvo/steroid_injection: next_observation 1/2 within, 8.52 (9.70) pp; landmark 0/2 within, 15.28 (20.08) pp; initial_state_group_mean 0/2 within, 12.32 (47.24) pp
- human/uveitic/steroid_implant: next_observation 1/1 within, 1.30 (1.53) pp; landmark 1/1 within, 1.30 (1.53) pp

## H5: primary against the frozen model 1.0.0, same training

- next_observation (tested): primary 7.91 against 1.0.0 7.79 pp on 250 common targets
- landmark (tested): primary 9.90 against 1.0.0 9.73 pp on 250 common targets
- initial_state (tested): primary 13.00 against 1.0.0 13.25 pp on 275 common targets
