# Data Fabric

Immutable local observation registry and strict scalar CSV ingestion with exact raw
bytes and transformation lineage. Multimodal/remote ingestion requires separate
connector and quality workflows. See docs/data-evaluation.md for the public workflow.

The separate CompuCell scalar importer accepts operator-mapped local delimited
exports with explicit Monte Carlo step/time mapping, seed and source/runtime
declarations. It preserves exact raw bytes and a dedicated immutable lineage;
it does not execute or discover an external native format. See docs/compucell-output.md.

The native vCornea importer additionally supports the inspected four-column cell
counts plus hourly Time, and ten-bin Parquet heights decoded by the recorded
trusted runtime reader. Missing seeds stay unknown; heights stay lattice y
coordinates. Native scalar count extraction pins its original lineage and can
enter repeated-seed protocols only with recorded seed/runtime evidence. No external
code executes during import. See docs/vcornea-native-output.md.

See docs/architecture.md, docs/status.md and docs/scientific-contracts.md at repository root.
