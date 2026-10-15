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

Update this context when these operational facts change; record only observed
commands/outcomes in `docs/verification.md`.
