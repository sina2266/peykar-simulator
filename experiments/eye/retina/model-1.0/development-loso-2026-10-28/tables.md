# Retinal-thickness model 1.0.0-r3 — development results

## Declared checks (group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_primary_better_than_reference_next_observation: pass

- H1 next observation 80% interval: 107/145 = 0.738 (window 0.70–0.90)
- H1 next observation 90% interval: 121/145 = 0.834 (window 0.80–0.97)
- H2 next observation vs persistence: 8.23 against 8.67 pp on 145 common targets
- H2 next observation vs last_two_log_linear: 8.23 against 23.66 pp on 145 common targets
- H2 next observation vs pooled_curve_ratio: 8.23 against 9.86 pp on 145 common targets
- H2 next observation vs analog_knn: 8.23 against 10.20 pp on 145 common targets
- H3 landmark vs persistence: 10.31 against 13.20 pp on 145 common targets
- H3 landmark vs pooled_curve_ratio: 10.31 against 13.96 pp on 145 common targets
- H3 landmark vs analog_knn: 10.31 against 14.72 pp on 145 common targets
- landmark 80% interval (reported): 113/145 = 0.779
- landmark 90% interval (reported): 123/145 = 0.848
- H4 initial state population_initial_state: 13.89 pp, 9/52 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 17.28 pp, 7/52 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 131/178 = 0.736
- H4 initial state 90% interval (reported): 151/178 = 0.848

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 61 | 31 | 16 | 8.21 | 27.98 | 145 |
| population_assimilation | 61 | 29 | 15 | 8.23 | 28.12 | 145 |
| population_assimilation_t4 | 61 | 31 | 16 | 8.25 | 27.85 | 145 |
| population_anchored | 61 | 31 | 12 | 8.65 | 31.77 | 145 |
| persistence | 61 | 26 | 0 | 8.67 | 35.06 | 145 |
| population_anchored_t4 | 61 | 31 | 12 | 8.69 | 31.84 | 145 |
| weak_prior | 61 | 24 | 2 | 9.29 | 35.66 | 145 |
| pooled_curve_ratio | 61 | 26 | 11 | 9.86 | 28.44 | 145 |
| analog_knn | 61 | 29 | 17 | 10.20 | 46.97 | 145 |
| reference_to_last_log_linear | 61 | 10 | 7 | 20.89 | 97.28 | 145 |
| last_two_log_linear | 61 | 8 | 6 | 23.66 | 146.31 | 145 |
| baseline_level | 61 | 4 | 3 | 33.93 | 65.93 | 145 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 61 | 26 | 15 | 10.31 | 31.62 | 145 |
| population_assimilation_robust | 61 | 25 | 14 | 10.37 | 31.56 | 145 |
| population_assimilation_t4 | 61 | 25 | 14 | 10.43 | 31.61 | 145 |
| population_anchored_t4 | 61 | 26 | 10 | 11.54 | 39.13 | 145 |
| population_anchored | 61 | 26 | 12 | 11.54 | 39.00 | 145 |
| persistence | 61 | 22 | 0 | 13.20 | 43.24 | 145 |
| weak_prior | 61 | 20 | 3 | 13.81 | 44.20 | 145 |
| pooled_curve_ratio | 61 | 25 | 12 | 13.96 | 42.13 | 145 |
| analog_knn | 61 | 22 | 13 | 14.72 | 58.05 | 145 |
| baseline_level | 61 | 4 | 3 | 33.93 | 65.93 | 145 |
| last_two_log_linear | 61 | 5 | 5 | 43.39 | 206.93 | 145 |
| reference_to_last_log_linear | 61 | 5 | 5 | 43.39 | 206.93 | 145 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 93 | 39 | 20 | 15.30 | 177.65 | 198 |
| population_assimilation_robust | 93 | 43 | 21 | 15.38 | 178.19 | 198 |
| population_assimilation_t4 | 93 | 43 | 21 | 15.40 | 178.20 | 198 |
| population_anchored | 93 | 45 | 19 | 15.80 | 181.89 | 198 |
| population_anchored_t4 | 93 | 45 | 19 | 15.86 | 181.86 | 198 |
| persistence | 93 | 38 | 0 | 15.95 | 180.24 | 198 |
| weak_prior | 93 | 36 | 4 | 16.30 | 178.45 | 198 |
| analog_knn | 93 | 36 | 22 | 16.79 | 176.45 | 198 |
| pooled_curve_ratio | 93 | 38 | 20 | 17.57 | 197.78 | 198 |
| reference_to_last_log_linear | 93 | 12 | 9 | 27.78 | 225.18 | 198 |
| last_two_log_linear | 93 | 10 | 8 | 31.18 | 225.18 | 198 |
| baseline_level | 93 | 5 | 3 | 34.49 | 132.83 | 198 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 93 | 38 | 20 | 16.10 | 177.65 | 198 |
| population_assimilation_robust | 93 | 37 | 18 | 16.13 | 178.19 | 198 |
| population_assimilation_t4 | 93 | 37 | 18 | 16.16 | 178.20 | 198 |
| population_anchored_t4 | 93 | 41 | 18 | 17.15 | 181.86 | 198 |
| population_anchored | 93 | 41 | 20 | 17.16 | 181.89 | 198 |
| persistence | 93 | 35 | 0 | 17.93 | 180.24 | 198 |
| weak_prior | 93 | 33 | 5 | 18.24 | 178.45 | 198 |
| pooled_curve_ratio | 93 | 37 | 21 | 19.99 | 197.78 | 198 |
| analog_knn | 93 | 29 | 18 | 20.27 | 176.45 | 198 |
| baseline_level | 93 | 5 | 3 | 34.49 | 132.83 | 198 |
| last_two_log_linear | 93 | 7 | 7 | 46.37 | 225.18 | 198 |
| reference_to_last_log_linear | 93 | 7 | 7 | 46.37 | 225.18 | 198 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 52 | 9 | 13.89 | 38.61 | 178 |
| population_initial_state | 52 | 9 | 13.89 | 38.61 | 178 |
| pooled_training_curve | 52 | 7 | 17.28 | 43.69 | 178 |

## single eyes (reported, not tested)

- next_observation vs persistence: 26.26 against 27.29 pp on 53 targets
- next_observation vs last_two_log_linear: 26.26 against 45.84 pp on 53 targets
- next_observation vs reference_to_last_log_linear: 26.26 against 41.10 pp on 53 targets
- next_observation vs pooled_curve_ratio: 26.26 against 29.78 pp on 53 targets
- next_observation vs analog_knn: 26.26 against 27.72 pp on 53 targets
- next_observation vs baseline_level: 26.26 against 35.98 pp on 53 targets
- landmark vs persistence: 26.02 against 26.92 pp on 53 targets
- landmark vs last_two_log_linear: 26.02 against 53.69 pp on 53 targets
- landmark vs reference_to_last_log_linear: 26.02 against 53.69 pp on 53 targets
- landmark vs pooled_curve_ratio: 26.02 against 30.98 pp on 53 targets
- landmark vs analog_knn: 26.02 against 30.70 pp on 53 targets
- landmark vs baseline_level: 26.02 against 35.98 pp on 53 targets
- next_observation 80% interval: 34/53 = 0.642
- next_observation 90% interval: 39/53 = 0.736
- landmark 80% interval: 39/53 = 0.736
- landmark 90% interval: 43/53 = 0.811

## by context (primary methods; comparator in parentheses)

- human/csc/laser: next_observation 0/2 within, 14.98 (17.21) pp; landmark 0/2 within, 19.94 (27.28) pp; initial_state_group_mean 0/1 within, 16.56 (10.21) pp
- human/csc/other: next_observation 0/1 within, 8.20 (4.62) pp; landmark 0/1 within, 8.20 (4.62) pp; initial_state_group_mean 0/1 within, 26.04 (23.73) pp
- human/dme/anti_vegf: next_observation 5/9 within, 6.06 (6.60) pp; landmark 5/9 within, 8.66 (9.76) pp; initial_state_group_mean 1/9 within, 10.83 (10.48) pp
- human/dme/combination: next_observation 0/6 within, 10.21 (12.73) pp; landmark 0/6 within, 13.98 (22.34) pp; initial_state_group_mean 0/3 within, 14.33 (30.11) pp
- human/dme/other: next_observation 0/1 within, 6.87 (1.73) pp; landmark 0/1 within, 6.87 (1.73) pp; initial_state_group_mean 0/1 within, 26.03 (23.40) pp
- human/dme/steroid_implant: next_observation 2/6 within, 14.87 (12.99) pp; landmark 2/6 within, 18.08 (14.96) pp; initial_state_group_mean 0/5 within, 20.78 (21.33) pp
- human/dme/steroid_injection: next_observation 5/7 within, 5.57 (8.62) pp; landmark 3/7 within, 7.19 (12.27) pp; initial_state_group_mean 0/2 within, 11.48 (8.53) pp
- human/dme/vitrectomy: next_observation 1/3 within, 7.87 (10.57) pp; landmark 1/3 within, 7.89 (10.75) pp; initial_state_group_mean 0/2 within, 7.85 (8.18) pp
- human/myopic_cnv/anti_vegf: next_observation 3/3 within, 2.66 (1.01) pp; landmark 1/3 within, 4.86 (0.99) pp; initial_state_group_mean 0/3 within, 9.71 (12.26) pp
- human/namd/anti_vegf: next_observation 14/28 within, 8.83 (9.28) pp; landmark 16/28 within, 8.33 (8.23) pp; initial_state_group_mean 3/11 within, 11.33 (13.19) pp
- human/namd/combination: next_observation 0/1 within, 9.20 (5.93) pp; landmark 0/1 within, 12.02 (8.96) pp; initial_state_group_mean 0/1 within, 22.46 (36.70) pp
- human/other/combination: next_observation 3/4 within, 6.05 (9.07) pp; landmark 2/4 within, 5.93 (18.14) pp; initial_state_group_mean 1/2 within, 14.98 (8.30) pp
- human/other/laser: next_observation 0/1 within, 10.23 (10.59) pp; landmark 0/1 within, 7.96 (21.76) pp; initial_state_group_mean 0/1 within, 12.46 (4.53) pp
- human/other/vitrectomy: next_observation 2/5 within, 7.08 (7.12) pp; landmark 2/5 within, 10.44 (10.46) pp; initial_state_group_mean 1/4 within, 21.31 (24.10) pp
- human/rvo/anti_vegf: next_observation 2/13 within, 38.94 (40.01) pp; landmark 4/13 within, 38.93 (40.11) pp; initial_state_group_mean 3/5 within, 6.89 (14.13) pp
- human/rvo/combination: next_observation 0/1 within, 9.12 (9.76) pp; landmark 0/1 within, 9.12 (9.76) pp
- human/rvo/steroid_implant: next_observation 1/1 within, 0.91 (3.66) pp; landmark 1/1 within, 1.26 (5.85) pp; initial_state_group_mean 0/1 within, 8.96 (32.80) pp
- human/rvo/steroid_injection: next_observation 1/1 within, 2.11 (1.31) pp; landmark 1/1 within, 2.11 (1.31) pp

## H5: primary against the reference configuration, same training

- next_observation: primary 8.23 against reference 8.76 pp on 145 common targets
- landmark: primary 10.31 against reference 10.35 pp on 145 common targets
- initial_state: primary 13.89 against reference 13.89 pp on 178 common targets
