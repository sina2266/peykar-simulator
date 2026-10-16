# Verification record

## E1 native vCornea: 2026-10-16 (unreleased, on 0.3.0)

Observed on this macOS checkout, Docker Desktop, and a separate emulated
Linux/AMD64 scientific runtime. [E1 acceptance](epics/E1-vcornea-native-runtime.md)
and its [portable receipt](../engine_adapters/compucell3d/vcornea-verification-2026-10-16.json)
distinguish engineering completion from the remaining P1 published-replay gate.

- The public Git checkout was cloned and pinned at
  `f5c03afb9c4c7d909494146c5db608389e957070`; the licensed execution source is the
  distinct exact Zenodo record **16884414**, declaring **CC-BY-4.0**. The complete
  228,842,524-byte code ZIP passed published MD5 and local SHA-256 checks.
  `scripts/prepare_vcornea.py` extracted 27 pinned reviewed files without `.git`;
  a fresh extraction and matching-source reuse were observed. The archive's Local
  paper XML differs from Git; these identities are not silently equated.
- `scripts/acquire_vcornea.py members` acquired six complete native reference
  files for three severities through checked HTTP Range/ZIP64 reads. Size/CRC32
  and local SHA-256 passed. The acquisition transferred **16,317,077 bytes** including
  ZIP indexing. Whole Data.zip MD5 is **unverified**; the stopped bulk download's
  214,036,480-byte partial file was preserved, not reported as a full download.
- `docker build --platform linux/amd64 -f engine_adapters/compucell3d/runtime/Dockerfile
  -t peykar-cc3d:4.7.0-linux64 engine_adapters/compucell3d/runtime` passed after
  repairing an observed missing `libGL.so.1` with explicit pinned GL/X11 dependencies.
  The final lock contains **424** packages; **405** package-provided notice
  directories were preserved with the full inventory. Engine/Qt imports and an
  actual PyArrow Parquet round trip passed. The runtime receipt records CC3D 4.7.0
  revision 5, Python 3.12.2, NumPy 2.2.6, pandas 3.0.6 and PyArrow 16.1.0.
  The authors' complete original environment was not recovered.
- `scripts/run_vcornea.py` first completed a 20-MCS, seed-7 native probe in
  32.013 seconds. Its private copy produced actual count, surfactant and thickness
  files. Source mechanisms were preserved; horizon and Python/NumPy/Potts seed
  controls were recorded. No API token, home directory or live data volume was
  mounted; execution was offline with resource/deadline/output bounds.
- The exact `scripts/verify_vcornea_epic.py` command in the E1 document completed
  six published imports and four real 60-MCS runs (reference/candidate seeds 7,11).
  Published count and height series have **5,041 / 4,621 / 5,041** hourly samples;
  archived seeds/runtime/per-run parameters stay unknown. Acceptance was written
  before runs and the reference protocol frozen before candidates.
- Both seven-sample Basal comparisons passed the frozen **0 count** tolerance with
  RMSE/max error **0**. All three raw native output hashes also match within each
  seed pair. Stored report retrieval passed:
  `seed-report.a4e212d71f85c7e198ab604e8bb26f9a0938b04ff9126923403b9051380f3d06`.
  Its class is `simulation_reference_comparison`, not biological validation.
- The E1 `scripts/run_vcornea.py --steps 600 --seed 7 --wall-seconds 900` command
  succeeded in **221.084646 seconds**, producing **61** count/height samples over
  **0–60 hours**. Inspection of the actual 601-row surfactant CSV found zero before
  hour 50 and first positive integral **750** at hour **50** (500 MCS), confirming
  execution crossed the injury event. Physical units of that raw integral remain
  unestablished. Its native Parquet was decoded/imported with ten lattice-coordinate
  series. No native-run containers remained after cleanup.
- Targeted importer/source/runtime/API tests passed before the final full run.
  Final **`make check-docker` passed**: Ruff lint and format (189 files), strict
  Mypy (**59 source files**), **242 tests** in 12.14 seconds, **88%** observed statement
  coverage, current generated contracts/OpenAPI and all three wheels/sdists.
  The host's subsequent **`make build`** also produced all three 0.3.0 wheel/sdist
  pairs. No application code changed after the final full regression.
- `PEYKAR_DATA_DIR=.peykar/vcornea-epic uv run peykar import-csv
  experiments/data/rho2015-scratch/control.csv --spec experiments/data/rho2015-scratch/import.json`
  stored two real experimental aggregate control means. Attribution, source XML
  hash, original eight-row means/SD transcription and percent-to-fraction transform
  are retained. The different monolayer/GM-CSF context is explicitly excluded from
  vCornea independent validation; SD remains raw metadata, not an uncertainty model.
- Before service recreation, active-job checks returned zero. SQLite's backup API
  and a separate artifact archive produced
  `backups/pre-vcornea-20261016T205315Z/`, also copied to local ignored backups.
  `docker compose build` and `docker compose up -d --wait` succeeded against the
  unchanged named volume; API healthy and the separate worker running.
- Authenticated live HTTP POST imported the three published count outputs and
  `vcornea.epic.injury600` (201). Output/lineage GETs returned 200, exact raw CRLF
  digests matched, identical reimport preserved lineage, and anonymous reads
  returned 401. Three published Parquet files and the actual injury Parquet were
  admitted by the deployed `peykar vcornea import-thickness` CLI and read through
  HTTP with ten `lattice_y_coordinate` series. The execution manifest/record were
  stored as content-addressed artifacts. Docker's read-only-root filesystem rejected
  `docker cp` into the container; streaming the same local files through `exec` into
  its existing writable `/tmp` succeeded without weakening container settings.
- A new live gap `vcornea.published-replay.evidence-gap.20261016` was registered/read;
  the historical `vcornea.integration.pending` record was preserved unchanged.
- The deployed `scripts/verify_api.py experiments/manifests/decay.json` workflow
  again passed real submission, idempotency, polling, manifest/result hashes and
  events. Run `888a8f16-7891-4c96-8874-d2ff5c8a0dec` succeeded with 11 samples.
- Final SQLite integrity was `ok`: **22 objects, 8 succeeded runs, 24 events**.
  Every object/run/event row from both the original and pre-E1 backups remained
  byte-identical; all **25** live artifact hashes passed. All **24** existing license
  files, all **27** pinned model source files and the original Persian blueprint
  checksum remained unchanged.
- A final local link/JSON/package inspection passed for **42** Markdown documents,
  generated schema JSON and the new receipt/data descriptors. All three host-built
  wheels retain byte-identical LICENSE text; the platform wheel includes native
  acquisition/runtime/import/CLI modules.

Local detailed logs are retained under ignored `.peykar/external/`, notably
`final-epic-check.log`, `epic60/`, `injury600/`, `live-native-api.json` and
`final-epic-preservation.json`. Neither bounded exact repeats nor selected published
simulation imports establish full-horizon reproduction, independent biological
validation, hostile-code isolation or a general queued vCornea adapter.

## Earlier engineering continuation: 2026-10-16 (unreleased, on 0.3.0)

Historical observations below precede E1; the native runtime/source permissions and
output-import status are superseded by the verified E1 record above.

Observed on this macOS checkout and Docker Desktop's Linux ARM64 runtime:

- `uv sync --frozen --all-packages --group dev` succeeded on the host using
  Python 3.12.2. The lockfile was not regenerated.
- Final `make check-docker` passed. It runs `make check` and `make build` in a
  separate Linux/Python 3.12.14 verification container without the live data
  volume or test-time network. Ruff lint/format passed, strict Mypy checked
  **54 source files**, **184 tests passed**, and observed statement coverage was
  **90%**. Generated contracts/examples/OpenAPI were current.
- Targeted checks preceded the complete regression. New coverage includes runtime
  lock/platform drift, result seed/grid/unit/port mismatches, cancellation/recovery
  ownership loss, offline artifact bounds/digests, explicit scalar mapping,
  equivalent physical time units, seed cohorts, concurrent cross-import provenance
  conflicts, legacy reimports, knowledge-gap immutability, exact model references,
  filters before pagination and API reader/admin roles.
- The first Docker-check attempt stopped before tests because `uv run` attempted
  to reconcile noneditable wheel installs into editable installs without a cached
  build dependency. The verification stage now sets `UV_NO_SYNC=1` and
  `UV_OFFLINE=1`; the complete final offline run above passed.
- `make build` also succeeded on the host: all three 0.3.0 wheels and source
  distributions are under `dist/`. Direct wheel inspection found the new modules,
  the current license expression and byte-identical LICENSE text in each wheel.
  This is a working-tree continuation, not a claim of a newly published release.
- `docker compose up -d --build --wait` rebuilt/recreated API and worker against
  the existing named volume. No queued/running jobs existed before recreation.
  The API became healthy; the separate worker subsequently completed real jobs.
  Previous image tags and SQLite/artifact backups were preserved for recovery.
- `scripts/verify_api.py` exercised authenticated live submission, idempotent
  resubmission, polling, manifest/result checksum verification and lifecycle events
  for both reference engines. Final deployed runs were
  `a34e51a5-dba3-484d-a1a2-5c0825fe1980` (decay) and
  `d3767d46-5c03-4842-a241-01439136b1e6` (logistic), both succeeded with 11 samples
  and a nonempty pinned lock digest.
- Authenticated live HTTP checks registered/retrieved/filtered
  `vcornea.integration.pending`; anonymous reads returned 401. The gap explicitly
  records missing integration evidence and does not claim that vCornea ran.
- Live endpoint evaluation of the decay run returned `passed: true`,
  RMSE **1.9229626863835638e-16 mole/liter**, and
  `evidence_class: synthetic_verification`. Retrieval of the run-bound record
  `report.1bd416795e14e6888ddd9ff85db18b30bb870c43f48ad1603cc725bca83059e9`
  succeeded. This is synthetic numerical verification, not biological validation.
- SQLite backup used its backup API while services were active; exact artifacts
  were archived separately. A copy of the initial backup is in ignored
  `.peykar/backups/`. Final comparison retained every original object/run/event
  row; `PRAGMA integrity_check` returned `ok`. There were 7 succeeded runs,
  5 objects and 21 events after retained verification work; every result checksum
  was readable and verified. Existing user rows were not reset or rewritten.
- All **24** license/notice/historical-license files checked against the local
  baseline remained byte-identical. Existing scientific schema/model exports were
  unchanged; only new contract files and the expanded OpenAPI were added/updated.
- The supplied full Persian architecture was archived byte for byte as
  `blueprints/universal-biological-platform-v3.fa.md`, SHA-256
  `5a65971deb58b186ae66bb6c7899f5585b35da9410304f048e5a23f0aa9d3e6d`.
  Its coverage document maps all 55 sections and separately retains the 24
  original biological architecture scenarios; neither table claims those future
  scientific scenarios passed software tests.
- A local Markdown link/count check passed for 23 current documents, all 55
  ordered source sections and all 24 ordered original scenarios. The archived
  source checksum still matched after the design/documentation updates.
- Read-only source-permission rechecks: GitHub's repository-license API returned
  404; the Zenodo web/API record could not be retrieved (web-tool failure/TLS
  timeout). Permission remains unverified. No external model code/archive was
  downloaded, installed or executed.

Child instrumentation is still excluded from the minimal child environment.
Coverage is the observed parent/test statement measurement. Docker testing here
does not validate an untrusted-code sandbox, multi-host deployment, Python 3.13,
the dashboard or any published biological reproduction. Remaining gates are in
`roadmap.md` and `status.md`; no independent approval or clinical validity is claimed.

## Earlier edition: 0.3.0

Verified locally on 2026-10-15 with Python 3.12.14 on Linux x86_64:

- `make check`: Ruff lint/format, strict Mypy (46 source files), all **114 tests**,
  and generated schema/example checks passed. Observed statement coverage: **91%**.
- New tests exercise hung-child termination/reaping, cancellation without engine
  cooperation, secret-free child environment, output bounds, invalid child results,
  bounded heartbeat lock waits, strict CSV failures, atomic rollback, frozen split
  leakage checks, immutable tolerance, run-specific report identity, API roles and CLI.
- `make build`: all three 0.3.0 wheels and source distributions built successfully.
- The three wheels installed into a fresh separate venv using locked runtime
  dependencies. The installed demo and complete CSV/freeze/submit/worker/evaluate/
  report workflow passed outside the checkout. The final rebuilt platform wheel
  was reinstalled and that workflow passed again.
- Analytical demo RMSE remained approximately 1.923e-16 mole/liter across 11 samples.
  This is synthetic numerical verification, not biological validation.
- Wheel metadata reports `LicenseRef-Peykar-Noncommercial-1.1.0`; all packages
  include current notices and historical 1.0.0/Apache texts and baseline checksums.
  Root and package licensing copies match byte for byte.
- Existing 0.2.0 model descriptors, existing ModelPackage/DatasetSpec/ValidationReport
  schemas, reference engine source and the 0.1.0 baseline record remain unchanged.
  Current licensing metadata receives new 0.3.0 reference descriptors.
- Relative links in top-level and docs Markdown resolve. Upstream vCornea branch
  and complete tree metadata were inspected read-only and recorded with an exact
  commit; its runtime and biological results were not executed.

Child processes run in integration tests, but their statement instrumentation is
not inherited into the minimal child environment. Coverage is the observed pytest
measurement, not proof of isolation, scientific validity or legal enforceability.

## Historical edition: 0.2.0

Verified locally on 2026-10-15 with Python 3.12.14 on Linux x86_64:

- `make check`: Ruff lint/format, strict Mypy, all **92 tests**, and generated
  schema/example checks passed. The observed current statement coverage was **90%**.
- `make build`: all three 0.2.0 wheels and source distributions built successfully.
- Built-wheel metadata for every package reports
  `LicenseRef-Peykar-Noncommercial-1.0.0`. Each wheel includes LICENSE, NOTICE,
  LICENSING.md, the historical Apache text and the baseline checksum record.
- The root and all three package copies of the licensing documents match byte for byte.
- Reference model/data metadata uses version 0.2.0 and the current license reference;
  the unchanged reference algorithms retain engine version 0.1.0. Historical
  scientific versions were not overwritten in place.
- Installing the three 0.2.0 wheels into the separate verification environment and
  running `peykar demo` outside the checkout succeeded, with model version 0.2.0
  and the same analytical RMSE approximately 1.923e-16 mole/liter.
- Relative Markdown links in the top-level and principal design documents resolve.

These checks verify packaging and engineering behavior, **not legal enforceability**.
The custom license draft has not been reviewed by a lawyer or tested in court.
No public GitHub release or third-party distribution is established by this record.

## Historical foundation: 0.1.0

Snapshot: Peykar 0.1.0. Verified locally on 2026-10-15 with Python **3.12.14** on
Linux x86_64. This is an observed engineering check, not biological validation.

| Check actually run | Observed result |
|---|---|
| `uv sync --frozen --all-packages --group dev` | Workspace and locked environment resolved |
| `uv run ruff check .` | Passed |
| `uv run ruff format --check .` | Passed |
| `uv run mypy` | Passed, 42 application/core/engine source files |
| `uv run pytest --cov --cov-report=term-missing` | **92 tests passed; 92% statement coverage** |
| `uv run python scripts/export_contracts.py --check` | Generated contracts and examples are current |
| `uv build --all-packages` | Three wheels and three source distributions built |
| Install the three built wheels into a separate fresh venv | Passed |
| Run installed `peykar demo` outside the source checkout | Passed |
| Compare installed engine source with checkout | Identical bytes |
| Verify installed SQL migration is packaged | Present and used successfully by the fresh demo |

The analytical decay demo compared 11 samples against `exp(-0.1*t)` and reported
RMSE approximately **1.923e-16 mole/liter**, below its **1e-12** verification tolerance.
This is synthetic numerical verification, not independent experimental validation.

Tests cover contracts, units, source-independent analytical comparison, stochastic
seed reproduction, time-aligned interventions, model immutability, dependency
references, scenario parents, concurrent queue claims, idempotency, cancellation
races, stale-worker fencing, redacted failures, corrupted data, API roles/body
limits, generated schemas, calibration split constraints and the HomBon facade.

## Checks not claimed by the historical 2026-10-15 checks

- Python 3.13 is configured in CI but was not executed locally.
- GitHub Actions jobs have not been observed running remotely.
- Docker/Compose were not executed in that earlier environment. The 2026-10-16
  continuation above records subsequent local Docker build/runtime verification.
- CompuCell3D/vCornea and specialist engines were not installed or reproduced.
- No independent biological dataset, clinical claim, generic co-simulation runtime,
  dashboard, GPU, HPC or multi-host deployment was validated.

Coverage describes which statements were executed; it is not a scientific trust
score and does not prove every branch or operational condition is covered.
