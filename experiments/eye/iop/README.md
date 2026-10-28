# Intraocular pressure after glaucoma procedures — model 1.0.0

Fourth eye component and the first outside the cornea, after epithelial wound
closure, endothelial cell density and central corneal thickness. The model covers
how intraocular pressure (IOP) falls after a glaucoma procedure and drifts back over
months to years. The procedures are trabeculectomy, drainage tubes, micro-shunts,
angle surgery, non-penetrating surgery, cyclodestruction, laser trabeculoplasty and
cataract surgery in glaucoma eyes. Values are fractions of each group's preoperative
IOP. Data: [dataset 2026-10-28](../../data/iop-glaucoma-surgery-2026-10-28/README.md),
with 47 development records (101 series) and 25 sealed records (68 series) extracted
by separate custodians. Nothing here is a clinical or biological validation claim.

## Model

`peykar.model_learning.iop_population` (numpy only):

- Kinetics `ln f(t) = ell + e·r(t)`, with `r(t) = exp(−(t − t1)/tau)` and `t1` = 1
  day. `ell` is the long-term level, `e` the deviation from it one day after surgery
  (early hypotony after filtering surgery, or an early rise), and `tau` the time scale
  of an exponential return. On development series with at least four follow-ups,
  one exponential phase fitted group means better than hyperbolic or power-law
  returns: dof-adjusted log-SD 0.0682 against 0.0716 (hyperbolic), 0.0698 (power 2)
  and 0.0760 (power 0.5) ([round 0](round0_law_comparison.py)).
- Regression toward a common target: `ell` = context level + `b·x` with
  `x = ln(IOP0/20 mmHg)`. One slope `b` is shared by all procedures and estimated as
  a fixed effect inside each REML fit. On all development controls `b` = −0.74 (SD
  0.06), so the postoperative pressure changes by only about 0.26% per 1% of
  preoperative pressure. A group starting at 30 mmHg is therefore expected to end at
  a lower fraction of its starting pressure than a group starting at 20 mmHg.
- One hierarchy per species, without mechanism families: species mean, procedure
  context, study, and series (group mean or single eye). A procedure without
  training studies falls back to the species mean. This avoids the thickness
  model's sealed failure, where laser capsulotomy assigned to the wrong family was
  badly predicted.
- Given `tau`, the law is linear in `(ell, e)`. Posteriors are exact Gaussian
  mixtures over an 80-point `tau` grid and over noise levels; quantiles come from the
  exact mixture distribution function, with no sampling. Variances are estimated by
  penalized REML on per-series likelihood summaries, as in the thickness model.
- Point forecast: the median of the posterior predictive distribution. The mean of
  `exp` of a Student-t variable does not exist, and posterior means exploded for the
  heavy-tailed variants in development round 1.
- Primary forecaster `population_assimilation_robust`: the population prior mixed
  0.8/0.2 with a broad component, then updated exactly by the observed follow-ups.

## Tasks and criteria

The tasks are the same as for the corneal components:

- U1 initial state: from context, series kind and preoperative IOP only.
- U2 next observation.
- U3 landmark: from the first follow-up only.

Per series, RMSE must be ≤ 5 pp and max error ≤ 10 pp. For U2 and U3, RMSE must
also be at least 0.5 pp below persistence. The calibration windows are 80% intervals
within [0.70, 0.90] and 90% intervals within [0.80, 0.97]. The tested hypotheses
cover group means only: in development, single-eye forecasts were worse than
persistence and their intervals too narrow. Possibly shared cohorts are handled with
[author-overlap-1.0.0.json](../../data/iop-glaucoma-surgery-2026-10-28/author-overlap-1.0.0.json)
(`scripts/link_study_authors.py`). Three sealed studies were trained without their
linked development study.

## Development (leave-one-study-out)

The [development log](development-log.md) records each declaration before its run.
Round 1 used posterior-mean point forecasts. Round 2 used predictive medians; it was
declared before the run and adopted by the declared rule. The primary forecaster was
chosen by a rule fixed before round 1. Results are in
[`development-loso-2026-10-28/`](development-loso-2026-10-28/tables.md); values of 24
studies with NC/ND/SA or publisher licences are redacted.

| task (group means) | primary | pooled RMSE pp | comparator | within 5/10 pp | meeting all criteria |
|---|---|---|---|---|---|
| U2 next observation | robust | 6.34 | persistence 7.67 | 42/80 | 19/80 |
| U3 landmark | robust | 8.66 | persistence 14.61 | 36/80 | 24/80 |
| U1 control group means | initial state | 12.04 | pooled curve 21.44 | 20/77 | 20/77 |

Over all series, including single eyes, next-observation RMSE was 9.45 pp against
10.15 for persistence and landmark RMSE 13.39 against 15.89.

## One-shot sealed test (2026-10-28)

The test was run once, after freeze commit `ff470345`, with protocol
[`protocol-1.0.0.json`](protocol-1.0.0.json) (SHA `f13238a2…`) and every pinned
hash matching. Operator: `scripts/evaluate_iop_population.py --sealed`, exit 0,
88.8 s, report SHA `9c62c0f4…`. Coverage: 25 studies and 54 forecast series, of
which 43 are group means with 198 next-observation targets. Public artifacts are in
[`sealed-test-2026-10-28/`](sealed-test-2026-10-28/tables.md): figure
`sealed-results.png` and declared analysis `sealed-analysis.json`. Values of 14
studies without CC BY/CC0 terms are redacted.

| declared check (group means) | result |
|---|---|
| H1 calibration, next observation | 151/198 = 0.763 and 177/198 = 0.894: **both inside** |
| H2 accuracy, next observation | **pass**: 7.74 pp against persistence 8.64, pooled-curve ratio 8.98, analog kNN 9.77 and last-two trend 39.98 |
| H3 landmark | **pass**: 9.92 pp against persistence 12.89, pooled-curve ratio 14.24 and analog kNN 14.82 |
| H4 initial state, 40 control group means | **pass**: 10.96 pp against pooled curve 17.99; 10/40 within 5/10 pp |
| per series | next observation 21/43 within 5/10 pp, 11/43 also ≥ 0.5 pp better than persistence; landmark 17/43 and 10/43 |

All four declared hypotheses passed, but most group means still miss the fixed
per-series criteria. The following were reported but not tested:

- **Where the gain comes from.** For next-observation targets in the first month
  (40 targets), the model scored 11.94 pp against 14.27 for persistence. For later
  targets (158) it scored 6.24 against 6.47, almost no gain. The landmark gain holds
  after the first month (9.01 against 12.39).
- **Landmark calibration.** The 80% intervals covered 0.652, below the window; the
  90% intervals covered 0.823. Long-horizon intervals were too narrow.
- **Variants.** The Gaussian and t4 variants were slightly better than the declared
  robust primary: 7.71 and 7.65 pp for next observation, and 9.57 for landmark.
- **Single eyes.** 11 series with at least two follow-ups (33 targets) scored 8.92 pp
  against 12.48 for persistence, with coverage 0.879/0.909. This is the opposite of
  development (19.58 against 19.17) and too small a sample to claim.
- **Treated groups** (16 targets): 5.12 against 5.57 pp.

Results by context for group-mean next observation (model RMSE, then persistence
RMSE):

- angle MIGS (17 series): 8.57/10.37 pp;
- trabeculectomy (9): 5.70/6.07 pp;
- cyclodestruction (6): 11.46/10.79 pp, worse than persistence;
- tube (4): 4.98/5.62 pp;
- cataract (3): 2.01/1.80 pp;
- non-penetrating surgery (2): 4.18/2.59 pp;
- subconjunctival MIGS (2): 4.00/4.40 pp.

The following failures were seen after unsealing. None was used to change the
frozen model.

- **Early rises after angle surgery.** In a GATT study
  ([PMC12109802](https://doi.org/10.3390/diagnostics15101226)), the angle-closure
  phaco-GATT group rose from 102% of its preoperative IOP on day 1 to 130% at one
  week. It then swung between 67% and 96% from visit to visit. The open-angle
  stand-alone GATT group rose from 51% on day 1 to 73% at weeks 1–3, then settled
  at 50–59%. In a canaloplasty study
  ([PMC5642877](https://doi.org/10.1155/2017/2904272)), one group was at 38% on day
  1 and 72% at one week. The kinetic law moves monotonically from the day-1
  deviation toward the long-term level, so it cannot follow a dip followed by a rise.
  The one-week forecasts missed by 20–34 pp. Hyphema, steroid response, stopping
  glaucoma medication (the reference is a medicated pressure) or early failure of
  outflow are possible causes; none was tested. A second early component or a wider
  early-noise model would be needed and must be tested on new data.
- **Waning cyclodestruction effect.** One micropulse transscleral laser study with
  five energy subgroups (CC BY-NC; values redacted) lost part of its effect after
  the first month. The model has no waning mechanism. Cyclodestruction group-mean
  next-observation RMSE was 11.46 pp against 10.79 for persistence, landmark 14.13
  against 11.80, and initial state 21.49 against 17.27 for the pooled curve.
- **Between-study level differences.** Only 10/40 control group means met the
  initial-state criterion. The preoperative pressure and the procedure do not fix a
  study's long-term level; for example, a trabeculectomy group with unusually strong
  lowering (values redacted) had an initial-state RMSE of 18.4 pp. Once one or two
  follow-ups are seen, assimilation corrects most of this.

Both the development and the sealed sets are now consumed. A further version must
be tested on the reserved tranches 2–5 (379 screened articles that no one has
triaged or read).

**Model 1.1** ([README](model-1.1/README.md),
[Persian results](model-1.1/results.fa.md)) adds a fast early transient and noise
scaled by group size. It was trained on all 72 tranche-1 records and tested once on
tranche 2, where all five declared hypotheses passed. These include beating 1.0.0
trained on the same records: 6.69 against 6.92 pp for next observation and 8.95
against 9.40 pp for landmark.
