# Development instructions

- Treat README and docs/status.md as the truthful implementation inventory.
- Read the nearest domain README before implementing an extension.
- Do not import platform/engine modules into peykar_core or platform into engines.
- Preserve schema/model version immutability and parameter units.
- Do not install or execute remote model code automatically.
- Use uv.lock; run `make check` for code changes and `make build` for package changes.
- A synthetic test does not authorize a biological or clinical validity claim.
- Implement a real behavior or document it as designed; do not return fake success.
- Keep HomBon-specific behavior inside the integration facade.
- Update docs/verification.md only with commands actually run and observed outcomes.
