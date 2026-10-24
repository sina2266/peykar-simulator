# Necessary-condition audit for a shared corneal wound front

The [frozen protocol](protocol-1.0.0.json) and [actual read-only report](observed-2026-10-24/report.json)
compare exact common measured times18/24h, without interpolated observations,
new simulation, speed/lag fitting or changed original thresholds.

For `a=max(0,1-d/R)^2`, each measured-area interval implies an interval of
admissible inward displacement `d`. All intervals must overlap if the **same
deterministic displacement** is to describe the2017PBS mean and2021Blade/Ocu/Amo
means with their nominal radii3500/3000µm.2021's printed2-decimal rounding is
allowed±0.005; this is not animal variability or a confidence interval.

Neither shared time is feasible at the10-percentage-point maximum error target.
Even an arbitrary common time function has an optimistic maximum-error lower
bound of**21.2858pp** across these times (16.6872pp at18h,21.2858pp at24h).
All six pairwise2017-vs-method checks also fail10pp. At24h the limiting PBS/Blade
means are0.386178 and0.760: a single inferred displacement cannot explain both.
Inferred displacements and errors are **inverse diagnostics on inspected data**,
not independently validated forward predictions or a causal biological finding.

The solver represents displacements in `[0,max(R)]` because every larger value
has the same fully closed observations. Its finite canonical interval does not
identify a unique physical post-closure displacement. Cohort means of nonlinear
individual trajectories need not equal a trajectory at a common displacement;
this audit is **not a bound on arbitrary stochastic/hierarchical models**.
Unmeasured geometry, substrate, care and injury differences remain unresolved.

```bash
uv run --frozen python scripts/audit_corneal_shared_front.py \
  --output .peykar/corneal-shared-front-new-audit
```

The actual report, executed solver/operator and frozen protocol are retained
without new biological validity or alteration of any earlier simulation.
