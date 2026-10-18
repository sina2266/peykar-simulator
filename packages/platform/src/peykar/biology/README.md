# Biology

Species-aware reference graph and persistent immutable graph snapshots.
`persistent_graph.py` supplies evidence/context/hierarchy checks, filtered discovery
and bounded neighborhood traversal through API/CLI. A sourced human EGF/EGFR example
is separate from kinetic simulation. E7 adds multiparty process/state declarations below; executable biological
model/entity correspondence and scientific claim reconciliation remain future
extensions. E6 adds vocabulary/identifier admission and unreviewed annotations.

KnowledgeGapService also persists explicit immutable unknowns in the local object
registry, resolves affected model versions, and supports context/severity/model
filters before bounded pagination. Admin API registration, authenticated reads and
the local `knowledge-gaps` CLI share this service. Severity is declared; there is
no automatic gap generation, closure or scientific approval. See docs/knowledge-gaps.md.

See docs/architecture.md, docs/status.md and docs/scientific-contracts.md at repository root.

`vocabulary_import.py` reads bounded pinned OBO/HGNC/UniProt/Reactome data without
remote fetching/code execution. `vocabularies.py` atomically admits immutable
source/term/alias snapshots, supports indexed reads and retains contextual unreviewed
annotations/competing declarations. See docs/epics/E6-biological-vocabularies.md.
This adds identifier provenance, not ontology reasoning or biological approval.

`reactome_processes.py` projects explicitly pinned local query records with original
raw bytes; `processes.py` validates immutable molecular states/reactions, exact
rational matrices, frozen model ownership/exclusions and original-unit sample
frames. Read docs/epics/E7-multiparty-processes.md before extending. None of these
declarations starts a solver or grants biological/mechanistic approval.
