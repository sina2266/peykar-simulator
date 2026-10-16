# CompuCell3D integration

Status: **working operator integration**, verified on 2026-10-16. The archived
vCornea source is licensed and pinned; a separately locked CompuCell3D 4.7.0 image
executes private copies with explicit resource/seed/horizon controls. Native
cell-count and thickness imports, frozen seed comparisons and authenticated output
retrieval are implemented. The general API worker does not execute this model.

See [EPIC E1](../../docs/epics/E1-vcornea-native-runtime.md),
[runtime](runtime/README.md), [native import](../../docs/vcornea-native-output.md)
and [mapped scalar import](../../docs/compucell-output.md). Full published-scenario
reproduction and independent biological validation remain open.

Official project/documentation: https://compucell3d.org/

See docs/vcornea-integration.md. The upstream vCornea README requests version 4.7.0.

Follow ../README.md before enabling an adapter. No license or interoperability guarantee is inferred from this link.
