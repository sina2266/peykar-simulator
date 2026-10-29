# Refraction after corneal laser refractive surgery — sixth eye component

How the spherical equivalent (SE, dioptres) of a group of eyes changes after
photorefractive keratectomy and other surface ablation, LASIK, and SMILE or other
lenticule extraction: from the preoperative refraction to near plano within days,
then a slow drift (usually a small myopic regression) over months to years. It is the
main refractive outcome of these operations and the fourth corneal component, after
epithelial wound closure, endothelial cell density and central corneal thickness.

- Dataset: [`experiments/data/cornea-refraction-2026-10-29`](../../data/cornea-refraction-2026-10-29/README.md)
  (2,169 screened open-access articles, 89 passed a masked-table rule, all triaged
  blind, 39 admitted, split 31/8 before any value was read; 24 development records with
  43 series and 8 custodian-extracted sealed records with 14 series).
- Model 1.0: [`model-1.0/`](model-1.0/README.md), generic eye model version 2 on the
  change of the SE through an exact pseudo-fraction, scored in tenths of a dioptre by
  `peykar.validation.eye_additive_evaluation`, with a plano-target (full correction)
  comparator. Its one-shot sealed test passed all five declared hypotheses on 13
  group-mean forecast series (next observation 0.29 D against persistence 0.37 D;
  initial state 0.40 D against full correction 0.50 D;
  [Persian results](model-1.0/results.fa.md)).

The dataset has no unread tranche left, and its README is pinned by protocol 1.0.0. A
later version needs a new screen (for example newer articles or other refraction
tables) reserved before triage.
