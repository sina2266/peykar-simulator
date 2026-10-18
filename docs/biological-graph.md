# Persistent biological graph snapshots

`BiologicalGraphSpec` stores a versioned context, entities and evidence-linked
relationships in the local immutable registry. This extends the original in-memory
reference graph. Entity/model/runtime-instance identity remains distinct.

Registration rejects duplicate IDs, absent endpoints, blank or oversized evidence,
species/context mismatches and cycles in `is_a`/`part_of`. Hierarchy validation runs
once per relation over the full snapshot, rather than rebuilding it after every edge.
A snapshot supports at most 5,000 entities and 20,000 relationships. This is a local
profile, not a full ontology server or automatically inferred biological knowledge.

A committed [human EGF/EGFR example](../biological_libraries/examples/human-egfr-graph.json)
was curated from inspected [Reactome EGFR signaling](https://reactome.org/content/detail/R-HSA-177929)
and UniProt [P01133](https://www.uniprot.org/uniprotkb/P01133/entry)/
[P00533](https://www.uniprot.org/uniprotkb/P00533/entry). Its
[evidence receipt](../biological_libraries/examples/human-egfr-evidence.json) pins the
retrieved source JSON hashes. It records a ligand/receptor interaction and pathway
regulation, not tissue-specific kinetic parameters. The EGF accession includes a
precursor; the graph explicitly labels the mature-ligand distinction.

```bash
uv run peykar biology register biological_libraries/examples/human-egfr-graph.json
uv run peykar biology show biology.human-egfr 1.0.0
```

| Method | Path | Role |
|---|---|---|
| POST | `/v1/biology/graphs` | Admin; immutable register |
| GET | `/v1/biology/graphs?species=NCBITaxon:9606` | Read; exact context species |
| GET | `/v1/biology/graphs/{id}/versions/{version}` | Read |
| GET | `/v1/biology/graphs/{id}/versions/{version}/neighborhood?entity_id=egf&depth=2` | Read |

Neighborhood traversal is explicitly undirected; original relationship direction,
type, context and evidence are retained in the returned subgraph. Depth is 1–4,
maximum nodes 1–500. A truncated neighborhood says so; ordering is deterministic.
Adjacency does not execute a mechanism or justify cross-scale causality. Ontology
synchronization, multiparty reactions, scientific claim/conflict resolution and
model-to-entity mappings remain the next graph extensions.

E6 now adds [pinned vocabulary and identifier snapshots](epics/E6-biological-vocabularies.md),
indexed current/obsolete/alias reads and immutable contextual graph-entity
annotations with competing-declaration provenance. Earlier graph versions and
legacy ontology_id strings remain unchanged. These annotations remain unreviewed;
no ontology reasoning, mechanism execution or independent claim approval is inferred.
