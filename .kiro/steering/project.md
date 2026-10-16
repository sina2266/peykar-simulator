# Peykar continuation context

No `.kiro` directory existed in the delivered checkout. Added on 2026-10-16 to
retain the requested project context. Root `AGENTS.md`, `README.md`,
`docs/status.md` and `docs/roadmap.md` remain the authoritative rules/inventory.

- The checkout has no Git metadata. Preserve existing files, license notices and
  immutable schema/model versions; do not assume a clean Git reset is available.
- Local Docker Compose API and worker share the `peykar3_peykar-data` volume.
  Never reset it to test or upgrade; check active jobs before recreating workers.
- Back up active SQLite with its backup API, and preserve referenced artifacts.
  Continuation backups live under the volume's `backups/` and local ignored
  `.peykar/backups/`; do not put credentials or scientific data in documentation.
  E1's pre-deployment backup is `backups/pre-vcornea-20261016T205315Z/` with a local
  copy; final verification retained all rows from that and the original backup.
  API/worker were rebuilt against the same volume and the live submission/result
  smoke test passed. Native output imports are additive to existing user records.
- Use the frozen uv lock. Linux child integration requires Docker on this macOS
  host. `make check-docker` runs the complete checks/build without live data.
- The user explicitly authorized reviewed external model/source/dataset acquisition
  and execution. EPIC E1 admits exact Zenodo record 16884414 under its CC-BY-4.0
  declaration, extracts only 27 pinned files (no .git), builds a separate locked
  CompuCell3D 4.7.0 Linux/AMD64 image and runs private seeded/horizon-controlled copies.
  It never fetches/executes code through the management API or general worker queue.
- E1 has real native CSV/Parquet imports, six published reference files, a frozen
  seed-7/11 repeat comparison and a successful 600-MCS injury-crossing run. Raw Time
  is already hours; height stays lattice_y_coordinate. Archived seeds/per-run
  parameters are unknown. P1 full published reproduction and independent biological
  validation remain open. Read docs/epics/E1-vcornea-native-runtime.md before extension.
- The 6.75 GB Data.zip was acquired only as selected complete ZIP members with
  size/CRC/local SHA checks. Whole-archive MD5 is unverified. The separate partial
  bulk download is preserved but stopped; do not restart automatically or call it complete.
- Rho2015 scratch-control experimental aggregate data is attributed and ingested in
  isolated .peykar/vcornea-epic state. Its monolayer/GM-CSF assay is not vCornea tissue
  validation. COPASI/BioModels candidates remain deferred; EGF already exists in vCornea.
- The complete original Persian v3.0 blueprint is preserved verbatim under
  `docs/blueprints/`; `docs/blueprint-v3-coverage.md` maps all 55 sections and the
  original 24 biological scenarios. Preserve its ten intervention categories and
  entity/model/instance distinctions in future design work. These target scenarios
  are separate from the executable software failure matrix in `docs/scenarios.md`.
- Persistent knowledge gaps are immutable manual declarations, not approvals,
  automatic resolution or scheduler enforcement. Scientific claims, conflicts and
  independent L0-L3 authority remain separate design requirements.

Update this context when these operational facts change; record only observed
commands/outcomes in `docs/verification.md`.
