# Local scalar exports and repeated seeds

Peykar can now import an explicitly mapped local delimited scalar export and freeze
a seed-paired regression protocol. This is a partial engineering increment for the
CompuCell3D integration. Native output discovery, external engine execution,
published vCornea scenario reproduction and biological validation remain gated.
No upstream source, output or dataset is bundled by this feature.

## Explicit import mapping

`CompuCellImportSpec` embeds the existing `CSVImportSpec` for endpoint, metadata,
column names and output units. Its time column contains Monte Carlo step numbers.
The operator must additionally declare the delimiter (comma, tab or semicolon),
positive physical time per step, time-mapping evidence, seed, engine version,
source SHA-256 and runtime SHA-256. None is inferred from a filename. No source or
runtime is loaded or executed, and these declared digests do not establish artifact
admission or permission. Use the separate external-artifact inspection workflow
for local source/runtime files.

The importer accepts strict UTF-8 with optional BOM, unique headers, consistent
row widths and 2–100,001 finite scalar rows within 20 MiB. Steps must be strictly
increasing nonnegative integers no larger than 4,294,967,295. Physical time equals
`step * time_per_step`; there is no implicit offset, interpolation, sorting,
aggregation, missing-value removal or Monte Carlo-to-second assumption. The time
unit must convert to seconds. Extra columns remain in the exact raw artifact and
are not interpreted. Origin can be `simulation` or `synthetic`, never `experimental`.

Raw export bytes and normalized observations are separate checksum-verified
artifacts. Dataset metadata and `CompuCellImportLineage` register atomically.
The lineage pins the complete mapping, raw/normalized hashes, seed and declared
environment/source identity. Reusing an ID/version with different bytes or mapping
fails without replacing previous metadata. Artifacts written before a rejected
registration may remain unreferenced, as with the generic CSV importer.
Both importers also commit one shared import-identity record, so concurrent imports
through different parsers cannot attach conflicting raw provenance to one version.
Historical imports remain readable and identical reimports add that identity
without replacing existing metadata or lineage.

These records use a dedicated `compucell_lineage` kind. Retrieve them with
`peykar compucell lineage DATASET_ID VERSION`; the generic CSV lineage endpoint
continues to describe generic CSV imports. Imported observations are ordinary
datasets and can be used by the scalar evaluation services when their scientific
context and endpoint match.

## Freeze and compare

`RepeatedSeedProtocol` pins at least two distinct declared seeds, benchmark
dataset versions, context, endpoint and one nonnegative scalar tolerance.
Freezing captures every reference dataset's metadata, observation and full import
lineage hashes. Each cohort must share source/runtime/engine identity, origin
and declared time mapping. Comparison requires exactly one candidate for every
frozen seed; an extra, missing or repeated seed is rejected. Candidate source and
runtime identities may differ from the reference cohort so an adapter change can
be compared, but each cohort is internally consistent.

Samples must have the same physical time grid and compatible value units. Unit
conversion permits relative roundoff of `1e-12` for times and time per step;
absolute tolerance is zero and no resampling occurs. Every sample of every seed
must satisfy the frozen absolute-error tolerance. Reports preserve per-seed RMSE,
maximum absolute error, pass/fail and candidate observation/lineage hashes.
Reports are immutable, content-addressed and retrievable. A failed comparison is
a stored failed report, not a successful reproduction claim.

This protocol checks paired scalar regression. It does not estimate confidence
intervals or stochastic-distribution equivalence, prove actual engine seed
behavior or independently certify preregistration. Simulation references produce
`simulation_reference_comparison`; any synthetic input produces
`synthetic_verification`. Neither means independent experimental validation.

## Synthetic local example

The fixture directory contains invented scalar tables and explicit declarations.
Its `source_digest` and `runtime_digest` hash the two local descriptor text files;
those descriptors explicitly state that no engine was executed. The version
`4.7.0` is example mapping metadata, not evidence of an installed environment.

```bash
export PEYKAR_DATA_DIR=.peykar/scalar-demo
for name in reference-7 reference-11 candidate-7 candidate-11; do
  uv run peykar compucell import-output \
    "experiments/data/compucell-scalar-synthetic/$name.tsv" \
    --spec "experiments/data/compucell-scalar-synthetic/$name.json"
done
uv run peykar compucell freeze-seeds experiments/data/compucell-scalar-synthetic/protocol.json
uv run peykar compucell compare-seeds cc3d.synthetic.seeds 1.0.0 \
  experiments/data/compucell-scalar-synthetic/candidates.json
uv run peykar compucell lineage cc3d.synthetic.reference.7 1.0.0
uv run peykar compucell seed-report REPORT_ID
```

Replace `REPORT_ID` with the comparison response ID. This example intentionally
uses a separate local state directory. It runs on the host without a bounded
Linux worker because it imports files and compares observations only.

Before importing actual vCornea output, inspect its real schema and verify the
source/data permissions, seed behavior, endpoint meaning and physical time mapping.
The existing [vCornea admission gates](vcornea-integration.md) still apply.
