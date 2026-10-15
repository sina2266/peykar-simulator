# Contributing

Read `docs/architecture.md`, `docs/status.md` and `docs/scientific-contracts.md` first.

1. Add a focused change with a stated user/scientific behavior.
2. Keep core independent of engines, transports and persistence.
3. Validate all external input at a public contract boundary. Do not trust
   Pydantic model_copy updates or registered metadata as external evidence.
4. Add meaningful tests for numerical changes, scientific constraints and failure
   paths. Compare against an independent reference, not a copy of the implementation.
5. Record source, license, applicability, solver/environment versions and known
   limitations for models and datasets. No copied external code without permission.
6. Never silently change an immutable model, dataset, scenario or benchmark version.
7. Do not call synthetic verification biological validation or present simulated
   predictions as laboratory measurements. Scope claims to a stated endpoint/context.
8. Numerical coupling changes need conservation, step-refinement and stability tests.
   Biological changes additionally need evidence appropriate to the intended use.
9. Update status, sources and ADRs when a capability or architectural boundary changes.
10. Run `make check` and `make build`. Include the exact results and limits in the PR.

Small software fixes need proportionate tests. Scientific changes require a clear
evaluation protocol. Reports of negative scientific results are useful and should
remain reproducible. No PR should claim an external CI/deployment has run without
observed evidence.

The current project uses the custom noncommercial terms described in LICENSING.md.
Declare the license and your authority to contribute before a contribution is
accepted. Contributors retain their rights; submitting a patch does not assign
copyright or give the owner an automatic right to sell contributor-owned material
under another license. A separate reviewed contributor agreement is needed where
additional commercial relicensing rights are required. Preserve valid earlier
Apache grants and third-party notices. Do not submit employer/client-owned or
incompatibly licensed material without authorization.
