# Offline external artifact admission

`peykar external-artifacts` admits an operator-supplied local inventory. It verifies
the exact source/archive bytes, every declared dependency file and a required
runtime-lock file against pinned SHA-256 values. It preserves those bytes in the
existing content-addressed store and registers the inventory immutably by ID/version.
No database migration or replacement of existing model/data versions is required.

This is an integrity gate for explicitly supplied files. The source URL and license
are declarations; they are retained without transferring ownership or establishing
redistribution rights. Expected digests need a separately trusted origin. Hashing a
file alone does not authenticate its author, detect malicious code or establish
scientific validity. Imports do not fetch URLs, extract archives, install packages,
import Python modules or execute model code.

## Inventory and commands

Create an `ExternalArtifactSpec` JSON document. `artifact`, each `dependencies`
entry and `runtime_lock` contain `path` and `digest`. The paths are distinct,
normalized relative POSIX file paths below the explicitly supplied directory.
For example, an inventory can refer to `source/model.zip`,
`dependencies/engine.whl` and `runtime.lock`. An empty dependency list is permitted
for an artifact that declares no dependency files. The operator supplies complete
dependency declarations; Peykar does not discover a dependency closure inside an
archive or lockfile.

```json
{
  "schema_version": "1.0.0",
  "id": "external.reference",
  "version": "1.0.0",
  "source": "Upstream source URL and pinned revision",
  "license": "Original upstream license declaration",
  "artifact": {"path": "source/model.zip", "digest": "EXPECTED_SOURCE_SHA256"},
  "dependencies": [
    {"path": "dependencies/engine.whl", "digest": "EXPECTED_DEPENDENCY_SHA256"}
  ],
  "runtime_lock": {"path": "runtime.lock", "digest": "EXPECTED_RUNTIME_LOCK_SHA256"}
}
```

Replace every `EXPECTED_...` placeholder with a lowercase 64-character SHA-256
before using this illustration. The generated schema is
[`ExternalArtifactSpec-1.0.0.json`](../schemas/ExternalArtifactSpec-1.0.0.json).

```bash
uv run peykar external-artifacts import external-inventory.json --directory ./local-bundle
uv run peykar external-artifacts verify external.reference 1.0.0
```

Successful commands print the admitted spec, its content digest, checked file count
and explicit `runtime_reproduced: false`, `code_executed: false` and
`source_fetched: false`. Verification uses stored bytes, so it still works after
the original input directory is disconnected. Source files and their permissions
are unchanged.

## Failure and resource boundaries

Every declared file must exist, be a regular file and match its digest. Symlinks in
the relative path, parent traversal, absolute paths, devices and named pipes are
rejected. The local filesystem implementation uses POSIX descriptor-relative opens
with no symlink following. The descriptor is limited to 2 MB, each file to 64 MiB,
the entire declared inventory to 256 MiB and dependencies to 200 files.

All digests and read bounds are checked before publishing any input bytes or
inventory metadata. Reimporting an identical inventory is idempotent. Changing its
license, source or pins requires a new version. A version conflict preserves the
existing inventory; content-addressed bytes written before that conflict may remain
unreferenced, as with the CSV importer. Retrieval checks the stored inventory hash
and rereads every declared artifact through the checksum-verifying store. Missing
or corrupted bytes fail verification.

The runtime lock is preserved and checked as opaque bytes. No runtime is resolved,
installed or reproduced, and no claim is made that a dependency matches its declared
runtime behavior. The existing descriptor-only model import and model registry
remain separate; admitting an external inventory does not register or approve an
executable engine. CompuCell3D/vCornea admission, a reviewed execution sandbox,
runtime reproduction and source-permission verification remain separate gates.
