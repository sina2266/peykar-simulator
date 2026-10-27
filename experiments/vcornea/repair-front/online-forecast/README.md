# Causal observation updates: retrospective three-study replay

This is an implemented **next-observation forecasting task**, separate from
full-horizon initial-state simulation, original vCornea reproduction or independent
biological validation. All three source studies were already inspected and used
in earlier work. Freezing this new algorithm before its first execution does
not make these consumed data an untouched validation set.

At each actual observation, `corneal_online.forecast_corneal_radius_trend` uses
exactly the last two available area fractions, transforms them to normalized
equivalent radii, and continues their signed secant over the actual next interval:

`s = (sqrt(q_last)-sqrt(q_previous))/(t_last-t_previous)`;
`qhat(t) = max(0,sqrt(q_last)+s*(t-t_last))**2`.

The slope is **estimated from the observed prefix**, not measured cell velocity.
No cross-series training, lag selection, parameter search, upper clipping or
future-value fitting occurs. Above-baseline fractions, expansion and a later
observed reopening remain possible. A zero forecast cannot anticipate an unseen
reopening. This algebraic estimator is not marketed as ML/RL or a cell mechanism.

## Frozen inputs and coverage

- [Protocol1.0.1](protocol-1.0.1.json), SHA256
  `92d7bb2f717b1122cdd23bd33a4bdc8db3e37fab5d037c793113ee1c57c2a9d5`,
  was frozen at2026-10-27T21:39:10Z before the successful actual replay.
- The original [2017 XLSX](../../../data/cornea-evidence-20261024/raw/rabbit-figure2.xlsx)
  supplies9 separate rabbit series, each with10 observations. All72 eligible
  next observations at12–54h are scored; intervals are6h. Group/replicate labels
  are combined to avoid treating repeated labels as the same animal.
- The original [2019 Table1](../conditioned-forecast/ijms2019-original.xml) supplies
  four separate group-mean series:20 forecasts at36–84h,12h intervals. Each value
  is a ratio of reported cohort mean areas, **not individual animal ratios or
  mean animal radii**. SD stays in its original mm²; no ratio SD/CI is invented.
- The original [2021 Table1](../chung2021-manuscript.xml) supplies three aggregate
  curves: three numeric24h forecasts from18h, plus Blade50h from24h. Ocu/Amo50h
  predictions are also generated and retained, but their categorical
  `Completely Healed` statements remain **unscored**, without invented zeros or
  a numerical detection limit. The0h normalization identity is explicitly derived
  and cannot supply an observation. First conditioning uses actual4h/18h values.
- Exactly32 first/warmup numeric rows cannot be forecast with two earlier actual
  observations. They remain in source/evidence/exclusions, not silently removed.
  All16 series/98 predictions/96 numeric scores are mandatory; no late-window
  selection or animal pooling is allowed.

Source provenance, raw units/cells, nominal versus measured geometry, care and
sample hierarchy are in [evidence.json](observed-2026-10-27/evidence.json). These
three sources retain their original terms in the linked acquisition collections;
no source or license is rewritten here. The2019 paired eyes and2021 sacrifices
prevent an independent-animal interpretation of published group aggregates.

## Comparators and fixed acceptance

Persistence uses the latest actual area fraction. First-to-current radius uses
the first **actual** numeric observation and current observation; in2021 this
starts at4h, never an invented measured0h. Both receive the same prefix and
forecast horizon as the candidate. The unchanged Crosson1986 formula uses64µm/h,
5.5h lag and injury-clock time: nominal radius3500µm for2017/3000µm for2021, and
the equivalent circle of the measured group baseline area for2019. It uses less
post-injury information and is a reference formula, not a new84h old-engine run.

Each individual/aggregate series must separately satisfy RMSE≤5 percentage
points, maximum error≤10 points, and at least0.5-point RMSE improvement over
persistence. No pooled average grants acceptance. Passing this retrospective
task would still not establish initial-state or biological validity.

## Actual execution and outcome

The first child exited1 **before any forecast**, because the restrictive profile
denied directory enumeration needed to import the module. Its original frozen
[protocol1.0.0](protocol-1.0.0.json), executed sources and
[failed setup](setup-failed-2026-10-27/execution-record.json) remain unchanged.
Revision1.0.1 allows only listing the reviewed module parent directories, while
neighboring data-file contents remain denied. No method/data/criteria changed;
an import-only engineering probe passed before retrying.

The explicit reviewed operator
`uv run --frozen python scripts/replay_corneal_online_forecasts.py --output .peykar/cornea-online-forecast-retry1-20261027`
completed **98 real child exits0 in16.2691s**, with10s per-child and300s total
bounds. Parent comparison exited2 (`rejected`), correctly retaining the failure.
Every child verified five source/future read denials plus network/write denials.
It received only its prefix, future query hour and initial geometry; its outcome
was copied into its own directory only after exit0. The reviewed trusted-code
profile is not a general untrusted-code security boundary.

All16 series improve on persistence, but only **5/16** meet both gates. Local
RMSE ranges1.8573–18.5612pp; maximum error reaches25.1600pp. It is not uniformly
better than first-to-current or the initial-only prior. Thus it is useful as a
tested generic observation-update building block, **not an accepted general
corneal simulator**. No API/default model registration or live service change
was performed. Read the [complete per-series table](results-table.md),
[machine-readable rows](observed-2026-10-27/report.json), and
[plain Persian report](results.fa.md).

The [per-series error figure](observed-2026-10-27/actual-replay-errors.png) and
[PDF](observed-2026-10-27/actual-replay-errors.pdf) plot the retained metrics only;
the plotting source is retained and the rendered output was visually inspected.
The two single-numeric-target groups have too few observations for a group-wide
accuracy guarantee, irrespective of their point errors.

The failure locations are preserved: all four2019 curves have their largest
local errors at36→48h; Blade2021 misses its longer24→50h interval by25.16pp.
These diagnose changes in the observed area trajectory, not their biological
cause. They do not justify silently adding acceleration, selecting a smoother,
or discarding long intervals after the result.

## Replay and next dependency

[actual-children.zip](observed-2026-10-27/actual-children.zip) retains every actual
prefix, protocol, control, log, prediction, receipt, execution and post-exit
outcome (784 members). Its hash/size are pinned in the archive inventory and the
retained-file manifest. Extract only validated relative `jobs/` members to a
fresh directory beside the retained top-level JSON files; use operator `--analyze`
to check them without launching any scientific process. Regression tests do this
and reject changed prefixes/predictions, rehashed arithmetic changes and a
complete-looking output with a failed child. Raw private runs are not deleted.

Before adding another model variant, require a new, independently reserved,
matched longitudinal dataset and a declared observation/QC protocol. Useful
missing inputs include calibrated wound boundary/shape and timing, measurement
repeatability, stable eye/animal identity, substrate/depth and supportive care.
Do not turn these consumed96 targets into a new untouched test, train an RL
reward on them or promise that more complexity alone will resolve the failures.
The separately acquired2025 partial image cohort still needs frozen quality/
segmentation rules and identity/completeness review; it is not automatically
admitted by this replay. Original cellular publication gates stay open.
