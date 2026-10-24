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

See docs/architecture.md, docs/status.md and docs/scientific-contracts.md at repository root.
