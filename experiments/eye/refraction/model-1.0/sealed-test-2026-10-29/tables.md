# Refraction model 1.0.0-f1 — sealed results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_below_plano_target_control_group_means: pass
- H5_initial_state_lower_than_f0_forecasts_within_0_05_pp: pass

- H1 next observation 80% interval: 23/28 = 0.821 (window 0.70–0.90)
- H1 next observation 90% interval: 24/28 = 0.857 (window 0.80–0.97)
- H2 next observation vs persistence: 2.93 against 3.70 pp on 28 common targets
- H2 next observation vs last_two_log_linear: 2.93 against 18.97 pp on 28 common targets
- H2 next observation vs pooled_curve_ratio: 2.93 against 16.63 pp on 28 common targets
- H2 next observation vs analog_knn: 2.93 against 8.00 pp on 28 common targets
- H2 next observation vs target_level: 2.93 against 5.24 pp on 28 common targets
- H3 landmark vs persistence: 4.68 against 7.80 pp on 28 common targets
- H3 landmark vs pooled_curve_ratio: 4.68 against 32.05 pp on 28 common targets
- H3 landmark vs analog_knn: 4.68 against 24.48 pp on 28 common targets
- H3 landmark vs target_level: 4.68 against 5.24 pp on 28 common targets
- landmark 80% interval (reported): 23/28 = 0.821
- landmark 90% interval (reported): 24/28 = 0.857
- initial state (reported) population_initial_state: 3.97 pp, 11/12 control group means within 5/10 pp
- initial state (reported) pooled_training_curve: 24.73 pp, 0/12 control group means within 5/10 pp
- initial state (reported) 80% interval (reported): 32/34 = 0.941
- initial state (reported) 90% interval (reported): 32/34 = 0.941

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 13 | 12 | 3 | 2.93 | 11.20 | 28 |
| population_assimilation | 13 | 12 | 3 | 2.93 | 11.17 | 28 |
| population_assimilation_t4 | 13 | 12 | 3 | 2.97 | 11.23 | 28 |
| population_anchored | 13 | 12 | 2 | 3.30 | 11.23 | 28 |
| population_anchored_t4 | 13 | 12 | 2 | 3.31 | 11.28 | 28 |
| persistence | 13 | 12 | 0 | 3.70 | 12.40 | 28 |
| target_level | 13 | 12 | 2 | 5.24 | 18.80 | 28 |
| weak_prior | 13 | 9 | 1 | 6.73 | 18.32 | 28 |
| analog_knn | 13 | 11 | 1 | 8.00 | 37.14 | 28 |
| pooled_curve_ratio | 13 | 4 | 0 | 16.63 | 40.86 | 28 |
| reference_to_last_log_linear | 13 | 2 | 0 | 17.44 | 35.43 | 28 |
| last_two_log_linear | 13 | 2 | 0 | 18.97 | 53.61 | 28 |
| baseline_level | 13 | 0 | 0 | 56.91 | 81.80 | 28 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 13 | 12 | 3 | 4.65 | 15.80 | 28 |
| population_assimilation_robust | 13 | 12 | 3 | 4.68 | 15.88 | 28 |
| population_assimilation_t4 | 13 | 12 | 3 | 4.72 | 15.99 | 28 |
| target_level | 13 | 12 | 3 | 5.24 | 18.80 | 28 |
| population_anchored | 13 | 12 | 3 | 6.24 | 20.85 | 28 |
| population_anchored_t4 | 13 | 12 | 3 | 6.26 | 20.91 | 28 |
| persistence | 13 | 10 | 0 | 7.80 | 25.40 | 28 |
| weak_prior | 13 | 8 | 1 | 11.20 | 29.33 | 28 |
| last_two_log_linear | 13 | 2 | 1 | 18.31 | 35.43 | 28 |
| reference_to_last_log_linear | 13 | 2 | 1 | 18.31 | 35.43 | 28 |
| analog_knn | 13 | 7 | 0 | 24.48 | 59.66 | 28 |
| pooled_curve_ratio | 13 | 4 | 0 | 32.05 | 67.25 | 28 |
| baseline_level | 13 | 0 | 0 | 56.91 | 81.80 | 28 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 13 | 12 | 3 | 2.93 | 11.20 | 28 |
| population_assimilation | 13 | 12 | 3 | 2.93 | 11.17 | 28 |
| population_assimilation_t4 | 13 | 12 | 3 | 2.97 | 11.23 | 28 |
| population_anchored | 13 | 12 | 2 | 3.30 | 11.23 | 28 |
| population_anchored_t4 | 13 | 12 | 2 | 3.31 | 11.28 | 28 |
| persistence | 13 | 12 | 0 | 3.70 | 12.40 | 28 |
| target_level | 13 | 12 | 2 | 5.24 | 18.80 | 28 |
| weak_prior | 13 | 9 | 1 | 6.73 | 18.32 | 28 |
| analog_knn | 13 | 11 | 1 | 8.00 | 37.14 | 28 |
| pooled_curve_ratio | 13 | 4 | 0 | 16.63 | 40.86 | 28 |
| reference_to_last_log_linear | 13 | 2 | 0 | 17.44 | 35.43 | 28 |
| last_two_log_linear | 13 | 2 | 0 | 18.97 | 53.61 | 28 |
| baseline_level | 13 | 0 | 0 | 56.91 | 81.80 | 28 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 13 | 12 | 3 | 4.65 | 15.80 | 28 |
| population_assimilation_robust | 13 | 12 | 3 | 4.68 | 15.88 | 28 |
| population_assimilation_t4 | 13 | 12 | 3 | 4.72 | 15.99 | 28 |
| target_level | 13 | 12 | 3 | 5.24 | 18.80 | 28 |
| population_anchored | 13 | 12 | 3 | 6.24 | 20.85 | 28 |
| population_anchored_t4 | 13 | 12 | 3 | 6.26 | 20.91 | 28 |
| persistence | 13 | 10 | 0 | 7.80 | 25.40 | 28 |
| weak_prior | 13 | 8 | 1 | 11.20 | 29.33 | 28 |
| last_two_log_linear | 13 | 2 | 1 | 18.31 | 35.43 | 28 |
| reference_to_last_log_linear | 13 | 2 | 1 | 18.31 | 35.43 | 28 |
| analog_knn | 13 | 7 | 0 | 24.48 | 59.66 | 28 |
| pooled_curve_ratio | 13 | 4 | 0 | 32.05 | 67.25 | 28 |
| baseline_level | 13 | 0 | 0 | 56.91 | 81.80 | 28 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 12 | 11 | 3.97 | 11.76 | 34 |
| population_initial_state | 12 | 11 | 3.97 | 11.73 | 34 |
| target_level | 12 | 11 | 5.01 | 18.80 | 34 |
| pooled_training_curve | 12 | 0 | 24.73 | 63.39 | 34 |

## single eyes (reported, not tested)

- next_observation vs persistence: n/a against n/a pp on 0 targets
- next_observation vs last_two_log_linear: n/a against n/a pp on 0 targets
- next_observation vs reference_to_last_log_linear: n/a against n/a pp on 0 targets
- next_observation vs pooled_curve_ratio: n/a against n/a pp on 0 targets
- next_observation vs analog_knn: n/a against n/a pp on 0 targets
- next_observation vs baseline_level: n/a against n/a pp on 0 targets
- next_observation vs target_level: n/a against n/a pp on 0 targets
- landmark vs persistence: n/a against n/a pp on 0 targets
- landmark vs last_two_log_linear: n/a against n/a pp on 0 targets
- landmark vs reference_to_last_log_linear: n/a against n/a pp on 0 targets
- landmark vs pooled_curve_ratio: n/a against n/a pp on 0 targets
- landmark vs analog_knn: n/a against n/a pp on 0 targets
- landmark vs baseline_level: n/a against n/a pp on 0 targets
- landmark vs target_level: n/a against n/a pp on 0 targets
- next_observation 80% interval: 0/0 = n/a
- next_observation 90% interval: 0/0 = n/a
- landmark 80% interval: 0/0 = n/a
- landmark 90% interval: 0/0 = n/a

## by procedure context (primary methods; comparator in parentheses)

- human/myopia/lasik: next_observation 8/9 within, 3.49 (4.46) pp; landmark 8/9 within, 5.61 (9.43) pp; initial_state_group_mean 8/9 within, 4.34 (26.32) pp
- human/myopia/lenticule_extraction: next_observation 3/3 within, 1.03 (0.80) pp; landmark 3/3 within, 1.48 (1.42) pp; initial_state_group_mean 2/2 within, 2.46 (20.15) pp
- human/myopia/surface_ablation: next_observation 1/1 within, 0.30 (0.41) pp; landmark 1/1 within, 0.29 (0.38) pp; initial_state_group_mean 1/1 within, 3.13 (18.53) pp

Units: 1 pp = 0.1 D of spherical equivalent change from the preoperative SE.

## H4: population initial state against the plano target (control group means)

- 3.97 against 5.01 pp on 34 common targets

## H5: model 1.0 (f1) against f0, same training

- initial_state: f1 3.97 against f0 4.13 pp on 34 common targets
- next_observation: f1 2.93 against f0 2.90 pp on 28 common targets
- landmark: f1 4.68 against f0 4.76 pp on 28 common targets
