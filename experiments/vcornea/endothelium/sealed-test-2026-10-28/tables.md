# Endothelial density model 1.0.0 — sealed results

## next_observation

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp |
|---|---|---|---|---|---|
| analog_knn | 30 | 21 | 13 | 5.35 | 16.14 |
| population_anchored | 30 | 23 | 14 | 5.58 | 19.42 |
| population_anchored_t4 | 30 | 23 | 14 | 5.60 | 19.51 |
| population_assimilation | 30 | 23 | 15 | 6.06 | 19.38 |
| population_assimilation_t4 | 30 | 23 | 15 | 6.08 | 19.41 |
| population_assimilation_robust | 30 | 23 | 15 | 6.09 | 19.45 |
| flat_prior_grid | 30 | 19 | 9 | 6.98 | 20.92 |
| pooled_curve_ratio | 30 | 22 | 13 | 7.00 | 21.56 |
| persistence | 30 | 17 | 0 | 7.36 | 22.65 |
| last_two_log_linear | 30 | 5 | 2 | 15.50 | 54.55 |
| reference_to_last_log_linear | 30 | 4 | 2 | 17.80 | 54.55 |

## landmark

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp |
|---|---|---|---|---|---|
| population_anchored_t4 | 30 | 19 | 10 | 6.85 | 18.23 |
| population_anchored | 30 | 18 | 9 | 6.89 | 18.70 |
| analog_knn | 30 | 18 | 12 | 7.30 | 20.18 |
| population_assimilation_t4 | 30 | 17 | 8 | 7.81 | 23.68 |
| population_assimilation | 30 | 17 | 8 | 7.84 | 23.36 |
| population_assimilation_robust | 30 | 17 | 8 | 7.86 | 23.87 |
| pooled_curve_ratio | 30 | 22 | 12 | 9.40 | 29.69 |
| flat_prior_grid | 30 | 17 | 9 | 9.42 | 30.58 |
| persistence | 30 | 13 | 0 | 12.41 | 34.52 |
| last_two_log_linear | 30 | 5 | 3 | 37.50 | 76.62 |
| reference_to_last_log_linear | 30 | 5 | 3 | 37.50 | 76.62 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp |
|---|---|---|---|---|
| population_initial_state | 29 | 13 | 7.05 | 23.30 |
| population_initial_state_t4 | 29 | 14 | 7.23 | 24.22 |
| pooled_training_curve | 29 | 13 | 8.16 | 22.72 |

## calibration of primary methods

- next observation, 80% interval: 54/76 = 0.711 (window 0.70–0.90)
- next observation, 90% interval: 63/76 = 0.829 (window 0.80–0.97)
- landmark, 80% interval: 55/76 = 0.724 (window 0.70–0.90)
- landmark, 90% interval: 69/76 = 0.908 (window 0.80–0.97)
- initial state group means, 80% interval: 84/99 = 0.848
- initial state group means, 90% interval: 91/99 = 0.919
