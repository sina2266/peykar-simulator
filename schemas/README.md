# Generated contracts

Run uv run python scripts/export_contracts.py; use --check in CI.
Scientific contracts use JSON Schema 2020-12. openapi.json describes HTTP transport.
Generated files are committed so clients can inspect contracts without executing Python.
Schema versioning is independent of scientific model/engine versions.
