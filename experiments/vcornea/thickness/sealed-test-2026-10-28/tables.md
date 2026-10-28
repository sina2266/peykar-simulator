# Corneal thickness model 1.0.0 — sealed results

## next_observation

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp |
|---|---|---|---|---|---|
| population_assimilation_t4 | 29 | 28 | 15 | 1.53 | 6.10 |
| analog_knn | 29 | 29 | 13 | 1.60 | 5.76 |
| population_assimilation_robust | 29 | 28 | 14 | 1.60 | 6.38 |
| population_assimilation | 29 | 28 | 15 | 1.64 | 6.27 |
| persistence | 29 | 29 | 0 | 2.06 | 6.35 |
| population_anchored_t4 | 29 | 27 | 13 | 2.37 | 9.36 |
| pooled_curve_ratio | 29 | 27 | 15 | 2.50 | 13.07 |
| population_anchored | 29 | 27 | 13 | 2.52 | 9.65 |
| weak_prior | 29 | 25 | 1 | 3.83 | 11.56 |
| preoperative_level | 29 | 28 | 8 | 6.05 | 18.15 |
| last_two_log_linear | 29 | 7 | 0 | 22.70 | 89.07 |
| reference_to_last_log_linear | 29 | 7 | 0 | 23.12 | 89.07 |

## landmark

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp |
|---|---|---|---|---|---|
| analog_knn | 29 | 29 | 16 | 1.47 | 4.79 |
| population_assimilation_t4 | 29 | 28 | 17 | 2.27 | 7.18 |
| population_assimilation | 29 | 28 | 17 | 2.30 | 7.17 |
| population_assimilation_robust | 29 | 28 | 17 | 2.31 | 7.21 |
| pooled_curve_ratio | 29 | 27 | 17 | 3.18 | 13.07 |
| population_anchored_t4 | 29 | 24 | 14 | 3.66 | 9.36 |
| population_anchored | 29 | 25 | 14 | 3.78 | 9.65 |
| persistence | 29 | 25 | 0 | 4.01 | 9.40 |
| preoperative_level | 29 | 28 | 16 | 6.05 | 18.15 |
| weak_prior | 29 | 12 | 2 | 9.07 | 22.74 |
| last_two_log_linear | 29 | 6 | 0 | 61.81 | 100.14 |
| reference_to_last_log_linear | 29 | 6 | 0 | 61.81 | 100.14 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp |
|---|---|---|---|---|
| population_initial_state | 21 | 18 | 5.17 | 23.54 |
| population_initial_state_t4 | 21 | 18 | 5.45 | 24.54 |
| pooled_training_curve | 21 | 18 | 6.82 | 18.05 |

## by family (primary methods)

- human/graft, next_observation: 0/3 meet criteria, 2 within 5/10 pp, pooled RMSE 3.07 pp; comparator 1.13 pp
- human/graft, landmark: 0/3 meet criteria, 2 within 5/10 pp, pooled RMSE 4.85 pp; comparator 4.11 pp
- human/graft, initial_state_group_mean: 1/3 meet criteria, 1 within 5/10 pp, pooled RMSE 10.13 pp; comparator 14.44 pp
- human/host, next_observation: 14/26 meet criteria, 26 within 5/10 pp, pooled RMSE 1.09 pp; comparator 2.20 pp
- human/host, landmark: 17/26 meet criteria, 26 within 5/10 pp, pooled RMSE 1.27 pp; comparator 3.99 pp
- human/host, initial_state_group_mean: 17/18 meet criteria, 17 within 5/10 pp, pooled RMSE 2.51 pp; comparator 1.68 pp

## calibration of primary methods

- next observation, 80% interval: 58/65 = 0.892 (window 0.70–0.90)
- next observation, 90% interval: 63/65 = 0.969 (window 0.80–0.97)
- landmark, 80% interval: 53/65 = 0.815 (window 0.70–0.90)
- landmark, 90% interval: 58/65 = 0.892 (window 0.80–0.97)
- initial state group means, 80% interval: 62/66 = 0.939
- initial state group means, 90% interval: 66/66 = 1.000
