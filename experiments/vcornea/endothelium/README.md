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
