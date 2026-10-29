# Best-corrected visual acuity after treatment of macular disease — model 1.0.0

Eighth eye component. It forecasts the best-corrected visual acuity (BCVA, logMAR) of a
group of eyes after macular edema (diabetic, vein occlusion, uveitic or of another
cause) or neovascular AMD is treated with anti-VEGF injections, a corticosteroid
implant or injection, laser or a combination: from the pretreatment acuity alone
(initial state), from the first follow-up visit (landmark), or one visit ahead (next
observation). It was developed on the 15 development records of dataset
[`retina-acuity-2026-10-29`](../../../data/retina-acuity-2026-10-29/README.md) (52
series, 20 of them control group means) and tested once on 9 sealed records (11
series), reserved by a salted hash before any value was read and extracted by three
custodians. **Model 1.0.0 failed all four declared hypotheses of its sealed test.** It
is therefore not offered by `peykar eye`. Nothing here is a clinical or biological
validation claim.

## Model

- **Additive outcome, scored in letters.** As for the corneal acuity component, the
  series of `peykar.validation.eye_retina_acuity_data` give generic eye model 2 the
  change from the pretreatment acuity as the exact pseudo-fraction
  `exp(change / 1 logMAR)` and the pretreatment acuity as `exp(logMAR0)`;
  `peykar.validation.eye_additive_evaluation2` rescores everything in ETDRS letters
  (**one reported pp is one letter, 0.02 logMAR**). The per-series criteria read
  RMSE ≤ 5 letters, maximum ≤ 10 letters and, for forecasts, at least 0.5 letter
  better than persistence.
- **Law.** `change(t) = ell + e r(t) + s q(t)` (logMAR) with
  `r = exp(−(t − 1 day)/tau)`, `q = exp(−(t − 1 day)/7 days)` and
  `ell = context level + b · logMAR0`; contexts are `human/<indication>/<treatment>`,
  with a study effect per context arm. The fitted slope b is about −0.005, so the
  pretreatment acuity barely moves the expected change.
- **Candidate v3** (configuration `peykar.model_learning.retina_acuity_population`):
  forecast intervals use the pooled study noise. Operator
  `scripts/evaluate_retina_acuity_population.py`, publisher
  `scripts/publish_retina_acuity_population.py`. Primaries:
  `population_assimilation_t4` for both forecast tasks and `population_initial_state`
  for the initial state.

## Development (leave-one-study-out over the 15 studies)

The [development log](development-log.md) records each declaration before its run.
Pooled RMSE in letters of each candidate's primary by the declared rule (the robust
variant for v0–v2, none of whose variants met both H1 windows; the Student-t variant
for v3), group means (69 next-observation and landmark targets, 83 initial-state
targets):

| candidate | next observation | landmark | initial state | next-obs coverage 80/90% |
|---|---|---|---|---|
| v0 one level per context | 2.778 | 3.066 | 4.737 | 0.623 / 0.768 |
| v1 contexts nested in treatments | 2.772 | 3.031 | 4.763 | 0.609 / 0.783 |
| v2 v0 + forecast-only persistent deviations | 2.737 | 3.066 | 4.737 | 0.551 / 0.667 |
| **v3 v0 + pooled forecast noise (adopted)** | 2.794 | 3.047 | 4.742 | 0.841 / 0.884 |
| persistence | 2.654 | 3.148 | — | |
| pooled-curve increment | 2.858 | 3.392 | — | |
| pooled training curve | — | — | 5.683 | |

v1 and v2 were rejected by the 0.10-letter rule; v3 was adopted under the declared
calibration clause (its intervals were the only ones inside both H1 windows). On
development v3 passed H1, H3 and H4 and **failed H2 against persistence** (2.794
against 2.654 letters).

## One-shot sealed test (2026-10-29)

The test was run once, after freeze commit `f4352217`, with protocol
[`protocol-1.0.0.json`](protocol-1.0.0.json) (SHA `34494401…`), all 71 pinned files and
the 15 training and 9 sealed record hashes matching. Exit 0, 18.11 s, report
`88e1b9ef…`. Public artifacts are in
[`sealed-test-2026-10-29/`](sealed-test-2026-10-29/tables.md), with figure
`sealed-results.png`, its script `plot-results.py` and the declared analysis
`sealed-analysis.json`. Values of 5 studies are redacted;
`.peykar/check_retina_acuity_redaction.py` loads the publisher's licence classes and
found no value of a redacted study in the public files (5 study reports and 20
per-series rows checked, 0 leaks).

Declared checks, human group means (11 forecast series, 31 targets; 9 control series
with 34 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | 21/31 = 0.677 and 25/31 = 0.806: **fail** (80% window 0.70–0.90 missed; 90% inside 0.80–0.97) |
| H2 accuracy, next observation | **fail**: 6.14 letters against persistence 4.81 and the pooled-curve increment 5.14 (better than analog kNN 12.40 and the last-two trend 12.61) |
| H3 landmark | **fail**: 9.17 letters against persistence 6.49 and the pooled-curve increment 8.07 (better than analog kNN 18.66) |
| H4 initial state | **fail**: 16.40 letters against the pooled training curve 5.18 on the same 34 targets |

Per series, 5/11 group means were within 5/10 letters for the next observation and 3
of them also at least 0.5 letter better than persistence; for the landmark task the
counts were 6 and 3; for the initial state 5 of 9 control groups were within 5/10
letters. The anchored population variants were far worse (27 and 47 letters).

## Why it failed (reported after unsealing, not tested)

Commands: `.peykar/retina_acuity10_sealed_breakdown.py` on the private report, and the
fitted parameters in `report-public.json`. Nothing below changes a verdict.

- **The early transient was never constrained.** The law keeps the transient term of
  the IOP law, `s q(t)` with a 7-day time constant, which describes early changes in
  the first weeks. No development series has a visit before day 30 (earliest 0.986
  months): at day 30 the term is 1.6% of its day-1 size, so the training data say
  almost nothing about it. The population fit left its mean at +1.99 logMAR at day 1
  (estimate SD 0.68; between 0.58 and 4.00 across the development folds): a predicted
  loss of about 92 letters at day 1, fading to nothing by month 2 (figure, upper
  right). Development never forecast an early visit, so this was invisible there, and
  the freeze review did not check which parts of the law the data identify.
- **One sealed study had a day-1 visit.** Macular edema of unstated cause treated with
  laser alone (control) or with bevacizumab plus laser (treated), under a licence that
  does not permit republishing its values. Its contexts, `human/other/laser` and
  `human/other/combination`, had no training study, so the species level was used.
  Its initial state has the largest error of the test (91.5 letters, at the day-1
  visit; series RMSE 46.46 against 7.62 for the pooled training curve). Every landmark
  forecast of that study starts from the day-1 visit, where the model reads the
  observation through the large transient and shifts the long-term level; its two
  series have landmark RMSEs of 13.31 and 22.53 letters against persistence 3.29 and
  12.84, and next-observation RMSEs of 9.23 and 13.33 against 3.86 and 7.22.
- **Sensitivity.** Without that study all four checks would pass: next-observation
  coverage 18/25 = 0.72 and 20/25 = 0.80; next observation 3.90 letters against
  persistence 4.54, the pooled-curve increment 4.23, analog kNN 7.43 and the last-two
  trend 9.62; landmark 4.69 against 5.59, 5.45 and 11.07; initial state 4.11 against
  4.76. Equally, forecasts whose origin is at least half a month after treatment give
  3.81 letters (29 targets) against persistence 4.35 and the increment 4.03 for the
  next observation, and 4.69 (25 targets) against 5.59 and 5.45 for the landmark task.
  These subsets were chosen after seeing the results; they explain the failure but
  cannot replace the declared test.
- **Dexamethasone implant wear-off.** A public DME group with one dexamethasone implant
  (figure, lower right; pretreatment logMAR 1.08) gained 18.5 letters at month 2 and
  kept only 6 at month 4 and 1 at month 6. The law is monotone after its transient and
  the acuity configuration has no wear-off term, so the landmark forecast from month 2
  kept the gain: RMSE 13.55 letters against persistence 15.21, both outside the
  criteria. Steroid implant groups as a whole: landmark 9.01 against persistence 10.09.
- **Where it works.** DME with anti-VEGF (6 series in 5 studies): next observation 2.88
  letters against persistence 3.64, landmark 2.82 against 3.93; the initial state is
  level with the pooled training curve (3.14 against 3.04). Vein occlusion with
  bevacizumab plus an adjunct (1 treated series): 2.22 against 2.72 and 1.72 against
  1.78. A public aflibercept group is shown in the figure, lower left.

## Limits and next version

- The law keeps terms the training data cannot identify. A model must know where its
  training data lie in time; the next generic step is a support check that flags or
  refuses forecasts before the earliest training visit of a family (or a transient
  prior centred on zero), and an acuity law without the early transient.
- Unseen contexts fall back to the species level. The generic option
  `treatment_fallback` (the treatment's pooled level first) exists and was not used.
- Steroid implant wear-off is not modelled for acuity. The retinal-thickness 1.1 dose
  rule (one dexamethasone implant, duration 3.5 months) is the candidate.
- The sealed test is small (9 studies, 11 series, 5 of them under non-public
  licences) and most series are DME with anti-VEGF.
- Treated groups use the control prior of their context; per-eye series, regimens,
  injection counts and retinal anatomy are not modelled.
- A new version needs fresh test data: tranches 2–4 of the dataset (258 articles) are
  untriaged and unread. The 21 sealed studies of tranche 1 are now consumed.
