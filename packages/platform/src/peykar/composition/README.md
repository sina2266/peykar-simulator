# Composition

Static scientific compatibility, exact exchange primitive and durable multi-model
experiment workflows. `experiments.py` freezes model/plan DAGs and dispatches
terminal-to-initial scalar transfers through the bounded worker with atomic child
association, retries, cancellation and result/event provenance. See
docs/experiment-workflows.md. General simultaneous feedback and spatial coupling
still require a numerical solver and biological bridge; DAG execution does not
establish those capabilities.

`batches.py` freezes Cartesian grids and seeded reference replicas, admits bounded
mixed reference/native cohorts and dispatches children atomically under per-cohort
capacity. `batch_results.py` verifies lineage and computes terminal sample statistics
with explicit incomplete/excluded members. Native drivers retain their original
seed/parameter limitations. See docs/epics/E2-reproducible-cohorts.md.

See docs/architecture.md, docs/status.md and docs/scientific-contracts.md at repository root.

`hybrid_flows.py` adds native/reference DAG initial-condition transfer, explicit
interfaces, unit/semantic/context gates and atomic derived-model/child association.
Original models remain immutable. Native inputs currently support only reviewed
SBML concentration derivation; PKML inputs/continuous coupling remain unsupported.
See docs/epics/E3-native-scalar-flows.md.
