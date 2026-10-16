# Peykar

**Biological Modeling & Simulation Platform**

Peykar is a source-available foundation for registering, composing, executing, and
evaluating versioned biological models. Its long-term scope extends from molecular
and cellular models to tissues, organs, and interacting systems.

**Release 0.3.0 is an executable engineering foundation, not a validated biological
simulator.** It includes two synthetic reference engines. vCornea integration,
general multiscale execution, independent biological validation, and the dashboard
have explicit designs and acceptance gates; they are not implemented claims.

The current engineering continuation adds checked worker result/runtime contracts,
offline admission of digest-pinned external files, operator-mapped CompuCell3D
scalar exports, frozen seed-paired comparisons and persistent knowledge-gap
declarations. None of these installs or runs
vCornea, reproduces a published scenario or establishes biological validity.

[راهنمای فارسی](README.fa.md) · [Master blueprint](docs/master-blueprint.md) · [Architecture](docs/architecture.md) ·
[Implementation status](docs/status.md) · [Roadmap](docs/roadmap.md) ·
[Scientific contracts](docs/scientific-contracts.md) · [Blueprint coverage](docs/requirements-traceability.md) ·
[Licensing](LICENSING.md) · [Research sources](docs/sources.md)

The complete original Persian v3.0 architecture, supplied before the Peykar name
was chosen, is [archived verbatim](docs/blueprints/README.md). Its
[55-section and 24-scenario coverage map](docs/blueprint-v3-coverage.md) keeps future
requirements distinct from the implemented inventory.

## Run in two commands

Requirements: Linux for the bounded worker, Python 3.12 or 3.13 and
[uv](https://docs.astral.sh/uv/).

```bash
uv sync --frozen --all-packages --group dev
uv run peykar demo
```

The demo registers a model, stores its immutable run manifest, enqueues a job,
executes it through the worker, saves a checksum-verified artifact, and compares
the result with an independently calculated analytical reference. The verification
report must say `synthetic_verification`, never independent biological validation.
It requires no API keys, GPU, external datasets, or engine downloads.

```bash
uv run peykar init
uv run peykar submit experiments/manifests/logistic.json
uv run peykar worker --once
uv run peykar models
```

Use `PEYKAR_DATA_DIR` to change the default local `.peykar` state directory.

## Import data and freeze an evaluation

```bash
uv run peykar import-csv experiments/data/analytic-decay.csv --spec experiments/data/analytic-decay-import.json
uv run peykar freeze-evaluation experiments/benchmarks/decay-evaluation-set.json
uv run peykar submit experiments/manifests/decay.json
uv run peykar worker --once
uv run peykar evaluate RUN_ID decay.holdout 1.0.0
uv run peykar report REPORT_ID
```

Replace `RUN_ID` and `REPORT_ID` with the IDs printed by the preceding commands.
These fixtures are synthetic. Raw CSV bytes and normalized observations are stored
separately with checksums. The frozen set pins dataset versions, declared split
groups, endpoint and tolerance. See [data and evaluation](docs/data-evaluation.md).

## Start the API and worker

Set a private administrator token first:

```bash
export PEYKAR_API_TOKEN="$(python -c 'import secrets; print(secrets.token_urlsafe(32))')"
uv run peykar init
uv run peykar serve
```

In a second terminal, in the same checkout and with the same `PEYKAR_DATA_DIR`:

```bash
uv run peykar worker
```

Swagger UI: http://127.0.0.1:8000/docs. Authorize with the token before using `/v1`.
`/healthz`, documentation and the OpenAPI schema are public; all data routes require
authentication. Optional `PEYKAR_READ_TOKEN` grants read access only.

```bash
curl -X POST http://127.0.0.1:8000/v1/simulations \
  -H "Authorization: Bearer $PEYKAR_API_TOKEN" \
  -H 'Content-Type: application/json' \
  -H 'Idempotency-Key: example-logistic-1' \
  --data-binary @experiments/manifests/logistic.json
```

The response contains a run ID and `Location` for polling. GET that location until
the status is terminal, then GET `/v1/simulations/RUN_ID/results`. The API and
worker must share the same persistent directory. With the existing administrator
token exported, verify this complete path against running services:

```bash
uv run python scripts/verify_api.py experiments/manifests/decay.json
uv run python scripts/verify_api.py experiments/manifests/logistic.json
```

Each verification creates one retained synthetic run and checks idempotency,
manifest/result hashes and lifecycle events. It never resets existing state.
See [external artifact admission](docs/external-artifacts.md) and
[mapped scalar imports](docs/compucell-output.md) for the new offline commands.
The [knowledge-gap API and CLI](docs/knowledge-gaps.md) record missing evidence or
mechanisms without pretending they are implemented or scientifically resolved.

## Repository boundaries

| Path | Responsibility |
|---|---|
| `packages/core` | Independent contracts, units, hashing and engine protocol |
| `packages/engines` | Trusted reference engines behind the shared protocol |
| `packages/platform` | Domain modules, registries, local infrastructure, API, CLI |
| `apps` | Entrypoints and dashboard design |
| `biological_libraries` | Reusable mechanisms and promotion criteria |
| `model_packages` | Immutable descriptors, separate from executable engines |
| `engine_adapters` | Integration plans and evidence for specialist engines |
| `dataset_connectors` | Data connector boundaries and external-source policies |
| `experiments` | Portable plans, baselines and benchmarks |
| `schemas` | Generated JSON Schema and OpenAPI contracts |
| `tests` | Unit, scientific verification and integration tests |
| `docs` | Architecture, ADRs, scientific limits and implementation roadmap |
| `infrastructure`, `deployment` | Local persistence and container operations |

The three Python distributions share one uv lockfile and have explicit one-way
dependencies. They can be extracted later without creating a microservice fleet.

## Quality checks

```bash
uv run ruff check .
uv run ruff format --check .
uv run mypy
uv run pytest --cov --cov-report=term-missing
uv run python scripts/export_contracts.py --check
uv build --all-packages
```

On macOS or Windows, run `make check-docker` for the Linux worker regression and
package build in an isolated verification container. It uses the frozen lockfile,
has no network during the tests and does not mount the live data volume.

See [CONTRIBUTING.md](CONTRIBUTING.md) for scientific and software change rules.
The GitHub workflow runs these checks for Python 3.12 and 3.13. A configured CI
workflow is not evidence that those remote jobs have already run.

## Scientific boundary

An executable model is not automatically a scientifically useful model. A unit
conversion is not a mechanistic mapping. Two individually validated models do not
automatically form a validated composite. A simulation result is not an experimental
observation. Model improvement needs held-out evidence for a stated context of use.

Peykar preserves these distinctions in contracts, outputs, tests and promotion gates.

## License

Original material in this distribution is offered under the **Peykar Noncommercial
Source License 1.1.0**, a project-specific draft requiring qualified legal review.
Its public grant permits defined noncommercial work and reserves commercial/paid
uses. It is **not an OSI-approved open-source license**. See [LICENSE](LICENSE) and
[LICENSING.md](LICENSING.md) for the actual scope, source conditions and exceptions.

Free humanitarian services are expressly permitted under the public conditions.
Section 7A preserves the owner's ability to use and separately license rights he
actually owns, including commercially, without buying a license from himself.

An earlier private 0.1.0 snapshot contained Apache-2.0 terms. Any valid rights actually
obtained under that grant remain effective; this change cannot withdraw them.
External packages, engines, models and datasets retain their own licenses. None of
the specialist engines is bundled. A licensing label is not a guarantee of ownership
or enforceability. [Licensing rationale](docs/licensing-rationale.md).
