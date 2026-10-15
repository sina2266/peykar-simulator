# Verification record

## Current edition: 0.3.0

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

## Checks not claimed

- Python 3.13 is configured in CI but was not executed locally.
- GitHub Actions jobs have not been observed running remotely.
- Docker/Compose build and runtime were not executed; Docker is not installed here.
- CompuCell3D/vCornea and specialist engines were not installed or reproduced.
- No independent biological dataset, clinical claim, generic co-simulation runtime,
  dashboard, GPU, HPC or multi-host deployment was validated.

Coverage describes which statements were executed; it is not a scientific trust
score and does not prove every branch or operational condition is covered.
