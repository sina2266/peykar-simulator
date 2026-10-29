# IOP after glaucoma procedures — model 1.2.0

Third version of the intraocular-pressure component ([model 1.0.0](../README.md),
[model 1.1.0](../model-1.1/README.md)). It was developed on the 146 record studies of
tranches 1 and 2 (361 series): the 72 tranche-1 records and the 74 tranche-2 records
consumed by the 1.1.0 test. It was then tested once on **tranche 3**, reserved before
triage, triaged blind and extracted by eight custodians: 83 records, 196 series
([tranche-3 README](../../../data/iop-glaucoma-surgery-2026-10-28/tranche-3/README.md)).
Nothing here is a clinical or biological validation claim.

## What changed from 1.1.0, and why

Model 1.1.0 passed its sealed test, but its first-month intervals were too narrow:
80% and 90% next-observation coverage 0.585 and 0.734 for targets within one month,
against 0.864 and 0.943 later. Training-data diagnostics showed that this is not
measurement noise: in-sample residuals are *smaller* in the first weeks. What is
uncertain early is how a series will move after its first visits.

- **Generic model.** Version 1.2 runs in `peykar.model_learning.eye_recovery_population2`
  (generic eye model 2), configured by `peykar.model_learning.iop_population_1_2`. With
  version-1.1 settings (candidate c0) it reproduced the frozen 1.1.0 in all 195,431
  numbers of the development report. Each arm's procedure keeps its own study effect
  (`study_effects = "per_group"`), as in 1.1.0.
- **Forecast-only early noise (adopted, c1).** Forecasts add independent noise with
  variance `sigma^2 · k · exp(−(t − 1 day)/T)` at target month `t`, with `k = 8` and
  `T = 1 month`. The population fit, the posteriors and the point forecasts are those
  of 1.1.0; only the intervals widen early. The constants were chosen before c1 ran,
  by the declared calibration script
  (`.peykar/iop12_early_noise_calibration.py`) on c0's development rows.
- **Rejected (c2).** Persistent (Ornstein–Uhlenbeck) deviations in forecasting, as in
  retinal-thickness model 1.0.0. They gained in no task and lowered first-month
  coverage: IOP forecast errors are only weakly correlated from one visit to the next
  (lag-1 0.16).

## Development (leave-one-study-out over the 146 studies)

The [development log](development-log.md) records each declaration before its run.
Results are in [`development-loso-2026-10-29/`](development-loso-2026-10-29/tables.md);
values of 74 studies are redacted.

| human group means (236 series) | 1.2 (c1) | 1.1.0 (c0) | persistence |
|---|---|---|---|
| next observation, pooled RMSE pp (946 targets) | 6.851 | 6.840 | 8.022 |
| landmark, pooled RMSE pp (946 targets) | 8.808 | 8.803 | 14.461 |
| initial state, pooled RMSE pp (1,062 targets) | 10.583 | 10.585 | pooled curve 18.159 |
| next-observation coverage 80% / 90% | 0.790 / 0.890 | 0.737 / 0.866 | — |
| first-month next-observation coverage (219 targets) | **0.826 / 0.904** | 0.639 / 0.822 | — |

The round-2 rule adopted c1: first-month coverage moved at least 0.05 closer to
nominal at both levels, pooled coverage stayed inside the H1 windows, and no task's
RMSE rose by more than 0.05 pp. The primary is the Gaussian variant.

## One-shot sealed test on tranche 3 (2026-10-29)

The test was run once, after freeze commit `df8f389a`, with protocol
[`protocol-1.2.0.json`](protocol-1.2.0.json) (SHA `4169750d…`), all 52 pinned files and
the 146 training and 83 sealed record hashes matching. The operator scored each sealed
study with model 1.2 and, in the same run, with the frozen 1.1.0 as the declared
reference. Each model was fitted on the 146 studies minus the training studies linked
to that sealed study by shared authors (24 sealed studies had such links). Exit 0,
7,260 s, reports `3275c150…` (1.2) and `e40762bd…` (1.1.0). Public artifacts are in
[`sealed-test-2026-10-29/`](sealed-test-2026-10-29/tables.md), with figure
`sealed-results.png`, its script `plot-results.py` and the declared analysis
`sealed-analysis.json`; values of 53 studies are redacted.

**Redaction correction.** The frozen publisher reuses the 1.1 redaction, which
predates the `reference_value` field (the preoperative IOP of an initial-state series)
that the 1.2 harness writes. The publisher therefore left that one field in the
initial-state entries of redacted studies: 92 entries in the sealed folder and 159 in
the development folder. [`redact-reference-values.py`](redact-reference-values.py)
removed them before anything was committed; it reads only the public file and changes
nothing else. A check that loads the publisher's licence classes then found no value
of a redacted study in either folder.

Declared checks, on human group means (130 series with population forecasts, 521
common next-observation targets):

| check | result |
|---|---|
| H1 calibration, next observation | 435/521 = 0.835 and 485/521 = 0.931: **both inside** (windows 0.70–0.90, 0.80–0.97) |
| H2 accuracy, next observation | **pass**: 5.89 pp against persistence 7.21, pooled-curve ratio 8.67, analog kNN 9.53 and last-two trend 33.28 |
| H3 landmark | **pass**: 7.76 pp against persistence 11.89, pooled-curve ratio 15.80 and analog kNN 15.71 |
| H4 initial state, 118 control group means | **pass**: 10.17 pp against the pooled curve's 18.06 on the same groups; 29/118 within 5/10 pp |
| H5 against 1.1.0 trained on the same studies | **pass**: first-month coverage (131 targets) 0.824 against 0.679 at 80% and 0.939 against 0.817 at 90%, both closer to nominal; next observation 5.885 against 5.877 pp (allowed: +0.05) |

Per series, 80/130 group means were within 5/10 pp for next observation and 38 of
them were also ≥ 0.5 pp better than persistence; for landmark 54 and 33. The 1.1.0
reference had the same counts, as expected when only intervals change.

The following were reported but not tested (`.peykar/iop12_sealed_breakdown.py`,
group means; RMSE and mean signed error in pp of the preoperative IOP, signed error =
forecast − observed):

- **Calibration by target time.** Next observation, 1.2 then 1.1.0:
  - first month (131 targets): coverage 0.82 / 0.68 at 80% and 0.94 / 0.82 at 90%;
  - months 1–6 (190): 0.86 / 0.84 and 0.95 / 0.93;
  - after month 6 (200): 0.82 and 0.91 for both.

  The first-month 90% interval is now slightly too wide, and so are the intervals of
  months 1–6. Landmark coverage was 0.795 and 0.910 (1.1.0: 0.77 and 0.88).
- **Accuracy by target time is unchanged.** Next-observation RMSE was 9.09 pp in the
  first month (persistence 11.62), 4.43 in months 1–6 (5.19) and 4.18 after month 6
  (4.63). The first-month error is still twice the later one.
- **By procedure** (next observation, 1.2 RMSE / persistence, 80% coverage):
  - angle MIGS (261 targets): 5.90 / 7.22, 0.88;
  - trabeculectomy (67): 4.69 / 6.25, 0.85;
  - cyclodestruction (51): 6.54 / 8.27, 0.73;
  - tube (46): 6.78 / 8.16, 0.74;
  - non-penetrating surgery (41): 5.34 / 5.66, 0.71;
  - cataract surgery (22): 6.70 / 9.40;
  - other (20): 4.76 / 3.78, persistence better;
  - subconjunctival MIGS (13): 6.74 / 7.70.

  Coverage is uneven: tubes, cyclodestruction and non-penetrating surgery are
  under-covered while angle MIGS is over-covered.
- **Systematic offsets by procedure.** Landmark forecasts were too high (less
  pressure reduction than observed) after subconjunctival MIGS (+8.8 pp), cataract
  surgery (+7.8), other procedures (+7.5) and non-penetrating surgery (+5.3), and too
  low after cyclodestruction (−2.7) and in the first month after angle MIGS (−4.6).
  Initial-state forecasts were too low after cyclodestruction (−5.1) and angle MIGS
  (−3.5) and too high after other procedures (+8.0), subconjunctival MIGS (+6.4) and
  non-penetrating surgery (+6.4); their error was largest in the first three days
  (16.1 pp, signed −4.6).
- **Single eyes** (210 targets): next observation 10.89 pp against persistence
  13.14, coverage 0.852/0.919; landmark 12.15 against 17.24.

The following failures were seen after unsealing (public CC BY studies). None was used
to change the frozen model.

- **The month-1 rise after Ahmed valves is still missed.** In the conventional Ahmed
  arm of [PMC7603617](https://doi.org/10.1155/2020/8386135) IOP was 40% of the
  preoperative value at week 1 and 61% at month 1; the month-1 forecast was 43% (90%
  interval 34–54%; 1.1.0: 37–50%). In
  [PMC7647454](https://doi.org/10.1371/journal.pone.0241886) the rise was from 44% to
  70%, with a forecast of 46% (38–57%). The wider intervals did not reach these
  hypertensive phases; a forecast from week 1 needs to know that valved tubes usually
  rise in weeks 2–6.
- **A second cyclophotocoagulation** ([PMC9314293](https://doi.org/10.1007/s10792-022-02234-4))
  fell to 52% at week 1 and rose to 68% at month 1. The forecasts ran opposite: 70%
  for week 1 and 51% for month 1 (series RMSE 15.1 pp).
- **Phaco with Sinskey-hook goniotomy**
  ([PMC11182272](https://doi.org/10.3389/fopht.2024.1294651)) went from 84% on day 1 to
  101% at month 1; the forecast was 82% (90% interval 63–106%, so covered), and the
  series RMSE was 13.3 pp.

The figure's lower panels show these two kinds of series: goniotomy arms with phaco
from [PMC12249627](https://doi.org/10.3390/jcm14134610), where 1.2 widens the
first-month interval and keeps the forecasts, and the two Ahmed arms.

Tranche 3 is now consumed, as are tranches 1 and 2. Tranche 4 (94 admitted studies,
79 records extracted blind, see its
[README](../../../data/iop-glaucoma-surgery-2026-10-28/tranche-4/README.md)) is
reserved for model 1.3, trained on tranches 1–3 (229 studies); tranche 5 (19 articles)
stays unread. The failures above point to procedure-specific early courses (valved
tubes, cyclodestruction) and procedure-specific noise as the next changes to test.
