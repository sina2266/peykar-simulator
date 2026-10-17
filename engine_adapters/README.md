# Engine adapter integrations

Execution protocol: packages/core/src/peykar_core/protocols.py.
Working reference adapters: packages/engines/src/peykar_engines/reference.py.

CompuCell3D now has an executable, separately built operator profile for the pinned
vCornea archive; see [EPIC E1](../docs/epics/E1-vcornea-native-runtime.md).
It is not registered as an engine in the general API/worker queue. Other folders
remain designed integrations. Every admission
needs a verified license, pinned source/container digest, capabilities, units,
state/time mapping, native seed/checkpoint behavior, resource limits and a frozen
reference benchmark. Do not turn optional imports into fake successful results.
Heavy scientific dependencies remain in engine-specific environments.

All eight specialist repositories named in the original architecture, plus
CompuCell3D, now have pinned local source snapshots and immutable inspection
receipts; see [source ecosystem](../docs/library-ecosystem.md). Source availability
does not change execution-adapter readiness. Changes belong in separate versioned
patches with measured regression, preserving pristine upstream copies.
