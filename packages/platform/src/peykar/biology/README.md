# Biology

Species-aware reference graph and persistent immutable graph snapshots.
`persistent_graph.py` supplies evidence/context/hierarchy checks, filtered discovery
and bounded neighborhood traversal through API/CLI. A sourced human EGF/EGFR example
is separate from kinetic simulation. Ontology import, multiparty reactions,
model/entity mappings and scientific claim reconciliation remain future extensions.

KnowledgeGapService also persists explicit immutable unknowns in the local object
registry, resolves affected model versions, and supports context/severity/model
filters before bounded pagination. Admin API registration, authenticated reads and
the local `knowledge-gaps` CLI share this service. Severity is declared; there is
no automatic gap generation, closure or scientific approval. See docs/knowledge-gaps.md.

See docs/architecture.md, docs/status.md and docs/scientific-contracts.md at repository root.
