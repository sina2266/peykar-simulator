# Durable multi-model experiment workflows

The platform now executes immutable DAG experiments across registered models,
using the existing bounded Worker queue. This implementation transfers a named
**final scalar output into a downstream initial parameter**. It is an executable
sequential workflow, not a simultaneous feedback solver or validated biological bridge.

`ExperimentSpec` pins nodes/plans, bindings/evidence and limitations. Freezing checks
model versions, context, directions, semantic IDs, units, coordinate frame/shape,
unique writers and DAG order; it retains exact model snapshots. Cross-context,
field/agent and feedback mappings are rejected. Bound parameters cannot also have
explicit overrides. Parameter bounds are checked again after actual transfer.

Each eligible node is prepared and inserted into the ordinary simulation queue
**in the same SQLite transaction** as its experiment association. Concurrent workers,
retries and cancellation cannot leave an unassociated child. Models run with the
same manifest, environment, result checks and resource supervisor as standalone jobs.
The next tick dispatches only dependencies whose results succeeded. There is no
interpolation, silent resampling, guessed transformation or invented result.

Failure stops downstream dispatch and cancels outstanding siblings. Cancellation
stays pending while a running child terminates, then reaches a terminal state;
unsubmitted nodes are explicitly skipped. A bad experiment cannot stop the ordinary
queue. Successful retrieval requires every node to succeed. Child manifest/result
hashes and the transferred initial value remain inspectable independently.

## CLI example

The committed example is synthetic and checks `exp(-0.1*t)` over two serial
10-second intervals. The expected intermediate/final concentrations are
`exp(-1)` and `exp(-2)` mole/liter; no tissue biology is implied.

```bash
uv run peykar register-model experiments/workflows/decay-transfer-model.json
uv run peykar experiments register experiments/workflows/decay-transfer.json
uv run peykar experiments submit workflow.decay-transfer 1.0.0 --idempotency-key my-chain
uv run peykar worker
uv run peykar experiments status EXPERIMENT_RUN_ID
uv run peykar experiments results EXPERIMENT_RUN_ID
uv run peykar experiments cancel EXPERIMENT_RUN_ID
```

The worker is continuous; one `--once` handles at most one ordinary child job.
Each child seed and execution budget stays pinned in its own node plan. Workflows
currently support at most 32 nodes/128 bindings; ensembles and aggregate budgeting
are separate roadmap tasks. Dependencies must use approved installed engines;
source inspection alone does not make an engine available for submission.

## API

| Method | Path | Role |
|---|---|---|
| POST | `/v1/experiments` | Admin, freeze immutable spec; 201/Location |
| GET | `/v1/experiments` | Read |
| GET | `/v1/experiments/{id}/versions/{version}` | Read |
| POST | `/v1/experiment-runs` | Admin, body exact Reference; 202/Location; Idempotency-Key |
| GET | `/v1/experiment-runs/{id}` | Read, child status/IDs and parent digest |
| GET | `/v1/experiment-runs/{id}/results` | Read, successful children with separate results |
| GET | `/v1/experiment-runs/{id}/events` | Read, ordered durable events |
| POST | `/v1/experiment-runs/{id}/cancel` | Admin |

The new additive tables live in `002_experiments.sql`; existing object/run/event
rows and schema/model versions are retained. Source snapshots and biological graphs
are independent registries, not automatically converted into executable workflows.
A genuine molecular/tissue bridge still needs stateful numerical coupling, process
ownership and context-specific evidence as defined in S5/S6 of the roadmap.
