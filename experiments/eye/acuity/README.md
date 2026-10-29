# Corrected visual acuity after keratoplasty and cross-linking — seventh eye component

How the corrected distance visual acuity (BCVA or CDVA, logMAR) of a group of eyes
changes after penetrating keratoplasty, deep anterior lamellar keratoplasty,
endothelial keratoplasty (DSAEK, DMEK) or corneal collagen cross-linking: a large
recovery over weeks to years after keratoplasty, and little change after
cross-linking, which aims to stop keratoconus progressing. Acuity is the functional
outcome of these operations; it is the fifth corneal component, after epithelial wound
closure, endothelial cell density, central corneal thickness and refraction.

- Dataset: [`experiments/data/cornea-acuity-2026-10-29`](../../data/cornea-acuity-2026-10-29/README.md)
  (1,994 screened open-access articles, 175 passed a masked-table rule, the 120 of
  tranche 1 triaged blind, 67 admitted, split 47/20 before any value was read; 27
  development records with 62 series and 15 custodian-extracted sealed records with 25
  series). **Tranche 2 (55 articles) is untriaged and unread**, reserved as the fresh
  test set of the next model version.
- Model 1.0: [`model-1.0/`](model-1.0/README.md), generic eye model version 2 on the
  change of acuity through an exact pseudo-fraction, scored in ETDRS letters by
  `peykar.validation.eye_additive_evaluation`. Its one-shot sealed test passed all four
  declared hypotheses on 21 group-mean forecast series, mostly after cross-linking and
  DMEK (next observation 5.00 letters against persistence 5.06; landmark 5.24 against
  7.27; initial state 9.04 against the pooled training curve 9.79;
  [Persian results](model-1.0/results.fa.md)). On development it had lost both forecast
  tasks to the pooled-curve increment, because keratoplasty groups that recover poorly
  for their preoperative acuity were forecast too optimistically.

The dataset README is pinned by protocol 1.0.0. Articles kept by other components for
untested or unread tranches (retinal thickness tranches 3–4, IOP tranches 4–5) were
dropped before triage.
