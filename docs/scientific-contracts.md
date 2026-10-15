# Scientific contracts

All public contracts are Pydantic v2 models with `extra="forbid"`, frozen model
attributes, finite numerical values and explicit schema version `1.0.0`. Collections
inside scientific snapshots are tuples. Pydantic `model_copy(update=...)` does not
validate updates; application or plugin authors must not use it with untrusted input.
API and file admission use validated constructors/model_validate_json.

| Contract | Required distinction |
|---|---|
| ModelPackage | Exact scientific model version vs engine implementation version |
| ValidityProfile | Context, endpoints, evidence, limitations; no global trust score |
| ScientificPort | Units and semantics vs geometry and numerical coupling |
| DatasetSpec | Experimental, synthetic or simulated origin; calibration vs held-out role |
| Observation | Measured endpoint and sampling vs engine state |
| SimulationPlan | Requested model/context/parameters/seed/budget vs actual execution |
| Scenario | Versioned design with an explicit optional parent |
| RunManifest | Snapshots and environment at submission vs mutable status |
| SimulationResult | Predicted values, never automatically experimental observations |
| ValidationReport | Endpoint-specific comparison, not automatic model certification |
| KnowledgeGap | Missing evidence or mechanism, not an invented biological relation |

## Identifiers and versions

Resource IDs start with a lowercase ASCII letter and use letters, numbers, dot,
underscore and hyphen, with a maximum of 80 characters. Model/scenario/data versions
use exact `major.minor.patch` strings. No floating `latest` dependencies.

Schema migration is a separate concern from model versioning. Incompatible schema
changes require a new schema version and an explicit converter; this release only
accepts version 1.0.0. SQL schema migration 001 is shipped as package data.

## Unit and time rules

Pint handles dimensional conversion; it does not prove semantic equivalence. Core
plans convert time to seconds and reject incompatible units, nonpositive steps,
nonintegral grids and oversized requests before allocating output arrays. Values
must be finite. Seeds and integer budgets are strict integers and reject booleans.

Reference engines use canonical model parameter units. Overrides and interventions
can use convertible units. Duplicate or ambiguous overrides are rejected. Runtime
parameter names are restricted per engine. Discrete cell counts are dimensionless
but carry their semantic identity separately at a port boundary.

## Hashing and integrity

SHA-256 checksums use sorted-key, compact UTF-8 JSON with no NaN/Infinity. This is a
Peykar canonical JSON convention, not RFC 8785 compliance. A result artifact is
published atomically and rehashed on read. Registry versions and manifests reject
changed content. Checksums detect corruption; they are not signatures or proof of
truth. A database owner can modify data and hashes, so external audit needs a
separate append-only or signed trust boundary.

## Reproducibility scope

Manifests capture the model snapshot, transitive exact dependencies, plan, seed,
engine file digest, interpreter version, package versions and uv.lock digest when
available. Original model-source digest is a declaration unless the import process
actually verifies the source. Byte-identical replay across arbitrary architectures,
GPU libraries or future dependency versions is not promised. The worker checks
engine code before executing a queued snapshot; preserve the lockfile and environment.

## Governance

API registration cannot mark a model scientifically validated. Reports preserve
their context and evidence class and do not change validity automatically. A future
promotion service needs evaluator identity, immutable benchmark/split manifests,
approval records, dataset/model impact analysis and revalidation triggers. Those
are specified extension points, not implemented access-control guarantees.
