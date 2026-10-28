# IOP model 1.2 — development log

Development data: the 146 record studies of tranches 1 and 2 of dataset
`iop-glaucoma-surgery-2026-10-28` (47 tranche-1 development records, 25 tranche-1
sealed records consumed by the test of model 1.0.0, 74 tranche-2 sealed records
consumed by the test of model 1.1.0; 361 series). The tranche-3 reservation lists
them as development
([`tranche-3/split-reservation-1.0.0.json`](../../../data/iop-glaucoma-surgery-2026-10-28/tranche-3/split-reservation-1.0.0.json)).
No tranche-3 value is read before the 1.2 protocol is frozen. Studies of one
author-overlap cluster (`tranche-3/author-overlap-1.0.0.json`, 24 development
clusters) leave training together in leave-one-study-out (LOSO). Each round declares
its change and its adoption rule before its LOSO results are seen.

## Starting point and target

Model 1.1.0 passed all five hypotheses of its tranche-2 sealed test, but its gain over
1.0.0 was only in the first month, and the first month was badly calibrated: 80% and
90% next-observation coverage 0.585 and 0.734 for targets within one month.

Diagnostics before round 1 (training data only; the 1.1 sealed report is consumed):

- **Calibration by target time.** In the 1.1 sealed report (human group means,
  Gaussian variant), next-observation targets at weeks 1–2 had 80% coverage 0.46 and
  RMSE 13.5 pp; targets from month 1 on were covered at 0.80–0.89. The 1.1
  development LOSO on tranche 1 showed the same pattern (weeks 1–2: 0.44).
- **Not measurement noise.** Best-fit residuals of the 92 human group means with at
  least six visits are *not* larger early: their RMS is 0.028 at days 1–3, 0.049 in
  weeks 1–2 and 0.055–0.064 after month 6. The law fits early visits well in-sample;
  what is uncertain is how a series will move after its first visits.
- **Early errors depend on the procedure.** From day-1 origins, the 1.1 forecasts were
  biased low after angle MIGS (sealed −11.7 pp, development −7.5 pp) and high after
  cyclodestruction and tubes. The model still beat persistence from these origins.
- Out-of-sample errors of consecutive next-observation forecasts are only weakly
  correlated (lag-1 0.16 in development, 0.20 sealed), so persistent deviations can
  help only a little.

## Model

`peykar.model_learning.eye_recovery_population2` (generic version 2, written for
retinal-thickness model 1.1 and IOP 1.2; with defaults it reproduces generic model 1
bit for bit, and generic model 1 reproduces IOP 1.1). The harness is
`peykar.validation.eye_recovery_evaluation2`; the series adapter
`peykar.validation.eye_iop_data_1_2`; the configurations
`peykar.model_learning.iop_population_1_2`; the operator
`scripts/evaluate_iop_population_1_2.py`. Two extensions are used, both in
forecasting only (the population fit is unchanged):

- persistent (Ornstein–Uhlenbeck) deviations, as in retinal-thickness model 1.0.0;
- early noise: forecasts add independent noise with variance
  `sigma^2 k exp(-(t - t1) / T)` at target month `t`; posteriors and point forecasts
  are those of the model without it.

## Round 1 — declared before results (2026-10-28)

- **c0**: model 1.1.0 in generic version 2. Check: its LOSO report must equal that of
  the frozen model 1.1.0 with its own harness (`--model 1.1.0`) on the same data.
- **c2**: c0 with forecast-only deviations, `phi = 0.5`, correlation time 3 months.

Adoption rule (as for model 1.1): a candidate replaces the incumbent only if, on human
group means, it improves the pooled RMSE of its primary method by at least 0.10 pp in
at least one task (initial state, next observation, landmark) and loses no more than
0.05 pp in any. The primary method is selected as in model 1.1: the lowest pooled
group-mean next-observation RMSE among the five population variants whose group-mean
coverage lies inside both H1 windows; ties within 0.05 pp go to the simpler variant
(Gaussian). (Corrected before any round-1 result: the first wording omitted the
coverage condition of the 1.1 rule.)

## Round 2 — procedure declared before round-1 results (2026-10-28)

**c1** adds forecast-only early noise to the incumbent after round 1. Its constants
`(k, T)` are chosen on c0's LOSO rows (human group means, next-observation task,
primary variant), by recomputing each row's interval with the added variance (Gaussian
approximation of the predictive in `ln f`, noise SD from the fit's noise centre and
group-mean scaling), from the grid `k` in {0.5, 1, 2, 4, 8, 16} and `T` in {0.1, 0.25,
0.5, 1} months: the pair that minimizes `|cov80 - 0.80| + |cov90 - 0.90|` over targets
within the first month, ties to the smaller `k`. c1 is then run by LOSO. Because point
forecasts barely change, c1's adoption rule is about calibration: it is adopted if its
first-month 80% and 90% next-observation coverage each move at least 0.05 closer to
nominal, the pooled 80% and 90% coverage stay inside the protocol windows ([0.70, 0.90]
and [0.80, 0.97]), and no task's pooled RMSE rises by more than 0.05 pp.

Correction before any round-1 result (2026-10-28): generic model 1 gives a study whose
arms belong to different procedures one shared study effect and fits those procedures
in one REML block; model 1.1.0 gives each arm's procedure its own study effect. Sixteen
training studies have arms in two procedures, so c0 as first configured could not
reproduce 1.1.0, and it ran about four times slower (one REML block for most
procedures). The first c0 and c2 runs were stopped unread. Generic version 2 now has
`study_effects = "per_group"` (checked against `iop_population_1_1` with a
multi-procedure study in `tests/unit/test_eye_recovery_population2.py`), and every IOP
1.2 candidate uses it; c0 and c2 are otherwise unchanged.

### Round 1 results (LOSO, 146 studies, human group means; each run in four shards merged by `--merge`)

| | model 1.1.0 (1.1 harness) | c0 | c2 (deviations 0.5, 3 mo) | persistence | pooled curve |
|---|---|---|---|---|---|
| next observation, 946 targets | 6.840 | 6.840 | 6.912 | 8.022 | — |
| landmark, 946 targets | 8.803 | 8.803 | 8.836 | 14.461 | — |
| initial state, 1,062 targets | 10.585 | 10.585 | 10.585 | — | 18.159 |
| next-obs coverage 80% / 90% | 0.737 / 0.866 | 0.737 / 0.866 | 0.718 / 0.857 | — | — |
| first-month next-obs coverage, 219 targets | 0.639 / 0.822 | 0.639 / 0.822 | 0.580 / 0.781 | — | — |
| first-month next-obs RMSE | 10.38 | 10.38 | 10.64 | — | — |

The primary of every run is the Gaussian variant (the Student-t variant is within
0.05 pp).

- **Check passed**: c0 equals the frozen model 1.1.0 with its own harness in all
  195,431 numbers of every study; the only differences are text (the reason given when
  a population prior is unavailable names the "baseline value" instead of the
  "preoperative IOP").
- **c2 is not adopted**: it gains in no task (+0.07, +0.03, 0.00 pp) and lowers the
  first-month coverage. As expected from the weak error persistence of IOP series,
  persistent deviations do not help. **Incumbent after round 1: c0.**

### Round 2 procedure result

`.peykar/iop12_early_noise_calibration.py` on c0's rows (219 first-month targets;
Gaussian approximation 0.626 / 0.804 against the exact 0.639 / 0.822) chose
**k = 8, T = 1 month** (first month 0.826 / 0.904, all targets 0.795 / 0.878 in the
approximation; loss 0.031). **c1** = c0 with forecast-only early noise `(8, 1)`; it is
run by LOSO next, under the round-2 adoption rule declared above.

### Round 2 results (LOSO, 146 studies, human group means)

| | c0 | c1 (early noise 8, 1 month) |
|---|---|---|
| next observation, 946 targets | 6.840 | 6.851 |
| landmark, 946 targets | 8.803 | 8.808 |
| initial state, 1,062 targets | 10.585 | 10.583 |
| next-obs coverage 80% / 90% | 0.737 / 0.866 | 0.790 / 0.890 |
| **first-month** next-obs coverage, 219 targets | 0.639 / 0.822 | **0.826 / 0.904** |
| first-month landmark coverage (reported) | 0.680 / 0.836 | 0.808 / 0.909 |
| first-month next-obs RMSE | 10.38 | 10.41 |

Under the round-2 rule c1 is **adopted**: the first-month 80% coverage moves from 0.161
to 0.026 away from nominal and the 90% coverage from 0.078 to 0.004 (both by more than
0.05); the pooled coverages lie inside the H1 windows; no task's RMSE rises by more
than 0.011 pp (point forecasts move only through the mixture medians). The primary is
the Gaussian variant (the Student-t variant is within 0.05 pp).

## Freeze decision (2026-10-29)

- Primary candidate **c1** (model 1.1.0's law and fit in generic version 2, with
  forecast-only early noise `k = 8`, `T = 1 month`). U1 primary
  `population_initial_state`; U2 and U3 primary `population_assimilation`.
- Reference: the frozen model 1.1.0 with its own harness, scored in the same sealed
  run. H5 tests what the change targets: first-month next-observation coverage closer
  to nominal at both levels, with the pooled next-observation RMSE at most 0.05 pp
  higher than 1.1.0's.
- Expected, from development: no accuracy gain; the gain is calibration only. The
  first-month errors themselves (about 10 pp) remain.
- No further development rounds. This decision was written before any result of the
  retinal-thickness 1.1 sealed test (run in parallel) was read.
