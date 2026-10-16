# Native vCornea output import

The native importer follows the actual writer inspected in
`Local/Project/{paper_version,paper_version_STEM_ONLY}/Simulation/vCornea_v2Steppables.py`.
The writer schema matches the pinned Git revision
`f5c03afb9c4c7d909494146c5db608389e957070` and the corresponding Python files in the
exact Zenodo source archive `10.5281/zenodo.16884414`. The archive and Git XML inputs
are not assumed identical. Actual input-tree, archive, runtime and parameter hashes
belong to the run/import provenance.

This feature imports data without importing or executing an upstream Python module.
Execution, source permissions, independent biological evidence and the interpretation
of a published reproduction are separate gates.
The admitted archive now has a working [operator runtime](epics/E1-vcornea-native-runtime.md);
native imports do not cause execution or automatically certify that runtime evidence.

## Observed schema and semantics

| Native file | Exact columns | Meaning retained by Peykar |
|---|---|---|
| `cell_count_{mcs}.csv` | `Superficial,Wing,Basal,Stem,Time` | Four integer cell counts; Time is hours |
| `thickness_rep{replicate}_{mcs}.parquet` | `Time,Bin,Height` | Hours; ten spatial bins; upstream mean top-cell yCOM in lattice coordinates |
| `surfactant_{mcs}.csv` | `Total Amount,Time` | Not imported; physical units of the field integral are not established |

The inspected writer sets `HOURtoMCS = 10` and writes `Time = mcs / HOURtoMCS`.
Cell counts and heights are sampled every ten MCS, so the native Time column already
contains whole hours. The importer preserves those hours. It does not treat Time as
MCS or multiply it by a second time mapping. Raw header order is checked against the
pinned writer. Numeric rows must be finite, aligned and strictly increasing in time;
counts must be nonnegative exact integers. No sorting, interpolation or imputation
is performed.

The thickness writer emits every bin 0–9 for each time, including zero for a bin
without selected cells. `Height` is the upstream average top-cell yCOM calculation.
It is retained as `lattice_y_coordinate`, not converted into calibrated physical
thickness. The source's nominal lattice-scale constant alone does not establish
the baseline, measurement operator or experimental comparability of that endpoint.

The native outputs contain no random seed. The inspected Python seed call is
commented out, and the source XML has no explicit `RandomSeed`. A filename's MCS
counter or replicate fragment is not seed evidence. An absent seed remains JSON
`null`. A derivative runner can supply a seed only with separately recorded runtime
evidence stating its Python/NumPy/Potts scope.

## Provenance and storage

`VCorneaOutputSpec` supplies an immutable ID/version, context, origin, license/source
declarations, variant and actual source-tree digest. Provenance identifies either
the pinned Git commit, the exact archive DOI plus archive SHA-256, or both where
appropriate. Runtime, parameter and seed evidence remain explicit; source identity
is never inferred from an output filename.

`VCorneaOutputArtifact` keeps all four count series or all ten lattice-coordinate
bin series. It is separate from `SimulationResult` so a missing upstream seed is
not fabricated. Its validity status remains `unvalidated`; origin can be
`simulation` or `synthetic`, never `experimental`.

Exact raw bytes and canonical normalized JSON have independent SHA-256 artifacts.
`VCorneaOutputLineage` pins both, the complete import spec, exact columns and row
count. Thickness additionally pins the exact decoded JSON bytes. Output and lineage
register atomically; conflicting ID/version reuse preserves the original records.
Identical reimport is idempotent. As with other importers, artifacts written before
a rejected registration can remain unreferenced.

Metadata declares source/runtime/context/permission facts; the output bytes alone
cannot independently prove them. Import schema checks establish engineering
compatibility, not experimental or clinical validity.

## Parquet reader boundary

Pandas and a Parquet reader remain in the isolated scientific runtime. They are not
management API or core dependencies. Invoke the trusted repository script inside
that runtime:

```bash
python /repo/scripts/vcornea_normalize.py /outputs/thickness_rep_21.parquet \
  --output /outputs/thickness-decoded.json
```

The parent runner supplies CPU/memory/time limits. The script reads at most 100 MiB,
requires numeric `Time,Bin,Height` columns and at most 1,000,010 ordered rows, and
records the raw Parquet SHA-256 plus its own script digest. It does not load pickle
or upstream simulation code. It rejects incomplete/repeated bins, changed time
groups, negative/nonfinite values and conflicting output-file reuse.

The descriptor uses `schema_reference_commit` for the reviewed schema's Git
reference. It does not claim that an archive-derived execution ran a Git checkout.
`import_thickness` independently checks that descriptor, the supplied normalizer
digest and exact raw bytes, then preserves all three artifacts. The descriptor is
a trusted-reader assertion; its hashes do not turn arbitrary caller-supplied JSON
into proof that Parquet was correctly decoded.

## CLI and scalar benchmarks

```bash
uv run peykar vcornea import-cell-counts cell_count_21.csv --spec count-output-spec.json
uv run peykar vcornea import-thickness thickness_rep_21.parquet \
  --spec thickness-output-spec.json --decoded thickness-decoded.json
uv run peykar vcornea output OUTPUT_ID VERSION
uv run peykar vcornea lineage OUTPUT_ID VERSION
uv run peykar vcornea extract-count OUTPUT_ID VERSION DATASET_ID DATASET_VERSION \
  --column Stem --independence-group DECLARED_GROUP
uv run peykar vcornea scalar-lineage DATASET_ID DATASET_VERSION
```

Use the exact specs and IDs produced for the actual run. Extraction copies one
unchanged count column to a benchmark Observation with endpoint
`vcornea.superficial_count`, `vcornea.wing_count`, `vcornea.basal_count` or
`vcornea.stem_count`, retaining hour/count units, context and origin. The dedicated
`VCorneaScalarLineage` pins the native lineage. Its shared import-identity record
prevents another parser from attaching conflicting provenance to the same dataset
version.

The existing repeated-seed protocol accepts these scalar benchmarks only when the
native import records seed evidence and a runtime digest. Unknown-seed outputs
remain readable but cannot enter a seed-paired protocol. Each cohort must share
original source identity, variant, parameter digest, runtime and origin. All sample
errors for every declared seed must pass the frozen tolerance. Synthetic inputs
remain `synthetic_verification`; simulation-only inputs produce
`simulation_reference_comparison`, never independent experimental validation.

## Authenticated HTTP access

`POST /v1/vcornea/outputs/cell-counts` accepts a JSON object with `spec` (the same
`VCorneaOutputSpec` used by CLI) and `csv_text` (the exact decoded UTF-8 bytes).
Serialize from `path.read_bytes().decode("utf-8")` so universal-newline translation
does not change the raw CRLF digest. Admin authorization is required.

The 201 response is the immutable lineage. Its `Location` points to
`GET /v1/vcornea/outputs/{id}/versions/{version}`; append `/lineage` for the import
record. Both GET routes accept reader authorization. Identical registration returns
the same lineage; changing an existing version returns 409. Anonymous access returns
401; readers cannot import. The existing HTTP body bound also applies.

Thickness is decoded inside the isolated runtime and admitted with the operator
CLI, then read through the same output/lineage GET routes. No Parquet or scientific
runtime dependency is added to the API. Neither HTTP route submits vCornea to the
general worker queue or fetches/executes remote source.
