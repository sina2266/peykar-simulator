# Model Import

Declarative model JSON inspection and a separate operator-only offline artifact
inventory gate. ExternalArtifactService verifies pinned source/dependency/runtime
lock bytes before immutable admission, retains original licenses and rechecks
stored integrity. No remote fetching, archive extraction, installation or execution.
The admitted inventory does not register or approve an engine. Runtime reproduction
and dependency discovery remain unsupported. See docs/external-artifacts.md.

`libraries.py` additionally inspects pinned local Git source trees, retains
SHA-256 file/license inventories, declares unresolved Git links/LFS pointers and
admits trusted offline inspection bundles. Authenticated reads expose source
readiness separately from runtime/adapter/validity status. Explicit network source
acquisition is confined to the operator script, not this service or the API.
See docs/library-ecosystem.md and the whole-platform roadmap.

See docs/architecture.md, docs/status.md and docs/scientific-contracts.md at repository root.
