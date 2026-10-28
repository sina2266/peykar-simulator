# Corneal thickness model 1.0.0 — development results

## next_observation

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp |
|---|---|---|---|---|---|
| population_assimilation_robust | 66 | 56 | 40 | 4.67 | 34.17 |
| population_assimilation_t4 | 66 | 56 | 40 | 4.77 | 34.17 |
| population_assimilation | 66 | 58 | 42 | 5.07 | 33.66 |
| population_anchored | 66 | 55 | 27 | 5.08 | 36.73 |
| population_anchored_t4 | 66 | 56 | 27 | 5.10 | 36.87 |
| persistence | 68 | 45 | 0 | 6.70 | 34.25 |
| analog_knn | 64 | 42 | 30 | 6.90 | 36.25 |
| weak_prior | 68 | 29 | 5 | 8.67 | 39.84 |
| pooled_curve_ratio | 66 | 45 | 24 | 9.23 | 37.27 |
| preoperative_level | 68 | 43 | 28 | 18.06 | 54.71 |
| reference_to_last_log_linear | 68 | 6 | 0 | 39.65 | 98.94 |
| last_two_log_linear | 68 | 6 | 0 | 40.17 | 98.94 |

## landmark

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp |
|---|---|---|---|---|---|
| population_assimilation_robust | 66 | 57 | 44 | 4.75 | 23.45 |
| population_assimilation_t4 | 66 | 57 | 44 | 4.88 | 23.30 |
| population_assimilation | 66 | 58 | 45 | 5.25 | 23.60 |
| population_anchored_t4 | 66 | 50 | 29 | 6.13 | 29.83 |
| population_anchored | 66 | 48 | 29 | 6.25 | 30.46 |
| persistence | 68 | 35 | 0 | 9.24 | 27.13 |
| analog_knn | 64 | 45 | 32 | 9.74 | 37.28 |
| pooled_curve_ratio | 66 | 37 | 21 | 11.22 | 51.46 |
| weak_prior | 68 | 26 | 4 | 14.98 | 35.30 |
| preoperative_level | 68 | 43 | 34 | 18.06 | 54.71 |
| last_two_log_linear | 68 | 4 | 0 | 71.31 | 101.66 |
| reference_to_last_log_linear | 68 | 4 | 0 | 71.31 | 101.66 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp |
|---|---|---|---|---|
| population_initial_state | 47 | 33 | 4.28 | 12.04 |
| population_initial_state_t4 | 47 | 32 | 4.53 | 12.64 |
| pooled_training_curve | 47 | 30 | 11.87 | 47.47 |

## by family (primary methods)

- human/graft, next_observation: 4/21 meet criteria, 12 within 5/10 pp, pooled RMSE 7.34 pp; comparator 7.83 pp
- human/graft, landmark: 6/21 meet criteria, 13 within 5/10 pp, pooled RMSE 7.44 pp; comparator 6.24 pp
- human/graft, initial_state_group_mean: 5/11 meet criteria, 5 within 5/10 pp, pooled RMSE 6.01 pp; comparator 23.42 pp
- human/host, next_observation: 36/45 meet criteria, 44 within 5/10 pp, pooled RMSE 1.86 pp; comparator 5.56 pp
- human/host, landmark: 38/45 meet criteria, 44 within 5/10 pp, pooled RMSE 1.93 pp; comparator 10.05 pp
- human/host, initial_state_group_mean: 28/36 meet criteria, 28 within 5/10 pp, pooled RMSE 3.60 pp; comparator 4.30 pp

## calibration of primary methods

- next observation, 80% interval: 125/148 = 0.845 (window 0.70–0.90)
- next observation, 90% interval: 131/148 = 0.885 (window 0.80–0.97)
- landmark, 80% interval: 127/148 = 0.858 (window 0.70–0.90)
- landmark, 90% interval: 135/148 = 0.912 (window 0.80–0.97)
- initial state group means, 80% interval: 129/143 = 0.902
- initial state group means, 90% interval: 141/143 = 0.986
