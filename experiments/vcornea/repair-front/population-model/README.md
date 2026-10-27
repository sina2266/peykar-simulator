# Cross-study population model of corneal wound closure

Status at this commit: **development finished, protocol frozen, sealed test not yet
run.** This file is updated with the sealed result in a later commit; the frozen
protocol and its hashes are committed first so the test cannot be tuned.

## Why a new approach

Every earlier candidate was a single-study rule tuned or checked on the same three
consumed rabbit studies (16 series). They could not say how much error to expect
in a new lab, because they had no model of between-study variation, no noise
model and no uncertainty. Their failures were informative: re-fitting the earlier
data shows migration speed is fairly conserved (≈60–90 µm/h in all rabbit
studies) while the latent phase varies from ≈1 to ≈17 h between studies.

This component therefore uses:

1. **Kinematics** — the projected radius shrinks at constant speed after a lag
   (Crosson 1986), so speed is in physical µm/h and transfers across wound sizes.
2. **Observation model** — Gaussian noise in √area with a study-level SD that is
   learned, not assumed; a reported area of 0 is censored below a 125 µm residual
   radius instead of being treated as an exact zero.
3. **Hierarchy** — animal within study within species/injury context, fitted by
   penalized REML on per-series grid posteriors. Predictions are distributions.
4. **Cohort assimilation (primary online method)** — at each measurement time all
   animals of the group measured so far update the group mean, the noise level and
   each animal's parameters together. Only values at or before that time are used.
5. **Comparators** — persistence, last-two radius trend, first-to-current trend,
   Crosson's fixed formula, a pooled empirical curve, and an instance-based
   machine-learning forecaster (k-nearest trajectories with time alignment).

Code: `peykar.model_learning.corneal_population`, `corneal_analog`,
`peykar.validation.cornea_population_evaluation`, `cornea_population_data`,
`cornea_population_sources`; operator `scripts/evaluate_corneal_population.py`.

## Development result (leave one study out, 5 studies, 47 series, 168 targets)

All development data were seen while building the model, so this is not an
independent test. Full tables: [development-loso-2026-10-27/tables.md](development-loso-2026-10-27/tables.md).

- No method meets the fixed 5/10 pp criterion in most series. Even the best
  in-sample radius–lag fit of each complete series misses it for 3/9 rabbits and
  9/15 rats, so for those data the criterion is below the measurement floor.
- Cohort assimilation: 6.3 pp (2017 rabbits) and 6.5 pp (DIO mice), similar to
  the local trends on rabbits and better on mice, but 26 pp on the coarse rat
  study whose speed is far from every other species.
- Initial-state prediction of new studies fails for every method (pooled
  20–27 pp); Crosson's formula passes only the 2017 rabbits it was checked on before.

## Frozen protocol

[protocol-1.0.0.json](protocol-1.0.0.json) pins code, development data, the split
reservation, the extraction protocol and both sealed record hashes. The sealed
test (Zeb1 2023 and LSC 2026 mouse studies) is scored once with:

```bash
uv run --frozen python scripts/evaluate_corneal_population.py --sealed \
  --protocol experiments/vcornea/repair-front/population-model/protocol-1.0.0.json \
  --output experiments/vcornea/repair-front/population-model/sealed-test-<date>
```

The operator refuses to run if any pinned hash changed.
