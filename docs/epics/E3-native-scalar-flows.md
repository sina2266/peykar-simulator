# E3 — explicit scalar workflows across native and reference solvers

This engineering EPIC extends S4.6 with actual dependency-driven native/reference
execution. It supports scalar terminal-output to initial-condition transfer.
Each stage starts its own local model time at zero. It does not implement shared
continuous time, feedback, checkpoint continuation or a validated biological bridge.

## Native interfaces and derived models

`NativeInterfaceSpec` binds exact admitted model versions to named scalar ports,
physical units, semantics, scale, direction and input write ownership. COPASI output
ports identify both their native CSV column and original SBML species ID; runtime
metadata must match. PKML can expose original output quantity paths. Rewriting PKML
inputs remains unsupported. Registering an interface executes no model.

The implemented SBML input transformation is deliberately bounded: core Level 3,
explicit dynamic `initialConcentration`, concentration interpretation, fixed 3D
litre compartment and mole substance units. Unknown/custom unit definitions,
amount-only input, assignment/initial-assignment/event overrides and negative
concentrations are rejected. Supported inherited units must be explicitly present
on the model; absent physical units are not guessed.

The [SBML L3V2 core specification](https://sbml.org/specifications/sbml-level-3/version-2/core/release-2/sbml-level-3-version-2-release-2-core.pdf)
distinguishes initial concentration from amount and the mathematical meaning of a
species identifier. E3 checks both instead of equating any numeric species value
with concentration. Its restricted transformation does not claim full SBML support.

Each transferred concentration creates a separately admitted, immutable derived
SBML model. A namespace-aware parser locates the exact initial-value attribute;
only its numeric bytes are replaced. All other bytes, including comments,
namespaces, units and notices, are preserved. The original model is never changed.
A content-addressed derivation receipt retains the complete parent descriptor,
interface, original license/source, source run/manifest/result identities, quantities,
conversion units and resulting XML digest. The child uses the same license and
context. A derived model is not presented as a published original or validated model.

## Scheduling and provenance

`HybridFlowSpec` freezes all node manifests/interfaces and explicit evidence-linked
bindings. Direction, semantics, scalar shape, coordinate frame, scale, context and
compatible units must agree. Reference destinations name a declared parameter;
native destinations name an admitted initial-concentration port. Duplicate writers,
conflicting explicit overrides, cycles and cross-context shortcuts are rejected.

The ordinary Worker reconciles hybrid flows and dispatches ready nodes into either
existing queue. The reviewed native host agent runs native children. Dependencies
must actually succeed before downstream input values are read. Source manifest,
result and native quantity identities are checked before conversion.

Derived-model registration, derivation receipt, child enqueue and flow association
share a SQLite transaction. Concurrent schedulers/retries cannot create detached
jobs or overwrite immutable model versions. CAS blobs prepared before rollback may
remain unreferenced, as with existing result publication; no unsuccessful child
result or derived registry version is falsely published.

Each node retains its transfer artifact digest and expected child manifest digest.
Completed results expose exact child/result identities and terminal quantities;
transfer records remain separately readable. Running/failed/cancelled flows have no
successful whole-flow result. Earlier successful child results remain available.
Cancellation/failure stops queued/running children in both queues and skips pending
dependencies. Native/ordinary ownership fencing still prevents late publication.

Admission bounds 16 nodes, 32 bindings, aggregate returned-sample/allowed wall
budgets and a 4 MiB snapshot. `max_in_flight` limits queued/running nodes per flow.
The original PKML schedule uses the conservative existing 100,001 returned-sample
bound. Solver setup/cleanup and queue waiting are separate from summed solver wall
budgets. No global multi-user quota, clinical validity or confidence level is inferred.

Migration `005_hybrid_flows.sql` adds separate flow/node/event tables, preserves
existing registries and queues and keeps SQLite `user_version=1`. API/ordinary
Worker receive no Docker socket; native image/driver allowlists remain unchanged.

## API, CLI and executable fixtures

```sh
uv run peykar register-model experiments/flows/reference-model.json
uv run peykar flows interface experiments/flows/native-interface.json
uv run peykar flows register experiments/flows/native-reference-native.json
uv run peykar flows submit flow.native-reference-native 1.0.0 --idempotency-key my-flow
uv run peykar worker
# Keep the separately admitted COPASI native host agent running too.
uv run peykar flows status FLOW_RUN_ID
uv run peykar flows results FLOW_RUN_ID
uv run peykar flows cancel FLOW_RUN_ID
```

All local commands must use the same state as the API. Compose's named volume is
different from host `.peykar`; use authenticated HTTP or an operator container with
the existing volume and read-only fixture mounts. No command resets user data.

Admin POST: `/v1/native-interfaces`, `/v1/hybrid-flows`, `/v1/hybrid-runs` and run
`/cancel`. Reader/admin GET: native interfaces, derivation receipts, flow versions,
run status, `/events`, `/results` and `/nodes/{node_id}/transfers`. Creates return
201/202 and `Location`; submission supports workspace-scoped idempotency. Missing
role/reference/conflicting state/invalid constraints use 401/403/404/409/422.
Feedback remains unsupported (501). Flow discovery returns bounded summaries.

The two authored synthetic fixtures execute COPASI → reference → COPASI and
COPASI → COPASI. Each stage represents ten seconds of A→B decay with rate
0.1/second. Expected terminal A is `exp(-3)` and `exp(-2)` respectively; units cross
molar, millimolar and micromolar declarations explicitly. These are engineering
verification scenarios, not evidence for tissue, pharmacological or clinical effects.

```sh
uv run python scripts/verify_hybrid_flows.py --output .peykar/my-flow-verification
```

The operator verifier admits interface/reference descriptors through HTTP, checks
idempotency, actual solver outputs, transfer/derivation hashes, unchanged original
XML and running cancellation. Actual observed commands/outcomes are retained in
[verification](../verification.md); remaining platform work stays in the roadmap.
