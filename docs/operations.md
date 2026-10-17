# Local operations

## Persistence

`PEYKAR_DATA_DIR` defaults to `.peykar`. The API, CLI and worker must point to the
same directory on the same host. State contains `peykar.sqlite3`, its WAL/SHM files,
and an `artifacts/` tree addressed by SHA-256. The schema is shipped in the platform
wheel. Instantiating the service creates/migrates the local database; importing its
module does not create state.

Back up the database using SQLite's backup API while active, or stop writers and
copy the complete state directory. Copying only the SQLite file during WAL writes
can omit committed data. Include referenced artifacts in backups. Restore to an
isolated directory and verify checksums before resuming use. No backup service is
configured in this release.

## Queue lifecycle

`queued -> running -> succeeded | failed | cancelled`; queued cancellation goes
directly to cancelled. Running cancellation sets a request flag. The worker checks
that flag from the parent process while the child executes and before committing results. Cancellation
wins a concurrent success commit when its transaction committed first.

A claim token prevents a recovered/stale worker from overwriting terminal state.
The worker does not retry automatically. Use `peykar recover --age-seconds 300` to
fail runs that stopped heartbeating, then submit a new run for explicit retry.
Choose an age above the parent heartbeat interval, allowing for host load.
An idempotency key returns the same existing run for the same plan, including a
failed/cancelled run. Use a new key when an intentional new run is required.

Before execution, the worker verifies manifest/model identity and the pinned
platform, Python, engine bytes and dependency versions. The runtime lock is resolved
beside the virtual environment (`sys.prefix/../uv.lock`), independently of cwd;
Docker includes that file in the runtime image. A pinned lock must still exist and
match. Older installed manifests without a lock digest remain executable after the
other checks; an absent lock is never presented as a verified lock. Standalone
installations should place the intended lock beside their virtual environment.

Before result publication, the worker revalidates the result contract, compares
the seed and sample times with the plan, checks units and declared scalar output
ports, and checks cancellation/ownership again. Equivalent physical time units
are accepted. Mismatches fail with `integrity_failure`; environment drift fails
with `execution_constraint`. These checks do not establish biological correctness.

The default Linux worker starts a fresh Python child for each run. The parent
enforces the wall deadline, polls cancellation/ownership about every 50 ms and kills
the child's process group on completion or failure. Linux limits apply before
heavy engine imports: 1 GiB address space, CPU seconds rounded up from the wall
budget, 32 MiB output-file size, 64 file descriptors and no core dumps. Startup time
counts toward the wall budget; small budgets may expire before an engine starts.
Host scheduling affects observed cancellation/deadline latency. Heartbeats wait at
most 50 ms for a SQLite writer lock; longer contention fails the run and kills the
child instead of blocking the resource supervisor. Terminal-state publication may
still wait for the database writer lock after the child has been stopped.

The child has a temporary cwd, no inherited API tokens and a minimal environment.
It runs installed builtins by default; an operator may pass an explicit installed
plugin allowlist to `SubprocessExecutor`. No HTTP field enables code loading.
This controls trusted engine lifecycle and resources. It does **not** isolate the
filesystem, network or user identity, prohibit process creation, or make hostile
code safe. Do not accept untrusted plugins. A container/namespace/seccomp profile
and independent security review are required for that threat model.

`InProcessExecutor` is an explicit Python-only cooperative option for trusted
integrations/tests. It lacks child process limits. The default bounded worker
currently requires Linux; it fails explicitly elsewhere. No checkpoint/resume,
native restart is supplied. A separate explicitly allowlisted host agent now
executes COPASI SBML and OSPSuite PKML; see [native operations](native-agent.md).

## API security

`PEYKAR_API_TOKEN` is mandatory and must be at least 24 characters. Use a random
secret, not a memorable password. Optional `PEYKAR_READ_TOKEN` must be different.
Static tokens are a local single-workspace profile, not multi-tenant authentication.
The API binds to 127.0.0.1 by default. Use TLS, a secret manager, OIDC/OAuth scopes,
tenant ownership, quotas and rate limiting before an Internet or shared deployment.

The public endpoints are health and generated documentation/schema. Data routes
require a reader/admin token; mutation routes require admin. No untrusted code,
shell command, network URL or model archive is executed through the HTTP API.
Request bytes are limited to 2 MB, including chunked bodies. Large dataset ingestion
will need a separate validated object-store upload workflow.

CLI users already have local filesystem/operator privileges. CLI model registration
is not a cryptographic approval service. Public error codes omit engine exception
details; operator logs can contain paths and need their own access/retention policy.

## Containers

```bash
docker compose --env-file .env up --build
```

Create `.env` from `.env.example` and replace the token field first. API and worker
share a local named volume. Compose binds HTTP to loopback, drops capabilities,
uses a non-root user and provides memory/PID limits. Containers do not make
arbitrary imported scientific code safe; that needs a separately reviewed execution
profile. Verify Docker/Compose on the target host before relying on these settings.

`make check-docker` builds the `verification` Docker stage and runs `make check`
and `make build` on Linux using the locked development dependencies. It does not
attach the live volume and test execution has no network. Rebuild API and worker
together after changes with `docker compose up -d --build`; preserve the named
volume. Do not use `down -v` as an upgrade step. Check for active runs before
recreating a worker; use explicit stale recovery if an interrupted worker left one.

For live verification use `scripts/verify_api.py` with an existing admin token and
registered reference model, as shown in README. The command prints the new run ID
immediately, so a timeout can be investigated without resubmitting. A timeout does
not cancel or remove the run. Credentials are read only from the environment and
are never forwarded through HTTP redirects.

## Multi-host migration

SQLite WAL is a single-host profile. Do not mount it across a distributed worker
fleet. Add a PostgreSQL registry/queue implementation with transactional claims,
leases/fencing and a transactional outbox. Add an S3-compatible artifact adapter
with checksum verification and atomic object publication. Preserve model IDs,
schema versions and manifest hashes during migration. Add tracing and metrics
through OpenTelemetry without putting scientific data or credentials in labels.

Monitor queue depth, queue age, worker heartbeats, failure categories, execution
time, artifact integrity, disk use and cancellation latency. Alerting and an
OpenTelemetry exporter are future integrations, not enabled services.
