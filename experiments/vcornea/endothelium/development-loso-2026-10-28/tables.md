# Endothelial density model 1.0.0 — development results

## next_observation

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp |
|---|---|---|---|---|---|
| population_anchored | 115 | 99 | 45 | 4.00 | 21.39 |
| population_anchored_t4 | 115 | 99 | 51 | 4.02 | 21.58 |
| population_assimilation_t4 | 115 | 95 | 47 | 4.28 | 21.80 |
| population_assimilation_robust | 115 | 96 | 47 | 4.29 | 21.81 |
| population_assimilation | 115 | 94 | 45 | 4.32 | 21.68 |
| flat_prior_grid | 117 | 92 | 42 | 4.91 | 26.30 |
| persistence | 117 | 91 | 0 | 5.14 | 24.93 |
| analog_knn | 115 | 87 | 33 | 5.28 | 27.40 |
| pooled_curve_ratio | 115 | 84 | 33 | 5.93 | 25.77 |
| last_two_log_linear | 117 | 17 | 6 | 19.65 | 93.22 |
| reference_to_last_log_linear | 117 | 19 | 5 | 21.32 | 84.42 |

## landmark

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp |
|---|---|---|---|---|---|
| population_anchored_t4 | 115 | 86 | 57 | 5.46 | 24.59 |
| population_anchored | 115 | 86 | 56 | 5.47 | 24.64 |
| population_assimilation_t4 | 115 | 85 | 57 | 5.98 | 25.46 |
| population_assimilation | 115 | 86 | 56 | 5.98 | 25.19 |
| population_assimilation_robust | 115 | 87 | 56 | 6.02 | 25.45 |
| flat_prior_grid | 117 | 80 | 44 | 7.73 | 31.41 |
| pooled_curve_ratio | 115 | 66 | 31 | 7.93 | 29.36 |
| analog_knn | 115 | 69 | 35 | 8.36 | 39.62 |
| persistence | 117 | 75 | 0 | 10.07 | 42.00 |
| last_two_log_linear | 117 | 17 | 7 | 40.69 | 89.70 |
| reference_to_last_log_linear | 117 | 17 | 7 | 40.69 | 89.70 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp |
|---|---|---|---|---|
| population_initial_state | 111 | 52 | 9.34 | 36.68 |
| pooled_training_curve | 111 | 52 | 9.45 | 34.74 |
| population_initial_state_t4 | 111 | 44 | 9.64 | 37.52 |

## calibration of primary methods

- next observation, 80% interval: 214/261 = 0.820 (window 0.70–0.90)
- next observation, 90% interval: 234/261 = 0.897 (window 0.80–0.97)
- landmark, 80% interval: 207/261 = 0.793 (window 0.70–0.90)
- landmark, 90% interval: 237/261 = 0.908 (window 0.80–0.97)
- initial state group means, 80% interval: 239/319 = 0.749
- initial state group means, 90% interval: 271/319 = 0.850
