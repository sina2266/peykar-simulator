# Full corneal replay operator

This explicitly invoked offline profile reconstructs the archived formation and
three chemical-injury cohorts. It never installs/runs scientific code during
API model admission. Source choices, original data and unresolved paper conflicts
are documented in the [experiment inventory](../../experiments/vcornea/publication/README.md).
Full candidate runs remain pending until their actual outputs pass comparison.

The separately reviewed `stash-hpc-5.0.0` profile accepts only the complete pinned
January2025 source at `vendor/models/vcornea-history-c7f95d28` and its matching
`engine_adapters/compucell3d/vcornea-history-c7f95d28.json` pins. It retains each
family's original HPC project/state/defaults: formation40/8/12/Temperature8/four
workers; injury25/7/20/Temperature8/one worker. Only declared injury activation,
7200MCS pulse,50400MCS horizon and750/1500/2500 dose differ from its batch controls.
Both use original ten bins and lifetime overlay2. See the
[source audit and unresolved correspondence](../../experiments/vcornea/publication/january-source-family-audit-2026-10-23.md).
`reproduce_vcornea.py --max-parallel 6` is allowed only when every prepared XML
requests one native worker. Multiworker series remain limited to four jobs/at
most16 requested workers. This bounds a series, not the operating system or
concurrent independent series; inspect those before starting additional jobs.

## Pinned installed environments

`macos-arm64-4.6.0-explicit.txt` contains 400 exact official/conda-forge package
build URLs/MD5s; `macos-arm64-4.6.0-profile.json` adds package SHA-256s, native
Python identity and installed CC3D file hashes. This is Python3.10.14,
NumPy1.24.4, pandas2.3.3 and PyArrow16.1.0 with CC3D4.6.0 revision2 (`397f8b1`).
The CC3D package itself has SHA-256
`2a7805bfe8416fd71faf2caed16fdae80a520fc27b94727151eafffdb9cff4e9`.
It matches the paper's engine version; the authors' exact full environment/build
and historical seeds remain unavailable.

The earlier 399-package 4.7.0 ARM64 environment (`d4d66e0`) and Linux/AMD64
4.7.0 image (`f401d6d`) retain their separate identities and historical results.
Never label these as identical binaries. The root `uv.lock` is unchanged.
Original package-shipped notices are preserved under `notices/`, indexed by
package URL/SHA and original file hashes in `notice-inventory.json`: 484 distinct
package identities, 607 notice files. Missing package license metadata stays null;
an empty notice list does not assert that a package supplied a notice.
CompuCell3D and its contributors retain their original MIT/component notices;
dependencies retain their individually recorded terms.

An operator can install the frozen environment explicitly with Conda, then capture
its installed identity at the new prefix (relocated native binaries may differ):

```bash
conda create --prefix "$PWD/.peykar/cornea-native46" \
  --file runtime_profiles/vcornea-publication/macos-arm64-4.6.0-explicit.txt --yes
uv run python runtime_profiles/vcornea-publication/capture_profile.py \
  --prefix .peykar/cornea-native46 --output .peykar/new-native46-profile.json
```

Choose a fresh profile output. Review its installed hashes before using it in a
new experiment; do not replace an executed job's copied runtime profile.

## Actual lifetime fixes

The original model deletes a native FPP wall link and then reads/updates the freed
wrapper. An unmodified native run actually crashed in `getDistance`. Version1
reacquires a live link after deletion. Version2 also exits the membrane pixel
iterator immediately after deleting its cell: its native backing container has
then been invalidated. The [version2 patch](native-lifetime-2.0.0.patch) and earlier
version1 patch are retained separately; pristine archive/vendor files are unchanged.
New runs use version2. Regression tests execute the actual extracted methods and
demonstrate both invalid lifetimes in the old source and their repaired behavior.
The fixes establish memory safety at these sites, not biological equivalence.

Actual 4.6.0 checks passed: a 20-MCS observer probe; an unobserved matching probe
with byte-identical count, surfactant and Parquet outputs; and a 600-MCS moderate
injury probe (injury deliberately at20MCS) in 127.76 seconds. The latter removed
200 of990 original membrane lattice sites; independent cell volumes and actual
lattice occupancy agreed. These short runs verify instrumentation/injury execution,
not full publication reproduction.

## Run and inspect

Acquire/verify selected data without executing downloaded code:

```bash
uv run python scripts/acquire_vcornea_publication.py \
  --destination .peykar/publication-inputs
```

The permanent original analysis lives in `experiments/vcornea/publication/upstream`.
Execute it explicitly in the scientific environment:

```bash
QT_QPA_PLATFORM=offscreen MPLBACKEND=Agg PYTHONDONTWRITEBYTECODE=1 \
  .peykar/cornea-native46/bin/python runtime_profiles/vcornea-publication/analyze_original.py \
  --source experiments/vcornea/publication/upstream \
  --data .peykar/publication-inputs/archive/Data --output .peykar/original-analysis-new
```

Use fresh run/output directories, exact reviewed source pins and a frozen protocol:

```bash
uv run python -u scripts/reproduce_vcornea.py \
  --source .peykar/external/vcornea-archive-16884414 \
  --pins engine_adapters/compucell3d/vcornea-source-16884414.json \
  --prefix .peykar/cornea-native46 \
  --profile runtime_profiles/vcornea-publication/macos-arm64-4.6.0-profile.json \
  --protocol experiments/vcornea/publication/comparison-1.0.0.json \
  --output .peykar/cornea-hpc46-full-20261018 \
  --parameter-profile hpc-2.0.0 \
  --case self-organization --case slight-injury --case mild-injury --case moderate-injury \
  --seed 7 --seed 11 --max-parallel 4 --wall-seconds 86400
```

The last command was actually started on2026-10-18. By October23 its supervisor
was gone: two formation runs genuinely completed, four injury outputs had no
captured exits and two jobs never started. The explicit resume operator now runs
the remaining six in fresh workspaces under `.peykar/cornea-hpc46-resumed-20261023`.
Do **not** start a duplicate. Inspect `plan.json`, individual `record.json` PIDs and
`progress.jsonl`. Each job copies source/driver/profile hashes before execution.
macOS Seatbelt denies network and writes outside the private job; the environment
contains no API credentials. Jobs have host and kernel deadlines, bounded output,
BLAS/OpenMP thread environment1 and an owned `caffeinate` assertion. The unchanged
CC3D XML separately requests four workers for formation and one for injury;
the environment alone is not a native CPU cap. No hostile-code sandbox,
hard native memory cap, machine-restart recovery or checkpoint resume is claimed.
Running record files are authoritative while the supervisor awaits futures.

`scripts/resume_vcornea.py` reuses only full source/runtime/output-verified exit0
records. Every new run also copies a pinned `completion_guard.py`, which writes
the actual solver child exit and output hashes atomically, independently of the
outer operator process. `recover_native_completion` can reconcile that actual
proof after supervisor loss, retaining the interrupted record. Old unguarded
files cannot be promoted from timestamps or final-looking rows.

`scripts/analyze_vcornea_series.py` is the explicitly started bounded follow-on
for an existing plan. It waits for actual completion, then runs comparison,
original mathematics and spatial analysis in sequence, retaining stage logs and
negative comparisons (exit2). It performs no Git, deployment or API mutation.
The October23 resumed plan's follow-on is `.peykar/cornea-hpc46-analysis-20261023`.

After **every** job actually succeeds, run all three analyses (fresh destinations):

```bash
.peykar/cornea-native46/bin/python runtime_profiles/vcornea-publication/compare.py \
  --runs .peykar/cornea-hpc46-full-20261018 \
  --references .peykar/external/vcornea-publication-cohorts-16884414/Data \
  --output .peykar/cornea-hpc46-comparison-20261018
.peykar/cornea-native46/bin/python runtime_profiles/vcornea-publication/analyze_candidates.py \
  --source experiments/vcornea/publication/upstream \
  --runs .peykar/cornea-hpc46-full-20261018 --output .peykar/cornea-hpc46-original-20261018
.peykar/cornea-native46/bin/python runtime_profiles/vcornea-publication/analyze_spatial.py \
  --runs .peykar/cornea-hpc46-full-20261018 --output .peykar/cornea-hpc46-spatial-20261018
```

Comparison rehashes the protocol, source, driver, runtime, all130 reference members
and every candidate artifact. It rejects failed/partial/mismatched grids and reports
`not_reproduced` (exit2) on a quantitative mismatch; it never widens thresholds.
Original-analysis replay uses the authors' unchanged code on checksum-verified
copies of the new outputs. Spatial analysis verifies lattice/cell/count consistency
and renders formation, injury/recovery and EGF images from actual arrays. It does
not turn a count-return time or 2D structural proxy into clinical wound closure.

Focused operator analysis tests need the scientific environment:

```bash
.peykar/cornea-native46/bin/python runtime_profiles/vcornea-publication/test_analysis.py
```

Those fixtures are synthetic software tests. The default `make check-docker`
also exercises the source-lifetime and acceptance/provenance logic without the
heavy scientific environment. See [verification](../../docs/verification.md) for
commands actually observed, including failed/superseded diagnostic runs.

## Separately recovered historical source

Recover the complete July14 2024 source from objects inside the exact original
licensed code ZIP, without loading downloaded Git hooks/configuration:

```bash
uv run python scripts/recover_vcornea_history.py \
  --archive .peykar/external/archives/vCornea-16884414.zip \
  --output .peykar/history-recovered-new
```

Its50 pristine source files already live in `vendor/models/vcornea-history-b6123adf`.
The `historical-hpc-3.0.0` preparation profile reads the95 original HPC batch controls
as numeric AST literals; it never imports the batch launcher. It copies the complete
historical HPC formation project, not selected constants into the later mechanics.
The source keeps Temperature10, diffusion10, original43200MCS/initial state and
all original controls. Private lifetime repair2.0.0 and a declared read-only
five→ten height-bin overlay are separately hashed. Actual20MCS checks showed
identical counts/cells/EGF/SLS with either observation grid.

The two full new-seed7/11 formation jobs genuinely completed in
`.peykar/cornea-historical-formation46-20261023`; do not duplicate them.
Their completed follow-on `.peykar/cornea-historical-formation-analysis-20261023` uses
`--diagnostic-case self-organization` and passes. This explicit option permits a completed
subset inspection while preserving all frozen thresholds and the default
four-cohort gate; a subset result is always labelled `diagnostic_*`.
Historical injury preparation is rejected because the July injury PIFF has a
different initial cell population; source/state correspondence remains unresolved.

Original spatial observations can be reacquired through the separate pinned
catalog and inspected explicitly:

```bash
uv run python scripts/acquire_vcornea_publication.py \
  --catalog experiments/vcornea/publication/spatial-source-catalog-1.0.0.json \
  --destination .peykar/original-spatial-new
.peykar/cornea-native46/bin/python \
  runtime_profiles/vcornea-publication/inspect_original_spatial.py \
  --source .peykar/original-spatial-new/archive \
  --catalog experiments/vcornea/publication/spatial-source-catalog-1.0.0.json \
  --output .peykar/original-spatial-inspection-new
```

The catalog declares that those three spatial groups differ from the selected
four scalar cohorts; partial spatial horizons are never labelled complete.

## Explicitly evidenced injury-state candidate

`audit_original_state.py` reads the original VTK pressure/volume/lineage fields,
checks the admitted PIFF identities/types and records the chemical injection
coordinates. Its immutable [receipt](../../experiments/vcornea/publication/source-state-correspondence-1.0.0.json)
conditions target-volume recovery on the known source lambda; sign ambiguity,
pixel differences and spatial-group mismatch remain explicit. It does not infer
unknown seeds, lambda, a full checkpoint or clinical validity.

The separate `observed-injury-4.0.0` profile requires an explicit `--injury-state`
file with the exact admitted SHA256, complete historical HPC source and the unchanged
frozen protocol. It preserves original controls and separately declares the7200MCS
pulse,50400MCS horizon and original scalar-cohort doses. A600MCS native probe actually
completed with the expected initial counts, ten-bin/hourly grid and pulse[100,75].

Six full jobs are already prepared in `.peykar/cornea-observed-injury46-20261023`.
They were explicitly invoked with `--wait-for-series` for both active earlier series.
That operator snapshots the prerequisite definitions, rejects changed identities/
failed exits, verifies real completed records, waits at most24h and then executes
at most four jobs. It is not an API-triggered scientific action or a checkpoint resume.
Do not duplicate or change those prepared workspaces.

The follow-on `.peykar/cornea-reconstructed-combined-analysis-retry1-20261023` uses
`--include-completed-series` for the historical formation plan. It accepts only
genuinely completed, disjoint job identities with exactly matching runtime/protocol,
copies original plans and creates a separate combined plan for the full four-cohort
gate. Parent plans and source versions are preserved; numerical failure still exits2.
The first follow-on race before initial plan publication is retained as a failed
attempt; the bounded preparation-wait fix is covered by a regression test.

## Formation result and original field integrity

The two historical formation jobs and all three follow-on stages have completed.
Both seeds pass every frozen formation count/trajectory/thickness check; actual
layers and original-analysis figures were inspected. This is a formation-only
`diagnostic_passed`, not the pending full four-group gate. See the
[portable result](../../experiments/vcornea/publication/observed-2026-10-23/formation-historical3-comparison.md).
Do not start replacement/duplicate formation runs.

The original finalCONTROL VTK is internally truncated despite its correct archive
checksum. `inspect_original_spatial.py` now reads/checks actual ASCII field values,
retains the9597/18000 Pressure values and three missing fields as explicit gaps,
and uses only complete required fields. All previous ten image hashes are unchanged.
The fresh actual inspection is `.peykar/cornea-original-spatial-inspection-layout-20261023`;
older receipts and raw bytes remain intact.

An explicit additional analysis exports original EGF sensing/summed/mean fields:

```bash
.peykar/cornea-native46/bin/python \
  runtime_profiles/vcornea-publication/audit_original_sensing.py \
  --source .peykar/external/vcornea-original-spatial-16884414 \
  --catalog experiments/vcornea/publication/spatial-source-catalog-1.0.0.json \
  --output .peykar/original-sensing-new
```

The actual retry2 audit retains1218 positive epithelial cells, one large original
outlier and the unavailable finalCONTROL sensing field. Earlier failed/read-only
and pre-format attempts remain separately retained. No missing value is replaced
or descriptive statistic turned into a candidate acceptance rule.

## Completed baseline and original-analysis export

The resumed later-source series and its follow-on genuinely completed:
`.peykar/cornea-hpc46-analysis-20261023` is `not_reproduced`, with numerical
comparison exit2 and original/spatial analysis exit0. Both moderate seeds pass
the frozen scalar checks; the other six baseline jobs fail. Actual source/runtime/
completion proofs and all six full chemical grids/pulse positions were checked.
Do not resume this completed series again.

The derived operator is now running four injury jobs, with two moderate jobs
waiting for capacity. Its existing combined follow-on still waits for all six
actual completions. These are previously invoked explicit offline operators;
the management API does not execute this model code.

All65 original height Parquets identify fastparquet2024.5.0 /pandas2.2.2 producer
metadata. The current runtime writes PyArrow16.1.0 /pandas2.3.3. This partial original
writer information does not identify CC3D binaries/seeds/full environment. Current
runtime locks and active jobs remain unchanged; numerical units/values are compared.

`analyze_original.py` now fits layout/bounds in its display hook so original titles
and labels are fully visible. Original code/math stay unchanged. Actual original65
and baseline8 replays preserve all numerical summaries and all eight mean CSV byte
sequences. Earlier clipped/bounds-only attempts remain as separate evidence; final
outputs are `.peykar/cornea-original-cohorts-layout-retry1-20261023` and
`.peykar/cornea-hpc46-original-layout-retry2-20261023`. The combined follow-on uses
the corrected exporter when the six new native runs actually finish.
