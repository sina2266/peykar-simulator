# E4 — operational research workbench

Delivered scope: a local, authenticated six-workspace application for the existing
shared scientific services. This EPIC closes the first complete browser workflow;
it does not close all biology, governance or multiscale requirements in S0–S13.

## Use

With the existing Docker API/Worker running, open
`http://127.0.0.1:8000/app/` and enter the existing administrator or reader token.
The shell is public; scientific API reads require authorization. A page reload or
Disconnect clears the in-memory token. Do not paste credentials into an issue,
shareable URL or frontend environment file.

1. Inspect exact model versions in Model Studio and choose **Create run**, or
   select a model directly in Experiment Studio. Model context is copied from its
   immutable package. Configure physical duration/sample units, declared parameter
   values, seed, purpose and wall budget. **Estimate** computes the reference sample
   count and budget; computational cost remains unknown.
2. **Submit run** freezes the real server manifest. The Viewer polls the durable
   record, exposes child runs, cancellation, events and manifests, and displays
   outputs only when the server publishes results. Resume through queue pagination
   or the exact run ID; run links survive a reload after reconnecting.
3. Native models use an admitted, format-compatible execution profile. COPASI has
   explicit duration/intervals; PKML retains its original schedule. The separate
   operator agent must run; the UI never claims its current availability based on
   a historical probe or a queued job.
4. In **Scalar flows → Design flow**, add exact reference/native model nodes, select
   declared output/input ports and target reference parameters, and supply transfer
   evidence. Freeze the definition, then run its version. Native choices require
   an admitted interface. Server context, units, direction, ownership, graph and
   aggregate-budget checks are unchanged. This is sequential terminal-to-initial
   transfer; the layout does not establish a biological bridge or feedback loop.
5. Dataset Studio imports a UTF-8 scalar CSV plus its explicit metadata/column/unit
   mapping. It displays the retained observation and generic CSV lineage. Other
   origins are explicitly identified as lacking generic CSV lineage rather than
   reported as an integrity failure. Large native XML remains an offline admission
   workflow under the existing HTTP limit.
6. Freeze an evaluation set in Validation Center, then evaluate a completed reference
   run. Reports retain endpoint, context, tolerance, numerical errors and all source
   digests. A passing synthetic report remains synthetic verification. Declared
   independence groups do not establish independent evaluator authority.

## Implementation and boundaries

React 19.3.1 / React DOM 19.3.1 / Scheduler 0.28.0 use retained original MIT notices.
The exact npm lock supplements the unchanged `uv.lock`. Vite compiles external
JS/CSS; the platform wheel/sdist and Docker runtime include the same static assets.
No Node service is needed in production. The earlier proposed Next.js frontend had
no implementation; a static build serves the current local architecture directly.
[Vite's build/deployment documentation](https://vite.dev/guide/build) describes
this packaging model. No CDN/font/analytics dependencies are used at runtime.

- Read-only `/v1/workspace/session` discloses the authenticated local role, never a
  secret. Bounded `/v1/workspace/catalog/{kind}` lists only dataset/scenario/
  evaluation-set/validation-record metadata with immutable-object integrity checks.
- New paginated `/v1/experiment-runs` and `/v1/hybrid-runs` reconstruct original
  service records. Native model and experiment definition discovery now accepts
  bounded pagination while preserving the previous default limits. No SQL schema,
  scientific model version, source inventory or native agent contract was replaced.
- StaticFiles confines `/app/` to packaged assets. API credentials do not enter
  cookies, browser storage, URLs, compiled assets or responses. Fetch rejects
  redirects, uses same-origin API paths and omits cookies. All responses are
  `no-store`; the client has no service-worker/offline cache of scientific records.
- The workbench has a strict external-script/style CSP, denied framing, no referrer
  and `nosniff`. API/Worker keep their existing non-root/read-only deployment and
  have no Docker socket. Current authorization is for one local workspace.
- Reads abort on navigation and poll without overlap while visible. Timeouts,
  unavailable services, 401/403, immutable conflicts and lifecycle races have
  explicit feedback. Mutation retries preserve an idempotency key after an
  uncertain response; there is no automatic POST retry. Download buttons cannot
  submit enclosing editor/run forms.
- Scalar plots use original physical units, strict finite CSV decoding and named
  OSPSuite quantities; `IndividualId` is not plotted as a physical measurement.
  The inspected COPASI key is `model_units.time_unit`. Narrow plots change their
  coordinate layout/font sizes rather than shrinking axis labels to illegibility.
  Large series are decimated only for drawing; tables/downloads retain every sample.
- Scientific metadata, source-authored statements and schemas stay in their original
  language. English/Persian application controls use logical CSS/RTL layout. Mobile
  menus leave the keyboard tree when closed and support Escape/focus return.

## Acceptance and deferred scope

Engineering acceptance covers reference/COPASI submit-to-result, existing original
OSP result display, browser-designed mixed flows, CSV import/immutable conflict,
frozen evaluation/report retrieval, cancel-without-result, reader controls,
reconnect/degraded states and desktop/tablet/mobile/RTL/keyboard checks. Observed
commands and outcomes are recorded in [verification](../verification.md) and
[the E4 receipt](../workbench-verification-2026-10-17.json).

Quality adjudication, donor/experiment split authority, independent scientific
approval, claim/conflict histories, ontology imports, graphical spatial simulation,
checkpoint/feedback control and other specialist queue adapters remain under their
own roadmap gates. Advanced package/graph/dataset/evaluation/cohort JSON editors
are operational interfaces for the current contracts, not no-code workflows for
those unimplemented scientific capabilities. Model selectors show bounded pages;
the run builder admits at most 200 listed model versions and points to Model Studio
pagination for selecting other exact versions. Evaluation selectors use the latest
200 runs and 50 frozen sets. Global full-text search and richer authoring remain
separate enhancements. Only the Chromium browser profile is verified here.
