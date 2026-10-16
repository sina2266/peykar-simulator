# Blueprint traceability and coverage

This table connects the supplied blueprint, accepted name/scope decisions and
subsequent clarifications to files in this repository. It is a requirement map,
not a claim that all planned capabilities are implemented. The complete original
55-section v3.0 proposal is [archived verbatim](blueprints/README.md); the
[original section and scenario map](blueprint-v3-coverage.md) supplements this
consolidated engineering map.

| Requirement | Self-contained explanation | Current implementation/evidence |
|---|---|---|
| General-purpose biological platform, first corneal use case | master-blueprint.md sections 1-2 | Synthetic reference engines; real-domain integration designed |
| Independent of HomBon; thin external adapter | architecture.md; master-blueprint.md 1,14 | ScientificInstrument facade and lifecycle integration test |
| Five fundamental capabilities | master-blueprint.md 2 | Separate modules; status.md distinguishes partial/designed parts |
| Hierarchy and interaction graph | master-blueprint.md 3 | Biology graph, hierarchy cycle/evidence checks |
| Separate entity, computational model and runtime instance | Original v3.0 section 5; master-blueprint.md 3 | Explicit design; no bulk simulation state in the knowledge graph |
| Stable ontology/species/context identities | scientific-contracts.md; master-blueprint.md 3 | Context contracts and exact applicability checks |
| Explicit missing knowledge | master-blueprint.md 3; knowledge-gaps.md | Persistent immutable KnowledgeGap declarations, exact model references, API roles and CLI |
| Source inspection and reuse/adapt/compose/build | master-blueprint.md 5 | Declarative inspector and offline digest-pinned inventory; executable remote import designed |
| Four admission paths and publication reconstruction label | Original v3.0 section 9; master-blueprint.md 5 | Standard/repository/reconstruction/data-construction routes designed; no equivalent-author-code claim |
| Scientific Model Package and engine separation | master-blueprint.md 4 | ModelPackage and EngineAdapter contracts |
| Five model interfaces | Original v3.0 section 10.1; master-blueprint.md 5 | Input/output/state/intervention/observation target; current contracts implement a subset |
| Licenses for every model and external engine | LICENSING.md; master-blueprint.md 4-5 | Model/dataset license fields; external permission gates |
| Capability-based discovery and dependencies | master-blueprint.md 4-5 | Exact-version registry, filters, dependency snapshots/tests |
| Typed scientific ports and unit conversion | master-blueprint.md 6 | Port contracts, Pint, compatibility tests |
| Coordinate, spatial and scale mappings | master-blueprint.md 6 | Mismatch rejection; full remapping designed |
| State/process responsibility and no double counting | master-blueprint.md 6 | Duplicate input writer checks; scientific ownership review designed |
| Feedback, multirate scheduling, numerical validity | master-blueprint.md 7 | Feedback rejected; generic coupled runtime designed |
| Conservation and numerical checks | master-blueprint.md 7 | Exact exchange verification and analytical decay tests |
| Independent scientific/run state | master-blueprint.md 8 | Immutable manifests separate from mutable queue state |
| Scenario branching and repeatable experiments | master-blueprint.md 8 | Scenario parent references, pinned plans and tests |
| Checkpoints, ensembles and adaptive fidelity | master-blueprint.md 8 | Explicit later engine capabilities; no fake implementations |
| All ten intervention categories | Original v3.0 section 22; master-blueprint.md 9 | Parameter-set events only; includes planned epigenetic/electrical/systemic categories |
| Structural and functional recovery separately | Original v3.0 section 24; master-blueprint.md 9 | Scientific composition requirement; no cell-count-to-function shortcut |
| Genetic perturbation to tissue response | master-blueprint.md 9,18 | P5 with evidence per bridge; not implemented |
| Multimodal data and connector ecosystem | master-blueprint.md 10 | Local observation registry; connector plans |
| Raw/normalized/derived data and lineage | master-blueprint.md 10 | CSV and explicitly mapped scalar export hashes/lineage; richer provenance planned |
| Observation operators and appropriate comparisons | master-blueprint.md 11 | Aligned scalar evaluator; unsupported mappings rejected |
| Calibration distinct from independent validation | master-blueprint.md 11-12 | Decay candidate estimator; dataset role/group checks |
| Feasibility, structure refinement, construction, surrogates and optimization | master-blueprint.md 12 | Limited calibration implemented; other learning designed |
| Champion/challenger and independent evaluation | master-blueprint.md 12-13 | No automatic promotion; independent authority designed |
| Sensitivity, identifiability and uncertainty | master-blueprint.md 11-12 | Requirements stated; no false numerical uncertainty output |
| Five trust axes and six scientific uncertainty sources | Original v3.0 sections 32-35; master-blueprint.md 11 | Explicit target with measured/unknown disclosure; no complete implementation claim |
| Provenance and immutable reproducibility | scientific-contracts.md; master-blueprint.md 8,13 | Checked runtime/result contracts, snapshots, artifacts, frozen seed protocols, event log, limited PROV-O export |
| Dependency impact and revalidation | master-blueprint.md 13 | Designed controls; full impact service not implemented |
| Permissions, governance, approvals and change control | operations.md; master-blueprint.md 13 | Local admin/read tokens; independent approvals designed |
| Scientific claims, conflicts and decision history | Original v3.0 sections 29-31, 39-40; master-blueprint.md 13 | Designed registries; simulation evidence must retain its origin/context |
| Four authority levels L0-L3 | Original v3.0 section 42; master-blueprint.md 13 | Designed separation of immutable policy, independent criteria, candidates and authorized runs |
| Section 45: one repository and complete target organization | target-layout.md | Three-package workspace with mapped future boundaries |
| Section 46: library/model/composite distinction | master-blueprint.md 4 | Immutable components; full composite admission designed |
| Section 47: full proposed API categories | api-target.md | api.md/openapi.json describe executable subset |
| Section 48: Scientific Instrument Adapter lifecycle | master-blueprint.md 14 | describe/validate/estimate/submit/status/collect/cancel |
| Section 49: technology stack and monolith rationale | master-blueprint.md 15-16; ADRs | Working local profile; future services not provisioned |
| Section 50: all six UI workspaces and viewer accuracy | master-blueprint.md 17; dashboard/README | Designed, no frontend claim |
| Section 51: P0-P7 with scientific exit gates | roadmap.md; master-blueprint.md 18 | P0 reference evidence; P1 onward gated |
| Section 52: architectural decisions | architecture.md; ADRs; master-blueprint.md | Preserved principles, actual package/local-profile adjustments documented |
| Section 53: species-aware registry and transfer evidence | master-blueprint.md 3 | Exact context gate; transfer not inferred |
| Section 54: populations/environment as analysis scales | master-blueprint.md 3 | Planned schema/engine expansion, not a containment claim |
| Section 55: milestones A-G and model evolution | roadmap.md; master-blueprint.md 18 | Acceptance definitions, not unearned success claims |
| Explicit initial deferrals | master-blueprint.md 19; roadmap.md | Status list keeps deferred capabilities visible |
| First real result: unchanged vCornea reproduction, evaluation, weakness and challenger | vcornea-integration.md; epics/E1-vcornea-native-runtime.md; roadmap.md | E1 operator runtime, native reference import and bounded repeats work; full published replay, independent evaluation and challenger improvement remain open |
| Later molecular-to-vCornea composition test | master-blueprint.md 7,18 | P4 requires numerical and biological evidence |
| Source-visible, noncommercial licensing request | LICENSING.md; licensing-rationale.md | Custom draft and explicit preservation of earlier Apache grants |

Code paths are under `packages/core/src/peykar_core`, `packages/engines/src/peykar_engines`
and `packages/platform/src/peykar`. An explanation being complete does not mean
that its future feature is executable. The authoritative status table is status.md.
