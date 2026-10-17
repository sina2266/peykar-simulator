# Peykar whole-platform delivery roadmap

Updated 2026-10-17 against the complete [55-section architecture and 24 scenarios](blueprint-v3-coverage.md).
Peykar is a general biological simulation platform. vCornea is one admitted test
case; completion of its operator workflow is not completion of this product.
This roadmap supersedes the earlier eye-centered ordering, retained in
[history](history/roadmap-before-system-continuation-2026-10-16.md).

The implementation inventory is [status.md](status.md). A source checkout, an
installed runtime, a working adapter, numerical verification, published reproduction
and context-specific biological validation are separate states. No percentage
combines them into a misleading overall completion score.

## Delivery order and dependencies

| EPIC | System outcome | Dependencies | Current delivery state |
|---|---|---|---|
| S0 | Preserve data, licenses and reliable local execution | None | Working local foundation; production deployment open |
| S1 | Managed ecosystem of reusable source libraries | S0 | Complete permanent vendor trees and 37 recursive dependencies; native operator profiles installed/executed; original benchmark coverage remains explicit |
| S2 | Persistent biological entities, relations and evidence | S0 | Versioned graph snapshots and bounded queries working; ontology imports/claim reconciliation open |
| S3 | Standard model/data package import | S1,S2 | JSON, scalar/native CSV/Parquet, SBML/PKML and CellML 2 working; reviewed CellML operator ODE solver; omics/image admission open |
| S4 | General experiment orchestration | S0,S3 | Durable reference/native scalar DAGs, COPASI/OSP agent and bounded cohorts/sweeps; remaining native adapters and richer bindings open |
| S5 | Numerical and biological coupling | S2,S3,S4 | Port/unit checks and exact exchange primitive; continuous feedback/spatial solver open |
| S6 | Evidence, measurement and independent validation | S2,S3,S4 | Frozen scalar evaluation and declared gaps; independent authority/UQ incomplete |
| S7 | Controlled model optimization and improvement | S1,S4,S6 | Limited decay calibration only; challenger/promotion workflow open |
| S8 | Molecular and gene-response execution | S1,S3,S6 | COPASI and GEARS runtimes execute; real perturbation H5AD acquired/read; original learned benchmark and biological bridges open |
| S9 | Multicellular/tissue execution across engines | S1,S4,S6 | vCornea, PhysiCell/PhysiBoSS, VCell and Chaste native operator paths execute; general queue adapters and validated bridges open |
| S10 | Organ and multi-organ composition | S5,S6,S8,S9 | NEURON and OSP PK-Sim/MoBi headless models execute; validated organ/system bridges and Windows GUI profile open |
| S11 | Governance and research-system integration | S0,S2,S6,S7 | Local roles/provenance/HomBon facade/gaps working; scientific approvals/claims/conflicts open |
| S12 | Interactive six-workspace application | S2,S3,S4,S6,S11 | E4 local six-workspace application implemented on current APIs; richer scientific authoring/quality authority/spatial workflows remain open |
| S13 | Scalable operation and release | S0,S4,S11,S12 | Single-host Docker working; PostgreSQL/object storage/HPC/multi-host open |

Independent engineering work on S1–S4 proceeds without waiting for missing
published vCornea seeds. Scientific gates remain mandatory before model validity
or a biological coupling is claimed. UI work depends on concrete operations and
states; an illustrative graph or animation cannot substitute for execution.

## S0 — preservation and platform contracts

- [x] S0.1 Preserve original licenses, historical model/schema versions and source blueprint.
- [x] S0.2 Exercise authenticated submit → worker → checksum-verified result; fix lifecycle races.
- [x] S0.3 Back up SQLite through its backup API and retain artifact bytes before upgrades.
- [x] S0.4 Bound trusted child execution; pin result/runtime contracts and redact failures.
- [ ] S0.5 Establish documented recovery drill, deployment permissions, tenant isolation and threat model for a multi-user release.

Exit: existing records survive an upgrade; cancellation/retries cannot publish a
stale result; the configured deployment's auth/isolation guarantees are tested.
Local single-user verification does not close S0.5.

## S1 — library/source ecosystem and dependency ownership

- [x] S1.1 Retrieve all eight named specialist repositories plus CompuCell3D at exact commits; preserve the existing vCornea Git/archive identities separately.
- [x] S1.2 Hash every tracked file and preserve license evidence; explicitly inventory Git links/LFS pointers and declared CRLF checkout transformations.
- [x] S1.3 Persist immutable source inspection receipts; expose discovery and offline operator bundle import without installing code in the API.
- [x] S1.4 Acquire/review recursive source dependencies: 37 Git links materialized, including nested NEURON, OSP and separate VCell solver dependencies; no remaining links/LFS pointers in the verified distribution.
- [ ] S1.5 Complete original benchmark coverage for every engine. Isolated native CPU/headless profiles are installed and executed for all named computational families; GEARS's full original screen/split/score and Windows graphical profiles remain separate unclosed requirements.
- [x] S1.6 Version compatibility overlays with upstream bases, exact file hashes and actual native execution results; preserve original scientific source.
- [x] S1.7 Materialize all named sources as permanent project files under `vendor/`, preserving both vCornea identities and all original notices.
- [x] S1.8 Admit native execution evidence through an offline operator bundle, immutable source references and authenticated API/CLI discovery; preserve earlier receipts.

Source catalogue: [engine_adapters/ecosystem-sources.json](../engine_adapters/ecosystem-sources.json).
Actual receipts: [source inspection](../engine_adapters/ecosystem-source-receipts-2026-10-16.json).
The permanent distribution now contains **26 source identities / 53,261 files /
1,814,782,314 bytes**, excluding Git administrative data, with **37 materialized
Git links**. See [vendor manifest](../vendor/manifest.json), [original notices](../THIRD_PARTY_NOTICES.md),
[native runtime profiles](../runtime_profiles/README.md) and [actual new receipts](../engine_adapters/runtime-evidence-2026-10-17.json).
Earlier 1.0.0 source inspections remain immutable historical observations; complete
recursive inventories are admitted as 2.0.0. Runtime evidence does not establish validity.

Exit per engine: exact source/dependencies/license inventory, reproducible runtime,
native benchmark, adapter capabilities, failure/cancellation/artifact tests and an
explicit scientific applicability statement. A source-only entry fails this gate.

## S2 — biological representation and evidence graph

- [x] S2.1 Persist immutable entity/relationship snapshots with context, species and evidence.
- [x] S2.2 Reject unknown endpoints, duplicate identities and hierarchy cycles; query bounded neighborhoods with truncation disclosure.
- [x] S2.3 Import a small sourced human EGF/EGFR graph from inspected Reactome/UniProt evidence; keep it separate from a kinetic mechanism.
- [ ] S2.4 Add pinned ontology connectors (anatomy, cell, gene, protein, pathway) with mapping/conflict provenance.
- [ ] S2.5 Represent multiparty reactions, stoichiometry, process ownership, state instances and contextual claims rather than treating every interaction as a pairwise edge.
- [ ] S2.6 Add claim/conflict records and evidence review; connect executable model versions to the biological entities they actually represent.

Exit: entity, model and runtime instance remain distinct; queries retain evidence
and context; a graph relation never silently authorizes an executable coupling.

## S3 — model and dataset admission

- [x] S3.1 Admit immutable JSON packages and digest-pinned local external inventories.
- [x] S3.2 Preserve raw/normalized CSV and native vCornea count/height artifacts with units and lineage.
- [x] S3.3 Acquire three severities of real published simulation references and a separate attributed experimental aggregate.
- [x] S3.4a Admit exact declarative SBML core L2/L3 and original PKML with context/license and unit metadata; execute with isolated pinned COPASI/OSPSuite drivers through the native agent.
- [x] S3.4b Add bounded self-contained CellML 2 admission and a reviewed libCellML/SciPy operator ODE solver, retaining original units, immutable raw bytes and separate execution receipts. E5 includes numerical/failure/artifact checks and workbench inspection; general CellML queue/flow support remains under S4.6b.2.
- [ ] S3.5 Add AnnData/SpatialData and image/geometry readers in separate runtimes with donor/assay/license metadata.
- [ ] S3.6 Add explicit GEO, CELLxGENE, HuBMAP, BioModels, Physiome/ModelDB and Hugging Face connectors only where source terms and a concrete experiment support them.
- [ ] S3.7 Add donor/experiment-level split authority, data quality decisions and dataset-to-model impact tracking.

Exit per format: malformed/adversarial inputs rejected, exact raw bytes retained,
units/context/transformations testable and no automatic execution from data import.

## S4 — reusable experiment execution

- [x] S4.1 Freeze multi-model DAG specifications, exact model snapshots and explicit endpoint bindings.
- [x] S4.2 Dispatch eligible child runs through the existing bounded Worker; map final scalar output to a declared downstream initial parameter.
- [x] S4.3 Enqueue child and attach it to its experiment atomically; test concurrency, retries, rollback, cancellation and failure propagation.
- [x] S4.4 Expose register/submit/status/cancel/results/events through authenticated API/CLI; retain each child's manifest/result provenance.
- [x] S4.5 Add frozen batch/ensemble/sweep plans with aggregate budgets, deterministic reference seed allocation and capacity-bounded atomic dispatch across reference/native queues. E2 includes terminal statistics and explicit incomplete groups; native seed/parameter overrides are rejected.
- [x] S4.6 Add a durable external-engine agent protocol consuming admitted runtime/source identities, with no API Docker socket or host credentials. Actual COPASI SBML and OSPSuite PKML jobs pass model admission → claim → solver → verified API output.
- [x] S4.6b.1 Implement explicit native/reference scalar DAGs with declared interfaces, unit/context/semantic checks and atomic initial-condition derivation/dispatch. E3 executes COPASI/reference chains; PKML provides output-only interfaces.
- [ ] S4.6b.2 Extend the reviewed agent/flow interfaces to remaining native engines, PKML inputs and richer mappings; runtime probes alone do not provide queue adapters.
- [ ] S4.7 Add checkpoint/restart and engine capability negotiation; never claim unsupported state restoration.

Exit for the delivered subset: a real upstream result initializes the downstream
run exactly once and survives process/service reconstruction. The current mode is
`sequential_endpoint_transfer`; it is not simultaneous or feedback co-simulation.

## S5 — numerical coupling and biological bridges

- [x] S5.1 Check direction, semantics, species, units, shape and coordinates; reject unsupported feedback scheduling.
- [x] S5.2 Preserve an exact conserved two-compartment numerical primitive.
- [ ] S5.3 Define a stateful engine step/checkpoint contract, time ownership, event ordering and rollback across engines.
- [ ] S5.4 Implement feed-forward and feedback coupling with convergence/error control, stiffness handling, conservation and positivity checks.
- [ ] S5.5 Add scalar↔field↔agent mappings with lattice/geometry/time metadata and mass/volume normalization.
- [ ] S5.6 Define a separately versioned biological bridge with applicable cell/context, parameter evidence, process ownership and independent endpoints.
- [ ] S5.7 Verify coupling against an uncoupled/simple analytical baseline and step-size refinement before a biological test.

Exit: numerical tests plus held-out coupled endpoint evidence for the stated
context. EGF uptake/endocytosis already abstracted by vCornea must be replaced or
partitioned explicitly before adding EGFR kinetics; a matching unit is insufficient.

## S6 — scientific validation, measurement and uncertainty

- [x] S6.1 Distinguish synthetic verification, simulation comparison and experimental origin.
- [x] S6.2 Freeze datasets/groups/endpoints/tolerances and repeated-seed scalar protocols.
- [x] S6.3 Persist missing mechanisms/evidence as queryable knowledge gaps.
- [ ] S6.4 Implement versioned observation operators with sampling/noise assumptions and original-analysis comparison.
- [ ] S6.5 Establish independent evaluator identity and prevent author/candidate-controlled holdout changes.
- [ ] S6.6 Extend delivered within-case terminal mean/sample SD with sensitivity/identifiability and uncertainty by source; no confidence interval or parameter uncertainty is inferred by E2.
- [ ] S6.7 Expand trust into separate execution, numerical, biological, applicability and evidence axes.

For vCornea, [E1](epics/E1-vcornea-native-runtime.md) closes native operator engineering.
Published replay still needs exact scenario/parameters, original analysis, seed
policy, full horizon and frozen stochastic acceptance. Experimental scratch-control
means have a different context and do not close independent tissue validation.

## S7 — model improvement and optimization

- [x] S7.1 Implement a limited analytical decay-rate calibration proposal.
- [ ] S7.2 Register baseline/challenger versions, measured weakness and a frozen optimization objective.
- [ ] S7.3 Separate runtime performance patches from scientific mechanism/parameter changes.
- [ ] S7.4 Run historical benchmarks and held-out endpoints; retain negative results, computational cost and applicability limits.
- [ ] S7.5 Require independent promotion approval with rollback/revalidation impact; never let a candidate certify itself.

Exit: reproducible supported improvement or an interpretable negative result.
Changing pristine upstream source solely to make it look “optimized” does not pass.

## S8–S10 — specialist engines and biological scales

| Engine/model | Source state | Next execution task | Interaction gate |
|---|---|---|---|
| vCornea / CC3D 4.7.0 | Git and licensed archive preserved; native runtime executed | Full published scenario and original-analysis correspondence; general external agent | Existing EGF process ownership, lattice/time/observation mapping |
| COPASI | Complete Build-309 source | Installed 4.47.309 runtime; actual admitted SBML API/agent jobs and analytical checks pass; biological model benchmarks next | Compartment volumes, species units, reaction ownership |
| VCell | Complete 8.2.1.01 plus eight native solver sources | Original BioModel1 OMEX runs with real HDF5; general model/queue adapter next | Intracellular geometry and PDE/ODE boundaries |
| PhysiCell / BioFVM | Complete 1.14.2 and source-built MaBoSS | Native template and original 486-cell Boolean fixture execute; general agent/field exchange next | Agent state, substrate flux and phenotype ownership |
| Chaste | Complete 2026.1 source; matching official native build | Original ODE and both Delta/Notch monolayers pass; refinement/context-specific tissue evidence next | Mesh, mechanics/electrophysiology and conservation |
| NEURON | Complete 9.0.2 with recursive submodules | Actual native HH current clamp passes; general model/morphology admission next | Voltage/current/ion semantics and morphology |
| PK-Sim | Complete v12.3.173 source/dependencies | Original PKML executes through the durable API/agent queue; Windows project-authoring GUI remains separate | Exposure/PBPK compartments, populations and physiological context |
| MoBi | Complete v12.3.136 source/dependencies | Original Aciclovir PKML runs through API/agent; original schedule/units retained; building-block admission next | Building-block ownership and kinetic/physiological interfaces |
| GEARS | Complete pinned source, own CPU lock | Network forward/backward and real Dixit H5AD reader pass; full original screen/split/benchmark next | Cell type/training context; transcript changes do not directly define tissue effects |

These workstreams can progress independently. No engine must be forced into an
incompatible host environment, re-licensed under Peykar or marketed as a universal
biological library. Gene → protein → cell → tissue → organ links require evidence
at every bridge; source availability does not establish those links.

## S11 — governance and research integration

- [x] S11.1 Keep HomBon-specific behavior in the transport-neutral facade.
- [x] S11.2 Preserve local roles, append-only execution events and checksum provenance.
- [ ] S11.3 Implement scientific claim/conflict registries, five histories and L0–L3 change authority.
- [ ] S11.4 Add independent approval, signatures/audit retention and reproducible research export (RO-Crate/SED-ML where supported).
- [ ] S11.5 Expose capability/limitation-aware model search and experiment collection to research agents without delegating scientific approval to them.

## S12 — interactive product, backed by real operations

The complete product needs an interactive UI. Build each workspace once its
required operations and error states exist; do not wait for every biological engine.

- [x] S12.1 Shared application shell, accessible design system, authentication, localization/RTL and responsive layouts.
- [x] S12.2 Biological Explorer using versioned graph/neighborhood/evidence APIs; distinguish evidence links from runnable bridges.
- [x] S12.3 Model Studio with source/runtime/adapter/validity states and a port-compatible experiment editor.
- [x] S12.4a Dataset Studio with origin/license/lineage, declared groups and original-unit scalar observations/CSV imports.
- [ ] S12.4b Add quality adjudication and independent split authority after their backend contracts exist.
- [x] S12.5 Experiment Studio with real submit/estimate/cancel/dependency states; resume persisted experiments.
- [x] S12.6 Simulation Viewer with computed series/artifacts and explicit geometry limits; no invented cell motion.
- [x] S12.7 Validation Center with frozen benchmarks, failure explanations, uncertainty and missing evidence.
- [x] S12.8 Browser-test desktop/tablet/mobile, keyboard access, loading/empty/error/forbidden/degraded/conflict states and RTL.

E4 closes this bounded browser delivery for existing contracts; see
[workbench scope and deferred scientific authoring](epics/E4-research-workbench.md).
Advanced metadata/frozen definitions use validated JSON editors. Current unit/context
gates do not implement biological bridges, independent quality review or spatial UI.

Exit: complete end-to-end user workflows on real APIs, with no decorative actions
or unlabeled illustrative scientific state. Swagger remains developer tooling.

## S13 — operation, scaling and release

- [x] S13.1 Docker API/Worker lifecycle and offline Linux regression/package checks.
- [ ] S13.2 Recovery drills, metrics, quotas, durable logging and operator runbooks.
- [ ] S13.3 PostgreSQL queue leases/outbox and object-store implementation with migration/backfill/rollback evidence.
- [ ] S13.4 SLURM/batch/external runtime agents, aggregate budgets and reproducible worker capabilities.
- [ ] S13.5 Multi-user permissions/tenant isolation, deployment threat model and data retention.
- [ ] S13.6 Reproducible releases, compatibility matrix, installed-wheel/client tests and monitored deployment.

## Program acceptance and next concrete sequence

1. **Delivered in this continuation:** S1–S4 shared source/graph/workflow backends,
   full Linux verification and live deployment without resetting data.
2. Native computational profiles are installed/executed, and S4.6 now delivers
   actual COPASI/OSP model jobs and E3 scalar DAGs. Extend remaining native adapters/mappings (S4.6b.2),
   ontology/data admission (S2.4/S3.5) and independent original benchmarks (S1.5).
   E5 now delivers bounded CellML 2 admission and reviewed operator ODE execution
   (S3.4b); its general queue/flow adapter remains part of S4.6b.2.
3. E4 now delivers the shared six-workspace UI on those APIs. Extend richer
   authoring and independent quality/split workflows as S3/S6/S11 add their contracts.
4. Add one explicit numerical coupling and its biological bridge (S5), then expand
   the remaining specialist profiles and workflows according to their dependencies.
5. Advance improvement, integrated organs and cross-organ endpoints only when their
   stated numerical/context/evidence gates pass.

All 55 source sections remain traceable in the existing coverage map; the original
24 scientific architecture scenarios remain acceptance targets, distinct from
passing software tests. Project completion requires the relevant EPIC gates, not
just a checked list of cloned repositories or a single working eye-model example.

## Original scientific milestones retained

The earlier P0–P7 gates still apply; S0–S13 reorganizes delivery across the whole
system rather than weakening or silently completing scientific requirements.

| Original phase | Scientific exit | Current EPICs |
|---|---|---|
| P0 | Two different verified synthetic engines and tested failure paths | S0,S4 |
| P1 | Reproduce selected published vCornea scenarios with frozen stochastic acceptance | S1,S6,S9 |
| P2 | Context-specific independent evidence report or explicit data gap | S3,S6 |
| P3 | Supported model improvement or interpretable negative result, independently reviewed | S7,S11 |
| P4 | Molecular/tissue coupling passes units, semantics, conservation, convergence and biological endpoints | S5,S8,S9 |
| P5 | Gene-to-tissue perturbation path with evidence/uncertainty at every bridge | S2,S5,S6,S8,S9 |
| P6 | Limited organ endpoint compared with a simpler baseline and suitable reference | S9,S10 |
| P7 | Defined multi-organ exposure/response interaction with independent evidence | S10,S11 |

Milestones A–G remain reproducible simulation, independent validation, supported
improvement, validated composition, cross-scale prediction, cross-organ simulation
and controlled scientific evolution. No atomistic whole-body model, universal
solver, unsupported cross-species transfer or automatic self-certifying promotion
is added by a source catalogue or a workflow graph.
