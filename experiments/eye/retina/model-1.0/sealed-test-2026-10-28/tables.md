# Retinal-thickness model 1.0.0-r3 — sealed results

## Declared checks (group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_primary_better_than_reference_next_observation: pass

- H1 next observation 80% interval: 80/98 = 0.816 (window 0.70–0.90)
- H1 next observation 90% interval: 90/98 = 0.918 (window 0.80–0.97)
- H2 next observation vs persistence: 5.75 against 6.34 pp on 98 common targets
- H2 next observation vs last_two_log_linear: 5.75 against 17.08 pp on 98 common targets
- H2 next observation vs pooled_curve_ratio: 5.75 against 8.03 pp on 98 common targets
- H2 next observation vs analog_knn: 5.75 against 6.56 pp on 98 common targets
- H3 landmark vs persistence: 7.50 against 9.50 pp on 98 common targets
- H3 landmark vs pooled_curve_ratio: 7.50 against 10.10 pp on 98 common targets
- H3 landmark vs analog_knn: 7.50 against 9.95 pp on 98 common targets
- landmark 80% interval (reported): 82/98 = 0.837
- landmark 90% interval (reported): 89/98 = 0.908
- H4 initial state population_initial_state: 11.35 pp, 13/31 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 17.09 pp, 2/31 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 99/112 = 0.884
- H4 initial state 90% interval (reported): 104/112 = 0.929

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 40 | 24 | 5 | 5.75 | 27.04 | 98 |
| population_assimilation_t4 | 40 | 25 | 5 | 5.76 | 27.03 | 98 |
| population_assimilation_robust | 40 | 25 | 5 | 5.76 | 27.02 | 98 |
| population_anchored_t4 | 40 | 25 | 4 | 6.19 | 26.93 | 98 |
| population_anchored | 40 | 25 | 4 | 6.23 | 26.96 | 98 |
| persistence | 40 | 25 | 0 | 6.34 | 26.41 | 98 |
| weak_prior | 40 | 23 | 0 | 6.44 | 25.91 | 98 |
| analog_knn | 40 | 20 | 9 | 6.56 | 24.57 | 98 |
| pooled_curve_ratio | 40 | 21 | 7 | 8.03 | 26.87 | 98 |
| last_two_log_linear | 40 | 3 | 2 | 17.08 | 52.88 | 98 |
| reference_to_last_log_linear | 40 | 4 | 3 | 20.28 | 52.88 | 98 |
| baseline_level | 40 | 3 | 1 | 35.63 | 60.50 | 98 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 40 | 22 | 8 | 7.50 | 30.43 | 98 |
| population_assimilation_robust | 40 | 23 | 8 | 7.58 | 30.41 | 98 |
| population_assimilation_t4 | 40 | 23 | 8 | 7.59 | 30.42 | 98 |
| population_anchored | 40 | 24 | 6 | 8.56 | 30.32 | 98 |
| population_anchored_t4 | 40 | 24 | 6 | 8.58 | 30.29 | 98 |
| persistence | 40 | 22 | 0 | 9.50 | 31.74 | 98 |
| weak_prior | 40 | 20 | 1 | 9.86 | 32.67 | 98 |
| analog_knn | 40 | 10 | 7 | 9.95 | 27.23 | 98 |
| pooled_curve_ratio | 40 | 17 | 7 | 10.10 | 26.87 | 98 |
| baseline_level | 40 | 3 | 1 | 35.63 | 60.50 | 98 |
| last_two_log_linear | 40 | 4 | 3 | 41.05 | 74.92 | 98 |
| reference_to_last_log_linear | 40 | 4 | 3 | 41.05 | 74.92 | 98 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 60 | 39 | 13 | 5.55 | 27.03 | 118 |
| population_assimilation_robust | 60 | 40 | 13 | 5.55 | 27.02 | 118 |
| population_assimilation | 60 | 36 | 13 | 5.57 | 27.04 | 118 |
| population_anchored_t4 | 60 | 42 | 12 | 5.91 | 26.93 | 118 |
| population_anchored | 60 | 42 | 12 | 5.94 | 26.96 | 118 |
| persistence | 60 | 38 | 0 | 6.18 | 26.41 | 118 |
| weak_prior | 60 | 35 | 0 | 6.37 | 25.91 | 118 |
| analog_knn | 60 | 27 | 15 | 6.57 | 24.57 | 118 |
| pooled_curve_ratio | 60 | 36 | 19 | 7.52 | 26.87 | 118 |
| last_two_log_linear | 60 | 3 | 2 | 26.16 | 55.81 | 118 |
| reference_to_last_log_linear | 60 | 4 | 3 | 28.00 | 55.81 | 118 |
| baseline_level | 60 | 3 | 1 | 36.21 | 60.50 | 118 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 60 | 34 | 16 | 7.10 | 30.43 | 118 |
| population_assimilation_robust | 60 | 38 | 16 | 7.14 | 30.41 | 118 |
| population_assimilation_t4 | 60 | 37 | 16 | 7.15 | 30.42 | 118 |
| population_anchored | 60 | 41 | 14 | 7.99 | 30.32 | 118 |
| population_anchored_t4 | 60 | 41 | 14 | 8.01 | 30.29 | 118 |
| persistence | 60 | 35 | 0 | 8.93 | 31.74 | 118 |
| weak_prior | 60 | 32 | 1 | 9.32 | 32.67 | 118 |
| pooled_curve_ratio | 60 | 32 | 19 | 9.36 | 26.87 | 118 |
| analog_knn | 60 | 17 | 13 | 9.47 | 27.23 | 118 |
| baseline_level | 60 | 3 | 1 | 36.21 | 60.50 | 118 |
| last_two_log_linear | 60 | 4 | 3 | 42.91 | 74.92 | 118 |
| reference_to_last_log_linear | 60 | 4 | 3 | 42.91 | 74.92 | 118 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state | 31 | 13 | 11.35 | 33.65 | 112 |
| population_initial_state_t4 | 31 | 13 | 11.35 | 33.65 | 112 |
| pooled_training_curve | 31 | 2 | 17.09 | 51.85 | 112 |

## single eyes (reported, not tested)

- next_observation vs persistence: 4.61 against 5.31 pp on 20 targets
- next_observation vs last_two_log_linear: 4.61 against 51.07 pp on 20 targets
- next_observation vs reference_to_last_log_linear: 4.61 against 51.07 pp on 20 targets
- next_observation vs pooled_curve_ratio: 4.61 against 4.15 pp on 20 targets
- next_observation vs analog_knn: 4.61 against 6.63 pp on 20 targets
- next_observation vs baseline_level: 4.61 against 38.94 pp on 20 targets
- landmark vs persistence: 4.61 against 5.31 pp on 20 targets
- landmark vs last_two_log_linear: 4.61 against 51.07 pp on 20 targets
- landmark vs reference_to_last_log_linear: 4.61 against 51.07 pp on 20 targets
- landmark vs pooled_curve_ratio: 4.61 against 4.15 pp on 20 targets
- landmark vs analog_knn: 4.61 against 6.63 pp on 20 targets
- landmark vs baseline_level: 4.61 against 38.94 pp on 20 targets
- next_observation 80% interval: 19/20 = 0.950
- next_observation 90% interval: 19/20 = 0.950
- landmark 80% interval: 19/20 = 0.950
- landmark 90% interval: 19/20 = 0.950

## by context (primary methods; comparator in parentheses)

- human/cme_postoperative/combination: next_observation 1/1 within, 4.48 (4.45) pp; landmark 0/1 within, 10.53 (12.69) pp
- human/cme_postoperative/topical: next_observation 0/1 within, 6.11 (8.39) pp; landmark 0/1 within, 14.10 (24.06) pp; initial_state_group_mean 0/1 within, 11.80 (5.52) pp
- human/csc/anti_vegf: next_observation 0/1 within, 10.06 (10.00) pp; landmark 0/1 within, 12.95 (14.71) pp; initial_state_group_mean 0/1 within, 13.02 (24.78) pp
- human/csc/laser: next_observation 1/1 within, 1.85 (2.30) pp; landmark 1/1 within, 1.24 (3.15) pp
- human/csc/other: next_observation 0/2 within, 9.93 (13.19) pp; landmark 0/2 within, 12.57 (20.63) pp
- human/dme/anti_vegf: next_observation 8/15 within, 4.05 (4.90) pp; landmark 8/15 within, 3.95 (4.85) pp; initial_state_group_mean 2/4 within, 7.87 (8.80) pp
- human/dme/combination: next_observation 0/1 within, 12.62 (20.07) pp; landmark 0/1 within, 12.62 (20.07) pp; initial_state_group_mean 0/1 within, 23.47 (12.77) pp
- human/dme/laser: next_observation 0/1 within, 9.67 (11.60) pp; landmark 0/1 within, 9.67 (11.60) pp
- human/dme/observation: next_observation 0/1 within, 5.83 (0.39) pp; landmark 0/1 within, 5.83 (0.39) pp; initial_state_group_mean 0/1 within, 33.16 (29.46) pp
- human/dme/steroid_implant: next_observation 2/3 within, 11.13 (9.52) pp; landmark 2/3 within, 14.43 (14.17) pp; initial_state_group_mean 1/3 within, 13.44 (15.00) pp
- human/dme/steroid_injection: next_observation 0/1 within, 9.13 (1.69) pp; landmark 0/1 within, 9.13 (1.69) pp; initial_state_group_mean 0/1 within, 31.96 (32.03) pp
- human/myopic_cnv/anti_vegf: next_observation 2/2 within, 1.07 (1.34) pp; landmark 2/2 within, 0.92 (1.43) pp; initial_state_group_mean 2/2 within, 3.11 (2.08) pp
- human/namd/anti_vegf: next_observation 13/17 within, 3.40 (3.85) pp; landmark 12/17 within, 3.52 (4.03) pp; initial_state_group_mean 4/6 within, 6.17 (15.14) pp
- human/namd/combination: next_observation 1/2 within, 4.51 (3.97) pp; landmark 1/2 within, 5.86 (5.29) pp; initial_state_group_mean 0/2 within, 15.58 (47.15) pp
- human/other/combination: next_observation 1/1 within, 1.42 (1.66) pp; landmark 1/1 within, 1.09 (0.94) pp; initial_state_group_mean 1/1 within, 1.31 (9.57) pp
- human/rvo/anti_vegf: next_observation 5/7 within, 3.72 (5.32) pp; landmark 5/7 within, 4.09 (6.06) pp; initial_state_group_mean 3/7 within, 8.25 (12.28) pp
- human/rvo/combination: next_observation 2/2 within, 3.95 (3.76) pp; landmark 2/2 within, 4.26 (3.27) pp
- human/uveitic/steroid_implant: next_observation 0/1 within, 11.14 (10.69) pp; landmark 0/1 within, 12.55 (9.83) pp; initial_state_group_mean 0/1 within, 13.46 (10.18) pp

## H5: primary against the reference configuration, same training

- next_observation: primary 5.75 against reference 6.14 pp on 98 common targets
- landmark: primary 7.50 against reference 7.37 pp on 98 common targets
- initial_state: primary 11.35 against reference 11.35 pp on 112 common targets
