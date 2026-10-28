# Corneal endothelial cell density model 1.0.0

Second eye component after epithelial wound closure: how endothelial cell density
(ECD) changes after keratoplasty, cataract, glaucoma, phakic-lens and other eye
surgery, as a fraction of each group's reference count (preoperative patient count,
or donor count for grafts). Data: [dataset 2026-10-28](../../data/cornea-endothelium-2026-10-28/README.md),
59 development records (148 series) and 17 sealed records extracted by separate
custodians. Nothing here is a clinical or biological validation claim.

## Model

`peykar.model_learning.endothelial_population` (numpy only):

- Kinetics `f(t) = (1 - A) (1 + t/tau)^(-p)`: loss `A` already present at the first
  postoperative count, then a decelerating decline (onset `tau` months, exponent
  `p`). On development series with at least four follow-ups this fitted graft
  trajectories better than one exponential phase plus a constant chronic rate with
  the same number of parameters (dof-adjusted log-SD 0.078 versus 0.093; host
  surgery equal).
- One hierarchy per tissue family (`human/host`, `human/graft`): family mean,
  procedure context, study and series levels with diagonal variances estimated by
  penalized REML; a procedure without training studies falls back to its family.
  A graft series referenced to the recipient's preoperative count is its own
  context.
- Noise: Gaussian in `ln f`, one SD per study, family and series kind; a new
  study's SD is marginalized around the family/kind median.
- Forecasts: the primary forecaster `population_anchored_t4` uses a Student-t(4)
  population prior and treats each series' level as carrying an unknown constant
  offset (reference-count and device differences persist across follow-ups), so it
  learns from the shape of the observed prefix and stays anchored to its level.

## Tasks and criteria

- U1 initial state: control group means at all follow-ups from context only.
- U2 next observation: after each follow-up, the next one (control and treated).
- U3 landmark: from the first follow-up only, every later follow-up.
- Per series: RMSE ≤ 5 pp and max ≤ 10 pp of the reference ECD; for U2/U3 also
  RMSE at least 0.5 pp below persistence. Calibration windows: 80% intervals within
  [0.70, 0.90], 90% intervals within [0.80, 0.97].

Studies sharing at least two Europe PMC author names
([author-overlap-1.0.0.json](../../data/cornea-endothelium-2026-10-28/author-overlap-1.0.0.json),
`scripts/link_endothelium_authors.py`) may share patients: linked development
studies leave training together, and each sealed study's linked development
studies are removed from its training set.

## Development (leave-one-study-out, reference round 3)

[`development-loso-2026-10-28/`](development-loso-2026-10-28/tables.md); values of
24 studies with NC/ND/SA licences are redacted in `report-public.json`.

| task | primary | pooled RMSE pp | persistence | series within 5/10 pp | meeting all criteria |
|---|---|---|---|---|---|
| U2 next observation | population_anchored_t4 | 4.02 | 5.14 | 99/115 | 51/115 |
| U3 landmark | population_anchored_t4 | 5.46 | 10.07 | 86/115 | 57/115 |
| U1 control group means | population_initial_state | 9.34 | pooled curve 9.45 | 52/111 | 52/111 |

Calibration of the primary forecaster: next observation 0.820 (80%) and 0.897
(90%); landmark 0.793 and 0.908; initial state 0.749 and 0.850. In host surgery the
next follow-up is already close to the last one, so persistence is hard to beat
(pooled 2.44 versus 2.43 pp); the model's gain comes from grafts (next observation
5.72 versus 7.79 pp) and from long horizons (landmark: host 3.35 versus 4.02 pp,
grafts 7.77 versus 15.59 pp). Predicting a new graft study's group means from
context alone is not accurate (4/22 within criteria, pooled 14.15 pp). Single eyes
are poorly predicted from context alone (1/26 within criteria, under-covered); they
are not claimed.

The frozen sealed-test protocol is [protocol-1.0.0.json](protocol-1.0.0.json).

## One-shot sealed test (2026-10-28)

Run once after commit `00d2cd09` with every pinned hash matching:
`scripts/evaluate_endothelial_population.py --sealed --protocol protocol-1.0.0.json`
(exit 0, 141.1 s, report SHA `a9de39ad…`). 17 studies, 30 group-mean series, 76
next-observation targets; three sealed studies trained without their
author-linked development study. Public artifacts:
[`sealed-test-2026-10-28/`](sealed-test-2026-10-28/tables.md) (values of five
studies without CC BY/CC0 terms redacted), figure `sealed-results.png`, declared analysis
`sealed-analysis.json`.

| declared check | result |
|---|---|
| H1 calibration, next observation (80% in [0.70, 0.90]; 90% in [0.80, 0.97]) | 54/76 = 0.711 and 63/76 = 0.829: **both inside**, both near the lower edge (development 0.820/0.897) |
| H2 accuracy, next observation | **pass**: 5.60 pp versus persistence 7.36 and last-two trend 15.50 (analog kNN 5.35 on its 73 targets, model 5.28 there) |
| H3 landmark (from the first follow-up) | **pass**: 6.85 versus persistence 12.41 pp; coverage 0.724/0.908 inside both windows |
| H4 initial state, 29 control groups | 7.05 pp versus pooled curve 8.16; 13/29 within 5/10 pp; coverage 0.848/0.919 |
| per series, next observation | 23/30 within 5/10 pp; 14/30 also ≥0.5 pp better than persistence |

By context (next observation, model versus persistence RMSE): cataract 2.06/3.00
(10/10 within), DSAEK 5.37/10.47 (6/9), vitreoretinal 5.04/5.94 (4/6; no training
study, host-family fallback), DMEK 8.69/5.73 (1/2), PK with a recipient-referenced
label 14.44/22.65 (0/1, graft-family fallback).

Failures and limits seen after unsealing (not used to change the frozen model):

- The worst public series (10-year DMEK group, PMC9156484) dropped from 53% to 29%
  of the donor count between 60 and 84 months and then stayed flat. The forecaster
  anchors to the mean offset of the whole prefix, so it followed the late drop
  slowly (9.05 versus persistence 5.36 pp). A local-level (random-walk) offset
  would address this but needs a new version and new sealed data.
- Long gaps (12 → 36 → 60 months in a DSAEK cohort with FECD, non-FECD and
  glaucoma-bleb groups) stay above 5 pp even though the model roughly halves the
  persistence error.
- Silicone-oil vitreoretinal groups and a PK group whose reference label was the
  recipient's preoperative count fail; both contexts had no training study.
- Initial-state prediction of a new study's group means is within 5/10 pp for 13
  of 29 groups; between-study differences remain larger than the criterion.

Both the development and the sealed sets are now consumed. A further version must
be tested on newly reserved studies.
