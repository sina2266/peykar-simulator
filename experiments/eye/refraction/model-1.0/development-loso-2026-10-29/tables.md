# Refraction model 1.0.0-f1 — development results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_below_plano_target_control_group_means: pass
- H5_initial_state_lower_than_f0_forecasts_within_0_05_pp: pass

- H1 next observation 80% interval: 79/98 = 0.806 (window 0.70–0.90)
- H1 next observation 90% interval: 89/98 = 0.908 (window 0.80–0.97)
- H2 next observation vs persistence: 1.57 against 1.63 pp on 98 common targets
- H2 next observation vs last_two_log_linear: 1.57 against 20.90 pp on 98 common targets
- H2 next observation vs pooled_curve_ratio: 1.57 against 11.73 pp on 98 common targets
- H2 next observation vs analog_knn: 1.49 against 10.46 pp on 97 common targets
- H2 next observation vs target_level: 1.57 against 6.16 pp on 98 common targets
- H3 landmark vs persistence: 3.22 against 3.48 pp on 98 common targets
- H3 landmark vs pooled_curve_ratio: 3.22 against 19.37 pp on 98 common targets
- H3 landmark vs analog_knn: 2.60 against 19.60 pp on 97 common targets
- H3 landmark vs target_level: 3.22 against 6.16 pp on 98 common targets
- landmark 80% interval (reported): 87/98 = 0.888
- landmark 90% interval (reported): 92/98 = 0.939
- initial state (reported) population_initial_state: 5.38 pp, 35/38 control group means within 5/10 pp
- initial state (reported) pooled_training_curve: 22.14 pp, 4/38 control group means within 5/10 pp
- initial state (reported) 80% interval (reported): 116/124 = 0.935
- initial state (reported) 90% interval (reported): 117/124 = 0.944

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 43 | 43 | 4 | 1.57 | 5.22 | 98 |
| population_assimilation_t4 | 43 | 43 | 4 | 1.63 | 5.30 | 98 |
| persistence | 43 | 43 | 0 | 1.63 | 6.00 | 98 |
| population_anchored_t4 | 43 | 43 | 3 | 1.84 | 8.67 | 98 |
| population_anchored | 43 | 43 | 2 | 1.91 | 8.79 | 98 |
| population_assimilation | 43 | 42 | 4 | 2.59 | 13.04 | 98 |
| target_level | 43 | 41 | 6 | 6.16 | 32.20 | 98 |
| weak_prior | 43 | 28 | 1 | 6.25 | 27.73 | 98 |
| analog_knn | 43 | 31 | 1 | 10.46 | 54.13 | 97 |
| pooled_curve_ratio | 43 | 17 | 0 | 11.73 | 40.78 | 98 |
| last_two_log_linear | 43 | 8 | 0 | 20.90 | 121.70 | 98 |
| reference_to_last_log_linear | 43 | 8 | 0 | 23.04 | 121.70 | 98 |
| baseline_level | 43 | 0 | 0 | 52.76 | 88.00 | 98 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 43 | 42 | 7 | 3.22 | 19.00 | 98 |
| population_assimilation_t4 | 43 | 42 | 7 | 3.37 | 19.70 | 98 |
| persistence | 43 | 41 | 0 | 3.48 | 19.00 | 98 |
| population_anchored_t4 | 43 | 40 | 5 | 3.86 | 18.95 | 98 |
| population_anchored | 43 | 40 | 5 | 3.89 | 18.96 | 98 |
| population_assimilation | 43 | 42 | 7 | 4.13 | 24.01 | 98 |
| target_level | 43 | 41 | 8 | 6.16 | 32.20 | 98 |
| weak_prior | 43 | 22 | 1 | 12.06 | 38.45 | 98 |
| pooled_curve_ratio | 43 | 16 | 0 | 19.37 | 61.71 | 98 |
| analog_knn | 43 | 27 | 3 | 19.60 | 60.17 | 97 |
| last_two_log_linear | 43 | 8 | 0 | 43.71 | 232.80 | 98 |
| reference_to_last_log_linear | 43 | 8 | 0 | 43.71 | 232.80 | 98 |
| baseline_level | 43 | 0 | 0 | 52.76 | 88.00 | 98 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 43 | 43 | 4 | 1.57 | 5.22 | 98 |
| population_assimilation_t4 | 43 | 43 | 4 | 1.63 | 5.30 | 98 |
| persistence | 43 | 43 | 0 | 1.63 | 6.00 | 98 |
| population_anchored_t4 | 43 | 43 | 3 | 1.84 | 8.67 | 98 |
| population_anchored | 43 | 43 | 2 | 1.91 | 8.79 | 98 |
| population_assimilation | 43 | 42 | 4 | 2.59 | 13.04 | 98 |
| target_level | 43 | 41 | 6 | 6.16 | 32.20 | 98 |
| weak_prior | 43 | 28 | 1 | 6.25 | 27.73 | 98 |
| analog_knn | 43 | 31 | 1 | 10.46 | 54.13 | 97 |
| pooled_curve_ratio | 43 | 17 | 0 | 11.73 | 40.78 | 98 |
| last_two_log_linear | 43 | 8 | 0 | 20.90 | 121.70 | 98 |
| reference_to_last_log_linear | 43 | 8 | 0 | 23.04 | 121.70 | 98 |
| baseline_level | 43 | 0 | 0 | 52.76 | 88.00 | 98 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_robust | 43 | 42 | 7 | 3.22 | 19.00 | 98 |
| population_assimilation_t4 | 43 | 42 | 7 | 3.37 | 19.70 | 98 |
| persistence | 43 | 41 | 0 | 3.48 | 19.00 | 98 |
| population_anchored_t4 | 43 | 40 | 5 | 3.86 | 18.95 | 98 |
| population_anchored | 43 | 40 | 5 | 3.89 | 18.96 | 98 |
| population_assimilation | 43 | 42 | 7 | 4.13 | 24.01 | 98 |
| target_level | 43 | 41 | 8 | 6.16 | 32.20 | 98 |
| weak_prior | 43 | 22 | 1 | 12.06 | 38.45 | 98 |
| pooled_curve_ratio | 43 | 16 | 0 | 19.37 | 61.71 | 98 |
| analog_knn | 43 | 27 | 3 | 19.60 | 60.17 | 97 |
| last_two_log_linear | 43 | 8 | 0 | 43.71 | 232.80 | 98 |
| reference_to_last_log_linear | 43 | 8 | 0 | 43.71 | 232.80 | 98 |
| baseline_level | 43 | 0 | 0 | 52.76 | 88.00 | 98 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state | 38 | 35 | 5.38 | 28.53 | 124 |
| population_initial_state_t4 | 38 | 35 | 5.38 | 28.52 | 124 |
| target_level | 38 | 36 | 5.75 | 32.20 | 124 |
| pooled_training_curve | 38 | 4 | 22.14 | 75.96 | 124 |

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

- human/hyperopia/lasik: next_observation 5/5 within, 1.26 (1.23) pp; landmark 5/5 within, 1.45 (1.45) pp; initial_state_group_mean 5/5 within, 3.83 (9.36) pp
- human/hyperopia/surface_ablation: next_observation 1/1 within, 0.55 (0.50) pp; landmark 1/1 within, 0.55 (0.50) pp; initial_state_group_mean 1/1 within, 1.75 (65.53) pp
- human/myopia/lasik: next_observation 13/13 within, 2.44 (2.16) pp; landmark 12/13 within, 5.65 (5.50) pp; initial_state_group_mean 10/12 within, 8.67 (24.71) pp
- human/myopia/lenticule_extraction: next_observation 17/17 within, 0.81 (1.11) pp; landmark 17/17 within, 1.21 (2.03) pp; initial_state_group_mean 15/15 within, 2.32 (21.83) pp
- human/myopia/other: next_observation 1/1 within, 0.18 (0.20) pp; landmark 1/1 within, 0.15 (0.32) pp
- human/myopia/surface_ablation: next_observation 6/6 within, 1.59 (2.24) pp; landmark 6/6 within, 1.70 (3.29) pp; initial_state_group_mean 4/5 within, 3.73 (11.44) pp

Units: 1 pp = 0.1 D of spherical equivalent change from the preoperative SE.

## H4: population initial state against the plano target (control group means)

- 5.38 against 5.75 pp on 124 common targets

## H5: model 1.0 (f1) against f0, same training

- initial_state: f1 5.38 against f0 5.52 pp on 124 common targets
- next_observation: f1 1.57 against f0 1.53 pp on 98 common targets
- landmark: f1 3.22 against f0 3.26 pp on 98 common targets
