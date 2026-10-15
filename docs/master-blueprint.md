# Peykar master blueprint

Architecture edition: 0.2.0. This is the consolidated product/design reference;
`status.md` is the authority for what is executable today. A designed capability
is not an implemented feature. Licensing is governed separately by `../LICENSE`
and `../LICENSING.md`; public source access does not establish OSI open-source status.

## 1. Product, scope, and first use case

Peykar is a general biological modeling and simulation platform. The scientific
ecosystem may contain specialist molecular, gene-regulatory, cellular, tissue,
organ, organ-system and organism models. Longer-term representations can include
populations, microbiomes and environments. This is not a promise that one model
can represent everything, or that current science supports a complete organism.

The first intended real domain is corneal epithelium, using the published vCornea
model through CompuCell3D. That domain tests the platform; it must not hard-code
cornea, humans, or a particular solver into the shared core. Current reference
engines are synthetic verification tools, not vCornea implementations.

HomBon is an independent autonomous research system. It may discover a question,
form a hypothesis, request a bounded computational experiment and interpret its
evidence. Peykar owns simulation planning/execution and reports its actual limits.
Only the Scientific Instrument Adapter depends on the research system's lifecycle.
Another research product should be able to use Peykar without a biological-core change.

## 2. The five capabilities

| Capability | Responsibilities | Scientific boundary |
|---|---|---|
| Biological representation | Anatomy, entities, mechanisms, state, interactions and context | A knowledge graph assertion is not a validated executable mechanism |
| Model ecosystem | Discover, inspect, adapt, construct, version, register and reuse models | An available model is not necessarily applicable to the question |
| Multiscale simulation | Combine and execute specialist models through explicit bridges | Successful execution does not prove numerical or biological validity |
| Scientific data and validation | Normalize observations, assess quality, evaluate endpoints | Simulation, training data and independent observations stay distinct |
| Model learning and evolution | Calibrate, propose challengers, refine structure, evaluate changes | A candidate cannot approve itself or redefine its success criteria |

These capabilities share contracts but remain replaceable modules. Changes need
explicit compatibility, provenance and evaluation gates rather than informal JSON
connections. No result should be presented as more credible than its model,
couplings, data and context-specific evidence support.

## 3. Biological structure and interaction graph

The same system has several views: anatomical containment, type hierarchy,
molecular interaction, spatial location and causal/mechanistic regulation. Eye,
retina, retinal ganglion cell, expressed genes and signaling pathways are related,
but do not form one exclusive parent/child tree. A reusable pathway can participate
in several tissues. Separate `part_of` from `is_a` and from feedback regulation.

An entity needs a stable ID, kind, species, ontology references and context. An
interaction additionally needs a direction/type, evidence, conditions and a stated
interpretation. Association alone must not silently become a causal effect. A
model's state variables, geometry and measurement definitions are separate from
general biological knowledge. The current graph only implements selected relations.

Species is a real applicability constraint, not a label users can switch to convert
a human model into a plant or mouse model. Cross-species use requires a separately
versioned mapping with justified parameters, assumptions and evidence. Population
and ecosystem views are future extension points, not implemented engines.

Unknown mechanisms, missing reference data, unsupported interventions and gaps in
couplings should create KnowledgeGap records. Do not fill them with invented model
parameters or LLM-generated facts merely to complete a pipeline.

## 4. Library, model package, composite, and engine

| Object | Meaning | Example |
|---|---|---|
| Biological library | Reusable mechanism without a single tissue identity | Diffusion, cell-cycle rule or constitutive relation |
| Model package | Executable scientific model for a stated context | A particular corneal epithelial model version |
| Composite model | Pinned component models plus pinned coupling models | A limited eye model joining cornea, optics and retinal input |
| Engine adapter | Solver execution and state/time translation | CompuCell3D or a future molecular solver adapter |

A model package declares its source and license, immutable version/checksum,
required engine version, dependencies, scientific context, capabilities, parameter
definitions, input/output ports and evidence-based limitations. A source checksum
declared in a descriptor is not proof that a remote source has actually been fetched
or verified. Admission must distinguish declared metadata from verified evidence.

An eye composite might pin a cornea model, an optical model, a retinal model and
explicit cornea-to-optics/optics-to-retina bridges. Replacing any component or bridge
creates a new composite version. Historical manifests must still identify the
original combination and remain interpretable/reproducible within stated tolerances.

## 5. Discovery and import: reuse, adapt, compose, build

Select models by capability and endpoint, not by engine popularity. Ask whether
the model's species, conditions, scale, intervention support and observation mapping
fit the question. Then choose reuse, minimal adaptation, composition, or construction.
Prefer an existing justified model over inventing a new general engine.

The target import workflow is discovery, source/license inspection, dependency
analysis, schema/parameter/port mapping, isolated execution, reference reproduction,
and admission. A Python or C++ import is executable code, not harmless scientific
metadata. The current inspector accepts JSON descriptors only and does not fetch
URLs, unpickle data, extract archives or run downloaded code.

External engine environments should be separate and pinned. No claim that an engine
has a particular redistribution license is made without checking its actual release
and dependencies. The platform's license never overrides an engine or dataset license.

## 6. Ports, geometry, and state ownership

A scientific port carries units, semantic quantity identity, direction, scale,
shape and coordinate/reference frame. Concentration, amount, flux and population
count are different quantities. A liter-to-milliliter conversion does not supply a
missing biological mechanism, tissue volume, spatial mapping or causal relation.

For spatial coupling, define coordinate axes, origin, orientation, geometry/mesh,
spatial units, resolution, interpolation/remapping method and any conservation
requirements. For scale changes, define aggregation/disaggregation rules and the
evidence supporting them. These mappings are model components with versions.

Give each state variable or physical process one owner. Do not count the same
molecular flux, cell proliferation process or metabolic sink in multiple components.
The present checker prevents duplicate writers to an input, but full process-level
responsibility accounting is a later composition feature.

## 7. Coupled numerical execution

The target execution planner builds a pinned model/dependency graph, checks ports
and ownership, negotiates engine capabilities and creates a numerical coupling
plan. It needs communication step sizes, interpolation, multi-rate synchronization,
solver error controls, discontinuity/event ordering and rollback rules.

Explicit feed-forward scheduling can suit some one-way dependencies. Feedback,
algebraic loops and stiff interactions may need iteration, implicit coupling or
another supported solver. A topological sort is not a feedback solver. Engines
must declare native step, snapshot, restore, restart and event capabilities rather
than having the platform assume those capabilities.

Test conservation, nonnegativity, step refinement, coupling error, convergence and
stability separately from biological endpoint agreement. Individually valid models
can produce an invalid composite. Validate the coupled system in its own context.
The current code supplies compatibility/DAG checks and an analytical conservative
exchange primitive; it does not implement generic multiscale execution.

## 8. Run state, scenarios, ensembles, and reproducibility

Separate a model definition, a scenario/experiment design, an individual run and
an observation/result. A scenario can branch from a pinned parent, changing one
intervention or assumption with a stated rationale. It must not overwrite its parent.
An ensemble contains separate seeded runs and a defined aggregation/evaluation plan.

Manifests pin the model/dependency snapshots, plan, parameters, seed and recorded
runtime identity. Mutable job status and engine state do not belong inside that
immutable manifest. Store artifacts by content, verify their checksums, and preserve
transformation lineage. Reproducibility may mean declared numerical/stochastic
tolerances rather than bitwise identity across hardware and libraries.

The local queue handles atomic claiming, idempotent submission, cancellation and
stale-worker fencing. Retry creates an explicit new run. Recovery does not silently
reuse partial state or manufacture a successful result. Native checkpoints,
resumption, ensembles and adaptive-fidelity scheduling need additional engine
capabilities and acceptance tests before being exposed as implemented functions.

## 9. General intervention framework

The planned classes are genetic, molecular, cellular, tissue, mechanical, chemical
and environmental. A request identifies a supported target, operation, magnitude,
units, time window, spatial extent and context. Admission checks model capability,
parameter range, available mechanisms and any required approvals/resources.

A genetic edit must not be translated directly into a tissue-healing percentage.
The path may require gene-response, signaling, cell-behavior and tissue mappings,
each supported by evidence and uncertainty. Without a bridge the correct outcome
is unsupported/unknown, not a confident generated effect.

The implemented intervention is a scalar parameter-set event on allowed parameters
at regular sample boundaries. At time t the new parameter applies to the following
interval. Runtime changes to initialization-only variables are rejected. The other
classes remain designed extension points.

## 10. Scientific data fabric

Target data sources include curated local data, NCBI GEO, CELLxGENE, HuBMAP and
versioned model/dataset repositories such as Hugging Face where appropriate.
Connectors have source-specific contracts and access/license checks; a connector
name is not evidence that a dataset was downloaded or validated.

Store raw data separately from normalized data and derived measurements. Retain
source identity, revision, checksum, license/access basis, biological context,
quality assessment, transformation code and lineage. Large imaging, spatial,
single-cell, array and time-series data have different representations and resource
needs; do not force all of them into a graph database or a small JSON response.

Datasets declare experimental/synthetic/simulated origin and calibration/validation/
benchmark role. Independence groups must reflect donors, experiments, publications
or related samples as appropriate. Caller-provided labels are not an independent
split authority. Immutable evaluated splits and reviewer identity are later controls.

## 11. Observation operators and scientific validation

An observation operator maps computed state to a measured endpoint, units,
sampling, geometry and measurement noise. Compare like with like: a transcriptomic
proxy is not automatically a tissue-function measurement. A mismatch should request
a justified operator rather than silently interpolate or relabel a quantity.

Verification asks whether equations/software execute as intended. Validation asks
whether a model is adequate for a stated real-world question. Calibration fits
parameters; it does not itself prove predictive validity. External evidence must
not be the same material used to tune and choose a candidate without acknowledging it.

Target reports include context, endpoint, reference data, assumptions, comparison
metrics, tolerances, sensitivity, identifiable parameters and separate uncertainty
sources. Numerical, observational, parameter, structural and cross-context
uncertainty should not become one unsupported confidence score. The current evaluator
only compares aligned scalar series and does not estimate these uncertainty components.

## 12. Model learning and evolution

Before learning, assess feasibility: available observations, parameter identifiability,
data quality, applicable context, compute budget and the scientific objective. A more
flexible model with no identifying data may be less useful than a simpler baseline.

The intended loop is: detect a measurable weakness, inspect evidence, propose a
versioned challenger, run verification and historical regression benchmarks, evaluate
frozen held-out endpoints, obtain independent review, and promote or reject. Keep
negative results, unsuccessful branches and rationale. A candidate cannot edit the
benchmark or quietly replace a champion.

Possible methods include parameter estimation, calibration, structure refinement,
new model construction, ML surrogates and optimization. Each has an applicability
and evidence contract. The current log-linear decay estimator returns a parameter
proposal only. No foundation model, autonomous biological model constructor,
general RL controller or automatic scientific promotion service is implemented.

## 13. Governance and dependency impact

Keep explicit dependencies between model versions, data revisions, observation
operators, couplings, benchmarks, manifests and reports. A corrected dataset or
invalidated mechanism should identify potentially affected models/runs and trigger
revalidation. Preserve the historical result and append a new interpretation/status;
do not rewrite the evidence record as if the mistake never happened.

Future independent evaluation needs author/evaluator separation, approval records,
role-based actions, frozen evaluation sets and change control. Current static API
tokens and application events are a local development profile. Database checksums
are integrity checks, not signatures or proof against a privileged database editor.
PROV-O vocabulary use is not a claim of full RO-Crate or SED-ML conformance.

## 14. Public API and scientific instrument boundary

The target API categories are biological entities, models/import/composition,
datasets/lineage, scenarios/branches, simulations/results/cancel, model learning,
validation and provenance/dependency impact. `api.md` lists implemented routes;
`api-target.md` maps the complete proposed surface to current or designed behavior.

All resource routes need versioned schemas, authentication/authorization,
resource limits and ownership rules appropriate to deployment. A long-running job
is accepted asynchronously and observed through its status, not executed inside
an HTTP request or represented as a fake instant completion.

The external Instrument lifecycle is describe, validate, estimate_cost, submit,
status, collect and cancel. A caller should receive the model/version, context,
artifacts, origin, uncertainty actually calculated and applicable validity limits.
It does not need to know whether the underlying engine is CompuCell3D or another
solver. Monetary cost is currently unknown; sample estimates are not billing quotes.

## 15. Repository and deployment architecture

Use one Peykar repository, independent of HomBon, with a modular management
monolith and separate computation processes/containers. Three real distributions
avoid premature packaging of every conceptual subfolder. Exact dependency direction
is enforced by tests. `target-layout.md` preserves the wider target folder design
and maps it to implemented code or future extension points.

Start with an executable single-host SQLite/WAL profile. PostgreSQL, S3-compatible
storage, distributed claims/outbox, SLURM, tracing and cloud batch execution are the
planned larger profile. Do not place a shared SQLite file under a multi-host fleet.
Engine sandboxes need hard time/memory/process/filesystem/network controls. The
bounded trusted-engine worker does not supply that untrusted-code boundary.

## 16. Technology choices and adoption gates

| Area | Technology direction | Current adoption |
|---|---|---|
| Backend/contracts | Python, FastAPI, Pydantic, JSON Schema, OpenAPI | Implemented |
| Units/numerical primitives | Pint, NumPy; SciPy for future solver adapters | Pint/NumPy used; SciPy not required by current engines |
| Statistical modeling | statsmodels, PyMC when a justified model needs them | Planned optional dependencies |
| ML/optimization | PyTorch, BoTorch for specific validated tasks | Planned optional engine/tool environments |
| Registry/query data | PostgreSQL in distributed profile | SQLite local implementation |
| Graphs | PostgreSQL initially; graph engine only if justified | In-memory biological reference graph |
| Artifacts | S3-compatible object storage at larger scale | Local content-addressed files |
| Experiment tracking | MLflow for ML-specific tracking | Planned, not the scientific provenance authority |
| Cell/spatial/array/image data | AnnData, SpatialData, Xarray, Zarr, OME-Zarr | Planned connector-specific choices |
| Jobs/containers/HPC | Durable workers, containers, SLURM adapter | Local queue; Docker/Compose configured; HPC planned |
| UI | Next.js, graph editor such as React Flow, scientific plots such as Plotly | Designed, not implemented |
| Operations | OpenTelemetry and GitHub Actions | CI configured; telemetry integration planned |

Python coordinates the platform; it does not require all numerical kernels to be
pure Python. C++, Rust or other specialist implementations can remain inside their
engine environments. Do not install an entire future stack just to populate a folder.

## 17. Six UI workspaces

| Workspace | Intended interaction | Truthfulness rule |
|---|---|---|
| Biological Explorer | Navigate species, systems, organs, tissue, cells, pathways and evidence | Show model availability and missing knowledge separately |
| Model Studio | Discover, version and connect models; inspect port incompatibilities | An edge is not proof of a valid coupling |
| Dataset Studio | Inspect provenance, quality, context, roles and transformations | Synthetic data remain visibly synthetic |
| Experiment Studio | Define initial state, interventions, baseline and branches | Admit only supported operations and explicit budgets |
| Simulation Viewer | Inspect structure and dynamics over time | Never portray uncomputed cell trajectories as measured state |
| Validation Center | Compare endpoints, uncertainty, regressions and review evidence | Passing a synthetic test is not biological approval |

For example, navigating Human -> Eye -> Retina -> Retinal Ganglion Cell should
show location, known attributes, relevant pathways, model/data links, interactions,
unsupported parts and evidence limits. The presentation must not imply that every
listed entity already has an executable or validated model.

## 18. Development gates and success criteria

P0 establishes contracts and runs two different synthetic model structures without
core changes. P1 reproduces untouched vCornea reference scenarios in a pinned
environment. P2 adds endpoint-specific independent evaluation or explicitly records
the absence of suitable data. P3 tests one justified model improvement against frozen
baselines; a negative result can still demonstrate a correct evaluation system.

P4 validates a scientifically justified molecular-to-tissue coupling independently
of the component models. P5 traces one defined genetic perturbation through a
supported cross-scale chain. P6 integrates a limited eye endpoint, for example a
corneal change to optical image quality, without claiming a complete eye twin.
P7 tests one inter-organ relationship, for example metabolism -> circulating
concentration -> tissue exposure -> cellular response. See roadmap.md for exit gates.

The long-term milestones are reproducibility, independent validation, supported
improvement, validated composition, cross-scale prediction, cross-organ prediction
and controlled model evolution. Complexity, model count and UI polish are not
substitutes for these demonstrated capabilities.

## 19. What not to build or claim at the start

Do not begin with atomistic whole-body simulation, every human cell, one universal
learned solver, an all-purpose custom numerical engine, dozens of microservices,
a graph database for every data shape, automatic self-certification, general RL
for all science, a complete human digital twin or one model for all diseases.
Do not equate fidelity with trust, successful execution with independent evidence,
or generated hypotheses with observations. The initial value is reliable
scientific model execution and evaluation, with a path to justified expansion.

## 20. Reading and coverage

This document consolidates the architecture and all requirements visible in the
supplied conversation. It is not a verbatim transcript of earlier, unprovided
sections. `requirements-traceability.md` shows the mapping to design, code,
tests and later phases; future work remains explicitly labeled.
