# Best-corrected visual acuity after treatment of macular disease — eighth eye component

How the best-corrected visual acuity (BCVA, logMAR or ETDRS letters) of a group of eyes
changes after macular edema (diabetic, vein occlusion, uveitic or of another cause) or
neovascular AMD is treated with anti-VEGF injections, corticosteroid implants or
injections, laser or a combination. Acuity is the functional counterpart of the
central retinal thickness component.

- Dataset: [`experiments/data/retina-acuity-2026-10-29`](../../data/retina-acuity-2026-10-29/README.md)
  (3,655 screened open-access full texts, 378 passed a masked-table rule, the 120 of
  tranche 1 triaged blind, 62 admitted, split 41/21 before any value was read; 15
  development records with 52 series and 9 custodian-extracted sealed records with 11
  series). Tranches 2–4 (258 articles) are untriaged and unread, reserved as fresh test
  data for the next version.
- Model 1.0: [`model-1.0/`](model-1.0/README.md), generic eye model version 2 on the
  change of acuity through an exact pseudo-fraction, scored in ETDRS letters. **Its
  one-shot sealed test failed all four declared hypotheses** (next observation 6.14
  letters against persistence 4.81; landmark 9.17 against 6.49; initial state 16.40
  against the pooled training curve 5.18; 80% coverage 0.677;
  [Persian results](model-1.0/results.fa.md)). After unsealing, the failure traced to
  one sealed study with a day-1 visit: the law's early transient term had no training
  visit before day 30 and was left at about +2 logMAR at day 1. Without that study the
  four checks would pass, which is a post-hoc reading and not a test. Model 1.0 is not
  offered by `peykar eye`.

The dataset README is pinned by protocol 1.0.0. Articles kept by other components for
untested or unread tranches (retinal thickness tranches 3–4, IOP tranches 4–5, corneal
acuity tranche 2) were dropped before triage.
