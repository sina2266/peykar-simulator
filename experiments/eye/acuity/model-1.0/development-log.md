# Visual-acuity model 1.0 — development log

Development data: the 27 development records (62 series) of dataset
[`cornea-acuity-2026-10-29`](../../../data/cornea-acuity-2026-10-29/README.md): 17
CC BY records in `development/records` and 10 non-commercial or publisher-licensed
records kept private in `.peykar/acuity-development-private/records`. Twenty of the 47
reserved development studies have no record (complete-case or outcome-selected
analysis sets, Snellen or decimal means, logMAR rows converted from decimal means).
The 62 series are 45 group means and 17 per-eye series (two studies), 60 control and
2 treated; contexts cross-linking 21 series (12 studies), penetrating keratoplasty 15
(4), DSAEK 12 (6), DMEK 9 (3), other 4 (4; all cross-linking combined with a ring
segment or a refractive ablation) and DALK 1 (1). No sealed value is read before the
1.0 protocol is frozen. Studies of one author-overlap cluster
(`author-overlap-1.0.0.json`, 2 development clusters) leave training together in
leave-one-study-out (LOSO). Each round declares its change and its adoption rule
before its LOSO results are seen.

## Model

Generic eye model version 2 (`eye_recovery_population2`, unchanged) with the
configurations of `peykar.model_learning.acuity_population`; series
`peykar.validation.cornea_acuity_data`; scoring
`peykar.validation.eye_additive_evaluation` 1.0.0 on harness 2; operator
`scripts/evaluate_acuity_population.py`.

The model works on the change of corrected distance acuity from its preoperative
value in logMAR, `change(t)/1 logMAR = ell + e r(t) + s q(t)` with
`r = exp(-(t - 1 day)/tau)` and `q = exp(-(t - 1 day)/7 days)`, and
`ell = context level + b logMAR0` (`b = -1` would mean every eye reaches logMAR 0).
The series give the change as the exact pseudo-fraction `exp(change/1 logMAR)` and
logMAR0 as `exp(logMAR0)`, so the generic model and harness apply unchanged and every
comparator becomes its additive analogue. The additive evaluation rewrites forecasts,
observations and intervals in ETDRS letters (0.02 logMAR) and recomputes the scores:
one reported pp is one letter, so the per-series criteria read RMSE ≤ 5 letters
(0.10 logMAR, one line), maximum ≤ 10 letters (two lines) and, for forecasts, at least
0.5 letter below persistence. There is no declared target comparator (no operation
aims at a fixed acuity); the initial state is compared with the pooled curve. Group
means whose group size the article does not give get the noise of a group of the
reference size.

## Round 1 — declared before results (2026-10-29)

- **a0**: contexts `human/procedure` (pk, dalk, dsaek, dmek, cxl, other), grouping
  `("context",)`, a study effect per procedure arm, the generic constants otherwise.
- **a1**: a0 with an indication level: contexts `human/<indication>/procedure`,
  grouping `("treatment", "context")`, so the hierarchy is species → procedure →
  indication (Fuchs dystrophy, bullous keratopathy, keratoconus, graft failure, scar
  or infection, mixed). Recovery after endothelial keratoplasty is known to be faster
  and more complete in Fuchs dystrophy than in bullous keratopathy.
- **a2**: a0 with one hierarchy per operation family: contexts
  `human-graft/procedure` for keratoplasty and `human-crosslinking/procedure` for
  cross-linking (`other` follows its indication: keratoconus → cross-linking). Each
  family gets its own level slope `b`, noise centres and population variances. After
  keratoplasty acuity improves by large amounts that depend strongly on the
  preoperative acuity; after cross-linking it changes little, so one shared slope may
  fit neither.

Adoption rule (as for the other components, in reported pp, here letters): a
candidate replaces a0 only if, on human group means, it improves the pooled RMSE of
its primary method by at least 0.10 pp in at least one task (initial state, next
observation, landmark) and loses no more than 0.05 pp in any. The primary method is
the lowest pooled group-mean next-observation RMSE among the population variants
whose group-mean coverage lies inside both H1 windows ([0.70, 0.90] and
[0.80, 0.97]); ties within 0.05 pp go to the simpler variant (Gaussian). If no
population variant lies inside both windows, the Gaussian variant is the primary and
the miss is recorded. If both a1 and a2 meet the rule, the one with the larger summed
gain over the three tasks is adopted, and round 2 runs **a3** (per-family hierarchies
with an indication level) against it under the same rule. Each run is split into four
shards (`--shard i/4`) and joined by `--merge`.
