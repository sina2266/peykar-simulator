# Pinned vocabulary and identifier sources

Read [E6 scope and operations](../../docs/epics/E6-biological-vocabularies.md) before
extending these connectors. `sources.json` pins original data bytes, release/entry
revision, attribution, license and limitations. Raw data is acquired explicitly
into `.peykar/external/vocabularies` and retained in the selected artifact store.
These files are data, not automatically installed scientific code or executable
biological bridges. Source updates require new reviewed immutable versions.

CL/Uberon/GO include their full original basic releases; HGNC/UniProt/Reactome
provide explicitly bounded original EGFR-related identifier entries. The graph
and nine annotation specifications distinguish gene/protein/precursor/mature
ligand/pathway/class identities. Every annotation remains unreviewed. Exact source
bytes, headers and normalized declarations remain separate; no imports, linked
media, taxon reasoning or replacement redirects are executed automatically.
