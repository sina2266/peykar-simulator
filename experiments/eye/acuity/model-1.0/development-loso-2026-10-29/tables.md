# Visual-acuity model 1.0.0-a0 — development results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: FAIL
- H3_landmark_group_means: FAIL
- H4_initial_state_control_group_means: pass

- H1 next observation 80% interval: 100/118 = 0.847 (window 0.70–0.90)
- H1 next observation 90% interval: 105/118 = 0.890 (window 0.80–0.97)
- H2 next observation vs persistence: 5.43 against 6.12 pp on 118 common targets
- H2 next observation vs last_two_log_linear: 5.43 against 24.81 pp on 118 common targets
- H2 next observation vs pooled_curve_ratio: 5.43 against 5.13 pp on 118 common targets
- H2 next observation vs analog_knn: 5.43 against 9.29 pp on 118 common targets
- H3 landmark vs persistence: 7.59 against 10.59 pp on 118 common targets
- H3 landmark vs pooled_curve_ratio: 7.59 against 7.18 pp on 118 common targets
- H3 landmark vs analog_knn: 7.59 against 15.60 pp on 118 common targets
- landmark 80% interval (reported): 106/118 = 0.898
- landmark 90% interval (reported): 108/118 = 0.915
- H4 initial state population_initial_state: 15.92 pp, 20/43 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 16.80 pp, 20/43 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 130/155 = 0.839
- H4 initial state 90% interval (reported): 134/155 = 0.865

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| pooled_curve_ratio | 38 | 28 | 13 | 5.13 | 21.22 | 118 |
| population_assimilation_robust | 38 | 29 | 11 | 5.43 | 25.06 | 118 |
| population_anchored_t4 | 38 | 27 | 12 | 5.59 | 24.87 | 118 |
| population_assimilation_t4 | 38 | 29 | 11 | 5.66 | 25.09 | 118 |
| population_anchored | 38 | 24 | 12 | 5.70 | 24.94 | 118 |
| persistence | 38 | 24 | 0 | 6.12 | 30.50 | 118 |
| population_assimilation | 38 | 29 | 11 | 6.54 | 25.58 | 118 |
| weak_prior | 38 | 23 | 1 | 6.99 | 37.70 | 118 |
| analog_knn | 38 | 23 | 6 | 9.29 | 50.60 | 118 |
| last_two_log_linear | 38 | 12 | 1 | 24.81 | 106.76 | 118 |
| baseline_level | 38 | 13 | 2 | 28.06 | 71.50 | 118 |
| reference_to_last_log_linear | 38 | 14 | 1 | 33.39 | 147.50 | 118 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| pooled_curve_ratio | 38 | 25 | 15 | 7.18 | 30.26 | 118 |
| population_assimilation_robust | 38 | 24 | 13 | 7.59 | 34.93 | 118 |
| population_assimilation_t4 | 38 | 23 | 13 | 8.24 | 34.89 | 118 |
| population_anchored_t4 | 38 | 20 | 10 | 8.30 | 30.41 | 118 |
| population_anchored | 38 | 20 | 10 | 8.41 | 30.30 | 118 |
| population_assimilation | 38 | 23 | 12 | 9.62 | 42.02 | 118 |
| persistence | 38 | 23 | 0 | 10.59 | 30.50 | 118 |
| weak_prior | 38 | 22 | 0 | 12.06 | 37.70 | 118 |
| analog_knn | 38 | 19 | 9 | 15.60 | 50.60 | 118 |
| baseline_level | 38 | 13 | 2 | 28.06 | 71.50 | 118 |
| last_two_log_linear | 38 | 8 | 3 | 169.90 | 847.00 | 118 |
| reference_to_last_log_linear | 38 | 8 | 3 | 169.90 | 847.00 | 118 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_anchored_t4 | 55 | 32 | 16 | 7.88 | 57.40 | 165 |
| population_anchored | 55 | 29 | 16 | 7.89 | 56.68 | 165 |
| population_assimilation_robust | 55 | 34 | 14 | 8.05 | 58.83 | 165 |
| persistence | 55 | 27 | 0 | 8.08 | 55.00 | 165 |
| population_assimilation_t4 | 55 | 34 | 14 | 8.10 | 58.88 | 165 |
| population_assimilation | 55 | 34 | 14 | 8.63 | 58.57 | 165 |
| weak_prior | 55 | 27 | 2 | 8.98 | 57.36 | 165 |
| pooled_curve_ratio | 55 | 31 | 15 | 9.35 | 56.42 | 165 |
| analog_knn | 55 | 28 | 10 | 9.99 | 60.00 | 165 |
| last_two_log_linear | 55 | 12 | 1 | 38.04 | 165.00 | 165 |
| baseline_level | 55 | 14 | 3 | 40.83 | 100.00 | 165 |
| reference_to_last_log_linear | 55 | 14 | 1 | 49.05 | 165.00 | 165 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_anchored_t4 | 55 | 24 | 14 | 10.02 | 39.39 | 165 |
| population_anchored | 55 | 24 | 14 | 10.04 | 39.56 | 165 |
| population_assimilation_robust | 55 | 28 | 16 | 10.37 | 49.74 | 165 |
| population_assimilation_t4 | 55 | 27 | 16 | 10.63 | 48.34 | 165 |
| population_assimilation | 55 | 27 | 15 | 11.54 | 50.73 | 165 |
| persistence | 55 | 27 | 0 | 12.23 | 35.00 | 165 |
| pooled_curve_ratio | 55 | 27 | 17 | 12.90 | 55.67 | 165 |
| weak_prior | 55 | 24 | 1 | 13.66 | 38.11 | 165 |
| analog_knn | 55 | 24 | 13 | 15.56 | 58.21 | 165 |
| baseline_level | 55 | 14 | 3 | 40.83 | 100.00 | 165 |
| last_two_log_linear | 55 | 8 | 3 | 235.79 | 930.00 | 165 |
| reference_to_last_log_linear | 55 | 8 | 3 | 235.79 | 930.00 | 165 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 43 | 20 | 15.88 | 55.79 | 155 |
| population_initial_state | 43 | 20 | 15.92 | 55.88 | 155 |
| pooled_training_curve | 43 | 20 | 16.80 | 59.45 | 155 |

## single eyes (reported, not tested)

- next_observation vs persistence: 12.38 against 11.62 pp on 47 targets
- next_observation vs last_two_log_linear: 12.38 against 59.46 pp on 47 targets
- next_observation vs reference_to_last_log_linear: 12.38 against 75.16 pp on 47 targets
- next_observation vs pooled_curve_ratio: 12.38 against 15.53 pp on 47 targets
- next_observation vs analog_knn: 12.38 against 11.56 pp on 47 targets
- next_observation vs baseline_level: 12.38 against 62.26 pp on 47 targets
- landmark vs persistence: 15.25 against 15.59 pp on 47 targets
- landmark vs last_two_log_linear: 15.25 against 350.30 pp on 47 targets
- landmark vs reference_to_last_log_linear: 15.25 against 350.30 pp on 47 targets
- landmark vs pooled_curve_ratio: 15.25 against 21.32 pp on 47 targets
- landmark vs analog_knn: 15.25 against 15.48 pp on 47 targets
- landmark vs baseline_level: 15.25 against 62.26 pp on 47 targets
- next_observation 80% interval: 33/47 = 0.702
- next_observation 90% interval: 37/47 = 0.787
- landmark 80% interval: 27/47 = 0.574
- landmark 90% interval: 33/47 = 0.702

## by procedure context (primary methods; comparator in parentheses)

- human/cxl: next_observation 17/17 within, 3.32 (3.22) pp; landmark 10/17 within, 4.71 (4.89) pp; initial_state_group_mean 11/19 within, 6.68 (3.47) pp
- human/dalk: initial_state_group_mean 0/1 within, 34.60 (15.42) pp
- human/dmek: next_observation 4/9 within, 6.96 (8.94) pp; landmark 3/9 within, 10.88 (14.34) pp; initial_state_group_mean 1/2 within, 5.72 (37.59) pp
- human/dsaek: next_observation 5/12 within, 6.47 (7.90) pp; landmark 8/12 within, 8.70 (15.29) pp; initial_state_group_mean 3/12 within, 20.66 (18.75) pp
- human/other: next_observation 4/4 within, 2.66 (2.71) pp; landmark 3/4 within, 3.85 (5.12) pp; initial_state_group_mean 3/4 within, 5.85 (6.42) pp
- human/pk: next_observation 4/13 within, 13.36 (11.66) pp; landmark 4/13 within, 16.02 (13.89) pp; initial_state_group_mean 2/5 within, 28.16 (18.09) pp

Units: 1 pp = 1 ETDRS letter (0.02 logMAR) of corrected distance acuity change from the preoperative acuity.
