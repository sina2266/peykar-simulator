# Best-corrected visual acuity after treatment of macular disease — model 1.2.0

Version 1.2 of the eighth eye component. Models 1.0.0 and 1.1.0 failed their sealed
tests because the law was extrapolated to weeks with no training visit
([1.0](../model-1.0/README.md), [1.1](../model-1.1/README.md)). Model 1.2.0 adds the
generic **support hold**: before the earliest training control visit of a series'
context the law is held flat, in the likelihood of the series' own visits and in every
forecast. It also adds the wear-off of one dexamethasone implant. It was developed on the
53 record studies of tranches 1–2 and tested once on tranches 3–4 of dataset
[`retina-acuity-2026-10-29`](../../../data/retina-acuity-2026-10-29/README.md): 27
records with 143 series (34 group means), reserved before triage and extracted by nine
custodians. **Model 1.2.0 failed all five declared hypotheses.** Model 1.1.0, scored in
the same run, failed all four of its own. No version of this component is offered by
`peykar eye`. Nothing here is a clinical or biological validation claim.

## Model

- Generic eye model 2 with `peykar.model_learning.retina_acuity_population_1_2`
  (candidate `x3`): model 1.1.0's law `change(t) = ell + e r(t)` (logMAR) with the
  support hold (`peykar.model_learning.eye_support.SupportConfig`, scored by harness 5,
  `peykar.validation.eye_recovery_evaluation5`) and 80% of the effect of one
  dexamethasone implant lost around month 3.5 (the retinal-thickness 1.1 dose rule).
  The hold changes no fit: training controls never have a visit before their own
  context's support.
- Primaries: `population_anchored` for the next observation and the landmark task,
  `population_initial_state` for the initial state. Operator
  `scripts/evaluate_retina_acuity_population_1_2.py`, publisher
  `scripts/publish_retina_acuity_population_1_2.py`. Scores in ETDRS letters (one
  reported pp is one letter, 0.02 logMAR).

## Development (leave-one-study-out over the 53 training studies)

The [development log](development-log.md) declared each candidate before its run. Human
group means: 206 next-observation and landmark targets, 251 initial-state targets.

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|
| x0 = 1.1.0 | 3.628 | 4.921 | 5.556 | 0.796 / 0.898 |
| x1 support hold | 3.477 | 4.366 | 5.429 | 0.796 / 0.903 |
| x2 x1 + treatment fallback | 3.482 | 4.380 | 5.428 | 0.796 / 0.903 |
| **x3 x1 + dexamethasone wear-off (adopted)** | 3.228 | 3.782 | 5.216 | 0.772 / 0.893 |
| x4 all three combined | 3.233 | 3.806 | 5.218 | 0.777 / 0.893 |
| persistence | 3.234 | 4.026 | — | |
| pooled-curve increment | 3.566 | 5.015 | — | |
| pooled training curve | — | — | 5.278 | |

On development x3 passed H1–H5, but H2 against persistence by 0.006 letters and H4
against the pooled training curve by 0.06 letters only.

## One-shot sealed test (2026-10-29)

The test ran once after freeze commit `e4ac6cb9`, with protocol
[`protocol-1.2.0.json`](protocol-1.2.0.json) (SHA `a0618bbf…`; all 94 pinned files and
the 53 training and 27 sealed record hashes matched). Exit 0, 234.0 s, report
`495c5f9f…`, reference (1.1.0) `bd8ec333…`. Public artifacts are in
[`sealed-test-2026-10-29/`](sealed-test-2026-10-29/tables.md). Values of 13 studies are
redacted; `.peykar/check_retina_acuity_redaction_1_2.py` found no value of a redacted
study in the public files (26 study reports and 178 per-series rows checked, 0 leaks).

Declared checks, human group means (34 forecast series, 71 targets; 27 control group
means with 86 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | **fail**: 42/71 = 0.592 at 80% (window 0.70–0.90); 59/71 = 0.831 at 90% (inside) |
| H2 accuracy, next observation | **fail**: 4.51 letters against persistence 4.06 and the pooled-curve increment 3.82 (better than analog kNN 4.63 and the last-two trend 11.17) |
| H3 landmark | **fail**: 5.85 letters against the pooled-curve increment 4.52 (better than persistence 5.91 and analog kNN 6.34) |
| H4 initial state | **fail**: 8.82 letters against the pooled training curve 8.70 |
| H5 against 1.1.0, same training | **fail**: next observation 4.51 against 4.42 (more than 0.05 higher); landmark 5.85 against 5.90 and initial state 8.82 against 8.88 (both lower) |

Reported, not tested: model 1.1.0's own H1–H4 all fail on the same studies (coverage
0.676 / 0.915; next observation 4.42; landmark 5.90; initial state 8.88). Per series, 26
of 34 group means were within 5/10 letters for the next observation (pooled-curve
increment 30, persistence 29) and 21 for the landmark task (increment 26); initial-state
80% coverage 42/86 and 90% coverage 50/86. Removing any single study changes no H1–H4
verdict.

## What the test shows (reported after unsealing, not tested)

Command: `.peykar/retina_acuity12_sealed_breakdown.py` on the private reports.

- **The support hold had little to act on.** No group-mean target of this set lies
  before month 1, and only 3 group means have an early visit. Model 1.2 and model 1.1
  differ by less than 0.1 letter in every task.
- **The pooled-curve increment wins after the first months.** For the next observation
  it is closer than the model at 3–6 months (2.63 against 3.94 letters, 17 targets) and
  from month 12 on (3.70 against 5.11, 22 targets); for the landmark task it is closer at
  every target time (for example 5.64 against 7.24 from month 12 on). Persistence beats
  the model for the next observation from month 6 on. The fitted law is a fast term
  (time constants of 1–22 days across contexts, species median 3.5 days, in the
  published population fit) followed by a constant level, so it cannot follow
  the slow gains and losses of group means over months and years that the pooled curve
  of the training data shows.
- **Intervals too narrow.** Only 59% of next-observation targets fell inside the 80%
  interval: the forecast noise learned on the training studies is too small for this
  set.
- **Initial state.** The context priors were level with one pooled training curve
  (8.82 against 8.70), as on tranche 2.

## Limits and next steps

- Three sealed tests, three failures, with the same pattern: persistence and the
  pooled-curve increment beat the population model on acuity forecasts. A next model
  should start from the pooled training curve's increments (a flexible, nonparametric time
  course shared across contexts, with context offsets only where the data support them),
  not from a single fast exponential. Its test needs a new screen: the dataset has no
  unread article left.
- The support hold (generic, unit-tested) removes the extrapolation failure of models
  1.0 and 1.1; this set had too few early visits to show it.
- Treated groups use the control prior of their context; per-eye series, regimens,
  injection counts, switches and retinal anatomy are not modelled.
