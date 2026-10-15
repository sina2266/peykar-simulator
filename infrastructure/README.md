# Infrastructure profile

Executable local adapters: peykar.infrastructure.database and .artifacts.
SQL migration 001 ships in the platform package. Local SQLite WAL is single-host.
Future distributed profile: PostgreSQL, S3-compatible artifacts, durable queue/outbox,
OpenTelemetry and optional SLURM. No Terraform/cloud account is provisioned here.
See docs/operations.md for concurrency, recovery, backup and migration boundaries.
