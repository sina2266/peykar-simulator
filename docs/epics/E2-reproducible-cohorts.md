# E2 — reproducible job cohorts, parameter sweeps and terminal statistics

This engineering EPIC delivers S4.5 across the existing reference and native queues.
It builds on the original scalar worker and reviewed COPASI/OSPSuite host agent.
It does not implement biological coupling, native DAG bindings or independent
validation. Those retain their separate roadmap gates.

## Delivered workflow

1. Register `BatchSpec` with named cases, optional Cartesian parameter axes,
   replica count, base seed, failure policy and aggregate budget.
2. Freeze exact per-member plans, models, dependencies, environment and native
   profile/runtime snapshots. Admission starts no simulation. The frozen templates
   receive fresh execution IDs/timestamps for each submitted cohort.
3. Submit idempotently. The ordinary Worker reconciles pending cohorts and dispatches
   eligible members into either queue. Every child insertion and association share
   one SQLite transaction. Native execution still requires the separate host agent.
4. Inspect member status/events or cancel the cohort. Retrieve complete or explicitly
   incomplete terminal statistics, with every child manifest/result digest retained.

The shipped [reference sweep](../../experiments/batches/reference-sweep.json) runs
two parameter levels and three seeded replicas for each of two synthetic models:
12 child jobs in total. [The mixed cohort](../../experiments/batches/mixed-native.json)
runs the reference decay, actual COPASI SBML and original Aciclovir PKML.
[The cancellation fixture](../../experiments/batches/native-cancel.json) starts one
native job and verifies that subsequent members are skipped after cancellation.

## Scientific and resource contracts

- Reference seeds are `base_seed + expanded ordinal`, unique within a cohort and
  identical on explicit replay. Ordering is case, declared Cartesian axes, replica.
  The case's original seed is replaced by this declared policy. Reordering a frozen
  specification requires a new version. Independent seeds are not independent data.
- Sweeps replace explicit initial parameter overrides, preserve their declared units
  and validate physical conversion, bounds and engine constraints before admission.
  Physically equivalent duplicate values and duplicate axes/case IDs are rejected.
  Existing scheduled interventions retain their original meaning.
- Native drivers currently provide no stochastic seed or parameter override protocol.
  Native cases therefore require one replica and reject parameter axes. Separate
  admitted native models can be separate cases. No fake native seed is recorded.
- Expansion is bounded by 256 jobs, 32 replicas, six axes and 16 values per axis,
  plus each declared smaller budget. Snapshots are capped at 4 MiB. Aggregate sample
  and allowed solver wall budgets are checked before registration. PKML's original
  schedule is not guessed: its conservative returned-sample bound is 100,001,
  matching the existing server output envelope. The bound is not an estimate of
  internal solver steps or a promise that a large output will fit the 1 MB bundle.
- `max_in_flight` (1–16) counts queued plus running members across both queues, per
  cohort, under the same writer transaction. This is not a global multi-user quota.
  Summed solver wall limits exclude queue wait, container setup and cleanup overhead.
  Existing worker/container limits still apply; no monetary cost estimate is invented.
- `continue` runs independent remaining members after a failure and ultimately marks
  the cohort failed. `fail_fast` cancels active/queued children and skips unstarted
  members. Explicit cancellation propagates to both queues; ownership checks prevent
  a late child success from publishing. Successful earlier child results are retained.
- Native creation is acknowledged before start, with cancellation checks around
  creation and final removal after the attached client stops. This closes an actual
  cancellation race that left a late-created container. Loop mode continues to other
  jobs after redacted failures/cancellations; it does not retry terminal jobs.
- SQLite migration `004_batches.sql` adds separate tables/indexes and foreign keys
  without rewriting prior registry/queue rows or changing `user_version=1`.

## Reports

Reports are available only after the cohort reaches a terminal state. They group
terminal scalar values within the exact case and parameter grid. Each group exposes
expected/successful replicas, excluded member IDs and `complete`. Failures,
cancellations and skipped work never become zero measurements. Empty groups have
no endpoint statistics. A single success has no sample standard deviation; two or
more use `n-1`. Native quantity units and original identity columns are preserved.

Every successful child manifest is compared with its frozen template, and result
hashes are verified before aggregation. Output sets and compatible physical units
must agree within each group. The report's scope is
`within_case_terminal_sample_variability`: it supplies no confidence interval,
parameter uncertainty, sensitivity rank, generalization claim or biological validity.
Incomplete groups may be biased and remain explicitly incomplete. HTTP reports have
a content-derived ETag and include all member manifest/result digests for replay.

## API and CLI

```sh
uv run peykar batches register experiments/batches/reference-sweep.json
uv run peykar batches submit batch.reference-sweep 1.0.0 --idempotency-key cohort-example
uv run peykar worker
# In another terminal with the same PEYKAR_DATA_DIR:
uv run peykar batches status BATCH_RUN_ID
uv run peykar batches results BATCH_RUN_ID
uv run peykar batches cancel BATCH_RUN_ID
```

For Compose, use the existing shared volume for operator commands or use HTTP;
host `.peykar` and the Compose volume are different registries. `batches advance`
is an explicit local reconciliation command when the ordinary Worker is stopped.

Reader/admin GET: `/v1/batches`, `/v1/batches/{id}/versions/{version}`,
`/v1/batch-runs`, `/v1/batch-runs/{id}`, `/events`, `/results`.
Admin POST: `/v1/batches` (201), `/v1/batch-runs` with an exact `Reference` (202,
`Location`, optional `Idempotency-Key`), `/v1/batch-runs/{id}/cancel`.
Lists are bounded/paginated. Missing authorization is 401, insufficient role 403,
unknown reference 404, immutable/state/key conflict 409 and invalid constraints 422.
No installation, code path, image name or remote fetch can be selected by the spec.

With the ordinary Worker, reviewed native agent and admitted native fixtures ready:

```sh
uv run python scripts/verify_batches.py --output .peykar/my-cohort-verification
```

The script retains created records. It checks actual sweep execution, idempotency,
statistics against downloaded child results, a mixed reference/native cohort and
running native cohort cancellation. Observed outcomes and engineering acceptance
are in [verification](../verification.md); the full system roadmap remains open.
