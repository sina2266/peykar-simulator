# Simulation

Immutable manifests, planning and a Linux worker with bounded trusted-engine child
processes. Explicit cooperative execution is available for operator integrations.
No untrusted-code sandbox, native checkpoint/resume or general multirate master yet.

`native_jobs.py` adds a separate durable native queue with frozen model/profile/
runtime snapshots, fenced 30-second leases, cancellation and verified numerical
artifacts. `native_outputs.py` validates finite grids and native quantity metadata.
`native_agent.py` runs on an explicitly authorized Docker operator host, with a
local immutable image/driver allowlist; the API/ordinary Worker has no Docker
access. Implemented kinds are COPASI SBML and OSPSuite PKML. See docs/native-agent.md.

See docs/architecture.md, docs/status.md and docs/scientific-contracts.md at repository root.
