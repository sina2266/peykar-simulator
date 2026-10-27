# Model Learning

Log-linear decay parameter proposal and an explicit offline constrained corneal
front learner are implemented. The latter fits exactly six complete endpoint-dose
rabbits, rejects intermediate-dose rows, caps objective evaluations, propagates
cancellation and records source/protocol/data hashes. It constructs four empirical
parameters; it does not register or approve a model or read a holdout implicitly.

The reviewed operator is `scripts/learn_corneal_front.py`. Its frozen experiment,
actual isolated execution and failed heldout/independent comparisons are retained
in `experiments/vcornea/repair-front/learned-front/`. New learned parameters do not
establish a causal drug mechanism, independent biological validity or full cellular
publication reproduction. General surrogate construction, structure refinement,
optimization and promotion remain future controlled changes.

`corneal_online.forecast_corneal_radius_trend` adds a pure prefix-only observation
state estimator: exactly two latest actual area fractions define a signed
normalized-radius slope over real hours. It preserves expansion and later
observed reopening, rejects future rows and does not train across series or
register a model. This is observation-informed forecasting, not an initial-state
simulator or measured cell speed. The explicit offline three-study replay in
`experiments/vcornea/repair-front/online-forecast/` retains98 real children and the
failed fixed precision gate (5/16 series pass); no biological promotion follows.

`corneal_population` adds a numpy-only hierarchical radius-lag model: per-series
grid posteriors, censored zero areas (125 µm detection radius), study noise SD
marginalized on a log-normal prior, penalized REML pooling across studies and
species/injury contexts, initial-state cohort predictions and exact grid
assimilation of an observed prefix; `cohort_posteriors` updates all animals of a
group measured up to the same hour together. `corneal_analog` is the declared
machine-learning comparator (k-nearest physical-displacement trajectories with time
alignment, fixed hyperparameters). Neither registers a model; their estimates are
kinematic summaries, not measured cell speeds or mechanisms.
See docs/architecture.md, docs/status.md and docs/scientific-contracts.md at repository root.
