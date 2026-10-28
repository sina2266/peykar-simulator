# IOP after glaucoma procedures — model 1.1.0

Second version of the intraocular-pressure component
([model 1.0.0](../README.md)). It was developed on the 72 tranche-1 records: the 47
development records plus the 25 sealed records consumed by the 1.0.0 test (169
series). It was then tested once on **tranche 2**, a set reserved before triage. The
tranche-2 articles were triaged blind and extracted by seven custodians: 74 records,
192 series ([tranche-2 README](../../../data/iop-glaucoma-surgery-2026-10-28/tranche-2/README.md)).
Nothing here is a clinical or biological validation claim.

## What changed from 1.0.0, and why

`peykar.model_learning.iop_population_1_1` (numpy only; 1.0.0 stays unchanged
because its protocol pins it):

- **Early transient.** The law becomes `ln f(t) = ell + e·r(t) + s·q(t)`, with
  `q(t) = exp(−(t − 1 day)/7 days)`. The 1.0.0 law moves monotonically from day 1
  toward the long-term level, so it cannot dip and then rise in the first weeks.
  Examples are the week-1 rises after angle surgery, and early hypotony before a
  tube's hypertensive phase. With all 72 training records, group means with at least
  five follow-ups had a dof-adjusted residual SD of 0.0755 without the term. With a
  fixed time constant it fell to 0.0636 at 3 days, 0.0621 at 7 days and 0.0629 at 14
  days, and the systematic early residuals of angle surgery and tubes disappeared
  ([diagnostics](diagnostics_1_1.py)).
- **Noise and group size.** The residual SD of a group mean falls with its number
  of eyes (`ln sd = −1.99 − 0.31 ln n`). In 1.1 the group-mean noise scales as
  `(n/40)^−g`, with `g` estimated in each fit (0.24 on all training records).
- **Unchanged parts.** Residuals do not grow with time and consecutive residuals are
  negatively correlated (−0.29), so no drift term was added. The level regression on
  the preoperative pressure, the one hierarchy per species, the exact
  Gaussian-mixture posteriors and the predictive-median point forecast are as in
  1.0.0.

## Development (leave-one-study-out over the 72 records)

The [development log](development-log.md) records each declaration before its run.
Because the context of use is group means, the primary forecaster is selected on
group means. Under that rule the 1.0.0 reference is its Gaussian variant. Results
are in [`development-loso-2026-10-28/`](development-loso-2026-10-28/tables.md); values
of 38 studies are redacted.

| group means (123 series) | 1.1 Gaussian | 1.0.0 Gaussian | persistence |
|---|---|---|---|
| next observation, pooled RMSE pp | 6.77 | 6.85 | 8.05 |
| landmark, pooled RMSE pp | 8.56 | 8.76 | 13.98 |
| initial state, 117 control groups | 11.46 | 11.43 | pooled curve 19.30 |

The adoption rule required a gain of at least 0.10 pp in one task with no loss above
0.05 pp; landmark improved by 0.20 pp, so 1.1 was adopted. A repeat run gave a
byte-identical report. The tie rule (0.05 pp) picked the Gaussian variant over t4.

## One-shot sealed test on tranche 2 (2026-10-28)

The test was run once, after freeze commit `ac8abf83`, with protocol
[`protocol-1.1.0.json`](protocol-1.1.0.json) (SHA `33dff220…`) and all 42 pinned
files and 146 record hashes matching. The operator scored each sealed study with
model 1.1 and, in the same run, with the frozen 1.0.0 as the declared reference.
Each model was fitted on the 72 records minus the training studies linked to that
sealed study (15 sealed studies had such links). Exit 0, 996.3 s, reports `3c126572…`
(1.1) and `7e04c9b9…` (1.0.0). Public artifacts are in
[`sealed-test-2026-10-28/`](sealed-test-2026-10-28/tables.md), with figure
`sealed-results.png` and declared analysis `sealed-analysis.json`; values of 36
studies are redacted.

Declared checks, on human group means (113 series with population forecasts, 425
common next-observation targets):

| check | result |
|---|---|
| H1 calibration, next observation | 341/425 = 0.802 and 381/425 = 0.896: **both inside** |
| H2 accuracy, next observation | **pass**: 6.69 pp against persistence 8.06, pooled-curve ratio 8.60, analog kNN 9.90 and last-two trend 44.56 |
| H3 landmark | **pass**: 8.95 pp against persistence 15.10, pooled-curve ratio 15.44 and analog kNN 17.60 |
| H4 initial state, 99 control group means | **pass**: 9.71 pp against the pooled curve's 18.06 on the same groups; 27/99 within 5/10 pp |
| H5 against 1.0.0 trained on the same records | **pass**: next observation 6.69 against 6.92 pp; landmark 8.95 against 9.40 pp |

Per series, model 1.1 had 58/113 group means within 5/10 pp for next observation,
25 of which were also ≥ 0.5 pp better than persistence. For landmark it had 42/113
and 21; model 1.0.0 had 55 and 22, and 43 and 20.

The following were reported but not tested:

- **Where 1.1 gains.** For next-observation targets within the first month (94), 1.1
  scored 11.12 pp against 11.94 for 1.0.0 and 14.52 for persistence. After the first
  month (331 targets) the two models were equal: 4.73 against 4.59, with persistence
  at 4.86. The gain is the early transient doing what it was added for.
- **Early intervals were too narrow.** First-month next-observation coverage was
  0.585/0.734; later targets were 0.864/0.943. Landmark coverage was 0.671/0.812, the
  80% value below its window.
- **By context.** Next-observation RMSE for 1.1, then 1.0.0 and persistence, on group
  means:
  - angle MIGS (48 series): 6.49 / 6.47 / 7.40;
  - trabeculectomy (17): 5.37 / 5.64 / 6.80;
  - cyclodestruction (14): 7.67 / 7.65 / 9.44;
  - subconjunctival MIGS (11): 8.93 / 9.80 / 10.27;
  - tube (10): 6.64 / 6.31 / 6.88 (1.0.0 better);
  - non-penetrating surgery (7): 4.22 / 5.16 / 7.78;
  - cataract (5): 8.52 / 11.13 / 16.02.
- **Single eyes** (166 targets, including one rabbit study with no population
  prior): 17.73 pp against 23.82 for persistence, coverage 0.831/0.916.

The following failures were seen after unsealing. None was used to change the
frozen model.

- **A rise far above the population prior.** In a KDB goniotomy group
  ([PMC8594178](https://doi.org/10.1186/s12886-021-02159-z)), IOP was 42% of the
  preoperative value on day 1 and 89% at one week. The 1.1 forecast for week 1 was 49%
  (90% interval 41–59%), the series' worst error being 40 pp. The transient prior for
  angle surgery is small (+0.09 in the fit on all training records), so one low day-1 value
  predicts a slow, low course. Such rises (IOP spikes after goniotomy) are what the
  transient can represent once a week-1 value is seen, but not before.
- **Micro-shunts with intrastent.** A PreserFlo study comparing stents with and
  without an intrastent ([PMC11758089](https://doi.org/10.1038/s41598-025-87259-2))
  had treated arms whose early course ran opposite to the context prior. In one arm IOP
  fell from 87% on day 1 to 60% at week 1, while the forecast stayed at 84%
  (next-observation RMSE 9–14 pp per arm). Subconjunctival MIGS was the weakest
  context for landmark (14.82 pp).
- **Tubes.** For the tube context the 1.0.0 model was slightly better (6.31 against
  6.64 pp next observation, 6.80 against 7.88 landmark). The largest tube error
  ([PMC3613091](https://doi.org/10.1155/2013/784709)) came from a hypertensive phase
  at month 1 followed by a fall to 45%.

Tranche 2 is now consumed, as are the development set and tranche 1. Tranches 3–5
(259 screened articles) stay untriaged and unread for the next version.
