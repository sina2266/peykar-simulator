# Verification record

## Engineering continuation: 2026-10-16 (unreleased, on 0.3.0)

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
