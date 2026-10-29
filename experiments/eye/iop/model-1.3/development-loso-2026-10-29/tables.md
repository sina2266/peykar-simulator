# IOP-after-surgery model 1.3.0-d1 — development results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_landmark_and_initial_state_lower_next_observation_within_0_05_pp: pass

- H1 next observation 80% interval: 1163/1467 = 0.793 (window 0.70–0.90)
- H1 next observation 90% interval: 1317/1467 = 0.898 (window 0.80–0.97)
- H2 next observation vs persistence: 6.38 against 7.77 pp on 1467 common targets
- H2 next observation vs last_two_log_linear: 6.38 against 38.77 pp on 1467 common targets
- H2 next observation vs pooled_curve_ratio: 6.38 against 8.77 pp on 1467 common targets
- H2 next observation vs analog_knn: 6.38 against 10.20 pp on 1463 common targets
- H3 landmark vs persistence: 8.06 against 13.62 pp on 1467 common targets
- H3 landmark vs pooled_curve_ratio: 8.06 against 15.73 pp on 1467 common targets
- H3 landmark vs analog_knn: 8.05 against 16.73 pp on 1463 common targets
- landmark 80% interval (reported): 1125/1467 = 0.767
- landmark 90% interval (reported): 1280/1467 = 0.873
- H4 initial state population_initial_state: 10.07 pp, 92/334 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 16.91 pp, 42/337 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 1357/1651 = 0.822
- H4 initial state 90% interval (reported): 1496/1651 = 0.906

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 366 | 209 | 100 | 6.38 | 39.89 | 1467 |
| population_assimilation_t4 | 366 | 208 | 95 | 6.42 | 39.93 | 1467 |
| population_assimilation_robust | 366 | 205 | 92 | 6.49 | 40.16 | 1467 |
| persistence | 369 | 161 | 0 | 7.75 | 47.13 | 1485 |
| population_anchored | 366 | 171 | 56 | 8.62 | 54.52 | 1467 |
| population_anchored_t4 | 366 | 169 | 54 | 8.66 | 55.20 | 1467 |
| weak_prior | 369 | 139 | 40 | 8.67 | 60.51 | 1485 |
| pooled_curve_ratio | 369 | 135 | 57 | 8.75 | 85.57 | 1485 |
| analog_knn | 369 | 122 | 49 | 10.25 | 70.81 | 1481 |
| reference_to_last_log_linear | 369 | 5 | 4 | 33.71 | 209.09 | 1485 |
| last_two_log_linear | 369 | 3 | 3 | 38.60 | 260.35 | 1485 |
| baseline_level | 369 | 4 | 4 | 40.59 | 79.66 | 1485 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 366 | 160 | 98 | 8.06 | 39.89 | 1467 |
| population_assimilation_t4 | 366 | 162 | 101 | 8.07 | 39.93 | 1467 |
| population_assimilation_robust | 366 | 161 | 97 | 8.17 | 40.16 | 1467 |
| persistence | 369 | 116 | 0 | 13.61 | 47.13 | 1485 |
| population_anchored | 366 | 119 | 34 | 15.09 | 82.72 | 1467 |
| population_anchored_t4 | 366 | 119 | 33 | 15.10 | 82.70 | 1467 |
| pooled_curve_ratio | 369 | 109 | 62 | 15.72 | 63.73 | 1485 |
| weak_prior | 369 | 100 | 29 | 16.60 | 51.03 | 1485 |
| analog_knn | 369 | 64 | 36 | 16.95 | 70.81 | 1481 |
| baseline_level | 369 | 4 | 2 | 40.59 | 79.66 | 1485 |
| last_two_log_linear | 369 | 5 | 3 | 65.38 | 232.72 | 1485 |
| reference_to_last_log_linear | 369 | 5 | 3 | 65.38 | 232.72 | 1485 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 488 | 217 | 96 | 9.30 | 74.51 | 1930 |
| population_assimilation_t4 | 488 | 221 | 100 | 9.30 | 76.49 | 1930 |
| population_assimilation | 488 | 225 | 108 | 9.74 | 89.73 | 1930 |
| persistence | 491 | 188 | 0 | 11.21 | 133.33 | 1948 |
| weak_prior | 491 | 162 | 49 | 11.26 | 92.94 | 1948 |
| population_anchored_t4 | 488 | 193 | 58 | 11.68 | 122.21 | 1930 |
| population_anchored | 488 | 195 | 60 | 11.68 | 122.18 | 1930 |
| pooled_curve_ratio | 491 | 153 | 62 | 11.87 | 132.62 | 1948 |
| analog_knn | 491 | 139 | 53 | 12.46 | 102.44 | 1944 |
| reference_to_last_log_linear | 491 | 6 | 5 | 36.03 | 246.67 | 1948 |
| baseline_level | 491 | 4 | 4 | 41.99 | 88.89 | 1948 |
| last_two_log_linear | 491 | 4 | 4 | 46.69 | 282.22 | 1948 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 488 | 174 | 104 | 11.32 | 96.63 | 1930 |
| population_assimilation_t4 | 488 | 175 | 106 | 11.38 | 95.68 | 1930 |
| population_assimilation_robust | 488 | 173 | 101 | 11.47 | 93.22 | 1930 |
| weak_prior | 491 | 122 | 38 | 18.39 | 92.94 | 1948 |
| persistence | 491 | 139 | 0 | 19.31 | 166.67 | 1948 |
| analog_knn | 491 | 79 | 42 | 19.33 | 105.79 | 1944 |
| population_anchored_t4 | 488 | 142 | 36 | 20.41 | 147.47 | 1930 |
| population_anchored | 488 | 142 | 37 | 20.41 | 147.43 | 1930 |
| pooled_curve_ratio | 491 | 122 | 64 | 20.43 | 141.50 | 1948 |
| baseline_level | 491 | 4 | 2 | 41.99 | 88.89 | 1948 |
| last_two_log_linear | 491 | 6 | 4 | 68.41 | 250.00 | 1948 |
| reference_to_last_log_linear | 491 | 6 | 4 | 68.41 | 250.00 | 1948 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 334 | 92 | 10.06 | 53.96 | 1651 |
| population_initial_state | 334 | 92 | 10.07 | 53.96 | 1651 |
| pooled_training_curve | 337 | 42 | 16.91 | 69.46 | 1672 |

## single eyes (reported, not tested)

- next_observation vs persistence: 16.33 against 18.33 pp on 463 targets
- next_observation vs last_two_log_linear: 16.33 against 66.27 pp on 463 targets
- next_observation vs reference_to_last_log_linear: 16.33 against 42.63 pp on 463 targets
- next_observation vs pooled_curve_ratio: 16.33 against 18.64 pp on 463 targets
- next_observation vs analog_knn: 16.33 against 17.77 pp on 463 targets
- next_observation vs preoperative_level: n/a against n/a pp on 0 targets
- landmark vs persistence: 18.11 against 31.20 pp on 463 targets
- landmark vs last_two_log_linear: 18.11 against 77.34 pp on 463 targets
- landmark vs reference_to_last_log_linear: 18.11 against 77.34 pp on 463 targets
- landmark vs pooled_curve_ratio: 18.11 against 31.05 pp on 463 targets
- landmark vs analog_knn: 18.11 against 25.50 pp on 463 targets
- landmark vs preoperative_level: n/a against n/a pp on 0 targets
- next_observation 80% interval: 392/463 = 0.847
- next_observation 90% interval: 432/463 = 0.933
- landmark 80% interval: 377/463 = 0.814
- landmark 90% interval: 425/463 = 0.918

## by procedure context (primary methods; comparator in parentheses)

- human/all/cataract: next_observation 13/21 within, 7.06 (10.32) pp; landmark 11/21 within, 8.70 (12.85) pp; initial_state_group_mean 3/18 within, 13.21 (25.41) pp
- human/all/laser_trabeculoplasty: next_observation 0/2 within, 12.41 (8.28) pp; landmark 0/2 within, 23.36 (17.45) pp; initial_state_group_mean 0/2 within, 25.66 (22.65) pp
- human/all/nonpenetrating: next_observation 14/25 within, 5.76 (6.87) pp; landmark 11/25 within, 8.48 (14.62) pp; initial_state_group_mean 5/21 within, 10.93 (20.04) pp
- human/all/other: next_observation 7/25 within, 8.47 (10.08) pp; landmark 5/25 within, 9.36 (11.73) pp; initial_state_group_mean 1/4 within, 6.87 (41.23) pp
- human/all/trabeculectomy: next_observation 33/56 within, 7.07 (8.62) pp; landmark 17/56 within, 9.45 (13.01) pp; initial_state_group_mean 7/42 within, 12.09 (15.87) pp
- human/canaloplasty/angle_mims: next_observation 7/14 within, 6.31 (7.74) pp; landmark 9/14 within, 5.84 (16.19) pp; initial_state_group_mean 7/15 within, 8.27 (20.26) pp
- human/continuous/cyclodestruction: next_observation 9/19 within, 7.80 (11.49) pp; landmark 6/19 within, 8.35 (16.82) pp; initial_state_group_mean 0/8 within, 13.58 (12.45) pp
- human/endoscopic/cyclodestruction: next_observation 2/3 within, 4.09 (4.27) pp; landmark 2/3 within, 6.66 (15.62) pp; initial_state_group_mean 2/4 within, 6.74 (19.56) pp
- human/goniotomy/angle_mims: next_observation 30/49 within, 7.11 (8.26) pp; landmark 26/49 within, 7.74 (12.67) pp; initial_state_group_mean 21/50 within, 8.14 (12.47) pp
- human/micropulse/cyclodestruction: next_observation 6/23 within, 9.08 (9.61) pp; landmark 6/23 within, 11.86 (13.10) pp; initial_state_group_mean 5/22 within, 12.82 (11.34) pp
- human/non_valved/tube: next_observation 7/26 within, 11.14 (13.72) pp; landmark 5/26 within, 11.95 (19.59) pp; initial_state_group_mean 5/13 within, 11.72 (20.59) pp
- human/other/angle_mims: next_observation 5/8 within, 4.95 (5.93) pp; landmark 3/8 within, 6.81 (17.53) pp; initial_state_group_mean 1/5 within, 8.68 (23.72) pp
- human/preserflo/subconjunctival_mims: next_observation 2/12 within, 9.28 (10.43) pp; landmark 2/12 within, 14.99 (20.43) pp; initial_state_group_mean 0/7 within, 9.71 (14.22) pp
- human/stent/angle_mims: next_observation 18/47 within, 12.83 (20.11) pp; landmark 17/47 within, 13.80 (38.87) pp; initial_state_group_mean 10/35 within, 7.92 (10.07) pp
- human/trabeculotomy/angle_mims: next_observation 43/82 within, 12.46 (9.50) pp; landmark 31/82 within, 13.91 (14.51) pp; initial_state_group_mean 17/57 within, 9.19 (16.57) pp
- human/ultrasound/cyclodestruction: next_observation 3/4 within, 5.72 (8.79) pp; landmark 2/4 within, 7.44 (12.01) pp; initial_state_group_mean 1/4 within, 8.74 (7.83) pp
- human/valved/tube: next_observation 15/29 within, 6.48 (7.00) pp; landmark 14/29 within, 7.45 (9.54) pp; initial_state_group_mean 5/12 within, 7.40 (16.21) pp
- human/xen/subconjunctival_mims: next_observation 11/43 within, 11.52 (12.69) pp; landmark 7/43 within, 13.27 (22.62) pp; initial_state_group_mean 2/15 within, 8.39 (16.68) pp

Non-human series removed before the tested analysis (reported per series): 20 forecast series, 15 control initial-state series.

## H5: model 1.3 against the frozen model 1.2.0, same training

- next_observation: 1.3 6.38 against 1.2 6.46 pp on 1467 common targets
- landmark: 1.3 8.06 against 1.2 8.35 pp on 1467 common targets
- initial_state: 1.3 10.07 against 1.2 10.21 pp on 1651 common targets
- first-month next-observation 80% coverage, reported (350 targets): 1.3 0.826 against 1.2 0.820
- first-month next-observation 90% coverage, reported (350 targets): 1.3 0.914 against 1.2 0.909
