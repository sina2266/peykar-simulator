"""Replay the actual pinned ontology/identifier sources and contextual annotations.

Reads already acquired data. No network, scientific code execution or production
volume access; deliberate conflicting annotations live only in this new workspace.
"""

import argparse
import json
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from peykar.biology.persistent_graph import GraphService
from peykar.biology.vocabularies import VocabularyService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    BiologicalEntity,
    BiologicalGraphSpec,
    EntityVocabularyMappingSpec,
    Reference,
    VocabularySourceSpec,
)
from peykar_core.errors import InvalidRequest
from peykar_core.hashing import bytes_digest

ROOT = Path(__file__).resolve().parents[1]


def verify(output: Path, sources: Path) -> dict:
    output.mkdir(parents=True, exist_ok=False)
    svc = SimulationService(output / "data")
    service = VocabularyService(svc.db, svc.artifacts)
    directory = ROOT / "biological_libraries/vocabularies"
    packages = []
    for source in json.loads((directory / "sources.json").read_bytes()):
        spec = VocabularySourceSpec.model_validate(source["spec"])
        with (sources / source["file"]).open("rb") as stream:
            raw = stream.read(64 * 1024**2 + 1)
        package = service.admit(spec, raw)
        assert bytes_digest(svc.artifacts.get(spec.raw_digest)) == spec.raw_digest
        assert (
            bytes_digest(svc.artifacts.get(package.normalized_digest)) == package.normalized_digest
        )
        assert service.admit(spec, raw) == package
        packages.append(package.model_dump(mode="json"))
        print(
            json.dumps(
                {
                    "snapshot": spec.id,
                    "terms": package.term_count,
                    "aliases": package.alias_count,
                    "obsolete": package.obsolete_count,
                }
            ),
            flush=True,
        )
    graph = BiologicalGraphSpec.model_validate_json(
        (directory / "egfr-annotations-graph.json").read_bytes()
    )
    GraphService(svc.db).register(graph)
    declared = []
    annotation_specs = tuple(
        EntityVocabularyMappingSpec.model_validate(value)
        for value in json.loads((directory / "egfr-annotations.json").read_bytes())
    )
    for spec in annotation_specs:
        record = service.map_entity(spec)
        assert service.mapping(Reference(id=spec.id, version=spec.version)) == record
        assert record.approval == "unreviewed_declaration"
        declared.append(record.model_dump(mode="json"))
    assert not service.conflicts()
    rejected = []
    prototype = next(spec for spec in annotation_specs if spec.entity_id == "egfr-protein")
    # A gene identifier is not an exact protein identity; mature ligand is not the precursor.
    for label, changes in (
        (
            "gene-as-protein",
            {
                "id": "negative.gene-protein",
                "vocabulary": Reference(id="vocabulary.hgnc.egfr", version="1.0.0"),
                "term_id": "HGNC:3236",
            },
        ),
        (
            "mature-as-precursor",
            {
                "id": "negative.mature-precursor",
                "entity_id": "mature-egf",
                "vocabulary": Reference(id="vocabulary.uniprot.egf", version="1.0.0"),
                "term_id": "UniProtKB:P01133",
            },
        ),
    ):
        try:
            service.map_entity(prototype.model_copy(update=changes))
        except InvalidRequest:
            rejected.append(label)
        else:
            raise AssertionError("Incompatible exact identifier mapping was admitted")
    go = Reference(id="vocabulary.go", version="1.0.0")
    obsolete = service.search(go, obsolete=True, limit=1)[0]
    try:
        service.map_entity(
            prototype.model_copy(
                update={
                    "id": "negative.obsolete",
                    "entity_id": "signal-transduction",
                    "vocabulary": go,
                    "term_id": obsolete.term.id,
                }
            )
        )
    except InvalidRequest:
        rejected.append("obsolete-canonical-target")
    else:
        raise AssertionError("An obsolete target was silently remapped")
    # Deliberately contradictory annotations are engineering fixtures, not curated facts.
    context = graph.context.model_copy(
        update={"biological_system": "Synthetic competing annotations; no scientific assertion"}
    )
    fixture = BiologicalGraphSpec(
        id="biology.annotation-conflict-fixture",
        version="1.0.0",
        title="Synthetic conflict-injection subject",
        context=context,
        entities=(
            BiologicalEntity(
                id="test-protein",
                label="Synthetic annotation test subject",
                kind="protein",
                species=context.species,
            ),
        ),
    )
    GraphService(svc.db).register(fixture)
    cases = []
    for name, target, term in [
        ("one", "vocabulary.uniprot.egfr", "UniProtKB:P00533"),
        ("two", "vocabulary.uniprot.egf", "UniProtKB:P01133"),
    ]:
        cases.append(
            EntityVocabularyMappingSpec(
                id="fixture.conflicting." + name,
                version="1.0.0",
                graph=Reference(id=fixture.id, version=fixture.version),
                entity_id="test-protein",
                vocabulary=Reference(id=target, version="1.0.0"),
                term_id=term,
                relation="exact_match",
                context=context,
                rationale="Deliberate engineering conflict; not a biological identity claim.",
                evidence=("Peykar conflict-injection verification fixture",),
            )
        )
    with ThreadPoolExecutor(max_workers=2) as pool:
        list(pool.map(service.map_entity, cases))
    conflicts = service.conflicts()
    assert len(conflicts) == 1 and conflicts[0].resolution == "unreviewed"
    reopened = VocabularyService(svc.db, svc.artifacts)
    assert len(reopened.mappings()) == 11 and len(reopened.conflicts()) == 1
    summary = {
        "status": "passed",
        "packages": packages,
        "term_count": sum(p["term_count"] for p in packages),
        "curated_annotation_declarations": declared,
        "rejected_mapping_cases": rejected,
        "synthetic_conflicts": [c.model_dump(mode="json") for c in conflicts],
        "interpretation": "Unreviewed declarations; no biological validation or execution",
    }
    (output / "verification.json").write_text(json.dumps(summary, indent=2) + "\n")
    return summary


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--sources", type=Path, default=ROOT / ".peykar/external/vocabularies")
    args = parser.parse_args()
    summary = verify(args.output, args.sources)
    print(
        json.dumps(
            {
                "status": "passed",
                "snapshots": len(summary["packages"]),
                "terms": summary["term_count"],
                "annotations": len(summary["curated_annotation_declarations"]),
                "synthetic_conflicts": len(summary["synthetic_conflicts"]),
            }
        )
    )


if __name__ == "__main__":
    main()
