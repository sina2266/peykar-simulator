# Scalar data and frozen evaluation

CSV import accepts UTF-8 (optional BOM), one explicit time column and one numeric
value column. Extra columns are retained in the raw artifact but not interpreted.
Headers must be unique and nonblank; all rows must have the same width. At least
two and at most 100,001 aligned finite samples are required. Times must increase
strictly and have units convertible to seconds. No sorting, duplicate merging,
missing-value removal, interpolation or imputation happens implicitly.

The caller supplies endpoint, units, species/context, origin, role, source,
license and independence group using CSVImportSpec. These are declarations, not
proof of experimental provenance or authority to share the data. Import quality
checks do not establish biological validity.

The CLI reads at most 20 MiB plus a one-byte overflow check. The authenticated
API accepts a JSON body with `spec` and `csv_text` under its existing 2 MB total
request limit. It stores `csv_text.encode('utf-8')`; use the CLI to preserve exact
original file bytes. The API never opens a caller-supplied filesystem path.

## Lineage and persistence

The raw CSV and normalized Observation JSON are separate SHA-256 artifacts.
DatasetLineage pins their hashes, the full import mapping, importer version,
row count and performed quality checks. DatasetSpec and DatasetLineage commit
in one SQLite transaction. A conflicting version leaves both registered objects
unchanged. Content-addressed artifacts written before a rejected registration can
remain unreferenced; there is no automatic garbage collector in this release.
Re-importing identical bytes/spec is idempotent. Different bytes require a new
version even when they parse to the same numeric observations.

## Freeze and evaluate

EvaluationSetSpec pins the declared calibration/evaluation dataset references,
context, endpoint and tolerance. Freezing resolves every dataset and stores its
metadata hash, observation hash and independence group. Calibration data must
have the calibration role; evaluation data must not. The two sides cannot share
a dataset or independence group. Endpoint/context/units must agree. Reusing a set
ID/version with a different split or tolerance is rejected.

Evaluation resolves the frozen references and verifies their snapshots. The
stored calibration groups determine the leakage check; the execution request
cannot override the set's tolerance or split. Aligned time grids and a supported
endpoint are still required. There is no implicit resampling or observation model.
All reports for a set evaluation are committed together after every comparison
succeeds. This is a scalar comparison workflow, not model promotion.

ValidationRecord contains the run ID, manifest/result hashes, dataset/observation
hashes, scalar report and optional evaluation-set reference/hash. Its ID hashes
the complete record body, so identical numbers from different runs have different
records. Repeating the same evaluation is idempotent. GET a record by ID or use
`peykar report REPORT_ID`. The old comparison endpoint returns the same report
shape plus a Location header to the richer record. Historical 0.1/0.2 report-only
rows are retained; no missing run identity is invented for them.

Declared groups cannot establish donor-level independence or prove that a model
author never saw evaluation data. This version has no evaluator identity,
pre-registration timestamp authority, confidence intervals, multiple-testing
control or independent review workflow. Experimental-origin metadata remains
subject to provenance and independence review. Synthetic fixtures always report
synthetic verification.
