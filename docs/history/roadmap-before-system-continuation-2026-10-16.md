# Evidence-gated roadmap

These are dependency gates, not estimates that all biological problems can be
solved on a software schedule. Work may overlap where dependencies permit it.

| Phase | Work | Exit gate |
|---|---|---|
| P0 Foundation | Scientific contracts, registry, runs, adapters, provenance | Two different synthetic engine structures run without core changes; error paths tested |
| P1 vCornea | Pin upstream code and CompuCell3D; reference output importer | Reproduce selected published scenarios within declared stochastic tolerances |
| P2 Data and validation | Independent dataset, measurement mappings, sensitivity | Reproducible context-specific report with independent evidence or explicit data gap |
| P3 Model improvement | One measurable weakness, challenger, frozen benchmarks | Supported improvement or an interpretable negative result; independent review |
| P4 Composition | Molecular/tissue bridge backed by data | Units, semantics, conservation, convergence and coupled biological endpoints pass |
| P5 Gene to tissue | Bounded perturbation-response-cell-tissue path | Evidence and uncertainty recorded at every link; no unsupported scale jump |
| P6 Integrated organ | Limited eye endpoint, geometry and optics mappings | Comparison with simpler baseline and reference data for the chosen endpoint |
| P7 Multi-organ | Defined PK/exposure/response interaction | Independent evidence for the selected inter-organ relation |

## Next engineering increments

### Completed EPIC E1: native vCornea engineering path, 2026-10-16

[E1 acceptance and replay commands](../epics/E1-vcornea-native-runtime.md) record the
delivered licensed archive acquisition, locked CompuCell3D 4.7.0 environment,
bounded execution, six published native count/height imports, seed-paired regression
and API/CLI retrieval. A 600-MCS run crossed the configured injury at 500 MCS.
This completes the operator engineering epic. P1's scientific exit remains open:
the published files omit verified seeds/per-run parameters, the original author
environment is unrecovered and full-length published replay has not been established.

### Next EPIC E2: published-scenario reproduction

1. Establish the exact published scenario, parameter combination, initial state,
   original analysis and seed policy for one reference; record unrecoverable items
   as evidence gaps rather than guessing from replicate folder names.
2. Freeze an affordable full-horizon protocol, biological observation definition,
   stochastic acceptance and reference distribution before candidate execution.
3. Run the admitted model at that horizon and compare native outputs through the
   pinned original analysis; preserve interpretable failures as well as successes.
4. Seek independent experimental evidence only after an assay/context/measurement
   mapping is defensible. The included scratch-control aggregate is not such a mapping.

General queued external execution, distribution-level statistics and independent
evaluator identity remain separate extensions. The bounded operator profile is for
the reviewed pinned model; it is not a general hostile-code execution service.

### Earlier continuation record (superseded implementation inventory)

Continuation on 2026-10-16: the live API/worker path has been exercised for both
reference engines. Result/manifest checks and the cancellation/recovery race are
fixed. Offline digest admission and mapped scalar/seed comparison tools are working
increments toward P1; they do not satisfy its published-reproduction exit gate.
The original full v3.0 proposal is now archived and mapped to all 55 sections.
Persistent manual knowledge-gap records make absent mechanisms/reference data
queryable without inventing them; claim/conflict registries, independent authority
and the scientific exit gates remain separate future work.

1. Source permissions and required 4.7.0 reconstruction are now delivered by E1;
   the earlier upstream commit/tree admission is retained as historical evidence.
2. Extend the working bounded child lifecycle and scalar CSV importer to the actual
   CompuCell3D output format; add a reviewed container sandbox for untrusted code.
   Operator-mapped delimited scalar exports now have explicit MCS/time mapping and
   provenance. E1 adds actual native formats and a restricted operator container;
   a general untrusted execution service remains open.
3. Freeze a published reference scenario and a repeated-seed comparison protocol.
   Local seed-paired protocols can now be frozen and compared; a licensed published
   reference and working reconstructed runtime are supplied by E1;
   full published replay and distribution-level acceptance remain open.
4. Extend the working frozen evaluation sets to donor-level provenance and independent evaluator identity.
5. Add dependency/runtime digest checks for every imported external artifact.
   Offline admission now verifies every declared source/dependency/runtime-lock
   file. Discovering undeclared dependencies and a reviewed external execution
   adapter that consumes admitted bundles remain open.
6. Introduce PostgreSQL/object-store ports and a distributed worker only after local demand.
7. Build the UI against the stable API, with truthful computed-state visualization.

## Long-term acceptance milestones

A reproducible published simulation; B independent validation; C supported model
improvement; D validated composition; E cross-scale prediction; F cross-organ
simulation; G controlled scientific model evolution. Quantity of models and UI
complexity are not substitutes for these outcomes.

## Explicit deferrals

No atomistic whole-body model; no universal replacement for all numerical engines;
no full digital human twin; no claim to simulate every disease; no universal RL
research policy; no dozens of microservices before integration works. Cross-species
mapping requires evidence and a versioned transformation, not a species-name switch.
