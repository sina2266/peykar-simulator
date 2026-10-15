# Proposed API surface and current route mapping

This preserves the blueprint's full public API categories. Target paths are
designs, not routes to call unless the implemented mapping is provided. All current
resource routes have `/v1` prefix. `api.md` and `../schemas/openapi.json` describe
the executable API, including auth, request bodies and actual response schemas.

| Blueprint target | Purpose | Current mapping or admission gate |
|---|---|---|
| GET /entities | Discover biological entities | Designed persistent explorer API |
| GET /entities/{id} | Inspect entity | Designed; in-memory reference graph exists |
| GET /entities/{id}/relationships | Inspect contextual/evidenced relationships | Designed query contract; no claimed endpoint |
| GET /models | Discover models | GET /v1/models with capability/species filters and pagination |
| GET /models/{id} | Inspect model | GET /v1/models/{id}/versions/{version}; exact version required |
| GET /models/{id}/versions | Version index | Designed dedicated index; versioned records are currently listed by /models |
| GET /models/{id}/capabilities | Scientific capabilities | Embedded in the current pinned ModelPackage response |
| GET /models/{id}/validity | Context-specific validity | Embedded profile; dedicated evidence-query route designed |
| POST /models/import | Inspect/admit model source | POST /v1/models admits JSON descriptors; remote executable import remains designed |
| POST /models/compose | Create pinned composite | Designed; compatibility/DAG checks exist as Python functions only |
| POST /models/validate | Evaluate model package | Descriptor checks exist; independent scientific evaluation is a separate action |
| POST /datasets/register | Register metadata/data | POST /v1/datasets with DatasetRegistration body |
| POST /datasets/inspect | Quality/schema inspection | Limited checks happen at local registration; dedicated inspection jobs designed |
| GET /datasets/{id} | Read dataset | GET /v1/datasets/{id}/versions/{version} |
| GET /datasets/{id}/lineage | Inspect transforms and provenance | GET /v1/datasets/{id}/versions/{version}/lineage for CSV imports; general graph designed |
| POST /scenarios | Register experimental design | POST /v1/scenarios |
| GET /scenarios/{id} | Read design | GET /v1/scenarios/{id}/versions/{version} |
| POST /scenarios/{id}/branch | Create a child design | Register a new Scenario via /v1/scenarios with exact parent Reference |
| POST /simulations | Submit a job | POST /v1/simulations; durable HTTP 202, optional Idempotency-Key |
| GET /simulations/{id} | Read execution status | GET /v1/simulations/{id} |
| POST /simulations/{id}/cancel | Request cancellation | POST /v1/simulations/{id}/cancel |
| GET /simulations/{id}/results | Collect results/artifacts | GET /v1/simulations/{id}/results after success |
| POST /models/{id}/calibrate | Submit calibration job | Designed; current decay estimator is a Python candidate-proposal function |
| POST /models/{id}/improvements | Create challenger proposal | Designed versioned change/evaluation workflow |
| POST /models/construct | Build a candidate model | Designed feasibility, construction and review workflow |
| POST /validation/runs | Compare run against reference | POST /v1/validation/runs; aligned scalar endpoint comparison only |
| GET /validation/reports/{id} | Read stored comparison report | GET /v1/validation/reports/{id}; report pins run/result/data digests |
| GET /models/{id}/benchmarks | Read applicable frozen benchmarks | Designed benchmark association and evaluation-set authority |
| GET /runs/{id}/provenance | Read run evidence trail | GET /v1/simulations/{id}/provenance plus /manifest and /events |
| GET /models/{id}/dependencies | Read pinned dependencies | Embedded references and snapshots; dedicated query designed |
| GET /datasets/{id}/impacted-models | Revalidation/impact analysis | Designed dependency-impact service |

Additional current operations are GET /v1/engines, GET /v1/simulations and
POST /v1/simulations/validate and /estimate. Execution-valid means a job can be
executed under the current contract, not that its biological predictions are valid.
No future route is implemented as a placeholder that returns a false success.

Release 0.3 adds POST /v1/datasets/import-csv, POST /v1/validation/sets,
GET /v1/validation/sets/{id}/versions/{version}, and POST /v1/validation/evaluations.
The older POST /v1/validation/runs response still returns a ValidationReport; its
Location header identifies the new stored, run-bound ValidationRecord.
