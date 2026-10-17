# Permanent upstream source distribution

All admitted specialist source trees are ordinary files here. They are not ignored
cache entries or Git links. `upstream/` holds the original nine specialist projects;
`dependencies/` holds separately published native solvers, MaBoSS and OSP packages;
`models/` retains the vCornea Zenodo and Git identities separately. Recursive Git
dependencies are materialized at their pinned paths inside each source tree.

`manifest.json` records exact commits, paths and immutable SHA-256 inventories.
Original source, notices, executable modes and links are retained. `.git` data lives
only in the acquisition cache; omitting upstream Git history does not omit tracked
source. Never build in these pristine trees: Docker profiles build private copies.

Verify every byte and detect missing dependencies/LFS payloads:

```sh
uv run python scripts/verify_vendor.py
```

Explicit reacquisition and materialization:

```sh
uv run python scripts/acquire_ecosystem_sources.py --catalog engine_adapters/ecosystem-sources.json --destination .peykar/external/ecosystem/sources --jobs 3
uv run python scripts/fetch_source_dependencies.py --sources .peykar/external/ecosystem/sources --receipt .peykar/dependencies.json
uv run python scripts/acquire_ecosystem_sources.py --catalog runtime_profiles/additional-sources.json --destination .peykar/external/extra-sources --jobs 3
uv run python scripts/fetch_source_dependencies.py --sources .peykar/external/extra-sources --receipt .peykar/additional-dependencies.json
uv run python scripts/vendor_sources.py
```

See [third-party notices](../THIRD_PARTY_NOTICES.md) and
[runtime installation and native probes](../runtime_profiles/README.md). Source
availability, runtime execution and biological validity have distinct evidence.

E5 additionally retains the complete libCellML 0.7.1 source under
`dependencies/libcellml`, with its original Apache-2.0 notice. Current rehash:
26 distributions, 53,261 files, 1,814,782,314 bytes and 37 materialized Git links.
