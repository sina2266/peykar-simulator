# IOP-after-surgery model 1.4.0-d1s — development results

## Declared checks (human group means)

- H1_calibration_next_observation_group_means: pass
- H2_accuracy_next_observation_group_means: pass
- H3_landmark_group_means: pass
- H4_initial_state_control_group_means: pass
- H5_initial_state_lower_next_observation_and_landmark_within_0_05_pp: pass

- H1 next observation 80% interval: 1603/2015 = 0.796 (window 0.70–0.90)
- H1 next observation 90% interval: 1804/2015 = 0.895 (window 0.80–0.97)
- H2 next observation vs persistence: 6.51 against 7.91 pp on 2015 common targets
- H2 next observation vs last_two_log_linear: 6.51 against 38.34 pp on 2015 common targets
- H2 next observation vs pooled_curve_ratio: 6.51 against 8.29 pp on 2015 common targets
- H2 next observation vs analog_knn: 6.51 against 9.71 pp on 2013 common targets
- H3 landmark vs persistence: 8.09 against 14.02 pp on 2015 common targets
- H3 landmark vs pooled_curve_ratio: 8.09 against 15.29 pp on 2015 common targets
- H3 landmark vs analog_knn: 8.08 against 15.37 pp on 2013 common targets
- landmark 80% interval (reported): 1547/2015 = 0.768
- landmark 90% interval (reported): 1771/2015 = 0.879
- H4 initial state population_initial_state: 10.05 pp, 115/456 control group means within 5/10 pp
- H4 initial state pooled_training_curve: 16.87 pp, 60/459 control group means within 5/10 pp
- H4 initial state 80% interval (reported): 1887/2263 = 0.834
- H4 initial state 90% interval (reported): 2083/2263 = 0.920

## next_observation — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 499 | 272 | 128 | 6.51 | 40.05 | 2015 |
| population_assimilation_t4 | 499 | 274 | 128 | 6.54 | 40.08 | 2015 |
| population_assimilation_robust | 499 | 273 | 124 | 6.64 | 41.06 | 2015 |
| persistence | 502 | 216 | 0 | 7.89 | 60.87 | 2033 |
| pooled_curve_ratio | 502 | 197 | 76 | 8.27 | 59.07 | 2033 |
| population_anchored | 499 | 231 | 69 | 8.64 | 58.59 | 2015 |
| population_anchored_t4 | 499 | 229 | 66 | 8.66 | 58.60 | 2015 |
| weak_prior | 502 | 189 | 50 | 8.66 | 60.17 | 2033 |
| analog_knn | 502 | 176 | 71 | 9.74 | 62.69 | 2031 |
| reference_to_last_log_linear | 502 | 11 | 4 | 33.59 | 209.09 | 2033 |
| last_two_log_linear | 502 | 8 | 3 | 38.21 | 260.35 | 2033 |
| baseline_level | 502 | 9 | 6 | 40.55 | 79.66 | 2033 |

## landmark — group means

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 499 | 213 | 122 | 8.09 | 42.22 | 2015 |
| population_assimilation_t4 | 499 | 211 | 124 | 8.11 | 42.24 | 2015 |
| population_assimilation_robust | 499 | 210 | 117 | 8.26 | 42.14 | 2015 |
| persistence | 502 | 157 | 0 | 14.01 | 78.26 | 2033 |
| population_anchored | 499 | 162 | 42 | 14.94 | 74.35 | 2015 |
| population_anchored_t4 | 499 | 162 | 42 | 14.95 | 74.35 | 2015 |
| pooled_curve_ratio | 502 | 138 | 71 | 15.25 | 61.56 | 2033 |
| analog_knn | 502 | 112 | 66 | 15.51 | 62.69 | 2031 |
| weak_prior | 502 | 135 | 37 | 16.47 | 51.03 | 2033 |
| baseline_level | 502 | 9 | 4 | 40.55 | 79.66 | 2033 |
| last_two_log_linear | 502 | 8 | 4 | 64.96 | 233.15 | 2033 |
| reference_to_last_log_linear | 502 | 8 | 4 | 64.96 | 233.15 | 2033 |

## next_observation — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation_t4 | 702 | 326 | 140 | 8.60 | 74.29 | 2959 |
| population_assimilation_robust | 702 | 325 | 136 | 8.67 | 74.13 | 2959 |
| population_assimilation | 702 | 324 | 141 | 8.70 | 77.73 | 2959 |
| persistence | 705 | 280 | 0 | 10.54 | 133.33 | 2977 |
| weak_prior | 705 | 217 | 60 | 10.63 | 92.92 | 2977 |
| pooled_curve_ratio | 705 | 234 | 78 | 10.79 | 123.43 | 2977 |
| population_anchored_t4 | 702 | 279 | 70 | 10.89 | 126.88 | 2959 |
| population_anchored | 702 | 281 | 75 | 10.91 | 126.86 | 2959 |
| analog_knn | 705 | 222 | 85 | 11.05 | 73.75 | 2975 |
| reference_to_last_log_linear | 705 | 12 | 5 | 34.67 | 246.67 | 2977 |
| last_two_log_linear | 705 | 9 | 4 | 44.64 | 282.22 | 2977 |
| baseline_level | 705 | 9 | 6 | 44.87 | 91.11 | 2977 |

## landmark — all series

| method | series | within 5/10 pp | criteria (incl. persistence gain) | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|---|
| population_assimilation | 702 | 247 | 147 | 10.43 | 91.46 | 2959 |
| population_assimilation_t4 | 702 | 246 | 150 | 10.50 | 91.43 | 2959 |
| population_assimilation_robust | 702 | 245 | 144 | 10.60 | 91.59 | 2959 |
| analog_knn | 705 | 142 | 86 | 16.74 | 108.78 | 2975 |
| weak_prior | 705 | 158 | 47 | 18.27 | 92.92 | 2977 |
| persistence | 705 | 180 | 0 | 18.58 | 166.67 | 2977 |
| pooled_curve_ratio | 705 | 153 | 73 | 19.15 | 123.43 | 2977 |
| population_anchored_t4 | 702 | 187 | 47 | 19.57 | 155.40 | 2959 |
| population_anchored | 702 | 187 | 47 | 19.58 | 155.38 | 2959 |
| baseline_level | 705 | 9 | 4 | 44.87 | 91.11 | 2977 |
| last_two_log_linear | 705 | 9 | 5 | 64.95 | 269.23 | 2977 |
| reference_to_last_log_linear | 705 | 9 | 5 | 64.95 | 269.23 | 2977 |

## initial state (control group means)

| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |
|---|---|---|---|---|---|
| population_initial_state_t4 | 456 | 115 | 10.05 | 68.29 | 2263 |
| population_initial_state | 456 | 115 | 10.05 | 68.29 | 2263 |
| pooled_training_curve | 459 | 60 | 16.87 | 73.90 | 2284 |

## single eyes (reported, not tested)

- next_observation vs persistence: 12.11 against 14.71 pp on 944 targets
- next_observation vs last_two_log_linear: 12.11 against 56.04 pp on 944 targets
- next_observation vs reference_to_last_log_linear: 12.11 against 36.87 pp on 944 targets
- next_observation vs pooled_curve_ratio: 12.11 against 14.82 pp on 944 targets
- next_observation vs analog_knn: 12.11 against 13.43 pp on 944 targets
- next_observation vs preoperative_level: n/a against n/a pp on 0 targets
- landmark vs persistence: 14.19 against 25.81 pp on 944 targets
- landmark vs last_two_log_linear: 14.19 against 64.93 pp on 944 targets
- landmark vs reference_to_last_log_linear: 14.19 against 64.93 pp on 944 targets
- landmark vs pooled_curve_ratio: 14.19 against 25.60 pp on 944 targets
- landmark vs analog_knn: 14.19 against 19.12 pp on 944 targets
- landmark vs preoperative_level: n/a against n/a pp on 0 targets
- next_observation 80% interval: 828/944 = 0.877
- next_observation 90% interval: 888/944 = 0.941
- landmark 80% interval: 814/944 = 0.862
- landmark 90% interval: 876/944 = 0.928

## by procedure context (primary methods; comparator in parentheses)

- human/all/angle_mims: next_observation 148/281 within, 9.15 (11.47) pp; landmark 115/281 within, 10.74 (20.89) pp; initial_state_group_mean 64/219 within, 8.88 (16.55) pp
- human/all/cataract: next_observation 13/27 within, 7.06 (12.29) pp; landmark 12/27 within, 9.63 (17.33) pp; initial_state_group_mean 4/23 within, 14.78 (25.02) pp
- human/all/laser_trabeculoplasty: next_observation 0/2 within, 11.88 (8.28) pp; landmark 0/2 within, 22.48 (17.45) pp; initial_state_group_mean 0/2 within, 24.82 (22.75) pp
- human/all/nonpenetrating: next_observation 16/30 within, 5.60 (7.51) pp; landmark 13/30 within, 8.10 (18.59) pp; initial_state_group_mean 6/25 within, 10.47 (18.54) pp
- human/all/other: next_observation 9/29 within, 8.02 (9.82) pp; landmark 6/29 within, 8.94 (11.46) pp; initial_state_group_mean 1/4 within, 7.23 (38.67) pp
- human/all/trabeculectomy: next_observation 51/83 within, 6.34 (7.69) pp; landmark 33/83 within, 8.43 (12.54) pp; initial_state_group_mean 14/63 within, 10.87 (15.09) pp
- human/all/tube: next_observation 39/100 within, 8.31 (9.59) pp; landmark 29/100 within, 9.31 (16.08) pp; initial_state_group_mean 11/39 within, 9.93 (16.16) pp
- human/continuous/cyclodestruction: next_observation 8/26 within, 8.95 (11.69) pp; landmark 7/26 within, 9.20 (16.17) pp; initial_state_group_mean 2/15 within, 13.62 (13.09) pp
- human/endoscopic/cyclodestruction: next_observation 7/28 within, 10.46 (10.99) pp; landmark 7/28 within, 12.56 (20.78) pp; initial_state_group_mean 1/4 within, 7.54 (18.80) pp
- human/micropulse/cyclodestruction: next_observation 13/31 within, 8.49 (9.10) pp; landmark 11/31 within, 10.36 (11.96) pp; initial_state_group_mean 7/31 within, 11.86 (11.18) pp
- human/preserflo/subconjunctival_mims: next_observation 4/15 within, 8.67 (9.61) pp; landmark 3/15 within, 13.70 (18.73) pp; initial_state_group_mean 0/9 within, 9.88 (13.24) pp
- human/ultrasound/cyclodestruction: next_observation 3/4 within, 5.91 (8.79) pp; landmark 2/4 within, 7.08 (12.01) pp; initial_state_group_mean 0/4 within, 7.98 (7.83) pp
- human/xen/subconjunctival_mims: next_observation 13/46 within, 11.22 (12.67) pp; landmark 9/46 within, 13.13 (22.39) pp; initial_state_group_mean 5/18 within, 8.62 (16.72) pp

Non-human series removed before the tested analysis (reported per series): 24 forecast series, 16 control initial-state series.

## H5: model 1.4 against the frozen model 1.2.0, same training

- next_observation: 1.4 6.51 against 1.2 6.55 pp on 2015 common targets
- landmark: 1.4 8.09 against 1.2 8.23 pp on 2015 common targets
- initial_state: 1.4 10.05 against 1.2 10.27 pp on 2263 common targets
- first-month next-observation 80% coverage, reported (502 targets): 1.4 0.833 against 1.2 0.829
- first-month next-observation 90% coverage, reported (502 targets): 1.4 0.916 against 1.2 0.918
