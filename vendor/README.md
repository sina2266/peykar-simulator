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
29 distributions, 53,387 files, 1,824,257,046 bytes and 37 materialized Git links.
The additional50 original files in `models/vcornea-history-b6123adf` were recovered
from Git objects embedded in the pinned licensed Zenodo code ZIP. Its July2024
identity, complete blob hashes and attribution remain separate from later sources.
The complete January2025 stash adds another50 original files in
`models/vcornea-history-c7f95d28`, preserving distinct formation/injury mechanisms
and original modes. Reacquire either reviewed snapshot with
`scripts/recover_vcornea_history.py --archive <original-code.zip> --pins
engine_adapters/compucell3d/vcornea-history-<commit-prefix>.json --output <fresh-path>`.
Only the two pinned commits are admitted; no downloaded Git config/hooks execute.
The earlier published record16764319 additionally retains all26 ordinary source
files under `models/vcornea-16764319`. Its whole ZIP MD5/SHA256 and per-file CRC/SHA
were actually checked; original nonexecutable identity and ZIP permission metadata
are retained. Its separate [source audit and exact operator](../experiments/vcornea/publication/prior-source-audit-2026-10-23.md)
describe reacquisition and byte comparison. No new runtime execution is inferred.
