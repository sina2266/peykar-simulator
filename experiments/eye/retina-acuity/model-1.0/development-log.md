# Retinal visual-acuity model 1.0 — development log

Development data: the 15 development records (52 series) of dataset
[`retina-acuity-2026-10-29`](../../../data/retina-acuity-2026-10-29/README.md): 13 CC BY
records in `development/records` and 2 non-commercial records kept private in
`.peykar/retina-acuity-development-private/records`. Twenty-six of the 41 reserved
development studies have no record (20 complete-case sets, 5 whose acuity is not
established as best-corrected logMAR or letters, 1 selected after baseline). The 52
series are 24 group means and 28 per-eye series (one study of a subretinal graft,
treated); 20 series are control group means in 11 studies: diabetic macular edema
with anti-VEGF 5, steroid implant 3 and steroid injection 2; vein occlusion with
anti-VEGF 4, steroid implant 2 and anti-VEGF plus laser 2; uveitic edema with a
steroid implant 1; neovascular AMD with anti-VEGF 1. No sealed value is read before
the 1.0 protocol is frozen. Studies of one author-overlap cluster
(`author-overlap-1.0.0.json`, 2 development clusters) leave training together in
leave-one-study-out (LOSO). Each round declares its change and its adoption rule
before its LOSO results are seen.

## Model

Generic eye model version 2 (`eye_recovery_population2`, unchanged) with the
configurations of `peykar.model_learning.retina_acuity_population`; series
`peykar.validation.eye_retina_acuity_data`; scoring
`peykar.validation.eye_additive_evaluation2` (version-2 configurations go to the harness
of the corneal acuity component, unchanged); operator
`scripts/evaluate_retina_acuity_population.py`.

The model is that of the corneal acuity component: the change of best-corrected acuity
from its pretreatment value in logMAR, `change(t)/1 logMAR = ell + e r(t) + s q(t)`
with `r = exp(-(t - 1 day)/tau)`, `q = exp(-(t - 1 day)/7 days)` and
`ell = context level + b logMAR0`, given to the generic model as the exact
pseudo-fraction `exp(change/1 logMAR)` with logMAR0 as `exp(logMAR0)`. Scores are in
ETDRS letters (0.02 logMAR): the per-series criteria read RMSE ≤ 5 letters, maximum
≤ 10 letters and, for forecasts, at least 0.5 letter below persistence. Contexts are
`human/<indication>/<treatment>`.

## Round 1 — declared before results (2026-10-29)

- **v0**: one level per context, grouping `("context",)`, a study effect per context
  arm, the generic constants otherwise (the corneal acuity 1.0.0 settings). A context
  without training studies falls back to the species level.
- **v1**: v0 with contexts nested in their treatment, grouping
  `("treatment", "context")`: an indication of a known treatment borrows that
  treatment's level, amplitude and time constant. With 11 control studies spread over 8
  contexts, most contexts have one or two training studies, and a treatment level may
  pool them better than the species level alone.

Primary method of a candidate: among the population variants whose next-observation
coverage of group means lies inside both H1 windows ([0.70, 0.90] at 80%, [0.80, 0.97]
at 90%), the lowest pooled next-observation RMSE; a variant within 0.05 letters of that
minimum that comes earlier in the order Gaussian, t4, robust, anchored, anchored t4 is
preferred (the earliest one). If no variant lies inside both windows, the robust
variant is the primary and the miss is recorded. The same variant is the primary of the
landmark task; the initial state uses `population_initial_state`.

Adoption rule: v1 replaces v0 only if it improves the pooled RMSE of its primary by at
least 0.10 letters in at least one task (next observation, landmark, initial state) and
loses no more than 0.05 letters in any. Otherwise v0 is model 1.0.0. The protocol keeps
the H1–H4 hypotheses and comparators of the other eye components (persistence, last-two
trend, pooled-curve increment and analog kNN for the forecasts; the pooled training
curve for the initial state), whatever development shows.
