# Peykar research workbench

Status: implemented local six-workspace client, EPIC E4. Read
[scope, operation and limits](../../docs/epics/E4-research-workbench.md).

The client is React/TypeScript, compiled with Vite and shipped in the platform
wheel at `/app/`. The earlier Next.js target was a design choice, not an existing
implementation. A static client keeps the current FastAPI origin/authorization
boundary and requires no additional production Node server. No API credentials,
remote fonts, analytics or scientific executables are bundled.

- Biological Explorer: immutable graph versions, bounded neighborhoods, entity
  selection, context, ontology identifiers and attributed evidence.
- Model Studio: paginated exact versions, parameters/ports, source/runtime/queue
  evidence distinctions, declarative XML admission and create-run handoff.
- Dataset Studio: scalar observations, origin/role/context/license/groups, physical
  plots and raw/normalized transformation lineage; strict CSV admission.
- Experiment Studio: real reference/native submit, reference resource estimate,
  frozen DAG/cohort definitions and a structured native/reference scalar flow
  editor using admitted interfaces and explicit transfer evidence.
- Simulation Viewer: all five persistent queues, exact-ID links, cancellation,
  child status, actual time series, complete sample tables, manifests, events,
  native artifacts and cohort endpoint statistics with excluded members.
- Validation Center: freeze declared evaluation sets, execute run-bound evaluation,
  retrieve reports and inspect manually declared knowledge gaps.

Advanced XML/CSV metadata and frozen definitions use validated JSON editors.
General quality decisions, independent split authority, ontology reconciliation,
continuous feedback, spatial geometry and unsupported native adapters remain open.
Graphs do not imply biological coupling; plots never invent cell trajectories.

## Development and builds

Use Node 22.14+ (at least 22.12) and the committed `package-lock.json`:

```sh
npm ci --ignore-scripts --no-fund --no-audit
npm run dev
```

The development proxy targets `http://127.0.0.1:8000`; use the production deployment
for browser security-header verification. `make dashboard` at repository root
builds static assets into `packages/platform/src/peykar/web_assets/`. Generated
assets/node_modules are ignored; `make build` and Docker build regenerate and
package them. `make check` includes strict TypeScript, frontend tests, formatting
and generated OpenAPI type freshness. Regenerate types with `npm run types` after
`make schemas`. Do not change historical scientific schemas to satisfy a UI type.

API tokens stay only in the current React session. Reload/disconnect clears them;
localStorage/sessionStorage/cookies/URLs never store credentials. All API requests
remain same-origin, reject redirects and omit cookies. Protected API roles remain
the authority; the UI's reader controls are disabled. No multi-user isolation is
claimed. English/Persian navigation, forms and feedback support RTL; original
scientific labels, units and source-authored limitations are retained verbatim.

Original React, React DOM and Scheduler MIT notices are retained under
`third-party/`, checked against the exact installed packages and shipped at
`/app/licenses/`. Application source remains under Peykar's existing terms.

Model Studio also exposes CellML 2 admission, original units/bytes and retained
operator executions with plots, tables and exact artifact downloads. This tab
explicitly separates operator execution from future CellML queue support. See E5.
