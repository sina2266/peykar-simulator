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
- Use the frozen uv lock. Linux child integration requires Docker on this macOS
  host. `make check-docker` runs the complete checks/build without live data.
- Offline artifact admission, mapped scalar output import and repeated-seed
  comparisons do not authorize remote model execution or biological claims.
  P1 still requires permission verification, the actual 4.7.0 runtime and a
  reproduced published reference before advancing its scientific exit gate.
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
