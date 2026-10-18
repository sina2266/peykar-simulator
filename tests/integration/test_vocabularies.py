import json
from concurrent.futures import ThreadPoolExecutor

import pytest
from fastapi.testclient import TestClient

from peykar.api.app import create_app
from peykar.biology.persistent_graph import GraphService
from peykar.biology.vocabularies import VocabularyService
from peykar.infrastructure.database import Database
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    BiologicalEntity,
    BiologicalGraphSpec,
    EntityVocabularyMappingSpec,
    Reference,
    ScientificContext,
    VocabularySourceSpec,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import bytes_digest

RAW = b"""format-version: 1.2
data-version: test/releases/2026-10-17
property_value: terms:license https://creativecommons.org/publicdomain/zero/1.0/

[Term]
id: CL:0001
name: parent cell
namespace: cell

[Term]
id: CL:0002
name: child cell
namespace: cell
alt_id: CL:old
def: "A source-defined cell." [TEST:1]
synonym: "daughter" EXACT []
is_a: CL:0001 ! parent cell
relationship: part_of UBERON:external ! declaration only

[Term]
id: CL:0003
name: obsolete cell
is_obsolete: true
replaced_by: CL:0002
consider: false
consider: http://purl.obolibrary.org/obo/CL_0002
"""
REF = Reference(id="vocabulary.test", version="1.0.0")
CONTEXT = ScientificContext(
    species="NCBITaxon:9606", biological_system="Cell annotation engineering test"
)
ADMIN, READER = "vocabulary-admin-test-at-least-24", "vocabulary-read-test-at-least-24"
AUTH = {"Authorization": "Bearer " + ADMIN}
READ = {"Authorization": "Bearer " + READER}


def spec(raw=RAW, **changes):
    values = dict(
        id=REF.id,
        version=REF.version,
        title="Synthetic OBO source",
        connector="obo",
        domain="cell",
        source_url="https://example.org/release.obo",
        revision="test/releases/2026-10-17",
        raw_digest=bytes_digest(raw),
        license="CC0-1.0",
        attribution=("Synthetic fixture",),
        limitations=("Engineering checks only",),
    )
    return VocabularySourceSpec(**{**values, **changes})


@pytest.fixture
def service(tmp_path):
    svc = SimulationService(tmp_path / "data")
    return VocabularyService(svc.db, svc.artifacts)


def graph(service):
    value = BiologicalGraphSpec(
        id="graph.test",
        version="1.0.0",
        title="Mapping test",
        context=CONTEXT,
        entities=(
            BiologicalEntity(
                id="cell", label="Declared cell", kind="cell", species=CONTEXT.species
            ),
        ),
    )
    GraphService(service.db).register(value)
    return Reference(id=value.id, version=value.version)


def mapping(service, id="mapping.one", term_id="CL:0002", **changes):
    values = dict(
        id=id,
        version="1.0.0",
        graph=graph(service),
        entity_id="cell",
        vocabulary=REF,
        term_id=term_id,
        relation="exact_match",
        context=CONTEXT,
        rationale="Explicit source-identifier annotation, not a biological approval.",
        evidence=("https://example.org/source",),
    )
    return EntityVocabularyMappingSpec(**{**values, **changes})


def test_raw_and_normalized_snapshots_are_immutable_and_reconstructable(service):
    package = service.admit(spec(), RAW)
    assert package.term_count == 3 and package.obsolete_count == 1
    assert service.artifacts.get(package.spec.raw_digest) == RAW
    assert service.admit(spec(), RAW) == package
    assert len(service.search(REF)) == 3
    assert len(service.search(REF, obsolete=False)) == 2
    assert service.search(REF, "%") == ()
    assert service.search(REF, "child")[0].term.id == "CL:0002"
    assert service.search(REF, offset=10) == ()
    assert service.term(REF, "CL:old").matched_alias
    assert service.term(REF, "CL:old").term.id == "CL:0002"
    obsolete = service.term(REF, "CL:0003").term
    assert obsolete.obsolete and obsolete.consider == (
        "false",
        "http://purl.obolibrary.org/obo/CL_0002",
    )
    restored = VocabularyService(service.db, service.artifacts)
    assert restored.get(REF) == package
    with pytest.raises(Conflict):
        service.admit(
            spec(RAW.replace(b"child cell", b"other cell")),
            RAW.replace(b"child cell", b"other cell"),
        )
    assert service.get(REF) == package
    with pytest.raises(NotFound):
        service.term(REF, "CL:absent")


@pytest.mark.parametrize(
    "raw",
    [
        b"",
        b"\xff",
        RAW.replace(b"format-version: 1.2", b"format-version: 2.0"),
        RAW.replace(b"data-version: test/releases/2026-10-17", b""),
        RAW.replace(b"name: child cell", b"name: child cell\nname: duplicate"),
        RAW.replace(b"id: CL:0002", b"id: CL:0001"),
        RAW.replace(b"alt_id: CL:old", b"alt_id: CL:0001"),
        RAW.replace(b"name: parent cell", b"name: parent cell\nis_a: CL:0002"),
        RAW.replace(b"[Term]", b"[Unknown]", 1),
        RAW.replace(b"name: child cell", b"name: bad\x00cell"),
        RAW.replace(b"namespace: cell", b"namespace: cell\nnamespace: other", 1),
        RAW.replace(b"is_obsolete: true", b"is_obsolete: true\nis_obsolete: false"),
    ],
)
def test_rejects_ambiguous_malformed_or_cyclic_sources_without_partial_index(service, raw):
    with pytest.raises(InvalidRequest):
        service.admit(spec(raw), raw)
    assert service.discover() == ()
    with service.db.connection() as db:
        assert db.execute("select count(*) from vocabulary_terms").fetchone()[0] == 0


def test_source_hash_and_provider_revision_are_enforced(service):
    with pytest.raises(InvalidRequest):
        service.admit(spec(raw_digest="a" * 64), RAW)
    with pytest.raises(InvalidRequest):
        service.admit(spec(revision="latest"), RAW)
    assert service.discover() == ()


def test_index_corruption_and_alias_tampering_are_detected(service):
    service.admit(spec(), RAW)
    with service.db.connection() as db:
        db.execute("UPDATE vocabulary_aliases SET term_id='CL:0001'")
    with pytest.raises(Conflict):
        service.term(REF, "CL:old")
    with service.db.connection() as db:
        db.execute("UPDATE vocabulary_terms SET label='tampered' WHERE term_id='CL:0002'")
    with pytest.raises(Conflict):
        service.search(REF, "tampered")
    with service.db.connection() as db:
        db.execute("DELETE FROM vocabulary_terms WHERE term_id='CL:0002'")
    with pytest.raises(Conflict):
        service.get(REF)


def test_concurrent_mappings_detect_competing_declarations_without_approving_either(service):
    service.admit(spec(), RAW)
    first = mapping(service, id="mapping.first", term_id="CL:0001")
    second = mapping(service, id="mapping.second", term_id="CL:0002")
    with ThreadPoolExecutor(max_workers=2) as pool:
        records = list(pool.map(service.map_entity, (first, second)))
    assert all(r.approval == "unreviewed_declaration" for r in records)
    conflicts = service.conflicts()
    assert len(conflicts) == 1
    assert conflicts[0].resolution == "unreviewed"
    assert {r.id for r in conflicts[0].mappings} == {"mapping.first", "mapping.second"}
    service.map_entity(first)
    service.map_entity(second)
    assert len(service.conflicts()) == 1
    assert len(service.mappings()) == 2
    with pytest.raises(Conflict):
        service.map_entity(first.model_copy(update={"rationale": "changed"}))


@pytest.mark.parametrize(
    "change",
    [
        {"term_id": "CL:old"},
        {"term_id": "CL:0003"},
        {"entity_id": "missing"},
        {"context": ScientificContext(species="NCBITaxon:10090", biological_system="Other")},
    ],
)
def test_mapping_rejects_alias_obsolete_unknown_or_incompatible_context(service, change):
    service.admit(spec(), RAW)
    with pytest.raises(InvalidRequest):
        service.map_entity(mapping(service, **change))
    assert service.mappings() == ()


def test_http_reads_and_annotation_writes_use_existing_roles_and_bounds(tmp_path):
    with TestClient(create_app(tmp_path / "api", ADMIN, READER)) as client:
        svc = client.app.state.service
        v = VocabularyService(svc.db, svc.artifacts)
        v.admit(spec(), RAW)
        assert client.get("/v1/biology/vocabularies").status_code == 401
        root = "/v1/biology/vocabularies/vocabulary.test/versions/1.0.0"
        assert client.get(root, headers=READ).json()["term_count"] == 3
        assert client.get(root + "/artifacts/raw", headers=READ).content == RAW
        assert client.get(root + "/term?term_id=CL:old", headers=READ).json()["matched_alias"]
        assert client.get(root + "/terms?limit=51", headers=READ).status_code == 422
        assert client.get(root + "/terms?offset=-1", headers=READ).status_code == 422
        assert (
            client.get(root + "/terms?obsolete=false", headers=READ).json()[1]["term"]["id"]
            == "CL:0002"
        )
        body = mapping(v).model_dump(mode="json")
        assert (
            client.post("/v1/biology/vocabulary-mappings", headers=READ, json=body).status_code
            == 403
        )
        assert (
            client.post("/v1/biology/vocabulary-mappings", headers=AUTH, json=body).status_code
            == 201
        )
        assert (
            client.get("/v1/biology/vocabulary-mappings", headers=READ).json()[0]["approval"]
            == "unreviewed_declaration"
        )
        assert client.get("/v1/biology/vocabulary-mapping-conflicts", headers=READ).json() == []
        assert client.post("/v1/biology/vocabularies", headers=AUTH, json={}).status_code == 405


def test_json_connectors_preserve_gene_protein_pathway_and_taxon_distinctions(service):
    protein = {
        "primaryAccession": "P00533",
        "secondaryAccessions": ["QOLD"],
        "proteinDescription": {"recommendedName": {"fullName": {"value": "EGFR protein"}}},
        "organism": {"taxonId": 9606},
        "entryAudit": {
            "entryVersion": 301,
            "sequenceVersion": 2,
            "lastAnnotationUpdateDate": "2026-09-02",
        },
    }
    raw = json.dumps(protein).encode()
    s = spec(raw, id="protein.test", connector="uniprot_json", domain="protein", revision="301")
    service.admit(s, raw)
    r = service.term(Reference(id=s.id, version=s.version), "UniProtKB:QOLD")
    assert (
        r.matched_alias and r.term.id == "UniProtKB:P00533" and r.term.species == "NCBITaxon:9606"
    )
    with pytest.raises(InvalidRequest):
        service.map_entity(
            mapping(
                service,
                vocabulary=Reference(id=s.id, version=s.version),
                term_id="UniProtKB:P00533",
            )
        )
    mouse = CONTEXT.model_copy(update={"species": "NCBITaxon:10090"})
    g = BiologicalGraphSpec(
        id="graph.mouse",
        version="1.0.0",
        title="Explicit mouse context",
        context=mouse,
        entities=(
            BiologicalEntity(
                id="protein", label="Test protein", kind="protein", species=mouse.species
            ),
        ),
    )
    GraphService(service.db).register(g)
    with pytest.raises(InvalidRequest, match="species"):
        service.map_entity(
            mapping(
                service,
                graph=Reference(id=g.id, version=g.version),
                entity_id="protein",
                context=mouse,
                vocabulary=Reference(id=s.id, version=s.version),
                term_id="UniProtKB:P00533",
            )
        )
    hgnc = {
        "response": {
            "numFound": 1,
            "docs": [
                {
                    "hgnc_id": "HGNC:3236",
                    "symbol": "EGFR",
                    "name": "epidermal growth factor receptor",
                    "status": "Approved",
                    "date_modified": "2023-03-15",
                    "uniprot_ids": ["P00533"],
                }
            ],
        }
    }
    raw = json.dumps(hgnc).encode()
    s = spec(
        raw, id="gene.test", connector="hgnc_json", domain="gene", revision="response-snapshot"
    )
    service.admit(s, raw)
    r = service.term(Reference(id=s.id, version=s.version), "HGNC:3236")
    assert r.term.id != "UniProtKB:P00533" and r.term.xrefs == ("UniProtKB:P00533",)
    hgnc["response"]["numFound"] = 2
    raw = json.dumps(hgnc).encode()
    with pytest.raises(InvalidRequest):
        service.admit(spec(raw, id="gene.truncated", connector="hgnc_json", domain="gene"), raw)
    pathway = {
        "schemaClass": "Pathway",
        "stId": "R-HSA-1",
        "stIdVersion": "R-HSA-1.2",
        "displayName": "Pathway",
        "species": [{"taxId": 9606, "displayName": "Homo sapiens"}],
    }
    raw = json.dumps(pathway).encode()
    s = spec(
        raw, id="pathway.test", connector="reactome_json", domain="pathway", revision="R-HSA-1.2"
    )
    service.admit(s, raw)
    assert (
        service.term(Reference(id=s.id, version=s.version), "Reactome:R-HSA-1").term.species
        == "NCBITaxon:9606"
    )


@pytest.mark.parametrize(
    "raw", [b'{"response":{},"response":{}}', b'{"x":NaN}', b"[]", b'{"schemaClass":"Reaction"}']
)
def test_json_ambiguity_and_unsupported_records_are_rejected(service, raw):
    with pytest.raises(InvalidRequest):
        service.admit(spec(raw, connector="reactome_json", domain="pathway"), raw)
    assert service.discover() == ()


def test_conflict_creation_failure_rolls_back_the_new_mapping(service, monkeypatch):
    service.admit(spec(), RAW)
    service.map_entity(mapping(service, id="mapping.first", term_id="CL:0001"))
    second = mapping(service, id="mapping.second", term_id="CL:0002")
    original = Database.register_in_transaction

    def fail(connection, kind, id, version, payload):
        if kind == "vocabulary_mapping_conflict":
            raise RuntimeError("injected conflict persistence failure")
        return original(connection, kind, id, version, payload)

    monkeypatch.setattr(Database, "register_in_transaction", staticmethod(fail))
    with pytest.raises(RuntimeError):
        service.map_entity(second)
    assert [r.spec.id for r in service.mappings()] == ["mapping.first"]
    assert service.conflicts() == ()


def test_alias_index_loss_is_not_silently_repaired_by_reimport(service):
    service.admit(spec(), RAW)
    with service.db.connection() as db:
        db.execute("DELETE FROM vocabulary_aliases")
    with pytest.raises(Conflict, match="alias"):
        service.get(REF)
    with pytest.raises(Conflict, match="alias"):
        service.admit(spec(), RAW)


def test_obo_qualified_obsolete_boolean_retains_status_and_original_provenance(service):
    raw = RAW.replace(b"is_obsolete: true", b'is_obsolete: true {source="doi:10.1002/ca.23705"}')
    package = service.admit(spec(raw), raw)
    term = service.term(REF, "CL:0003").term
    assert package.obsolete_count == 1 and term.obsolete
    assert term.obsolete_declaration == 'true {source="doi:10.1002/ca.23705"}'
    with pytest.raises(InvalidRequest):
        service.map_entity(mapping(service, term_id=term.id))
