# IOP after glaucoma procedures — model 1.3.0

Fourth version of the intraocular-pressure component ([model 1.0.0](../README.md),
[1.1.0](../model-1.1/README.md), [1.2.0](../model-1.2/README.md)). It was developed on
the 229 record studies of tranches 1–3 (557 series) and tested once on **tranche 4**,
reserved before triage, triaged blind and extracted by custodians: 79 records, 222
series ([tranche-4 README](../../../data/iop-glaucoma-surgery-2026-10-28/tranche-4/README.md)).
**Model 1.3.0 passed H1–H4 and failed H5, its declared improvement over 1.2.0**, so
`peykar eye` keeps model 1.2.0. Nothing here is a clinical or biological validation
claim.

## What changed from 1.2.0, and why

On the training data, the initial state of 1.2 was biased by device or technique
within one procedure: too low after micropulse cyclophotocoagulation, too high after
PreserFlo, too low in the first month after stents and goniotomy. Candidate **d1**
reads a sub-procedure from each arm's own description with declared keyword rules
(`peykar.validation.eye_iop_data_1_3`) and nests it in the procedure: contexts
`human/<sub-procedure>/<procedure>` (for example `human/micropulse/cyclodestruction`),
hierarchy species → procedure → sub-procedure. Everything else is 1.2.0 (generic eye
model 2, forecast-only early noise `k = 8`, `T = 1 month`, a study effect per
procedure arm). Configuration `peykar.model_learning.iop_population_1_3`, operator
`scripts/evaluate_iop_population_1_3.py`, publisher
`scripts/publish_iop_population_1_3.py`.

## Development (leave-one-study-out over the 229 studies)

From the [development log](development-log.md) (human group means, pooled RMSE in pp
of the preoperative IOP; 1,467 forecast and 1,651 initial-state targets):

| candidate | next observation | landmark | initial state |
|---|---|---|---|
| d0 = model 1.2.0 | 6.464 | 8.348 | 10.209 |
| **d1 sub-procedure (adopted)** | **6.382** | **8.061** | **10.066** |
| d2 lognormal-mean point (rejected) | 6.528 | 8.421 | 10.215 |

## One-shot sealed test on tranche 4 (2026-10-29)

The test was run once, after freeze commit `38f4f874`, with protocol
[`protocol-1.3.0.json`](protocol-1.3.0.json) (SHA `0cac4daf…`), all 68 pinned files and
the 229 training and 79 sealed record hashes matching. The operator scored each sealed
study with model 1.3 and, in the same run, with the frozen 1.2.0 (d0) as the declared
reference; for 25 sealed studies the training studies linked to them by shared authors
were left out of their fits. The first attempt was stopped by a hardware watchdog
restart of the machine after 41 of 79 studies, before any report or execution record
was written; the same command was then run once from the start (this is not a second
test, because no result had existed). Exit 0, 18,216 s, reports `df84eff4…` (1.3) and
`3950f0b0…` (1.2.0). Public artifacts are in
[`sealed-test-2026-10-29/`](sealed-test-2026-10-29/tables.md), with figure
`sealed-results.png`, its script `plot-results.py` and the declared analysis
`sealed-analysis.json`. Values of 43 of the 79 studies are redacted;
`.peykar/check_iop_redaction_1_3.py` (86 redacted study reports in the two public
reports) and a check of the 233 analysis rows of redacted studies found no value.

Declared checks, human group means (133 forecast series, 548 targets; 122 control
series with 612 initial-state targets):

| check | result |
|---|---|
| H1 calibration, next observation | 424/548 = 0.774 and 484/548 = 0.883: **both inside** (windows 0.70–0.90, 0.80–0.97) |
| H2 accuracy, next observation | **pass**: 6.64 pp against persistence 8.27, pooled-curve ratio 8.52, analog kNN 10.67 and last-two trend 37.15 |
| H3 landmark | **pass**: 7.92 pp against persistence 15.02, pooled-curve ratio 16.03 and analog kNN 17.08 |
| H4 initial state | **pass**: 10.94 pp against the pooled training curve's 15.94; 27/122 groups within 5/10 pp |
| H5 against 1.2.0 trained on the same studies | **fail**: initial state 10.94 against **10.69** pp (must be lower); landmark 7.92 against 8.00 (lower, as required); next observation 6.64 against 6.67 (allowed: +0.05) |

Per series, 76/133 group means were within 5/10 pp for the next observation and 36 of
them also at least 0.5 pp better than persistence (1.2.0: 75 and 35); for the landmark
task 56 and 29 (1.2.0: 61 and 32).

Reported, not tested:

- **Model 1.2.0 on the same sealed set.** Scored by the same declared analysis, the
  reference meets H1–H4 as well: coverage 0.774/0.872; next observation 6.67 pp against
  persistence 8.27; landmark 8.00 against 15.02; initial state 10.69 against its pooled
  training curve 16.33. It has now held on tranche 3 (its own sealed test) and on
  tranche 4.
- **Where the sub-procedure level helped and hurt** (initial state, 1.3 against 1.2.0,
  pp; `.peykar/iop13_sealed_breakdown.py`): micropulse cyclophotocoagulation 14.20
  against 15.96 and PreserFlo 7.51 against 8.33, as the training diagnostics had
  suggested; but continuous-wave cyclophotocoagulation 13.84 against 12.51, non-valved
  tubes 12.42 against 9.91, valved tubes 11.54 against 10.31 and stents 10.67 against
  9.54. On the training data valved and non-valved tubes had shown no difference in
  bias, so their split added variance without removing bias. By procedure: tubes 11.89
  against 10.16 and angle MIGS 10.06 against 9.75 lost; cyclodestruction 14.06 against
  14.68 and subconjunctival MIGS 8.48 against 9.11 gained (figure, upper right).
- **Forecasts are almost unchanged.** Next-observation RMSE differs from 1.2.0's by
  less than 0.1 pp overall and at every target time; the largest landmark gain is after
  month 6 (6.59 against 6.88 pp, 197 targets). First-month next-observation coverage
  was 0.842/0.934 (1.2.0: 0.855/0.914) on 152 targets.
- **Single eyes** (481 targets): next observation 8.91 pp against persistence 10.08,
  coverage 0.909/0.965; landmark 10.35 against 19.25.
- **Rabbit series** (4 forecast series, 1 control initial-state series) were removed
  before the tested analysis and are reported per series only.

Failures seen after unsealing (public CC BY studies); none was used to change the
frozen model:

- **A valved-tube study** ([PMC11722142](https://doi.org/10.3390/jcm14010042), four
  baseline subgroups): IOP was 25–37% of the preoperative value at month 1 and 43–64%
  from month 3 to 12. The initial state of 1.3 was 54–68% throughout (1.2.0: 51–64%),
  series RMSE 15.8–16.5 pp against 13.0–14.0 for 1.2.0 (figure, lower right). Both
  models miss the low first month; the valved-tube level moved them further up.
- **An early pressure spike after cataract surgery with undiluted viscoelastic**
  ([PMC9825933](https://doi.org/10.7759/cureus.33499)): 174% of the preoperative IOP
  on day 1, against an initial state of 107% from both models (39 pp RMSE).
- **Micropulse transscleral laser measured in the first hours**
  ([PMC12880698](https://doi.org/10.1371/journal.pone.0340625)): IOP 73–89% of the
  preoperative value from 1 to 13 hours and 60% at 24 hours, against an initial state
  of 49–52% (1.2.0: 47–49%; 28 pp RMSE). Four of these visits lie before the earliest
  visit of any of the 229 training studies' control series (day 1); `peykar eye` now
  warns for such months.
- **Kahook Dual Blade goniotomy alone in POAG**
  ([PMC9338461](https://doi.org/10.1186/s12886-022-02545-1)): a rise to 102% at month 1
  that neither model expected (24 pp RMSE).

Tranche 4 is now consumed, as are tranches 1–3; tranche 5 (19 articles) stays unread.
A later IOP version needs a new screen whose test part is reserved before triage. The
results point to a sub-procedure level only where the training diagnostics show a bias
(cyclodestruction, subconjunctival MIGS), the early hypertensive phase of valved tubes,
and early spikes after cataract surgery.
