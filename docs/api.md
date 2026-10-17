# HTTP API

Version prefix: `/v1`. Machine-readable source: `schemas/openapi.json`.

Frozen cohort `/v1/batches` and execution `/v1/batch-runs` now expose parameter
sweeps, mixed queues, status/cancel/events and complete/partial terminal reports.
Exact contracts, roles, budgets and commands: [E2](epics/E2-reproducible-cohorts.md).
Authentication, local deployment and body limits: [operations.md](operations.md).

Shared system routes now include source library discovery (`/v1/libraries`),
persistent graphs/neighborhoods (`/v1/biology/graphs`), frozen experiments
(`/v1/experiments`) and durable child-run workflows (`/v1/experiment-runs`).
Exact routes/roles are in [libraries](library-ecosystem.md),
[biology](biological-graph.md) and [experiments](experiment-workflows.md); generated
OpenAPI includes them. Data reads require authentication; writes require admin.
There is no HTTP Git/source-path import or remote code execution.

| Method | Path | Purpose | Access |
|---|---|---|---|
| GET | /healthz | Process health | Public |
| GET | /v1/engines | Available trusted engine versions | Read |
| GET | /v1/models | Discover capability/species and versioned packages | Read |
| POST | /v1/models | Register unvalidated immutable descriptor | Admin |
| GET | /v1/models/{id}/versions/{version} | Read pinned model | Read |
| POST | /v1/knowledge-gaps | Register immutable explicit unknown | Admin |
| GET | /v1/knowledge-gaps | Filter by context, severity and affected model before pagination | Read |
| GET | /v1/knowledge-gaps/{id} | Read an explicit unknown | Read |
| POST | /v1/scenarios | Register baseline or branch with parent reference | Admin |
| GET | /v1/scenarios/{id}/versions/{version} | Read pinned scenario | Read |
| POST | /v1/datasets | Register metadata and normalized observation | Admin |
| GET | /v1/datasets/{id}/versions/{version} | Read data and lineage metadata | Read |
| POST | /v1/datasets/import-csv | Import scalar CSV and retain raw bytes | Admin |
| GET | /v1/datasets/{id}/versions/{version}/lineage | Read CSV transformation record | Read |
| POST | /v1/vcornea/outputs/cell-counts | Import native four-column counts and retain exact raw bytes | Admin |
| GET | /v1/vcornea/outputs/{id}/versions/{version} | Read immutable native output and provenance spec | Read |
| GET | /v1/vcornea/outputs/{id}/versions/{version}/lineage | Read raw/normalized native lineage | Read |
| POST | /v1/simulations/validate | Check execution feasibility | Admin |
| POST | /v1/simulations/estimate | Resource estimate; money cost unknown | Admin |
| POST | /v1/simulations | Durable submission, HTTP 202 | Admin |
| GET | /v1/simulations | Paginated runs | Read |
| GET | /v1/simulations/{id} | Mutable execution status | Read |
| POST | /v1/simulations/{id}/cancel | Request cancellation | Admin |
| GET | /v1/simulations/{id}/results | Successful normalized result | Read |
| GET | /v1/simulations/{id}/manifest | Immutable inputs and environment | Read |
| GET | /v1/simulations/{id}/events | Ordered lifecycle events | Read |
| GET | /v1/simulations/{id}/provenance | PROV-O vocabulary mapping | Read |
| POST | /v1/validation/runs | Store endpoint comparison report | Admin |
| GET | /v1/validation/reports/{id} | Read run-bound comparison record | Read |
| POST | /v1/validation/sets | Freeze declared datasets, groups and tolerance | Admin |
| GET | /v1/validation/sets/{id}/versions/{version} | Read frozen evaluation set | Read |
| POST | /v1/validation/evaluations | Evaluate a run using a frozen set | Admin |

Creation endpoints are not promises of scientific approval. Results are available
only after worker success. Domain errors: 404 unknown reference; 409 immutable
content/state/idempotency conflict; 422 scientific/execution constraint or schema
validation; 501 unsupported engine capability. Authentication uses 401 and 403.

Native vCornea ingestion uses `{ "spec": VCorneaOutputSpec, "csv_text": "..." }`.
It returns HTTP 201 with the lineage and an output `Location`. Reusing an identical
version is idempotent; changing its bytes/spec returns 409. Counts are in `count`
and native Time is in `hour`. Preserve CRLF when serializing the original CSV.
Thickness decoding/import is a separate operator CLI operation. These endpoints
do not download or execute model code. See [native outputs](vcornea-native-output.md).

`Idempotency-Key` is scoped to the local workspace and checked against the normalized
plan digest. Reuse with the same plan returns the existing run; use with a different
plan returns a conflict. Do not retry scientific work with a different key by accident.
Submission responses include `Location: /v1/simulations/RUN_ID` and, while queued
or running, `Retry-After: 1`. Poll the location, then retrieve its `/results` only
after success; pending/failed/cancelled results return 409.

Lists use bounded limit/offset. Model capability/species filters apply before
pagination. All SQL filter values are bound parameters, never interpolated SQL.

The existing comparison response includes a Location header for its stored record.
CSV and frozen-set contracts and limits are described in [data-evaluation.md](data-evaluation.md).

Future endpoints for importing remote executable models, composing full engines,
model construction, calibration jobs, approvals and dataset impact graphs are
specified in the architecture but not exposed as fake successful operations.

Knowledge gaps preserve declared missing evidence/mechanisms. Registration resolves
all affected model references exactly; a general gap may name none. IDs are
immutable and reader tokens cannot register them. See [knowledge gaps](knowledge-gaps.md)
for filters, limits and the lack of automatic job blocking/closure.

Native operator evidence is available through authenticated reader/admin calls:
`GET /v1/runtimes?engine=copasi` and `GET /v1/runtimes/{id}/versions/{version}`.
`NativeRuntimeEvidence` includes source references, image/output digests,
`managed_queue_supported=false` and limitations for the historical operator probe.
Offline CLI admission verifies the retained bundle. New `/v1/native-profiles`
separately advertise reviewed queue drivers; `/v1/native-models` admits declarative
SBML/PKML and `/v1/native-runs` manages explicit native jobs through an operator
agent. Installation/profile registration stays offline; API has no Docker access.
Full routes, permissions, limits and commands: [native agent](native-agent.md).

`/v1/native-interfaces`, `/v1/native-derivations`, `/v1/hybrid-flows` and
`/v1/hybrid-runs` now expose explicit native/reference scalar DAGs. Transfers retain
source/target quantities and child identities, with original models preserved.
Routes/roles and supported SBML subset: [E3](epics/E3-native-scalar-flows.md).
See [runtime profiles](../runtime_profiles/README.md).
