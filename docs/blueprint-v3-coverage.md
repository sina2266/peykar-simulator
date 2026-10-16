# Original blueprint v3 coverage

This maps the complete [original Persian blueprint](blueprints/universal-biological-platform-v3.fa.md)
to Peykar's design and executable subset. Original section numbers and source
line numbers refer to that archived document, including its section 7.1. There
are 55 numbered sections; section 7.1 is included in row 7.

This is a requirements inventory, not a statement that all requirements have been
implemented or scientifically validated. [status.md](status.md) remains the
implementation inventory; [verification.md](verification.md) records commands
actually run. A partial implementation does not satisfy the full original section.

## All 55 original sections

| Original section and source line | Requirement | Design reference | Current scope and remaining gate |
|---|---|---|---|
| 1 · 55 | Simulation, virtual experiments and model development share a general platform | [Master §§1–2](master-blueprint.md) | Synthetic model execution and bounded parameter experiments work; general biological model development is designed. |
| 2 · 77 | Independent research product connected through an Instrument Adapter | [Architecture](architecture.md); [Instrument](../integrations/scientific_instrument/README.md) | Transport-neutral in-process facade implemented; research orchestration stays outside Peykar. |
| 3 · 127 | Ten logical subsystems and four shared services, initially a modular monolith | [Architecture](architecture.md); [Layout](target-layout.md) | Three distributions and domain boundaries implemented; diagrams do not imply fourteen deployed services. |
| 4 · 259 | Structural, molecular, cellular and interaction views; evidence-aware relationships | [Master §3](master-blueprint.md); [Biology](../packages/platform/src/peykar/biology/README.md) | Selected in-memory entity/relationship checks and persistent explicit knowledge gaps exist; ontology integration and a complete biological graph are designed. |
| 5 · 369 | Separate Biological Entity, Model and Runtime Instance; keep large state outside the knowledge graph | [Master §§3–4,8](master-blueprint.md); [Restored requirements](#explicit-requirements-retained-from-the-original) | Entity, model and run contracts are distinct; general model-instance state allocation and spatial state storage remain designed. |
| 6 · 410 | Reusable biological libraries; code reuse does not transfer scientific validity | [Libraries](../biological_libraries/README.md); [Master §4](master-blueprint.md) | Library admission guidance and small mathematical primitives exist; specialist mechanism libraries are not implemented. |
| 7 · 467; 7.1 · 486 | Reuse, adapt, compose or build; assess existing specialist tools | [Master §5](master-blueprint.md); [Adapters](../engine_adapters/README.md) | Descriptor inspection and offline artifact inventory work. Specialist integrations, including MoBi and GEARS possibilities, remain subject to separate admission and evidence. |
| 8 · 519 | Discovery, source/license/dependency inspection, isolated replay, mapping and independent validation | [External artifacts](external-artifacts.md); [vCornea](vcornea-integration.md) | Local declared-byte integrity checks implemented; remote discovery, untrusted execution and published replay are not. Execution readiness remains distinct from scientific validity. |
| 9 · 601 | Standard-format import, original repository, publication reconstruction, or construction from data | [Master §5](master-blueprint.md); [Restored requirements](#explicit-requirements-retained-from-the-original) | JSON descriptors and opaque local inventories are supported. SBML execution, repository adapters, reconstructed-model admission and data-driven construction are designed. |
| 10 · 653 | Scientific package metadata and five interfaces: input, output, state, intervention, observation | [Scientific contracts](scientific-contracts.md); [Master §§4,8–11](master-blueprint.md) | Versioned packages, ports and limited scalar operations exist; general state save/restore and observation interfaces are not implemented. |
| 11 · 716 | Illustrative future vCornea manifest with declared limitations and pinned provenance | [vCornea admission](vcornea-integration.md); [Schemas](../schemas/README.md) | Adapter admission record and pinned upstream identity exist. The blueprint YAML is an example, not an executable vCornea package or an upstream official manifest. |
| 12 · 781 | Capability discovery plus context/intervention/applicability checks; report no valid model | [Registry](../packages/platform/src/peykar/model_registry/README.md) | Exact capability/species filtering and execution-context checks work; ranking, training-context assessment and a full applicability service remain designed. |
| 13 · 825 | Coupling must preserve semantics, units, temporal interpretation and spatial meaning | [Composition](../packages/platform/src/peykar/composition/README.md); [Master §6](master-blueprint.md) | Port compatibility checks exist; a unit conversion does not supply missing biological or spatial mappings. Coupling execution remains designed. |
| 14 · 873 | Twelve interaction families, explicit mechanism/evidence/uncertainty, multiparty reaction/process nodes | [Master §§3,6](master-blueprint.md); [Restored requirements](#explicit-requirements-retained-from-the-original) | Selected pairwise relationship types exist; full interaction metadata and multiparty process-node contracts are designed. |
| 15 · 913 | Sequential, bidirectional and hierarchical coupling; justified fidelity changes | [Master §§6–7](master-blueprint.md) | DAG ordering and mismatch rejection exist. No generic coupled engine, feedback solver or adaptive fidelity implementation is claimed. |
| 16 · 981 | Numerical validator for units, conservation, time, space, stability, boundaries and solver convergence | [Composition](../packages/platform/src/peykar/composition/README.md); [Scientific contracts](scientific-contracts.md) | Analytical conservative exchange and compatibility checks are verified primitives; general coupled numerical validation remains designed. |
| 17 · 1017 | Eight overlapping data families and rich experimental metadata | [Master §10](master-blueprint.md); [Data fabric](../packages/platform/src/peykar/data_fabric/README.md) | Scalar time-series contracts and importers work. Omics, imaging, clinical/privacy workflows, instruments, batches and donor metadata require new contracts and adapters. |
| 18 · 1084 | Separate model repositories from observation repositories; retain suitable standard formats | [Connectors](../dataset_connectors/README.md); [Master §§10,16](master-blueprint.md) | Local scalar ingestion works. Named remote connectors and SBML/CellML/SED-ML/OMEX, AnnData, SpatialData and imaging adapters are designed, not installed capabilities. |
| 19 · 1134 | Quality, privacy/license checks, ontology matching, versioning and dataset-to-model lineage | [Data/evaluation](data-evaluation.md); [Master §§10,13](master-blueprint.md) | Raw/normalized hashes, strict scalar checks and immutable registration work; general harmonization, record-level lineage, privacy review and training dependency tracking remain designed. |
| 20 · 1216 | Feasibility analyzer distinguishes observable structure, dynamics and intervention response | [Master §12](master-blueprint.md); [Learning](../packages/platform/src/peykar/model_learning/README.md) | Requirement retained; no executable general feasibility or identifiability analyzer exists. Static images are not treated as sufficient dynamic evidence. |
| 21 · 1252 | Evidence-gated rule/ODE/PDE/agent/stochastic/constraint/statistical/ML/hybrid construction | [Master §12](master-blueprint.md) | Two fixed synthetic engines and a decay parameter proposal exist; autonomous construction and modeling-approach selection are designed. |
| 22 · 1318 | Ten intervention categories, with local/systemic and temporary/permanent extent | [Master §9](master-blueprint.md); [Restored requirements](#explicit-requirements-retained-from-the-original) | Only approved scalar parameter-set events execute. Genetic, epigenetic, molecular, cellular, tissue, mechanical, chemical, electrical, environmental and systemic categories remain design requirements. |
| 23 · 1345 | Separate genotype, cell state, direct perturbation and intervention mechanism; evidence at each scale | [Master §9](master-blueprint.md); [Roadmap P5](roadmap.md) | No genetic mechanism or gene-to-tissue engine exists; missing bridges must remain explicit rather than invented. |
| 24 · 1411 | Regeneration includes survival, integration, vascular/neural support, development and functional recovery | [Master §§9,18](master-blueprint.md); [Restored requirements](#explicit-requirements-retained-from-the-original) | Biological regeneration models are designed. Increasing a synthetic population count does not establish structural or functional recovery. |
| 25 · 1444 | Runtime orchestrates models; support multiple experiment/execution modes | [Simulation](../packages/platform/src/peykar/simulation/README.md); [Master §§7–8](master-blueprint.md) | Durable single-model execution, scalar events and benchmark/comparison tools exist. Replication orchestration, general sweeps, ensembles, hybrid execution, coupled runtime and adaptive resolution remain designed. |
| 26 · 1504 | Independent run state, branches, RNG/checkpoint state, physical time and explicit spatial mappings | [Master §§6–8](master-blueprint.md); [Scalar import](compucell-output.md) | Immutable plans, seeds and scenario parents work; scalar MCS/time mapping is declared explicitly. Native checkpoint branching, RNG-state restore and spatial mapping engines are not implemented. |
| 27 · 1581 | Original architecture scenarios 1–6: imported models and composition | [Original scenario map below](#the-original-24-architecture-scenarios); [Roadmap](roadmap.md) | Partial infrastructure supports future work; none of these six biological scenarios is claimed reproduced. |
| 28 · 1728 | Original scenarios 7–12: genetics, stem cells and regeneration | [Original scenario map below](#the-original-24-architecture-scenarios); [Roadmap P5–P6](roadmap.md) | Designed; no gene, stem-cell, transplantation or organ-development success claim. |
| 29 · 1847 | Original scenarios 13–18: data sufficiency, dynamic measurement, species transfer and conflicts | [Original scenario map below](#the-original-24-architecture-scenarios); [Master §§10–12](master-blueprint.md) | Scalar data checks and exact context restrictions exist; image/omics inference, cross-species mappings and scientific conflict management remain designed. |
| 30 · 1964 | Original scenarios 19–24: complex interactions, instability, regression and invalidated evidence | [Original scenario map below](#the-original-24-architecture-scenarios); [Master §§7,13](master-blueprint.md) | Version pins and strict context gates work; physiological coupling, full numerical rejection and dependency-impact/revalidation services remain designed. |
| 31 · 2097 | Twelve stress-test additions, including responsibility, gaps, conflicts, feasibility and impact | [Restored requirements](#explicit-requirements-retained-from-the-original); [Knowledge gaps](knowledge-gaps.md) | Some checks and persistent immutable knowledge-gap registration/filtering through API/CLI exist; automatic missing-mechanism inference, conflict management and the full subsystem set remain designed. |
| 32 · 2122 | Context/version-specific Trust Profile with verification, validation, applicability, robustness and reproducibility | [Trust ADR](adr/0003-scientific-trust.md); [Restored requirements](#explicit-requirements-retained-from-the-original) | Current ValidityProfile is a limited evidence/status contract; five independently assessed trust axes and risk-based review remain designed. |
| 33 · 2166 | Independent validation workflow; synthetic, published and independent experimental benchmark levels | [Validation](../packages/platform/src/peykar/validation/README.md); [Data/evaluation](data-evaluation.md) | Scalar comparisons and frozen local protocols work; author/evaluator independence and published/experimental evidence are not inferred from a passing test. |
| 34 · 2224 | Keep engineering metrics separate from model-specific scientific metrics | [Scientific contracts](scientific-contracts.md); [Master §11](master-blueprint.md) | Scalar RMSE/max-error and engineering lifecycle evidence exist; complete model-specific metric portfolios, calibration assessment and biological constraint counts are designed. |
| 35 · 2259 | Disclose six uncertainty sources separately, including what is not quantified | [Restored requirements](#explicit-requirements-retained-from-the-original); [Master §11](master-blueprint.md) | No calibrated uncertainty intervals are calculated. Parameter, structural, measurement, biological, coupling and extrapolation uncertainty require explicit future assessments; numerical error is an additional concern. |
| 36 · 2309 | Seven versioned evolution operations and independently reviewed better/worse/inconclusive decisions | [Master §12](master-blueprint.md); [Roadmap P3](roadmap.md) | Immutable versions and a bounded decay-rate proposal exist; structural evolution, promotion/rejection authority and independent review are designed. |
| 37 · 2371 | Separate parameter optimization, experiment selection and sequential control | [Master §12](master-blueprint.md); [Roadmap](roadmap.md) | Limited calibration exists; sensitivity, Bayesian optimization/design and benchmarked sequential RL remain future work. No general RL research policy. |
| 38 · 2407 | Active development based on identifiability, sensitivity and expected information gain | [Master §12](master-blueprint.md) | Designed; no general experiment-value estimator or automatic claim of knowing the best next measurement. |
| 39 · 2438 | Five independent histories and traceable model/data/run/environment/result/review links | [Master §§8,13](master-blueprint.md); [Restored requirements](#explicit-requirements-retained-from-the-original) | Model/data/run/validation records and limited PROV-O vocabulary export exist. Scientific decision history and conformant RO-Crate packaging are designed. |
| 40 · 2490 | Bounded Scientific Claim Registry linked to simulator evidence, not a second research platform | [Restored requirements](#explicit-requirements-retained-from-the-original); [Master §13](master-blueprint.md) | Designed; no scientific claim registration or automatic conversion of simulated effects into human/clinical claims. |
| 41 · 2531 | Versioned dependency graph and targeted impact/revalidation analysis | [Master §13](master-blueprint.md); [API target](api-target.md) | Exact model references and run snapshots exist; endpoint-specific impact, dataset-to-training links and revalidation planning remain designed. |
| 42 · 2566 | L0–L3 change authority; independent acceptance criteria and sandboxed structural proposals | [Restored requirements](#explicit-requirements-retained-from-the-original); [Operations](operations.md) | Static administrator/reader tokens are local access control only; they do not implement independent L1 approval or a four-level governance engine. |
| 43 · 2587 | Isolate imported execution, restrict network/files/resources and pin code/container identity | [Operations](operations.md); [External-engine ADR](adr/0004-external-engines.md) | Trusted child processes have resource/lifecycle bounds and no inherited secrets. Untrusted-code filesystem/network isolation and reviewed sandbox execution remain unimplemented. |
| 44 · 2604 | Check actual model/code/data rights; retain license metadata | [Licensing](../LICENSING.md); [External artifacts](external-artifacts.md) | License declarations are preserved. Public access, a publication license or a matching hash does not establish execution, modification or redistribution rights. |
| 45 · 2626 | Independent monorepo with extractable package boundaries and target organization | [Layout](target-layout.md); [Monorepo ADR](adr/0001-modular-monorepo.md) | Three real distributions and one lockfile implemented; conceptual directories are not claims of complete packages. |
| 46 · 2794 | Distinguish reusable library, contextual model and pinned composite | [Master §4](master-blueprint.md); [Model packages](../model_packages/README.md) | Registry and dependency pins exist; full composite admission/execution and historical external-runtime reproduction remain designed. |
| 47 · 2829 | Authenticated/versioned API and SDK across biology, models, data, scenarios, learning, validation and provenance | [API target](api-target.md); [Actual API](api.md) | Simulation/model/data/evaluation subset and Python facade exist; the target endpoint inventory is not a list of implemented routes or a standalone SDK claim. |
| 48 · 2904 | Generic Instrument lifecycle and evidence-bearing collection | [Instrument](../integrations/scientific_instrument/README.md); [Master §14](master-blueprint.md) | In-process lifecycle implemented; collection reports actual model/result limits. Unmeasured uncertainty is not manufactured. |
| 49 · 2959 | Python/FastAPI modular stack with specialist engines and justified infrastructure adoption | [Master §§15–16](master-blueprint.md); [Operations](operations.md) | Local Python/SQLite/artifact/worker/Docker profile works; PostgreSQL/S3/HPC, richer data tools, telemetry and frontend stack remain gated additions. |
| 50 · 3012 | Six UI workspaces and truthful structural/dynamic visualization | [Dashboard design](../apps/dashboard/README.md); [Master §17](master-blueprint.md) | Designed; no dashboard implementation. Uncomputed individual trajectories must never be presented as simulated state. |
| 51 · 3078 | P0–P7 dependency gates, with independent evidence or explicit gaps | [Roadmap](roadmap.md); [Master §18](master-blueprint.md) | P0 synthetic execution evidence exists. New import/comparison tools assist P1; actual vCornea replay and later scientific exit gates remain open. |
| 52 · 3268 | Preserve the full architecture decisions and their scientific boundaries | [ADRs](adr/0001-modular-monorepo.md); [Architecture](architecture.md) | Core decisions are adopted; documented local-profile choices do not imply that every target subsystem is implemented. |
| 53 · 3334 | Species-aware core and evidence-based transfer | [Master §3](master-blueprint.md); [Registry](../packages/platform/src/peykar/model_registry/README.md) | Explicit species/context and exact matching work; cross-species transfer requires new mappings, evidence and model versions. |
| 54 · 3361 | Population/environment are analysis views, initially schema/roadmap extension points | [Master §3](master-blueprint.md); [Restored requirements](#explicit-requirements-retained-from-the-original) | Population appears in current port scale; complete organ-system/environment/ecosystem contracts and engines remain designed and need versioned schema evolution. |
| 55 · 3409 | Milestones A–G: reproduced science through controlled model evolution | [Roadmap](roadmap.md); [Master §18](master-blueprint.md) | Engineering test success does not complete A–G. Published reproduction, real independent validation and controlled biological improvement are not claimed. |

## Explicit requirements retained from the original

The following details are requirements even where the shorter consolidated design
previously grouped them under broader headings. Listing them here restores their
visibility; it does not implement their missing contracts or workflows.

- **Entity, Model, Instance:** a shared biological identity does not imply shared
  simulation state. Knowledge graphs describe entities and relations; numeric
  instance state belongs to engines and appropriate state/artifact stores (§5).
- **Four import forms:** standard models, original repositories, publication-only
  reconstruction and data-driven construction need separate admission evidence.
  A reconstruction retains `RECONSTRUCTED_FROM_PUBLICATION` provenance. Claims
  of result agreement need justified comparison; agreement does not turn its
  source into the authors' original executable model (§9).
- **Five model interfaces:** input, output, state, intervention and observation
  each have separate responsibilities. Current scalar ports and events do not
  imply implemented state save/restore or general measurement mappings (§10).
- **Multiparty interactions:** ligand/receptor/cofactor and similar mechanisms
  may need reaction/process nodes rather than only binary edges (§14).
- **Ten intervention categories:** genetic, epigenetic, molecular, cellular,
  tissue, mechanical, chemical, electrical, environmental and systemic. The
  shorter seven-category directory sketch in §45 does not remove the additional
  categories specified in §22. Only supported parameter events execute today.
- **Structural and functional recovery:** geometry, cell count, survival,
  integration and recovered physiological function require distinct endpoints;
  none is a substitute for the others (§24 and scenarios 5, 10–12).
- **Trust and uncertainty:** the five trust axes are verification, validation,
  applicability, robustness and reproducibility (§32). The six uncertainty
  sources are parameter, structural, measurement, biological, coupling and
  extrapolation (§35). Reports must distinguish quantified from unknown sources.
  Numerical uncertainty/error is an additional requirement, not a replacement
  for biological or coupling uncertainty. A single confidence score is insufficient.
- **Five histories and bounded claims:** model, dataset, simulation, validation
  and scientific decision histories remain distinct (§39). The simulator's
  Scientific Claim Registry is limited to model/run evidence and its context,
  with independent and clinical evidence explicitly disclosed (§40). It must
  not duplicate the external research system or elevate simulation to clinical fact.
- **Scientific conflicts and missing knowledge:** retain conflicting evidence
  and competing hypotheses instead of deleting inconvenient observations
  (scenario 17; §31). Knowledge-gap records do not automatically discover unknown
  mechanisms, resolve scientific conflicts or validate a proposed replacement.
- **Change authority:** L0 protects security, audit, permissions and fundamental
  scientific rules from automatic modification; L1 acceptance criteria require
  independent approval; L2 structural/method proposals are tested in a sandbox;
  L3 authorized bounded experiments may execute automatically (§42). Current
  administrator access is not evidence that this governance hierarchy exists.
- **Versioned schema expansion:** organism, population and environment/ecosystem
  are analysis scales, not mandatory containment. Missing scale/instance/state
  contracts must be added through explicit compatible versioning, preserving
  existing schema/model versions and parameter units (§§26,53–54).

## The original 24 architecture scenarios

These IDs are the blueprint's biological architecture stress tests (§§27–30).
They are **different from** the numbered software/test cases in
[docs/scenarios.md](scenarios.md). A supporting software primitive is not evidence
that the corresponding original biological scenario has been executed or validated.

| Original ID / source line | Original scenario and required behavior | Current support and remaining gate |
|---|---|---|
| 1 · 1592 | Import unchanged vCornea, run it and standardize outputs without silently changing biology | Source identity, offline inventories and mapped scalar/seed tools exist; licensed runtime reconstruction and published replay remain open. |
| 2 · 1620 | Add a growth model without counting growth twice; assign computational ownership | Duplicate input-writer check exists; process responsibility/replacement and composite revalidation remain designed. |
| 3 · 1646 | Couple molecular EGF to cornea through evidenced time/space/phenotype mappings | Unit/semantic mismatch checks exist; a validated molecular-to-tissue bridge and double-counting review remain open. |
| 4 · 1665 | Connect cornea, optics and retina; report a missing tissue-to-optical model | No optical/retinal coupling engine; no vision-quality inference is made from synthetic scalar output. |
| 5 · 1698 | Combine neural regeneration models while accounting for circuit and functional integration | Designed; more cells or axon growth alone cannot establish recovered vision. |
| 6 · 1712 | Couple liver metabolism, circulating exposure and eye response with justified feedback | Designed P7 scenario; conservative two-compartment math is not PBPK or a biological liver/eye model. |
| 7 · 1739 | Gene activity change affects tissue only through supported perturbation and cellular mappings | No genetic execution chain; missing bridges and uncertainty must be explicit. |
| 8 · 1757 | Real DNA variant effects depend on RNA/protein/cell context, alleles and affected cells | Variant, zygosity and mosaic-state models are designed; no direct variant-to-tissue prediction. |
| 9 · 1773 | Combined gene changes require nonlinear interaction/epistasis evidence | Designed; single-perturbation evidence does not authorize additive effects or combinations. |
| 10 · 1799 | Stem-cell differentiation includes survival, stable identity and specialized function | Designed; a predicted transcriptomic state is not sufficient differentiation evidence. |
| 11 · 1817 | Retinal replacement requires survival, positioning, compatibility, neural and electrical integration | Designed; cell count does not certify functional integration. |
| 12 · 1835 | Organ redevelopment needs growth, differentiation, vascular/neural support and changing geometry | Designed; available component models would still require developmental coupling and validation. |
| 13 · 1858 | Static images support structural measurements, not uniquely determined dynamic laws | No imaging learning pipeline; feasibility and temporal-data requirements retained. |
| 14 · 1875 | RNA observations are not automatically protein activity or cellular behavior | No omics-to-phenotype pipeline; an evidenced intermediate model is required. |
| 15 · 1887 | Learn from time-lapse measurements while accounting for tracking/measurement error | Scalar ingestion only; imaging/tracking pipeline and propagated measurement uncertainty are designed. |
| 16 · 1905 | Human/mouse transfer requires orthology, context and biological evidence | Exact context gate works; cross-species transformation and uncertainty assessment are designed. |
| 17 · 1922 | Preserve conflicting studies and alternative hypotheses with their conditions | General immutable records exist; a scientific conflict registry/resolution workflow is designed. |
| 18 · 1942 | Abundant data without a model require feasibility, identifiability and missing-data analysis | No general model constructor/analyzer; insufficient evidence must remain an explicit limitation. |
| 19 · 1975 | Couple ocular pressure/mechanics, neural survival and vision with other factors and uncertainty | Designed; no glaucoma, neural-damage or visual-function prediction is claimed. |
| 20 · 1991 | Apply environmental oxygen/temperature/chemical changes at the correct time and place | Aligned scalar parameter events work; dynamic spatial environment models and biological response are designed. |
| 21 · 2016 | Detect unstable coupling, compare smaller steps, enforce constraints and separate invalid output | Limited conservation/port checks exist; general coupled step-refinement, convergence and validity classification remain designed. |
| 22 · 2032 | A new shared model may improve cornea and harm skin; retain both versions | Exact immutable versions/dependencies work; targeted impact analysis, revalidation and reviewed promotion/rollback remain designed. |
| 23 · 2052 | Invalidated data must expose affected training/models/runs/claims and preserve all history | Checksummed data and run-bound reports exist; training-to-claim impact graph and appended validity notices remain designed. |
| 24 · 2081 | Reject or explicitly label use outside the model's applicability domain | Exact species/context and parameter-range rejection work; separately authorized exploratory out-of-context mode is not implemented. |

The first real result remains the original end-to-end target: reproduce unchanged
vCornea, evaluate an appropriate endpoint with independent data or state the data
gap, identify a weakness, and evaluate a versioned challenger. A later molecular
coupling must earn its own numerical and biological evidence. Synthetic fixtures
and infrastructure tests alone do not satisfy those scientific milestones.
