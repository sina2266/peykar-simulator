# Maximum keratometry after cross-linking or ring segments — ninth eye component

How the maximum keratometry (Kmax, the steepest anterior corneal power, in dioptres) of a
group of eyes with keratoconus or corneal ectasia moves from its baseline value after
corneal collagen cross-linking (any protocol), intracorneal ring segments, both, or under
observation without treatment. Kmax is the main measure of keratoconus progression and of
the flattening these treatments aim at. It is the sixth corneal component, after
epithelial wound closure, endothelial cell density, central corneal thickness,
refraction and corrected visual acuity.

- Dataset: [`experiments/data/cornea-kmax-2026-10-30`](../../data/cornea-kmax-2026-10-30/README.md)
  (1,526 screened open-access articles, 102 passed a masked-table rule with at least one
  follow-up time, all in tranche 1; triaged blind, 62 admitted, split 40/22 before any
  value was read; 25 development records with 35 series and 17 custodian-extracted
  sealed records with 33 series). The sealed set was consumed by the one-shot test of
  model 1.0.0 on 2026-10-30; no unread article is left in this dataset.
- Model 1.0: [`model-1.0/`](model-1.0/README.md), generic eye model version 2 on the
  change of Kmax through an exact pseudo-fraction, scored in tenths of a dioptre by
  `peykar.validation.eye_additive_evaluation4`, without the generic 7-day transient and
  with forecast-only pooled noise. Its one-shot sealed test passed H1 (coverage
  0.745/0.855) and H2 (next observation 10.20 pp against persistence 11.91 and the
  pooled-curve increment 10.73) and **failed H3** (landmark 12.60 against the
  pooled-curve increment 12.35) and **H4** (initial state 14.61 against the pooled
  training curve 14.17; [Persian results](model-1.0/results.fa.md)). One study of eyes
  treated under 18 years, which kept flattening for five years while the law levelled
  off after the first year, carries both failures. Kmax is therefore not offered by
  `peykar eye`.
