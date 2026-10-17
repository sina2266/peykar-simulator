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

`native_runtimes.py` imports trusted offline native-execution evidence, validating
source references, image identity, successful status and artifact hashes before
immutable registration. API reads expose this independently of queue capability.
It never starts processes, installs dependencies, fetches images or executes
upstream code. See runtime_profiles/README.md.

`native_models.py` admits bounded declarative SBML/PKML XML, preserves exact bytes
and original units and rejects entities/includes. Operator-only execution profiles
pin an existing runtime and reviewed driver; admission starts no engine. A separate
external agent executes explicitly submitted native jobs. See docs/native-agent.md.

See docs/architecture.md, docs/status.md and docs/scientific-contracts.md at repository root.

`native_interfaces.py` admits explicit scalar native ports and prepares immutable
SBML initial-concentration derivatives. Only numeric attribute bytes are changed;
parent XML, notices, units/context and exact upstream result lineage are retained.
It starts no engine. See docs/epics/E3-native-scalar-flows.md for the supported subset.

`cellml.py` adds separate immutable CellML 2 admission and trusted offline result
import. It preserves original units/bytes and validates retained model/runtime/driver,
sample grid, output units and hashes. It never executes code. See docs/epics/E5-cellml-models.md.
