# ADR 0002: An executable local profile before distributed infrastructure

Status: accepted. Date: 2026-10-15.

Use SQLite WAL and local content-addressed artifacts for reproducible development.
Claims use transactions and fencing tokens. Stale work fails explicitly. Heavy
distributed execution will use PostgreSQL/object storage, not a shared SQLite file.

Consequence: the foundation runs without Docker, AWS or a service account. This
profile does not provide multi-tenant isolation or a distributed scheduler.
