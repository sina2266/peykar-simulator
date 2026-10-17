# Native models and external execution agent

The platform now admits declarative SBML/PKML and manages their actual execution
through a separate operator agent. Two reviewed drivers are implemented:
`copasi.sbml@1.0.0` and `osp.pkml@1.0.0`. Other native runtime probes remain
operator workflows; they do not gain queue support from this change. CellML,
general native composition, checkpoints, feedback coupling and the frontend remain open.
[Hybrid scalar flows](epics/E3-native-scalar-flows.md) now support explicit
COPASI/reference dependencies through immutable initial-concentration derivatives.

## Admission and execution identities

`NativeModelSpec` includes context, source, license and limitations. Admission
retains the original UTF-8 XML bytes and SHA-256, including line endings/BOM. It
rejects DTD/entities/XInclude, unsupported roots and external SBML composition.
SBML core Level 2/3 inspection records compartment/species/parameter unit identifiers;
absent unit attributes remain absent. Inspection does not establish solver support
or biological validity. PKML retains its original schedule and unit declarations;
the real OSPSuite solver reports quantity dimensions and units at execution.

Model admission never starts an engine. The local operator separately registers an
immutable `NativeExecutionProfile` referring to an existing `NativeRuntimeEvidence`
and an exact reviewed driver digest. Historical runtime receipts retain
`scope=operator_native_probe` and `managed_queue_supported=false`: those fields
describe that earlier probe. Discover the new queue capability through
`/v1/native-profiles`, with actual availability checked against the agent's local
catalog and installed image before execution. A registry entry alone does not
prove that an agent is running on another host.

Each job freezes its exact model, profile, runtime and physical controls in a
`NativeRunManifest`. A local catalog independently allowlists profile/version,
driver kind/hash and image hash. The server cannot select a new Docker image,
shell command or driver path. The agent checks raw model and driver bytes, requires
the exact image to exist locally and uses `--pull never`.

## Run explicitly

First install/review the separate [native runtimes](../runtime_profiles/README.md)
and admit their source/runtime evidence. Use the same state directory as the API
for these offline operator commands:

```sh
uv run peykar native register-profile runtime_profiles/native_adapters/copasi-profile.json
uv run peykar native register-profile runtime_profiles/native_adapters/osp-profile.json
uv run peykar native register-model model_packages/native/transfer/1.0.0/spec.json \
  --model model_packages/native/transfer/1.0.0/model.sbml
uv run peykar native submit model_packages/native/transfer/1.0.0/plan.json \
  --idempotency-key my-native-transfer
```

For Compose, the state is inside the existing named volume. Run admission with an
operator container sharing that volume and read-only input mounts; the host's
default `.peykar` is a different state directory. Do not reset/copy over the volume.
HTTP admission accepts `{spec, xml_text}` at `POST /v1/native-models`. The existing
2 MB HTTP body limit is preserved. Larger XML, including the original 7.1 MB
Aciclovir PKML, uses offline admission (maximum 10 MiB), followed by authenticated
read/download and normal API job submission. No ZIP, Python model or executable
is accepted as a native declarative model.

On the non-root Docker operator host, export the existing administrator token
privately and run:

```sh
uv run peykar native agent \
  --catalog runtime_profiles/native_adapters/local-catalog.json \
  --output .peykar/native-agent-runs --base-url http://127.0.0.1:8000 --loop
```

Without `--loop`, it processes at most one job. The catalog contains installation
specific image IDs; a rebuild requires explicit review, new evidence/profile
versions where identities change, and an updated local catalog. No command above
installs code automatically. API and ordinary Worker containers have no Docker
socket. The host agent is a separate process and needs lifecycle supervision for
continuous availability; Compose does not start it.

The COPASI plan uses an explicit positive physical duration and 1–10,000 intervals.
Equivalent units are converted into the model's native time unit; no kinetic rate
is rewritten. PKML rejects duration/interval overrides and executes the original
model output schedule. Wall budget defaults to 180 seconds, capped at 600 seconds.

## Lifecycle, limits and recovery

The additive `003_native_runs.sql` creates dedicated queue/event tables and leaves
existing objects, ordinary runs, workflows and SQLite `user_version=1` unchanged.
Atomic claims use private per-job tokens. A 30-second lease is renewed about every
two seconds while the solver runs. Expired agents cannot renew/publish; explicit
`peykar native recover` (or the next claim) fails abandoned jobs as `agent_lost`.
Recovery does not retry a scientific run. Resubmit intentionally with a new key.

Cancellation of a queued run immediately prevents a claim. Running cancellation
stops the native container; the final transaction checks cancellation and lease
again, so a stale or cancelled success cannot publish results. HTTP errors,
timeouts and unavailable local profiles produce a failed run, never simulated
success. Single-job CLI mode exits on failure. With `--loop`, the agent reports a redacted
error category, waits the configured poll interval and continues to other queued
jobs. Cancelled/failed jobs remain terminal; this is not an automatic scientific
retry. Host restart supervision is still required.

Each fresh private directory retains the model, reviewed driver, controls,
manifest, engine log and actual outputs. The Docker container has no network,
read-only root/input mounts, a non-root UID, no extra capabilities, no API token or
Docker socket, two CPUs, 3 GiB memory, 128 PIDs and a 128 MiB temporary filesystem.
The parent acknowledges `docker create` before a separately supervised
`docker start --attach`. Cancellation is checked before creation and again before
start, preventing a late create from racing an earlier removal. Creation has its
own 15-second RPC bound; the solver wall budget starts after creation. The parent
enforces wall/log/output limits and removes the named container on
success, cancellation or failure. If the Docker daemon is unreachable, a retained
cleanup warning identifies the container for operator inspection. This is a local
single-workspace execution profile, not a multi-tenant hostile-code guarantee.

Only regular non-symlink bounded output files are read. The server checks exact
manifest/image/driver and artifact digests, finite CSV values, rectangular/unique
columns and sample metadata before publishing. COPASI's physical sample grid must
match the frozen plan; PKML permits original repeated event times and requires
quantity metadata matching every physical CSV column. Its original `IndividualId`
is retained as an integer identity, without an invented physical unit; this driver
supports one individual and rejects mixed identities. Combined raw results are capped at
1,000,000 bytes, fitting base64 within the existing request limit. Failed or
cancelled jobs expose no result. Private failed logs remain on the operator host.

## API and verification

Reader/admin: `GET /v1/native-models`, model `/versions/{version}` and `/content`,
`GET /v1/native-profiles`, `GET /v1/native-runs`, run status, `/manifest`, `/events`,
`/results` and `/artifacts/{name}`. Admin: model/job POST, job `/cancel`, queue
`/claim`, `/recover`, job `/heartbeat` and `/complete`. Claim/heartbeat/completion
require administrator authorization; the latter two also require
`Native-Claim-Token`. Profile registration is offline only. Submission returns 201
and `Location`; idempotency pins the normalized plan. Invalid state returns 409,
invalid numerical envelopes 422, missing objects 404 and missing/weak roles 401/403.

With both profiles installed/admitted, an idle queue and other native agents stopped:

```sh
uv run python scripts/verify_native_agent.py \
  --output .peykar/native-live-verification --pkml-model-id native.aciclovir
```

The script registers the synthetic SBML through HTTP, verifies idempotent jobs,
runs the actual allowlisted containers, retrieves/hashes results and checks
analytical A→B conservation, minute-to-second controls and queued cancellation.
The optional PKML must already be admitted from the original source. Every run is
retained. Native execution proves engineering behavior; it does not establish
independent biological or clinical validity. Actual outcomes are recorded in
[verification](verification.md).
