# E6 — pinned biological vocabularies and contextual identifier annotations

Delivered bounded S2.4 across anatomy, cell, gene, protein, pathway and GO aspects:
offline source admission, indexed exact/search queries, original-version/alias/
obsolete provenance, contextual graph annotations and immutable competing-mapping
records. The workbench exposes these operations in Biological Explorer.

## Actual sources and scope

The [observed verification receipt](../vocabulary-verification-2026-10-18.json)
records actual source, annotation, browser and preservation checks.

The reviewed [catalog](../../biological_libraries/vocabularies/sources.json) pins
every original response/file by SHA-256, source URL, revision, license, attribution
and explicit limitations. The implemented source families are:

| Source snapshot | Terms | Obsolete | Original declared scope |
|---|---:|---:|---|
| CL basic 2026-06-08 | 3,540 | 205 | Complete original basic release |
| Uberon basic 2026-10-01 | 16,141 | 1,096 | Complete original basic release |
| GO basic 2026-01-23 | 48,251 | 9,512 | Complete original basic release |
| HGNC EGF / EGFR responses | 2 | 0 | Two human gene identifier entries |
| UniProtKB P01133 / P00533 | 2 | 0 | Two protein entries, entry versions 250 / 301 |
| Reactome R-HSA-177929.6 | 1 | 0 | Original EGFR pathway identifier response |

Total: **8 snapshots, 67,937 terms/identifiers, 4,030 declared aliases, 10,813
obsolete terms**. Entry subsets are not complete HGNC, UniProt or Reactome releases.
The earlier UniProt/Reactome response bytes are reused with their original hashes;
new ontology/HGNC acquisition never overwrites existing evidence.

OBO 1.2 readers retain term IDs, labels, original definition/synonym/xref text,
aliases, declared `is_a` parents, relationship declarations and obsolete/replacement
annotations. Exact source bytes and normalized JSONL are stored separately.
Headers retain the original data-version/license declarations. Typedef/Instance
axioms and other annotations remain in the raw file; this is **not OWL reasoning,
import resolution, taxon-constraint inference or a complete OBO reasoner**.
External URLs, imports and linked media are never fetched by admission.

Malformed/ambiguous IDs, aliases, duplicate JSON keys, unsupported source records,
changed source checksums/revisions and cyclic `is_a` declarations fail admission.
Qualified obsolete flags retain their original provenance text and parsed boolean.
Two Uberon terms with a DOI modifier were initially misclassified as current;
real-source replay exposed and corrected that error before publication.
Replacement annotations can contain provider IRIs or unusual literal values;
these remain verbatim and never become automatic redirects. Exact reads explicitly
disclose an alias match. Source context does not silently assign a species to
ontology-wide classes. HGNC entries are human; UniProt/Reactome taxon metadata is
retained, without inventing a single taxon for mixed-species pathway records.

## Admission and queries

```sh
uv run python scripts/acquire_vocabularies.py
uv run python scripts/verify_vocabularies.py --output .peykar/my-vocabulary-verification
```

Use a **new** verification output directory. Acquisition is an explicit operator
network action; it verifies existing files, refuses overwrite and rejects changed
provider bytes. Providers may not preserve byte-identical historical entry
responses, so retained raw evidence is essential. The replay itself uses only
local acquired data and creates a separate workspace with actual source/annotation
checks. No scientific software is downloaded or executed.

Individual operator admission uses a `VocabularySourceSpec` JSON and local source:

```sh
uv run peykar vocabularies admit SOURCE_SPEC.json LOCAL_SOURCE.obo
uv run peykar vocabularies list
uv run peykar vocabularies search vocabulary.cl 1.0.0 --query epithelial
uv run peykar vocabularies term vocabulary.cl 1.0.0 CL:0000066
uv run peykar vocabularies map-entity MAPPING_SPEC.json
uv run peykar vocabularies conflicts
```

`PEYKAR_DATA_DIR` selects the workspace. The source file is limited to 64 MiB,
normalized JSONL to 64 MiB and each snapshot to 100,000 terms. Additive
`006_vocabularies.sql` creates term/alias indexes; source metadata plus all indexed
rows are admitted in one SQLite transaction. CAS bytes may remain after rollback.
Re-imports cannot replace an immutable snapshot or silently repair its index.
Metadata checks term/alias counts; each returned row checks its canonical digest
and indexed fields. Raw/normalized artifact reads rehash their original bytes.
These are integrity checks, not signatures or protection against a database owner
who can rewrite both data and hashes.

Authenticated read endpoints:

- `/v1/biology/vocabularies` and `/{id}/versions/{version}`
- `/{id}/versions/{version}/terms?q=epithelial&obsolete=false`
- `/{id}/versions/{version}/term?term_id=CL:0000066`
- `/{id}/versions/{version}/artifacts/{raw|normalized}`
- `/v1/biology/vocabulary-mappings` and `/{id}/versions/{version}`
- `/v1/biology/vocabulary-mapping-conflicts`

The abbreviated term/artifact paths above extend `/v1/biology/vocabularies`.
Discovery/search/annotation lists default to 20 and cap at 50, with explicit
offsets. Search treats wildcard characters literally. Source admission remains
operator-only; the API never fetches remote ontologies. Administrator
`POST /v1/biology/vocabulary-mappings` registers annotations; readers cannot write.

## Annotation and conflict semantics

An annotation freezes the exact graph/entity/context, vocabulary snapshot,
canonical current target, graph/source/term digests, relation, rationale and
evidence. Species mismatches are rejected when the source declares a taxon.
Exact annotations additionally require compatible declared entity kinds,
supported domain identifier namespaces and the original GO aspect. An alias or
obsolete term must be reviewed explicitly; it is not silently replaced.

All annotations have `approval=unreviewed_declaration`. Two different exact
targets from the same identifier namespace for one graph/entity/context create
an immutable `competing_exact_mappings` record with `resolution=unreviewed`.
Creation is serialized and atomic with the new annotation, including concurrent
requests. Both declarations remain; neither source, graph or annotation is edited,
approved, removed or made executable. Biological claim review, approval authority,
reconciliation, model-version annotations and executable bridges remain S2.6/S5/S11.

The [human identifier example](../../biological_libraries/vocabularies/egfr-annotations-graph.json)
contains separate gene, whole precursor protein, receptor, mature ligand, pathway,
cell/organ/process class entities. Its [nine declarations](../../biological_libraries/vocabularies/egfr-annotations.json)
preserve the distinction between HGNC gene IDs and UniProt protein IDs. Mature EGF
has only a qualified related link to the Pro-EGF accession. This is an annotation
example with no kinetic or tissue coupling, experimental evidence or scientific
approval. Deliberately contradictory test declarations are confined to the
separate verification workspace and excluded from production source admission.

## Workbench

Biological Explorer now separates graph snapshots, vocabulary terms and entity
annotations. Users can inspect release/license/provenance, filter current/obsolete
terms, resolve an exact ID/alias, traverse declared parents within the pinned
snapshot and download original bytes/normalized terms. A structured annotation
form uses actual graph entities, source snapshots, explicit relation/rationale/
evidence and frozen context. Annotation/conflict lists disclose unreviewed state.
Existing loading/empty/error/retry/forbidden/conflict handling, responsive layouts,
keyboard access and Persian RTL apply. No source label is presented as an approved
model mechanism or biological equivalence.

Primary source documentation: [CL](https://obofoundry.org/ontology/cl.html),
[Uberon](https://obofoundry.org/ontology/uberon.html),
[GO releases](https://www.geneontology.org/docs/download-ontology/),
[HGNC](https://www.genenames.org/), [UniProt API](https://www.uniprot.org/api-documentation),
[Reactome data](https://reactome.org/download-data/).
