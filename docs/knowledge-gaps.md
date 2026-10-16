# Explicit knowledge gaps

Peykar can persist and query declared unknown mechanisms, missing reference data,
unsupported interventions and missing coupling evidence using the existing
`KnowledgeGap` 1.0.0 contract. This implements a local Missing Knowledge Registry
from the architecture. It does not infer unknowns automatically or fill them with
invented mechanisms.

Each record declares its description, scientific context, exact affected model
references and severity (`blocking`, `limitation` or `informational`). Referenced
models must already exist at those exact versions. A general knowledge gap may
have no affected models; the registry does not invent a model to attach it to.
Cross-context concerns may reference a model without extending that model's
validated context.

Records are immutable by ID and stored in the existing local object registry at
fixed storage version `1.0.0`. Identical registration is idempotent. Changed
description, context, affected references or severity requires a new ID; conflicting
registration returns 409 and preserves the old record. Reads verify content hashes.
The service requires a nonblank description of at most 20,000 characters, nonblank
species/system and at most 200 unique affected model references. It preserves the
existing public contract and does not migrate or rewrite historical model data.

Severity is a submitter declaration. Registering a `blocking` gap does not add a
new simulation execution gate, change a model validity profile, prove missing data
were independently reviewed or create scientific approval. There is no automatic
resolution, closure, generation, reviewer identity or promotion workflow in this
increment. Those need separate contracts and evidence.

## API

All routes require the existing bearer-token authentication:

| Method | Route | Access | Result |
|---|---|---|---|
| POST | `/v1/knowledge-gaps` | Admin | Register a declaration; digest and Location header |
| GET | `/v1/knowledge-gaps/{id}` | Read | Retrieve the immutable declaration |
| GET | `/v1/knowledge-gaps` | Read | Filtered, ordered and bounded list |

The list supports exact `species`, `biological_system` and `severity`, exact
membership of one `condition`, and affected `model_id` with optional exact
`model_version`. A model-version filter requires its model ID. An ID-only model
filter matches all pinned versions of that model. Conditions are declarations,
not ontology inference. Combined filters use AND and run before pagination.
Ordering is ascending ID; `limit` is 1–200 (default 100), and `offset` is nonnegative.
An unmatched filter returns an empty list. Unknown record/model references return
404; malformed declarations or filters return 422. Reader writes return 403.

```json
{
  "schema_version": "1.0.0",
  "id": "gap.synthetic-reference",
  "description": "Independent biological reference evidence is unavailable for this synthetic fixture.",
  "context": {
    "schema_version": "1.0.0",
    "species": "synthetic",
    "biological_system": "reference verification",
    "conditions": []
  },
  "affected_models": [
    {"schema_version": "1.0.0", "id": "reference.decay", "version": "0.3.0"}
  ],
  "severity": "blocking"
}
```

This is an infrastructure example, not an assessment of an external model or a
biological finding. Save it as `gap.json` after registering the reference models
with `peykar init` or use your own exact model references and declared context.

## Local operator CLI

```bash
uv run peykar knowledge-gaps register gap.json
uv run peykar knowledge-gaps show gap.synthetic-reference
uv run peykar knowledge-gaps list --severity blocking --model-id reference.decay --model-version 0.3.0
uv run peykar knowledge-gaps list --species synthetic --limit 20 --offset 0
```

The CLI uses the local `PEYKAR_DATA_DIR` and the same reference, integrity and
immutability checks as the API. These commands register/read declarations only;
they do not download, install or execute model code.
