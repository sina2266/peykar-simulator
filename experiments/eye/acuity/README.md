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
  series). Tranche 2 (55 articles) was reserved before triage as the sealed test set
  of the next version ([tranche-2 README](../../data/cornea-acuity-2026-10-29/tranche-2/README.md)):
  27 admitted blind, 11 custodian records with 23 series, **still sealed and unread**.
- Model 1.0: [`model-1.0/`](model-1.0/README.md), generic eye model version 2 on the
  change of acuity through an exact pseudo-fraction, scored in ETDRS letters by
  `peykar.validation.eye_additive_evaluation`. Its one-shot sealed test passed all four
  declared hypotheses on 21 group-mean forecast series, mostly after cross-linking and
  DMEK (next observation 5.00 letters against persistence 5.06; landmark 5.24 against
  7.27; initial state 9.04 against the pooled training curve 9.79;
  [Persian results](model-1.0/results.fa.md)). On development it had lost both forecast
  tasks to the pooled-curve increment, because keratoplasty groups that recover poorly
  for their preoperative acuity were forecast too optimistically.
- Model 1.1 development: [`model-1.1/development-log.md`](model-1.1/development-log.md).
  On the 42 tranche-1 records (development plus the 15 consumed sealed records), model
  1.0.0 now beats the pooled-curve increment for the next observation (5.41 against
  6.02 letters) and is level with it for the landmark task. Two candidates in the new
  generic eye model 4 (`eye_recovery_population4`) were declared and rejected by the
  0.10-letter rule: correlated long-term level and transient amplitude (gains 0.08,
  0.07 and 0.00 letters) and a separate cross-linking slope (initial state 0.12
  letters worse, because the keratoplasty slope steepened). There is no 1.1 protocol;
  model 1.0.0 stays the component's model.

The dataset README is pinned by protocol 1.0.0. Articles kept by other components for
untested or unread tranches (retinal thickness tranches 3–4, IOP tranches 4–5) were
dropped before triage.
