# Scenario and failure matrix

Tests listed here are the local reference profile. Designed scenarios have their
own future acceptance gates and are deliberately not represented as passing tests.

| # | Scenario | Expected behavior | Current evidence |
|---|---|---|---|
| 1 | Import unchanged descriptor twice | Same immutable digest | Registry integration test |
| 2 | Change content at same model version | Conflict | Immutable-model test |
| 3 | Exact model dependency | Resolve and snapshot closure | Dependency test |
| 4 | Dependency cycle/missing model | Reject before execution | Dependency test |
| 5 | Convert minutes to seconds | Equivalent numerical output | Unit/runtime tests |
| 6 | Connect concentration to incompatible unit | Reject | Unit/port tests |
| 7 | Compatible units but different semantic identity | Reject | Port tests |
| 8 | Coordinate/shape/scale mismatch | Require explicit mapping | Port tests |
| 9 | Cross-species execution | Require a new supported context | Context test |
| 10 | Two writers target one input | Reject ambiguity | Composition test |
| 11 | Feedback/algebraic loop | Reject until solver exists | Composition test |
| 12 | Two-compartment exchange | Conserve mass; partition-invariant exact step | Analytical tests |
| 13 | Deterministic scalar model | Match analytical solution | Decay verification |
| 14 | Stochastic population model | Repeat with same seed; differ with another | Logistic tests |
| 15 | Intervention at aligned time | Apply only to following interval | Piecewise decay test |
| 16 | Unsupported state reset or off-grid event | Reject | Intervention tests |
| 17 | Duplicate request with same idempotency key | Same run ID | Service/API tests |
| 18 | Same key, different request | Conflict | Idempotency test |
| 19 | Concurrent worker claims | Every job claimed at most once | Parallel claim test |
| 20 | Cancel queued job | Never execute it | Cancellation test |
| 21 | Cancel running job while result completes | Committed cancel request wins | Race test |
| 22 | Restart service | Queued work and manifest survive | Restart test |
| 23 | Recover dead worker | Explicit failed state; fence stale result | Recovery test |
| 24 | Corrupt artifact or manifest | Fail integrity check | Corruption tests |
| 25 | Path traversal in artifact ID | Reject | Digest validation tests |
| 26 | Nonfinite values or oversized sample request | Reject before allocation | Contract/budget tests |
| 27 | Calibration set used as holdout | Reject by declared role/group | Leakage tests |
| 28 | Synthetic benchmark passes | Remain synthetic verification | API evaluation test |
| 29 | Dataset grid/endpoint differs | Require observation operator | Evaluation tests |
| 30 | Anonymous or reader mutation | 401 or 403 | API auth tests |
| 31 | Oversized/chunked request | 413 | Body-limit tests |
| 32 | Unsupported future DB schema | Refuse startup | Migration test |
| 33 | Candidate improves training only | No automatic promotion | Designed governance gate |
| 34 | Valid models compose unstably | Reject based on convergence/error | Designed coupling gate |
| 35 | Gene intervention to tissue effect | Evidence at every cross-scale bridge | P5 gate |
| 36 | Liver exposure to eye response | Explicit PK/exposure coupling | P7 gate |
| 37 | External model tries network/file access | Isolated execution denies it | External sandbox gate |
| 38 | Dashboard animates uncomputed cells | Mark illustrative or omit | UI gate |
| 39 | New data invalidate historical claim | Impact graph and revalidation | Governance roadmap |
| 40 | GPU/parallel run differs numerically | Declared tolerance/environment analysis | External engine gate |
