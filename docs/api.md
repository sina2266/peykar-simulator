# HTTP API

Version prefix: `/v1`. Machine-readable source: `schemas/openapi.json`.
Authentication, local deployment and body limits: [operations.md](operations.md).

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
