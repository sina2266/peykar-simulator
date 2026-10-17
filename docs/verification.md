# Verification record

## E3 native/reference scalar DAGs: 2026-10-17 local

[Observed hybrid-flow evidence](hybrid-flow-verification-2026-10-17.json) retains
actual node/transfer/result identities and numerical outputs.

- Targeted hybrid/cohort/native/workflow checks passed **95 tests**; after adding
  source-integrity/failure/fanout gates, **26 E3 tests** passed. Tests cover exact
  unit transfer, byte-preserving XML edits including BOM/comments, immutable parents,
  atomic derived registry/receipt/child rollback, concurrency/capacity, restart,
  cancellation/late success, cycles/duplicate writers, semantics/context, unsupported
  amount/assigned/event inputs, corrupted source manifests and API roles/idempotency.
- Final **`make check-docker` passed: 366 tests in 24.61 seconds, 88% coverage**,
  strict Mypy **80** source files, Ruff lint/format, current schemas/OpenAPI and all
  three wheel/sdist pairs. **`make build` passed** on the host; the platform wheel
  includes `005_hybrid_flows.sql` and hybrid modules. Earlier schema JSON versions
  and `uv.lock` are unchanged. An earlier Docker check stopped at lint before tests;
  the corrected final command above completed successfully.
- SQLite backup API/artifact copy created
  `backups/pre-hybrid-flows-20261017T103604Z/`, also copied locally. All existing
  queues were idle before `docker compose build` / `docker compose up -d --no-build`.
  API/ordinary Worker use the same original volume; native host image/driver pins
  and the absence of API Docker access remain unchanged.
- `uv run python scripts/verify_hybrid_flows.py --output
  .peykar/e3-live-verification` passed against the deployed services and real COPASI
  containers. It admitted the interface/reference descriptors through HTTP, froze
  both flows idempotently, executed dependencies and verified transfer/derivation
  hashes and unchanged original model bytes.
- Three-stage COPASI → reference → COPASI flow
  `95649676-962b-454f-b3de-98096e19deb8` returned terminal A
  **0.04978705189575012** against analytical `exp(-3)` = **0.049787068367863944**,
  absolute error **1.6472114e-8**. Units passed through explicit millimolar,
  micromolar and molar declarations. The downstream native model was admitted as
  a separate derivative with original license/context and parent/upstream lineage.
- Direct native → native flow `823eb118-fc25-4416-be6e-67d032fb79cd` returned
  **0.13533523846075707** against `exp(-2)` = **0.1353352832366127**,
  absolute error **4.4775856e-8**. These are authored synthetic numerical checks.
- Running native-source cancellation in flow
  `817d3c6d-4474-4a17-bb6e-a5c34b7494f2` ended cancelled and skipped both remaining
  nodes. Actual source timeout in `fe0d823b-ce94-43ab-ba30-8cd2450f1e99` ended failed
  and skipped downstream nodes. No native container remained; all queues were idle.
- Preservation comparisons retained every earlier **65 object versions, 40 ordinary
  runs/120 events, 2 experiments/4 nodes/8 events, 15 native runs/47 events and
  7 batches/38 members/57 events**. All **367** previous artifact byte sequences
  and **24** original license hashes match. SQLite `user_version` remains **1**.

E3 establishes the restricted sequential scalar initial-condition workflow. It does
not close general native mappings, PKML inputs, shared-time feedback, checkpoint
continuation, independent biological bridges or full UI gates.

## E2 cohorts, parameter sweeps and terminal statistics: 2026-10-17 local

[Actual cohort evidence](cohort-verification-2026-10-17.json) retains exact member,
manifest/result hashes, sample statistics and preservation counts.

- Targeted cohort/native/workflow checks passed **70 tests** before the final
  cleanup fix. After adding creation/cancellation and persistent-loop regressions,
  targeted cohort/native checks passed **60 tests**.
- Final **`make check-docker` passed: 340 tests in 22.04 seconds, 88% coverage**,
  strict Mypy **75** source files, Ruff lint/format, current schemas/OpenAPI and all
  three package wheel/sdist pairs. Final host **`make build` passed**. The platform
  wheel includes batch modules and `004_batches.sql`. Earlier versioned schema
  JSON and `uv.lock` are unchanged; new batch contracts have their own schemas.
- Added admission tests cover Cartesian grids, physically equivalent duplicate
  values, parameter units/ranges, deterministic seed allocation/overflow, job/sample/
  wall/snapshot-size limits and unsupported native seeds/parameter overrides.
  Lifecycle tests cover concurrent capacity, atomic rollback in both queues,
  restart, cancel/late-success fencing, continue/fail-fast, role enforcement,
  idempotency and immutable versions. Reports check stochastic replay, n-1 sample
  SD, missing units, tampered lineage and explicit incomplete/excluded members.
- SQLite backup API and artifact copy made
  `backups/pre-cohorts-20261017T091920Z/`, also copied locally. All existing queue
  families were idle. `docker compose build` and `docker compose up -d --no-build`
  deployed API/ordinary Worker on the original volume; no Docker socket was added.
- Live command `uv run python scripts/verify_batches.py --output
  .peykar/cohort-live-final-verification` passed with the real ordinary Worker and
  installed native host agent. Cohort `13402df3-3a2e-41e0-b373-42397bb252fc` ran
  **12** actual seeded parameter-grid jobs. Report statistics matched downloaded
  child values. Cohort `bfc913b8-0903-4b90-87f3-dcc13b6066e1` completed reference
  decay, actual COPASI SBML and original Aciclovir PKML with native unit metadata.
- Cancelling cohort `ea6fc927-5ded-4a4f-9aca-b69e23afbb26` during its first native
  member cancelled that member and skipped both unstarted members. Terminal report
  groups remained explicitly incomplete, with no invented measurements.
- The initial cancellation exercise exposed a real Docker create/removal race:
  cancelled native job `6543f3bb-9144-4655-a975-d33137e39832` left a container in
  `Created`. Its mounts identified the cancelled job; it was inspected and removed.
  The corrected agent acknowledges bounded creation before a separately supervised
  start, checks cancellation around creation and stops the attach client before
  final removal. The repeated actual cohort checks above left **no native container**.
- Loop mode now continues after redacted failed/cancelled attempts without retrying
  terminal jobs. Following the real cancellation, the same loop consumed native
  job `e7c4e79d-ed31-4f28-b607-0588532095cd` successfully. Creation uses a separate
  15-second RPC bound; the solver wall budget starts after creation. An unreachable
  daemon/uncertain cleanup remains an explicit retained operator warning.
- An additional actual deadline test created cohort
  `3d3224e0-456c-4f46-b30a-83e5229d79d2`: its first child failed under a 0.001-second
  budget, its independent next child succeeded, and overall status was failed.
  The failed group had no endpoint statistic; the successful group stayed complete.
- Final preservation comparisons retained all previous **61 objects, 12 ordinary
  runs/36 events, 2 experiments/4 nodes/8 events, 8 native runs/24 events**. All
  **349** earlier artifact byte sequences and **24** original license hashes match.
  SQLite `user_version` remains **1**. All queues were idle after verification.

This closes the bounded E2/S4.5 engineering workflow. Native parameter/seed control,
native DAG bindings, other engine adapters, sensitivity/identifiability, uncertainty
by source, full UI and independent biological evidence remain separate open tasks.

## Declarative SBML/PKML and native agent: 2026-10-17 local

[Actual retained outcomes](native-agent-verification-2026-10-17.json) include exact
live run/result/image/driver/artifact identities and preservation counts. This is
engineering verification of two native drivers, not completion of the platform.

- `uv run pytest -q tests/integration/test_native_jobs.py`: **32 passed** after the
  real-output correction. Covers immutable raw XML/units, hostile XML, model/profile
  compatibility, idempotency/concurrent claims, additive migration preservation,
  stale leases, cancellation, numerical envelopes, API roles/claim tokens, local
  image/driver allowlists, output symlinks and supervised cancellation/timeout.
- Final **`make check-docker` passed**: Ruff lint/format, strict Mypy **71** source
  files, **312 tests in 16.93 seconds**, **87%** statement coverage, current generated
  schemas/OpenAPI and all three wheel/sdist pairs. `make build` also passed on the
  host. The platform wheel includes `003_native_runs.sql` and native modules.
  Earlier immutable schema JSON files and the workspace uv lock were not changed.
- `scripts/verify_native_agent.py` passed against isolated API port 8012 for actual
  COPASI SBML and original OSPSuite PKML, including minute-to-second conversion.
  The temporary development server was stopped after verification.
- Before deployment, SQLite's backup API plus artifact copy created
  `backups/pre-native-agent-20261017T082908Z/`, copied to local `.peykar/backups/`.
  Ordinary/experiment queues were idle. `docker compose build` and
  `docker compose up -d --no-build` rebuilt/recreated API and Worker on the same
  `peykar3_peykar-data` volume, without adding Docker socket access.
- Offline `peykar native register-profile` admitted both reviewed profiles; offline
  `register-model` admitted original **7,428,313-byte** Aciclovir PKML from the
  vendored OSPSuite-R source. Its exact SHA-256 is retained in the model metadata.
  The existing 2 MB HTTP limit remains; declarative XML admission is capped at 10 MiB.
- The first live PKML attempt correctly failed publication because the validator
  mistook native `IndividualId` for a physical quantity. Run
  `f424b790-ba29-4e28-b85d-0e6e3daa089b` remains failed. The fix retains that original
  integer identity without assigning a unit and rejects mixed/noninteger identities;
  native quantity metadata must match every other column. Actual output replay,
  targeted tests and the final full regression above passed after this correction.
- Live command `uv run python scripts/verify_native_agent.py --output
  .peykar/native-agent-live-final-verification --pkml-model-id native.aciclovir`
  passed: HTTP SBML admission, idempotent native jobs, real Docker solvers, downloaded
  artifact hashes, event order and queued cancellation. COPASI run
  `a80fb520-715c-4604-9969-213bc1af526b` returned **11** points over **one minute**,
  max analytical error **1.5673282e-7**, conservation error **3.3306691e-16**.
  Original PKML run `e682d2cc-6404-493a-9cda-d725eafb9381` returned **491** samples,
  original **µmol/l** and **minute** metadata, with no schedule override.
- An explicit local failure check used the real installed containers: cancelling
  running PKML `5e7b7029-167d-4199-b255-2e2e9339df2c` ended cancelled; a 0.05-second
  COPASI budget `b558d682-c9f9-47aa-9392-1d1624156587` ended failed with `TimeoutError`.
  Both published no result and left no named native container behind.
- Started `peykar native agent ... --loop` separately on the host. It consumed a new
  HTTP-submitted job without the verifier executing the container itself: run
  `a68cb549-d325-4a2a-b4b4-5ce4d063f316` succeeded and output hashes matched. Anonymous
  native model access returned **401**. Agent startup is separate from Compose and
  needs host lifecycle supervision across restarts; it is not installed as an OS service.
- Existing ordinary Worker verification via `scripts/verify_api.py
  experiments/manifests/decay.json` passed again, run
  `df7b36ae-09d2-469f-a9ce-e77779254991`, **11 samples**, idempotency and hashes checked.
- Post-upgrade comparisons retained every pre-upgrade row: **57 objects, 11 ordinary
  runs, 33 events, 2 experiments, 4 nodes and 8 experiment events**. All **342** prior
  artifact hashes and **24** original license hashes match; SQLite `user_version`
  remains **1**. New records are additive, including the retained failed attempt.

CellML, other native engine queue adapters, native DAG integration, independent
biological validation, checkpoint/resume, feedback coupling and complete UI remain
open in the roadmap. Historical operator-probe evidence was not rewritten to
advertise the new, separately registered execution profiles.

## Permanent sources and installed runtimes: 2026-10-17 local

The [observed summary](native-runtime-verification-2026-10-17.json) and
[source/runtime evidence records](../engine_adapters/runtime-evidence-2026-10-17.json)
retain this continuation independently of earlier observations below.

- Explicit acquisition resolved all original recursive Git links plus nested
  nanobind/robin-map, separate VCell solvers and OSP helper dependencies. Reviewed
  relative HTTPS Git origins are supported. `scripts/vendor_sources.py` materialized
  sources as ordinary project files, preserving notices and validating Git blobs.
  `scripts/verify_vendor.py` passed: **25 source identities, 52,167 files,
  1,799,838,581 bytes, 37 materialized Git links**, zero missing dependencies/LFS.
- The separately pinned native runtime builds/pulls and explicit
  `scripts/probe_native_runtimes.py PROFILE --output NEW_DIRECTORY` executions
  passed for COPASI, NEURON, PhysiCell, Chaste, VCell, GEARS, PhysiBoSS and OSP.
  Original Chaste Delta/Notch vertex/node monolayers both passed. Existing real
  vCornea injury-crossing evidence was retained without rewriting its record.
  Exact image/artifact identities and model limitations are in the linked receipts.
- COPASI initially exposed an actual SWIG output-mask type error; its version/layout
  guarded compatibility bridge passed a real SBML integration (max error 6.09e-8,
  conservation error 2.22e-16). PhysiCell's initial ARM64 build failed on x86-only
  flags; portable C++/OpenMP builds then passed. Chaste's official 0750 home blocked
  the caller UID; a traversal-only wrapper passed original executables unchanged.
- OSP's exact release supports rSharp 1.2.3/.NET 8. The real Aciclovir PKML solver
  produced **491** times and original **µmol/l** metadata; the metadata field was
  corrected to the original API's `metaData` name and the final native run passed.
  Windows graphical applications were not executed in Linux.
- Figshare article 24049200 v3/file 44498681 was downloaded: **253,158,197 ZIP bytes**,
  provider MD5 checked, one complete H5AD member with CRC/SHA checks. The isolated
  AnnData reader reported **37,160 K562 cells / 19,264 genes / 21 conditions** and
  finite values in its bounded 256×512 sample. This was not GEARS training or its
  original published benchmark. The Harvard Dataverse metadata requests returned
  HTTP 403; that failure remains explicit in the connector guide.
- Targeted source/evidence checks passed: **21 tests**, including modified source,
  ignored dependencies, unsafe paths, LFS pointers, partial OMEX failure, timeout
  cleanup, immutable evidence, artifact corruption and API authentication.
- Final **`make check-docker` passed**: Ruff lint/format (**199 copied files**),
  strict Mypy (**65** source files), **280 tests in 15.13 seconds**, **88%** coverage,
  current schemas/OpenAPI and all three wheel/sdist pairs. **`make build` passed**
  on the host. An earlier final-check attempt stopped at two long-line lint errors
  before testing; the corrected final command above completed successfully.
- `package_native_evidence.py` retained **12 complete source receipts**, **11 native
  execution/QC receipts**, **310 artifact blobs**. Identical reimport passed; earlier
  source 1.0.0 receipts remain unchanged and complete inventories use 2.0.0.
  The first operational importer printed a summary-method error after registration;
  database and authenticated API rereads verified all 11 admitted records.
- SQLite backup API and artifact copy made
  `backups/pre-runtimes-20261016T220136Z/`, also copied locally. No jobs were active.
  `docker compose build api worker` and `docker compose up -d api worker` passed
  against the same volume. Every earlier row survived: **34 objects, 10 runs,
  30 run events, 2 experiments, 4 nodes, 8 experiment events**. All **46** earlier
  artifact byte sequences and all **24** original license file hashes match.
- Live authenticated `/v1/runtimes` returned **11** records; the OSP filter returned
  one, anonymous access returned **401**, and both historical and complete NEURON
  source receipts remained readable. Real submit→worker→result passed again with
  idempotency, exact manifest/result hashes and **11 samples**, run
  `f0944977-87c8-4b46-96f3-adac9ad42ae8`.

These observations establish local source/runtime/engineering behavior. Original
paper reproduction, independent biological validity, general native-engine queue
adapters, Windows graphical applications and the complete UI remain open.

## Whole-platform shared backend: 2026-10-16 (unreleased, on 0.3.0)

This continuation broadens delivery beyond the eye-model path. The
[whole-platform roadmap](roadmap.md) covers S0–S13 while retaining original P0–P7
scientific gates. [The observed receipt](system-continuation-2026-10-16.json)
records source counts, live workflow results and preservation checks; it is not a
claim that the entire platform or all native engine integrations are complete.

- `scripts/acquire_ecosystem_sources.py --catalog engine_adapters/ecosystem-sources.json
  --destination .peykar/external/ecosystem/sources --jobs 3` fetched all nine exact
  top-level source snapshots: CompuCell3D, PhysiCell, VCell, COPASI, Chaste, NEURON,
  PK-Sim, MoBi and GEARS. Hooks were disabled; no upstream code/build script was
  executed. Existing vCornea Git and separately licensed archive identities were
  preserved. These depth-one working trees are not complete Git histories.
- Local `LibraryService.inspect` verified **36,291 tracked entries /
  1,308,090,422 bytes**, source commits/modes, symlink targets and license bytes.
  **24 unresolved Git links** and zero top-level LFS pointers were recorded.
  MoBi's Git-declared CRLF checkout initially failed strict blob checking;
  inspection now retains actual-byte SHA and canonical blob identity while allowing
  only declared text line-ending transformations. It does not rewrite source.
- The bundled OSP license PDFs were preserved; `pdftotext` read the MoBi copy's
  GPLv2 plus Clarifying Addendum. License evidence remains component-scoped; no
  third-party code was re-licensed. This is not a legal compatibility determination.
- Reactome `R-HSA-177929` and UniProt `P01133`/`P00533` JSON were retrieved and
  hashed. Their curated human EGF/EGFR graph was imported as evidence-linked
  metadata, with precursor/ligand and tissue-kinetic limitations retained. This
  does not create a calibrated EGF mechanism or a vCornea molecular coupling.
- Targeted new-service checks passed: **17 tests** for graph bounds,
  evidence/context/cycles, source tampering/license/bundle checks, declared CRLF,
  workflow transfer, atomic rollback, concurrent dispatch/cancellation, incompatible
  bindings, failure propagation, persistence and API roles/idempotency. An earlier
  host batch included Linux-only legacy tests; its six bounded-child failures
  were the macOS limitation, and the final Linux run below passed them.
- Final **`make check-docker` passed**: Ruff lint/format (**200 files**), strict Mypy
  (**64 source files**), **259 tests** in **13.90 seconds**, **88%** observed statement
  coverage, current generated contracts/OpenAPI and all three wheel/sdist pairs.
  Git was added only to the verification stage for local source-fixture inspection.
  Test-time network and the live data volume remain absent. Host **`make build`
  passed**; wheel inspection verified new modules, additive SQL migration and
  byte-identical LICENSE text. No application code changed afterward.
- Before recreation, no queued/running user jobs existed. SQLite backup API plus
  a separate artifact tar preserved `backups/pre-system-20261016T211636Z/`, also
  copied under local ignored `.peykar/backups/`. An optional attempt to tag the old
  container image ID failed because Docker's image store did not expose that ID;
  no new rollback tag is claimed. The data backup and earlier recovery tags remain.
  `docker compose build` and `docker compose up -d --wait` succeeded; API healthy.
- Nine offline inspection bundles were imported with the deployed
  `peykar libraries import-receipt` CLI into the existing volume. Live authenticated
  library listing returned nine entries; `capability=pbpk` returned PK-Sim/MoBi;
  anonymous reads returned 401. No Git/scientific runtime dependency was added to
  the API image, and no HTTP source filesystem-import route was exposed.
- Live graph POST/GET and depth-two neighborhood succeeded for
  `biology.human-egfr@1.0.0`, returning three entities/two evidence-linked edges.
- Live HTTP model registration, experiment freezing, idempotent submission,
  separate Worker execution, result/manifest retrieval and events all passed.
  Experiment `3f2b262a-f367-485e-b2d3-c34abcd1d076` succeeded with children
  `2ec78ad7-c272-4acc-a563-3e4b04c7ed34` and
  `ac33758f-7d3e-43c5-b61f-2a933f671e71`. The second initial concentration was
  **0.3678794411714421**, and final was **0.1353352832366125 mole/liter**, matching
  `exp(-1)`/`exp(-2)` within 1e-12. Its runtime lock was nonempty and two child
  dispatch events were retained. This is synthetic engineering verification.
- Live cancellation of experiment `3fa6fdfa-4ebf-43f1-9328-a60384a06291` reached
  `cancelled` with both unsubmitted nodes skipped. Atomic dispatch left
  **zero unassociated workflow child jobs**.
- Final integrity/foreign-key checks were `ok`: **34 objects, 10 succeeded ordinary
  runs, 30 ordinary events**, two experiment runs (one succeeded, one cancelled),
  four node records and eight experiment events. Every pre-upgrade
  object/run/event row remained unchanged; all **46** live artifact hashes passed.
  All **24** original license files and **27** pinned vCornea files remained
  unchanged. New contracts/routes and additive experiment tables were introduced
  without changing existing schema/model versions or SQLite user_version 1.

Detailed ignored logs are under `.peykar/external/ecosystem/`: `inspection.log`,
`final-check.log`, `live-platform.log`, `preservation.json` and source/research
receipts. General feedback/spatial coupling, new native runtime builds, remaining
source dependencies, independent evaluator/UQ, full UI and distributed/tenant
deployment remain open in the comprehensive roadmap.

The final documentation/schema check also passed: local links across **45** current
Markdown files resolve, generated contracts remain current, source descriptors and
the portable receipt parse, all 55 source sections remain represented, and the
original Persian blueprint checksum is unchanged.

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
