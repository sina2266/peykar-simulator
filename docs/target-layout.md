# Target repository layout and implementation mapping

The target architecture is broader than the current implementation. A target path
names a responsibility and potential extraction boundary; it is not a claim that
all child folders contain working code today. Avoid creating empty pseudo-packages
to make a diagram look implemented. See status.md for executable capabilities.

The actual workspace contains `packages/core`, `packages/engines`, and
`packages/platform`. The domain modules below currently live under the platform
distribution unless shown otherwise. This preserves a modular monolith without
one independently versioned repository per conceptual subfolder.

| Target responsibility | Target children | Current location/state |
|---|---|---|
| apps | api, dashboard, cli, worker | apps/ entrypoints/design; actual transport in peykar.api and peykar.cli |
| core | contracts, types, units, coordinates, events | peykar_core contracts/units/protocols/hash/errors/planning; full coordinate/event infrastructure designed |
| biology | ontology, anatomy, entities, mechanisms, interactions, knowledge_graph | peykar.biology.graph and core entity/relationship/gap contracts; ontology import designed |
| model_registry | models, capabilities, versions, dependencies, applicability, licenses | peykar.model_registry with immutable local registry and trusted engine catalog |
| model_import | discovery, inspection, importers, source_mapping, dependency_analysis, sandbox | peykar.model_import.inspection; remote discovery/execution admission designed |
| composition | ports, compatibility, coupling, scale_mapping, responsibility, numerical_validation | peykar.composition compatibility/exchange; full coupled solver designed |
| simulation | runtime, scheduler, state, events, checkpoints, ensemble, execution | peykar.simulation service/worker; local queue/events; general checkpoint/ensemble/multirate features designed |
| interventions | genetic, molecular, cellular, tissue, mechanical, chemical, environmental | core Intervention + engine parameter events; other typed implementations designed |
| data_fabric | connectors, ingestion, normalization, quality, registry, lineage, observation | peykar.data_fabric.service and csv_import; scalar CSV/lineage implemented; external/multimodal paths designed |
| model_learning | feasibility, calibration, parameter_estimation, structure_refinement, model_construction, surrogate_models, optimization | peykar.model_learning.calibration limited to decay; wider workflow designed |
| validation | verification, scientific_validation, benchmarks, uncertainty, regression, independent_evaluation | peykar.validation.evaluation/service, frozen splits, run-bound records and tests; uncertainty/independent authority designed |
| governance | provenance, audit, permissions, approvals, change_control | peykar.governance.provenance; local roles/events; approval/promotion service designed |
| biological_libraries | molecular, gene_regulation, cellular, signaling, tissue, mechanics, neural, metabolism, physiology | Corresponding top-level admission/design directories; not specialist implementations |
| model_packages | molecular, cellular, tissue, organ, organ_system, organism, composite | Corresponding design directories plus actual versioned synthetic descriptors in reference/ |
| engine_adapters | compucell3d, physicell, vcell, copasi, chaste, neuron, pk_sim, ml_runtime | Integration requirements per adapter; executable synthetic engines live in packages/engines |
| dataset_connectors | ncbi_geo, cellxgene, hubmap, huggingface, local | Admission requirements; local normalized observation path implemented |
| experiments | scenarios, baselines, benchmarks, manifests | Scenario guidance, synthetic plans, analytical observations and dataset specs |
| integrations | scientific_instrument | Transport-neutral facade plus top-level integration documentation |
| schemas | Versioned scientific and transport schemas | Generated JSON Schema 2020-12 and OpenAPI |
| tests | Unit, numerical and integration evidence | Actual executable reference suite |
| docs | Blueprint, architecture, decisions, status and operations | Self-contained documentation and traceability map |
| infrastructure | Database, artifacts, queue, compute, telemetry | Local SQLite/artifacts code plus distributed-profile design |
| deployment | Containers, CI and future hosting | Docker/Compose and GitHub workflow configured; no hosting provisioned |

## Later package extraction

Extract a domain only when it has a stable scientific/API boundary, independent
dependencies or release needs, adequate contract tests and a credible owner. Keep
exact scientific model versions distinct from Python package versions. External
engine code belongs in its own reviewed environment even while management remains
a monolith. Do not split HomBon's research methodology into Peykar's core.
