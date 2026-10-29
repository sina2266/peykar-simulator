# IOP-after-surgery model 1.3.0-d1 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_landmark_and_initial_state_lower_next_observation_within_0_05_pp: FAIL

- H1 next observation 80% interval: 424/548 = 0.774 (window 0.70–0.90)
- H1 next observation 90% interval: 484/548 = 0.883 (window 0.80–0.97)
- H2 next observation vs persistence: 6.64 against 8.27 pp on 548 common targets
- H2 next observation vs last_two_log_linear: 6.64 against 37.15 pp on 548 common targets
- H2 next observation vs pooled_curve_ratio: 6.64 against 8.52 pp on 548 common targets
- H2 next observation vs analog_knn: 6.64 against 10.67 pp on 548 common targets
- H3 landmark vs persistence: 7.92 against 15.02 pp on 548 common targets
- H3 landmark vs pooled_curve_ratio: 7.92 against 16.03 pp on 548 common targets
- H3 landmark vs analog_knn: 7.92 against 17.08 pp on 548 common targets
- landmark 80% interval (reported): 438/548 = 0.799
- landmark 90% interval (reported): 484/548 = 0.883
- H4 initial state population_initial_state: 10.94 pp, 27/122 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 15.94 pp, 16/122 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 505/612 = 0.825
- H4 initial state 90% interval (reported): 567/612 = 0.926

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 133 | 76 | 36 | 6.64 | 29.75 | 548 |
| population_assimilation_t4 | 133 | 74 | 35 | 6.77 | 29.76 | 548 |
| population_assimilation_robust | 133 | 75 | 34 | 6.89 | 29.70 | 548 |
| persistence | 133 | 55 | 0 | 8.27 | 60.87 | 548 |
| pooled_curve_ratio | 133 | 38 | 13 | 8.52 | 36.87 | 548 |
| weak_prior | 133 | 48 | 10 | 8.68 | 40.54 | 548 |
| population_anchored_t4 | 133 | 57 | 14 | 8.92 | 54.94 | 548 |
| population_anchored | 133 | 58 | 15 | 8.99 | 54.90 | 548 |
| analog_knn | 133 | 56 | 20 | 10.67 | 62.69 | 548 |
| reference_to_last_log_linear | 133 | 6 | 0 | 33.29 | 193.73 | 548 |
| last_two_log_linear | 133 | 5 | 0 | 37.15 | 197.82 | 548 |
| baseline_level | 133 | 5 | 2 | 40.43 | 76.26 | 548 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 133 | 56 | 29 | 7.92 | 42.42 | 548 |
| population_assimilation_t4 | 133 | 56 | 29 | 8.13 | 42.43 | 548 |
| population_assimilation_robust | 133 | 57 | 30 | 8.45 | 42.35 | 548 |
| persistence | 133 | 41 | 0 | 15.02 | 78.26 | 548 |
| population_anchored | 133 | 43 | 11 | 15.24 | 67.90 | 548 |
| population_anchored_t4 | 133 | 43 | 11 | 15.24 | 67.91 | 548 |
| pooled_curve_ratio | 133 | 31 | 12 | 16.03 | 52.26 | 548 |
| weak_prior | 133 | 35 | 8 | 16.09 | 42.81 | 548 |
| analog_knn | 133 | 34 | 14 | 17.08 | 62.69 | 548 |
| baseline_level | 133 | 5 | 2 | 40.43 | 76.26 | 548 |
| last_two_log_linear | 133 | 3 | 1 | 63.79 | 233.15 | 548 |
| reference_to_last_log_linear | 133 | 3 | 1 | 63.79 | 233.15 | 548 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 214 | 94 | 38 | 7.76 | 34.06 | 1029 |
| population_assimilation | 214 | 92 | 39 | 7.79 | 33.89 | 1029 |
| population_assimilation_robust | 214 | 95 | 37 | 7.79 | 35.32 | 1029 |
| persistence | 214 | 92 | 0 | 9.16 | 100.00 | 1029 |
| weak_prior | 214 | 53 | 11 | 9.33 | 80.78 | 1029 |
| pooled_curve_ratio | 214 | 59 | 13 | 9.58 | 103.67 | 1029 |
| population_anchored_t4 | 214 | 70 | 14 | 9.76 | 95.35 | 1029 |
| population_anchored | 214 | 71 | 15 | 9.78 | 95.37 | 1029 |
| analog_knn | 214 | 90 | 33 | 10.03 | 62.69 | 1029 |
| reference_to_last_log_linear | 214 | 6 | 0 | 31.93 | 246.15 | 1029 |
| last_two_log_linear | 214 | 5 | 0 | 40.50 | 279.55 | 1029 |
| baseline_level | 214 | 5 | 2 | 49.85 | 91.11 | 1029 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 214 | 63 | 36 | 9.14 | 42.42 | 1029 |
| population_assimilation_t4 | 214 | 63 | 36 | 9.23 | 42.43 | 1029 |
| population_assimilation_robust | 214 | 65 | 38 | 9.34 | 42.35 | 1029 |
| analog_knn | 214 | 45 | 25 | 15.60 | 62.69 | 1029 |
| persistence | 214 | 41 | 0 | 17.13 | 123.08 | 1029 |
| weak_prior | 214 | 36 | 9 | 18.06 | 90.95 | 1029 |
| population_anchored | 214 | 44 | 12 | 18.21 | 115.18 | 1029 |
| population_anchored_t4 | 214 | 44 | 12 | 18.21 | 115.15 | 1029 |
| pooled_curve_ratio | 214 | 31 | 12 | 18.73 | 103.67 | 1029 |
| baseline_level | 214 | 5 | 2 | 49.85 | 91.11 | 1029 |
| last_two_log_linear | 214 | 3 | 1 | 57.83 | 269.23 | 1029 |
| reference_to_last_log_linear | 214 | 3 | 1 | 57.83 | 269.23 | 1029 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state | 122 | 27 | 10.94 | 66.86 | 612 |
| population_initial_state_t4 | 122 | 27 | 10.94 | 66.86 | 612 |
| pooled_training_curve | 122 | 16 | 15.94 | 73.89 | 612 |

## single eyes (reported, not tested)

- next_observation vs persistence: 8.91 against 10.08 pp on 481 targets
- next_observation vs last_two_log_linear: 8.91 against 44.00 pp on 481 targets
- next_observation vs reference_to_last_log_linear: 8.91 against 30.31 pp on 481 targets
- next_observation vs pooled_curve_ratio: 8.91 against 10.65 pp on 481 targets
- next_observation vs analog_knn: 8.91 against 9.24 pp on 481 targets
- next_observation vs preoperative_level: n/a against n/a pp on 0 targets
- landmark vs persistence: 10.35 against 19.25 pp on 481 targets
- landmark vs last_two_log_linear: 10.35 against 50.17 pp on 481 targets
- landmark vs reference_to_last_log_linear: 10.35 against 50.17 pp on 481 targets
- landmark vs pooled_curve_ratio: 10.35 against 21.40 pp on 481 targets
- landmark vs analog_knn: 10.35 against 13.71 pp on 481 targets
- landmark vs preoperative_level: n/a against n/a pp on 0 targets
- next_observation 80% interval: 437/481 = 0.909
- next_observation 90% interval: 464/481 = 0.965
- landmark 80% interval: 437/481 = 0.909
- landmark 90% interval: 461/481 = 0.958

## by procedure context (primary methods; comparator in parentheses)

- human/all/cataract: next_observation 5/6 within, 5.52 (16.62) pp; landmark 3/6 within, 7.49 (26.10) pp; initial_state_group_mean 1/5 within, 19.91 (27.67) pp
- human/all/nonpenetrating: next_observation 3/5 within, 4.65 (9.90) pp; landmark 1/5 within, 7.05 (30.58) pp; initial_state_group_mean 0/4 within, 8.08 (18.99) pp
- human/all/other: next_observation 1/4 within, 8.38 (8.90) pp; landmark 1/4 within, 9.38 (10.53) pp
- human/all/trabeculectomy: next_observation 19/27 within, 4.82 (5.47) pp; landmark 15/27 within, 6.50 (11.57) pp; initial_state_group_mean 6/21 within, 9.57 (14.38) pp
- human/canaloplasty/angle_mims: next_observation 3/3 within, 1.71 (3.86) pp; landmark 3/3 within, 1.67 (7.36) pp; initial_state_group_mean 0/3 within, 7.97 (25.03) pp
- human/continuous/cyclodestruction: next_observation 1/7 within, 11.54 (12.31) pp; landmark 1/7 within, 11.64 (13.79) pp; initial_state_group_mean 2/7 within, 13.84 (14.44) pp
- human/endoscopic/cyclodestruction: next_observation 5/25 within, 11.15 (11.67) pp; landmark 4/25 within, 13.13 (21.44) pp
- human/goniotomy/angle_mims: next_observation 13/24 within, 8.55 (13.04) pp; landmark 7/24 within, 10.41 (22.77) pp; initial_state_group_mean 4/23 within, 10.29 (16.61) pp
- human/micropulse/cyclodestruction: next_observation 5/8 within, 7.56 (7.48) pp; landmark 4/8 within, 8.20 (7.88) pp; initial_state_group_mean 0/9 within, 14.20 (12.46) pp
- human/non_valved/tube: next_observation 2/15 within, 9.15 (10.53) pp; landmark 0/15 within, 10.29 (15.53) pp; initial_state_group_mean 0/6 within, 12.42 (16.60) pp
- human/preserflo/subconjunctival_mims: next_observation 2/3 within, 5.23 (4.87) pp; landmark 1/3 within, 6.34 (8.60) pp; initial_state_group_mean 1/2 within, 7.51 (9.81) pp
- human/stent/angle_mims: next_observation 6/11 within, 6.21 (9.39) pp; landmark 6/11 within, 5.30 (11.36) pp; initial_state_group_mean 2/11 within, 10.67 (12.96) pp
- human/trabeculotomy/angle_mims: next_observation 19/43 within, 6.58 (6.30) pp; landmark 11/43 within, 7.44 (14.27) pp; initial_state_group_mean 7/20 within, 9.64 (15.43) pp
- human/valved/tube: next_observation 6/30 within, 6.54 (6.43) pp; landmark 4/30 within, 8.19 (16.93) pp; initial_state_group_mean 2/8 within, 11.54 (16.16) pp
- human/xen/subconjunctival_mims: next_observation 2/3 within, 8.06 (12.07) pp; landmark 2/3 within, 8.42 (13.34) pp; initial_state_group_mean 2/3 within, 9.63 (9.39) pp

Non-human series removed before the tested analysis (reported per series): 4 forecast series, 1 control initial-state series.

## H5: model 1.3 against the frozen model 1.2.0, same training

- next_observation: 1.3 6.64 against 1.2 6.67 pp on 548 common targets
- landmark: 1.3 7.92 against 1.2 8.00 pp on 548 common targets
- initial_state: 1.3 10.94 against 1.2 10.69 pp on 612 common targets
- first-month next-observation 80% coverage, reported (152 targets): 1.3 0.842 against 1.2 0.855
- first-month next-observation 90% coverage, reported (152 targets): 1.3 0.934 against 1.2 0.914
