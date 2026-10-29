# Retinal visual-acuity model 1.0.0-v3 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: FAIL
- H2_accuracy_next_observation_group_means: FAIL
- H3_landmark_group_means: FAIL
- H4_initial_state_control_group_means: FAIL

- H1 next observation 80% interval: 21/31 = 0.677 (window 0.70–0.90)
- H1 next observation 90% interval: 25/31 = 0.806 (window 0.80–0.97)
- H2 next observation vs persistence: 6.14 against 4.81 pp on 31 common targets
- H2 next observation vs last_two_log_linear: 6.14 against 12.61 pp on 31 common targets
- H2 next observation vs pooled_curve_ratio: 6.14 against 5.14 pp on 31 common targets
- H2 next observation vs analog_knn: 6.14 against 12.40 pp on 31 common targets
- H3 landmark vs persistence: 9.17 against 6.49 pp on 31 common targets
- H3 landmark vs pooled_curve_ratio: 9.17 against 8.07 pp on 31 common targets
- H3 landmark vs analog_knn: 9.17 against 18.66 pp on 31 common targets
- landmark 80% interval (reported): 20/31 = 0.645
- landmark 90% interval (reported): 23/31 = 0.742
- H4 initial state population_initial_state: 16.40 pp, 5/9 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 5.18 pp, 5/9 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 25/34 = 0.735
- H4 initial state 90% interval (reported): 31/34 = 0.912

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| persistence | 11 | 6 | 0 | 4.81 | 12.50 | 31 |
| weak_prior | 11 | 6 | 0 | 5.06 | 12.31 | 31 |
| pooled_curve_ratio | 11 | 6 | 2 | 5.14 | 16.86 | 31 |
| population_assimilation | 11 | 5 | 2 | 6.10 | 22.61 | 31 |
| population_assimilation_t4 | 11 | 5 | 3 | 6.14 | 22.99 | 31 |
| population_assimilation_robust | 11 | 5 | 2 | 6.51 | 24.84 | 31 |
| baseline_level | 11 | 3 | 2 | 8.13 | 13.50 | 31 |
| reference_to_last_log_linear | 11 | 5 | 3 | 11.95 | 41.90 | 31 |
| analog_knn | 11 | 5 | 2 | 12.40 | 41.51 | 31 |
| last_two_log_linear | 11 | 5 | 3 | 12.61 | 41.90 | 31 |
| population_anchored | 11 | 5 | 1 | 27.07 | 109.80 | 31 |
| population_anchored_t4 | 11 | 5 | 1 | 27.09 | 109.79 | 31 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| weak_prior | 11 | 7 | 0 | 6.37 | 17.26 | 31 |
| persistence | 11 | 7 | 0 | 6.49 | 17.50 | 31 |
| pooled_curve_ratio | 11 | 6 | 3 | 8.07 | 19.96 | 31 |
| baseline_level | 11 | 3 | 1 | 8.13 | 13.50 | 31 |
| population_assimilation | 11 | 6 | 3 | 8.91 | 22.61 | 31 |
| population_assimilation_t4 | 11 | 6 | 3 | 9.17 | 22.99 | 31 |
| population_assimilation_robust | 11 | 6 | 3 | 9.75 | 24.84 | 31 |
| analog_knn | 11 | 5 | 3 | 18.66 | 46.01 | 31 |
| population_anchored_t4 | 11 | 6 | 2 | 46.88 | 111.60 | 31 |
| population_anchored | 11 | 6 | 2 | 46.88 | 111.60 | 31 |
| last_two_log_linear | 11 | 5 | 4 | 48.43 | 194.37 | 31 |
| reference_to_last_log_linear | 11 | 5 | 4 | 48.43 | 194.37 | 31 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| persistence | 11 | 6 | 0 | 4.81 | 12.50 | 31 |
| weak_prior | 11 | 6 | 0 | 5.06 | 12.31 | 31 |
| pooled_curve_ratio | 11 | 6 | 2 | 5.14 | 16.86 | 31 |
| population_assimilation | 11 | 5 | 2 | 6.10 | 22.61 | 31 |
| population_assimilation_t4 | 11 | 5 | 3 | 6.14 | 22.99 | 31 |
| population_assimilation_robust | 11 | 5 | 2 | 6.51 | 24.84 | 31 |
| baseline_level | 11 | 3 | 2 | 8.13 | 13.50 | 31 |
| reference_to_last_log_linear | 11 | 5 | 3 | 11.95 | 41.90 | 31 |
| analog_knn | 11 | 5 | 2 | 12.40 | 41.51 | 31 |
| last_two_log_linear | 11 | 5 | 3 | 12.61 | 41.90 | 31 |
| population_anchored | 11 | 5 | 1 | 27.07 | 109.80 | 31 |
| population_anchored_t4 | 11 | 5 | 1 | 27.09 | 109.79 | 31 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| weak_prior | 11 | 7 | 0 | 6.37 | 17.26 | 31 |
| persistence | 11 | 7 | 0 | 6.49 | 17.50 | 31 |
| pooled_curve_ratio | 11 | 6 | 3 | 8.07 | 19.96 | 31 |
| baseline_level | 11 | 3 | 1 | 8.13 | 13.50 | 31 |
| population_assimilation | 11 | 6 | 3 | 8.91 | 22.61 | 31 |
| population_assimilation_t4 | 11 | 6 | 3 | 9.17 | 22.99 | 31 |
| population_assimilation_robust | 11 | 6 | 3 | 9.75 | 24.84 | 31 |
| analog_knn | 11 | 5 | 3 | 18.66 | 46.01 | 31 |
| population_anchored_t4 | 11 | 6 | 2 | 46.88 | 111.60 | 31 |
| population_anchored | 11 | 6 | 2 | 46.88 | 111.60 | 31 |
| last_two_log_linear | 11 | 5 | 4 | 48.43 | 194.37 | 31 |
| reference_to_last_log_linear | 11 | 5 | 4 | 48.43 | 194.37 | 31 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| pooled_training_curve | 9 | 5 | 5.18 | 15.94 | 34 |
| population_initial_state_t4 | 9 | 5 | 16.39 | 91.47 | 34 |
| population_initial_state | 9 | 5 | 16.40 | 91.47 | 34 |

## single eyes (reported, not tested)

- next_observation vs persistence: n/a against n/a pp on 0 targets
- next_observation vs last_two_log_linear: n/a against n/a pp on 0 targets
- next_observation vs reference_to_last_log_linear: n/a against n/a pp on 0 targets
- next_observation vs pooled_curve_ratio: n/a against n/a pp on 0 targets
- next_observation vs analog_knn: n/a against n/a pp on 0 targets
- next_observation vs baseline_level: n/a against n/a pp on 0 targets
- landmark vs persistence: n/a against n/a pp on 0 targets
- landmark vs last_two_log_linear: n/a against n/a pp on 0 targets
- landmark vs reference_to_last_log_linear: n/a against n/a pp on 0 targets
- landmark vs pooled_curve_ratio: n/a against n/a pp on 0 targets
- landmark vs analog_knn: n/a against n/a pp on 0 targets
- landmark vs baseline_level: n/a against n/a pp on 0 targets
- next_observation 80% interval: 0/0 = n/a
- next_observation 90% interval: 0/0 = n/a
- landmark 80% interval: 0/0 = n/a
- landmark 90% interval: 0/0 = n/a

## by procedure context (primary methods; comparator in parentheses)

- human/dme/anti_vegf: next_observation 4/6 within, 2.88 (3.64) pp; landmark 4/6 within, 2.82 (3.93) pp; initial_state_group_mean 5/6 within, 3.14 (3.04) pp
- human/dme/steroid_implant: next_observation 0/2 within, 6.70 (7.33) pp; landmark 1/2 within, 9.01 (10.09) pp; initial_state_group_mean 0/2 within, 6.32 (8.17) pp
- human/other/combination: next_observation 0/1 within, 13.33 (7.22) pp; landmark 0/1 within, 22.53 (12.84) pp
- human/other/laser: next_observation 0/1 within, 9.23 (3.86) pp; landmark 0/1 within, 13.31 (3.29) pp; initial_state_group_mean 0/1 within, 46.46 (7.62) pp
- human/rvo/combination: next_observation 1/1 within, 2.22 (2.72) pp; landmark 1/1 within, 1.72 (1.78) pp

Units: 1 pp = 1 ETDRS letter (0.02 logMAR) of best-corrected acuity change from the pretreatment acuity.
