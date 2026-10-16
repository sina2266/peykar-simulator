# Biology

Species-aware reference graph. Persistent ontology import and large graph queries are future extensions.

KnowledgeGapService also persists explicit immutable unknowns in the local object
registry, resolves affected model versions, and supports context/severity/model
filters before bounded pagination. Admin API registration, authenticated reads and
the local `knowledge-gaps` CLI share this service. Severity is declared; there is
no automatic gap generation, closure or scientific approval. See docs/knowledge-gaps.md.

See docs/architecture.md, docs/status.md and docs/scientific-contracts.md at repository root.
