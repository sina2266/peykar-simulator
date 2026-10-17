# Implementation status

Definitions: **implemented** means executable code with local verification;
**partial** means a limited working implementation; **designed** means documented
interfaces/acceptance criteria without an executable integration.

| Capability | State | Scope and limit |
|---|---|---|
| uv monorepo with three distributions | Implemented | Core, engines, platform; one lockfile |
| Model/data/scenario contracts and JSON Schemas | Implemented | Schema 1.0.0 only |
| Versioned model registry and pinned dependencies | Implemented | Local persistent profile |
| Declarative model import inspection | Partial | JSON only; no remote fetching or execution |
| Offline external artifact admission | Partial | SHA-256 checks for every declared source/dependency/runtime-lock file; immutable local inventory; no installation, execution or undeclared dependency discovery |
| Managed specialist source ecosystem | Implemented local source distribution | 25 permanent vendor distributions / 52,167 files / 37 materialized recursive Git dependencies, zero unresolved links/LFS pointers; original notices retained; 12 new complete source receipts coexist with 9 historical receipts |
| Native runtime evidence registry | Implemented locally | 11 actual retained execution/QC receipts, checksum/source/image validation, authenticated reads and offline operator CLI; no automatic installation or API Docker access |
| Species/context checks and capability search | Implemented | Exact matching; no inferred transfer |
| Biological graph | Partial | Persistent versioned snapshots, hierarchy/context/evidence checks, bounded neighborhoods and API/CLI; curated human EGF/EGFR example; no full ontology/claim reconciliation |
| Persistent knowledge-gap declarations | Implemented locally | Immutable manual records; exact affected-model versions; filtered API/CLI reads; no automatic closure or execution blocking |
| Typed port compatibility | Implemented | Scalars/declared shape; no automatic scale mapping |
| Composite DAG planning | Partial | Compatibility/order only, no general engine composition |
| Durable multi-model experiments | Partial | Working sequential terminal-output to initial-parameter transfer; frozen models, atomic child dispatch, retries/cancellation/results/events; no simultaneous feedback or spatial coupling |
| Conserved two-compartment exchange | Implemented | Analytical verification primitive |
| Persistent runs and separate worker | Implemented | Single-host SQLite WAL queue |
| Idempotency, token fencing, cancellation | Implemented | Parent checks child lifecycle; stale runs explicitly fail; cancellation/recovery ownership race handled |
| Bounded child execution | Implemented | Linux, trusted installed code, wall/CPU/address-space/file limits; not an untrusted-code sandbox |
| Immutable manifests and content-addressed results | Implemented | No signatures or external audit store |
| Runtime and result contract enforcement | Implemented | Pinned platform/Python/engine/dependencies/available lock verified; result seed, grid, scalar output ports and units checked before publication |
| Scheduled interventions | Partial | Approved scalar parameter changes at sample boundaries |
| Decay and stochastic population engines | Implemented | Synthetic infrastructure tests only |
| Observation registry and endpoint comparison | Partial | Aligned scalar series; no general observation operators |
| CSV ingestion and transformation lineage | Implemented | Strict UTF-8 scalar time series, exact raw bytes and normalized hashes |
| Frozen evaluation sets and run-bound report retrieval | Implemented | Pinned datasets/groups/tolerance; declarations still require independent review |
| Mapped CompuCell3D scalar export ingestion | Partial | Explicit columns, MCS/time mapping and declared source/runtime/seed; exact raw bytes retained; no universal native-output reader |
| Native vCornea output ingestion | Implemented locally | Strict four-cell-count CSV and ten-bin Parquet height import; raw/decoded/normalized lineage; hours and lattice coordinates preserved; seed unknown unless evidenced |
| Frozen repeated-seed scalar comparisons | Partial | Seed-paired datasets and fixed tolerance; no statistical distribution test or published reproduction |
| Calibration | Partial | Log-linear decay-rate proposal only |
| Hold-out separation | Partial | Declared role/group checks; no independent split authority |
| Contextual uncertainty estimation | Designed | No confidence intervals reported |
| Full Scientific Trust Profile | Designed | Five separate axes; current validity contract is a limited subset |
| Scientific claim/conflict registries and decision approvals | Designed | Explicit v3.0 requirements; local gap records do not implement those workflows |
| Champion/challenger promotion | Designed | Never automatic self-certification |
| HomBon Instrument Adapter | Implemented | In-process facade, transport-neutral lifecycle |
| HTTP API and CLI | Implemented | Static admin/read tokens, bounded bodies, local operator CLI |
| PROV-O vocabulary export | Partial | Not a conformant RO-Crate/SED-ML exporter |
| vCornea / CompuCell3D execution | Partial | EPIC E1 operator profile works: licensed archive, locked 4.7.0 image, bounded seeded runs and injury crossing; no general queued adapter, full published replay or biological validation |
| Published vCornea reference acquisition | Implemented locally | Six complete count/height files for three archived severities; selected ZIP member checks only; per-run seed/parameters unknown |
| Small experimental scratch dataset | Implemented ingestion only | Attributed Rho et al. 2015 aggregate control means/SD; different assay and GM-CSF context; not vCornea validation |
| PhysiCell, VCell, COPASI, Chaste, NEURON, GEARS | Native operator execution working | Locked/isolated builds or pinned native images; real native outputs, ODE/HH/OMEX/Boolean/gradient checks; general worker adapters and full published benchmark/validation gates remain open |
| PK-Sim/MoBi headless computation | Implemented operator profile | OSPSuite-R 12.4.5/rSharp 1.2.3/.NET 8 runs original Aciclovir PKML: 491 samples, original concentration/time units; Windows graphical applications are separate and not installed |
| Chaste Delta/Notch cell interaction | Original native example executed | Both vertex/node monolayers pass; original model/source and numerical warning retained; no independent coupled tissue validation |
| Real perturbation AnnData | Operator reader/QC working | Complete provider ZIP/member checksums, 37,160-cell/19,264-gene Dixit K562 release, bounded backed matrix inspection; original GEARS split/training/accuracy and general API admission remain open |
| GEO, CELLxGENE, HuBMAP, Hugging Face connectors | Designed | No automatic dataset import or clinical data handling |
| PostgreSQL, S3, SLURM, distributed leases | Designed | Local migration interfaces and operational design only |
| Six-workspace dashboard | Designed | No frontend implementation in this release |
| Genetic interventions / cross-organ / whole body | Designed | No biological efficacy claims |
| Docker/Compose | Implemented locally | API/worker shared-volume lifecycle verified on Docker Desktop; isolated Linux regression target supplied; no production or multi-host claim |
| GitHub Actions | Configured | Remote matrix runs after publishing to GitHub |

The full design is consolidated in master-blueprint.md. Requirement coverage is
mapped in requirements-traceability.md, with target paths in target-layout.md and
future APIs in api-target.md. The current source-available, noncommercial license
is a project-specific draft awaiting legal review; see ../LICENSING.md. This is a
licensing status, not a scientific-validity status.

The complete original 55-section Persian proposal and its 24 biological stress
scenarios are retained under `docs/blueprints/` and `blueprint-v3-coverage.md`.
They describe the product target; the original scenarios are not passing software
tests or implemented biological integrations.

Validation details for this delivered snapshot are in `docs/verification.md`.
The completed engineering acceptance and remaining P1 scientific gate are recorded
in [EPIC E1](epics/E1-vcornea-native-runtime.md). Native execution is an explicit
operator workflow outside the management API; archived simulation benchmarks remain
distinct from experimental observations.

The [whole-platform roadmap](roadmap.md) now separates shared S0–S13 delivery from
individual biological-model gates. The source registry, persistent graphs and
sequential experiment service are shared platform capabilities, documented in
[libraries](library-ecosystem.md), [biology](biological-graph.md) and
[workflows](experiment-workflows.md). The full system is not declared finished.
