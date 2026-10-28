# Retinal-thickness model 1.1.0-s5 — development results

## Declared checks (group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_primary_better_than_model_1_0_0: pass

- H1 next observation 80% interval: 206/243 = 0.848 (window 0.70–0.90)
- H1 next observation 90% interval: 226/243 = 0.930 (window 0.80–0.97)
- H2 next observation vs persistence: 6.37 against 7.82 pp on 243 common targets
- H2 next observation vs last_two_log_linear: 6.37 against 21.25 pp on 243 common targets
- H2 next observation vs pooled_curve_ratio: 6.37 against 8.40 pp on 243 common targets
- H2 next observation vs analog_knn: 6.37 against 8.81 pp on 243 common targets
- H3 landmark vs persistence: 7.95 against 11.85 pp on 243 common targets
- H3 landmark vs pooled_curve_ratio: 7.95 against 11.57 pp on 243 common targets
- H3 landmark vs analog_knn: 7.95 against 12.61 pp on 243 common targets
- landmark 80% interval (reported): 195/243 = 0.802
- landmark 90% interval (reported): 220/243 = 0.905
- H4 initial state population_initial_state: 12.15 pp, 24/83 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 17.35 pp, 15/83 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 230/290 = 0.793
- H4 initial state 90% interval (reported): 266/290 = 0.917

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 101 | 55 | 21 | 6.37 | 24.84 | 243 |
| population_assimilation_robust | 101 | 58 | 23 | 6.37 | 24.87 | 243 |
| population_assimilation_t4 | 101 | 58 | 22 | 6.54 | 24.82 | 243 |
| population_anchored | 101 | 58 | 20 | 6.86 | 29.59 | 243 |
| population_anchored_t4 | 101 | 58 | 19 | 7.16 | 30.91 | 243 |
| persistence | 101 | 51 | 0 | 7.82 | 35.06 | 243 |
| weak_prior | 101 | 51 | 4 | 7.94 | 36.11 | 243 |
| pooled_curve_ratio | 101 | 48 | 19 | 8.40 | 28.44 | 243 |
| analog_knn | 101 | 50 | 20 | 8.81 | 48.24 | 243 |
| reference_to_last_log_linear | 101 | 14 | 10 | 20.65 | 97.28 | 243 |
| last_two_log_linear | 101 | 11 | 8 | 21.25 | 146.31 | 243 |
| baseline_level | 101 | 7 | 4 | 34.62 | 65.93 | 243 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 101 | 49 | 25 | 7.95 | 27.97 | 243 |
| population_assimilation_robust | 101 | 48 | 25 | 7.96 | 28.98 | 243 |
| population_assimilation_t4 | 101 | 48 | 25 | 7.99 | 28.93 | 243 |
| population_anchored | 101 | 51 | 23 | 9.15 | 35.49 | 243 |
| population_anchored_t4 | 101 | 52 | 23 | 9.18 | 35.83 | 243 |
| pooled_curve_ratio | 101 | 39 | 20 | 11.57 | 42.13 | 243 |
| persistence | 101 | 44 | 0 | 11.85 | 43.24 | 243 |
| weak_prior | 101 | 41 | 5 | 11.96 | 44.16 | 243 |
| analog_knn | 101 | 30 | 21 | 12.61 | 58.05 | 243 |
| baseline_level | 101 | 7 | 4 | 34.62 | 65.93 | 243 |
| last_two_log_linear | 101 | 9 | 8 | 42.47 | 206.93 | 243 |
| reference_to_last_log_linear | 101 | 9 | 8 | 42.47 | 206.93 | 243 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 153 | 83 | 38 | 12.28 | 180.04 | 316 |
| population_assimilation_robust | 153 | 88 | 40 | 12.36 | 180.28 | 316 |
| population_assimilation_t4 | 153 | 88 | 39 | 12.48 | 180.28 | 316 |
| population_anchored | 153 | 90 | 37 | 12.64 | 182.88 | 316 |
| population_anchored_t4 | 153 | 90 | 39 | 12.93 | 182.80 | 316 |
| persistence | 153 | 76 | 0 | 13.18 | 180.24 | 316 |
| weak_prior | 153 | 76 | 6 | 13.70 | 178.52 | 316 |
| analog_knn | 153 | 70 | 30 | 13.96 | 176.45 | 316 |
| pooled_curve_ratio | 153 | 81 | 39 | 14.24 | 195.18 | 316 |
| reference_to_last_log_linear | 153 | 16 | 12 | 27.86 | 225.18 | 316 |
| last_two_log_linear | 153 | 13 | 10 | 29.41 | 225.18 | 316 |
| baseline_level | 153 | 8 | 4 | 35.14 | 132.83 | 316 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 153 | 78 | 41 | 12.94 | 180.04 | 316 |
| population_assimilation_robust | 153 | 79 | 41 | 12.96 | 180.28 | 316 |
| population_assimilation_t4 | 153 | 79 | 41 | 12.97 | 180.28 | 316 |
| population_anchored | 153 | 84 | 41 | 13.76 | 182.88 | 316 |
| population_anchored_t4 | 153 | 85 | 44 | 13.78 | 182.80 | 316 |
| persistence | 153 | 70 | 0 | 15.21 | 180.24 | 316 |
| weak_prior | 153 | 67 | 7 | 15.27 | 178.52 | 316 |
| pooled_curve_ratio | 153 | 72 | 40 | 16.41 | 195.18 | 316 |
| analog_knn | 153 | 50 | 31 | 16.93 | 176.45 | 316 |
| baseline_level | 153 | 8 | 4 | 35.14 | 132.83 | 316 |
| last_two_log_linear | 153 | 11 | 10 | 45.11 | 225.18 | 316 |
| reference_to_last_log_linear | 153 | 11 | 10 | 45.11 | 225.18 | 316 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 83 | 24 | 12.14 | 33.69 | 290 |
| population_initial_state | 83 | 24 | 12.15 | 33.70 | 290 |
| pooled_training_curve | 83 | 15 | 17.35 | 51.85 | 290 |

## single eyes (reported, not tested)

- next_observation vs persistence: 22.74 against 23.42 pp on 73 targets
- next_observation vs last_two_log_linear: 22.74 against 47.33 pp on 73 targets
- next_observation vs reference_to_last_log_linear: 22.74 against 44.06 pp on 73 targets
- next_observation vs pooled_curve_ratio: 22.74 against 25.35 pp on 73 targets
- next_observation vs analog_knn: 22.74 against 24.18 pp on 73 targets
- next_observation vs baseline_level: 22.74 against 36.82 pp on 73 targets
- landmark vs persistence: 22.68 against 23.10 pp on 73 targets
- landmark vs last_two_log_linear: 22.68 against 52.98 pp on 73 targets
- landmark vs reference_to_last_log_linear: 22.68 against 52.98 pp on 73 targets
- landmark vs pooled_curve_ratio: 22.68 against 26.84 pp on 73 targets
- landmark vs analog_knn: 22.68 against 26.68 pp on 73 targets
- landmark vs baseline_level: 22.68 against 36.82 pp on 73 targets
- next_observation 80% interval: 55/73 = 0.753
- next_observation 90% interval: 61/73 = 0.836
- landmark 80% interval: 55/73 = 0.753
- landmark 90% interval: 63/73 = 0.863

## by context (primary methods; comparator in parentheses)

- human/cme_postoperative/combination: next_observation 0/1 within, 5.24 (4.45) pp; landmark 0/1 within, 11.29 (12.69) pp
- human/cme_postoperative/topical: next_observation 0/1 within, 5.75 (8.39) pp; landmark 0/1 within, 13.42 (24.06) pp; initial_state_group_mean 0/1 within, 10.84 (6.19) pp
- human/csc/anti_vegf: next_observation 0/1 within, 9.97 (10.00) pp; landmark 0/1 within, 13.10 (14.71) pp; initial_state_group_mean 0/1 within, 15.85 (25.14) pp
- human/csc/laser: next_observation 1/3 within, 10.13 (13.41) pp; landmark 1/3 within, 14.71 (21.22) pp; initial_state_group_mean 0/1 within, 18.81 (10.54) pp
- human/csc/other: next_observation 0/3 within, 9.57 (11.98) pp; landmark 0/3 within, 12.83 (18.57) pp; initial_state_group_mean 0/1 within, 27.76 (24.05) pp
- human/dme/anti_vegf: next_observation 18/24 within, 5.13 (5.92) pp; landmark 17/24 within, 6.78 (8.02) pp; initial_state_group_mean 3/13 within, 11.34 (9.33) pp
- human/dme/combination: next_observation 0/7 within, 9.77 (13.27) pp; landmark 0/7 within, 11.36 (22.21) pp; initial_state_group_mean 0/4 within, 14.04 (28.37) pp
- human/dme/laser: next_observation 0/1 within, 9.17 (11.60) pp; landmark 0/1 within, 9.17 (11.60) pp
- human/dme/observation: next_observation 0/1 within, 8.46 (0.39) pp; landmark 0/1 within, 8.46 (0.39) pp; initial_state_group_mean 0/1 within, 30.44 (30.28) pp
- human/dme/other: next_observation 0/1 within, 7.11 (1.73) pp; landmark 0/1 within, 7.11 (1.73) pp; initial_state_group_mean 0/1 within, 21.03 (23.23) pp
- human/dme/steroid_implant: next_observation 5/9 within, 5.27 (11.73) pp; landmark 5/9 within, 5.40 (14.65) pp; initial_state_group_mean 1/8 within, 9.21 (16.78) pp
- human/dme/steroid_injection: next_observation 5/8 within, 5.68 (8.26) pp; landmark 3/8 within, 7.16 (11.75) pp; initial_state_group_mean 0/3 within, 17.74 (30.37) pp
- human/dme/vitrectomy: next_observation 1/3 within, 7.59 (10.57) pp; landmark 1/3 within, 7.64 (10.75) pp; initial_state_group_mean 1/2 within, 5.68 (8.18) pp
- human/myopic_cnv/anti_vegf: next_observation 5/5 within, 2.05 (1.19) pp; landmark 3/5 within, 3.56 (1.23) pp; initial_state_group_mean 3/5 within, 6.57 (8.52) pp
- human/namd/anti_vegf: next_observation 29/45 within, 7.56 (8.06) pp; landmark 28/45 within, 7.46 (7.24) pp; initial_state_group_mean 8/17 within, 9.83 (12.15) pp
- human/namd/combination: next_observation 1/3 within, 6.72 (4.72) pp; landmark 1/3 within, 8.78 (6.74) pp; initial_state_group_mean 0/3 within, 19.63 (47.22) pp
- human/other/combination: next_observation 4/5 within, 4.98 (7.99) pp; landmark 4/5 within, 5.08 (15.91) pp; initial_state_group_mean 2/3 within, 10.86 (8.86) pp
- human/other/laser: next_observation 0/1 within, 10.20 (10.59) pp; landmark 0/1 within, 8.60 (21.76) pp; initial_state_group_mean 0/1 within, 7.71 (4.62) pp
- human/other/vitrectomy: next_observation 2/5 within, 8.13 (7.12) pp; landmark 2/5 within, 12.37 (10.46) pp; initial_state_group_mean 0/4 within, 21.70 (24.10) pp
- human/rvo/anti_vegf: next_observation 8/20 within, 28.41 (28.86) pp; landmark 9/20 within, 28.46 (29.00) pp; initial_state_group_mean 6/12 within, 7.85 (10.64) pp
- human/rvo/combination: next_observation 2/3 within, 5.32 (5.51) pp; landmark 2/3 within, 5.56 (5.25) pp
- human/rvo/steroid_implant: next_observation 1/1 within, 2.10 (3.66) pp; landmark 1/1 within, 3.25 (5.85) pp; initial_state_group_mean 0/1 within, 11.92 (32.82) pp
- human/rvo/steroid_injection: next_observation 1/1 within, 2.05 (1.31) pp; landmark 1/1 within, 2.05 (1.31) pp
- human/uveitic/steroid_implant: next_observation 0/1 within, 10.87 (10.69) pp; landmark 0/1 within, 11.16 (9.83) pp; initial_state_group_mean 0/1 within, 9.23 (10.18) pp

## H5: primary against the frozen model 1.0.0, same training

- next_observation (tested): primary 6.37 against 1.0.0 7.29 pp on 243 common targets
- landmark (tested): primary 7.95 against 1.0.0 9.04 pp on 243 common targets
- initial_state (tested): primary 12.15 against 1.0.0 13.28 pp on 290 common targets
