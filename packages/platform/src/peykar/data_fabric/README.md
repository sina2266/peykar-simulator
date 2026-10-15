# Data Fabric

Immutable local observation registry and strict scalar CSV ingestion with exact raw
bytes and transformation lineage. Multimodal/remote ingestion requires separate
connector and quality workflows. See docs/data-evaluation.md for the public workflow.

The separate CompuCell scalar importer accepts operator-mapped local delimited
exports with explicit Monte Carlo step/time mapping, seed and source/runtime
declarations. It preserves exact raw bytes and a dedicated immutable lineage;
it does not execute or discover an external native format. See docs/compucell-output.md.

See docs/architecture.md, docs/status.md and docs/scientific-contracts.md at repository root.
