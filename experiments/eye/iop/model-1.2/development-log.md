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
