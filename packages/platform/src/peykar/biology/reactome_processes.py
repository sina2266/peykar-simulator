"""Bounded offline projection of explicitly pinned Reactome query responses."""

import json
from collections import Counter
from pathlib import Path
from typing import Any, Literal

from peykar_core.contracts import (
    BiologicalProcessNetwork,
    BiologicalReaction,
    MolecularAttribute,
    MolecularLocation,
    MolecularState,
    ProcessModifier,
    ProcessSource,
    ProcessSourceRelation,
    ScientificContext,
    StoichiometricTerm,
)
from peykar_core.errors import InvalidRequest
from peykar_core.hashing import bytes_digest


def read_json(raw: bytes) -> dict[str, Any]:
    def pairs(values: list[tuple[str, Any]]) -> dict[str, Any]:
        result: dict[str, Any] = {}
        for key, value in values:
            if key in result:
                raise ValueError("Duplicate JSON key")
            result[key] = value
        return result

    def nonfinite(value: str) -> None:
        raise ValueError("Nonfinite JSON number")

    if not 0 < len(raw) <= 10 * 1024**2:
        raise InvalidRequest("Reactome query requires 1..10 MiB")
    try:
        result = json.loads(
            raw.decode("utf-8-sig"), object_pairs_hook=pairs, parse_constant=nonfinite
        )
    except (ValueError, UnicodeError, RecursionError) as error:
        raise InvalidRequest("Reactome source requires unambiguous bounded UTF-8 JSON") from error
    if not isinstance(result, dict):
        raise InvalidRequest("Reactome query response must be an object")
    return result


def read_bundle(path: Path) -> tuple[BiologicalProcessNetwork, dict[str, bytes]]:
    catalog = read_json(path.read_bytes())
    if catalog.get("connector") != "reactome_query_v1":
        raise InvalidRequest("Unsupported process bundle connector")
    sources = tuple(ProcessSource.model_validate(s["spec"]) for s in catalog["sources"])
    if not 1 <= len(sources) <= 100:
        raise InvalidRequest("Process bundle requires 1..100 sources")
    raw_sources = {}
    total_bytes = 0
    records: dict[int, dict[str, Any]] = {}
    source_ids = {}
    for source, entry in zip(sources, catalog["sources"], strict=True):
        filename = entry["file"]
        if not isinstance(filename, str) or Path(filename).name != filename:
            raise InvalidRequest("Process source must be a local bundle filename")
        with (path.parent / filename).open("rb") as stream:
            raw = stream.read(10 * 1024**2 + 1)
        total_bytes += len(raw)
        if total_bytes > 64 * 1024**2:
            raise InvalidRequest("Process source bundle exceeds 64 MiB")
        if bytes_digest(raw) != source.raw_digest:
            raise InvalidRequest("Reactome source checksum differs")
        record = read_json(raw)
        db_id = record.get("dbId")
        if type(db_id) is not int or db_id <= 0 or db_id in records:
            raise InvalidRequest("Reactome root identities must be unique positive integers")
        revision = record.get("stIdVersion", f"dbId:{db_id}")
        if revision != source.revision:
            raise InvalidRequest("Reactome source revision differs")
        records[db_id], source_ids[db_id], raw_sources[source.id] = record, source.id, raw

    def resolve(value: Any) -> dict[str, Any]:
        identity = value.get("dbId") if isinstance(value, dict) else value
        if type(identity) is not int or identity not in records:
            raise InvalidRequest(
                "Unresolved Reactome physical entity/activity in selected projection"
            )
        record = records[identity]
        if isinstance(value, dict) and value.get("stIdVersion") != record.get("stIdVersion"):
            raise InvalidRequest("Reactome nested/root versions differ")
        return record

    def state_id(value: Any) -> str:
        return "state." + str(resolve(value)["dbId"])

    def terms(values: Any) -> tuple[StoichiometricTerm, ...]:
        if not isinstance(values, list) or not values or len(values) > 10000:
            raise InvalidRequest("Reactome participants require a bounded nonempty list")
        # Jackson serializes the first object in full and repeated occurrences as dbId.
        # Count every occurrence; deduplicating would lose source stoichiometry.
        counts = Counter(state_id(value) for value in values)
        return tuple(
            StoichiometricTerm(state_id=key, numerator=count)
            for key, count in sorted(counts.items())
        )

    states = []
    classes: dict[str, Literal["protein", "chemical", "complex", "set", "other"]] = {
        "EntityWithAccessionedSequence": "protein",
        "SimpleEntity": "chemical",
        "Complex": "complex",
        "DefinedSet": "set",
    }
    for db_id, record in sorted(records.items()):
        kind = classes.get(str(record.get("schemaClass")))
        if kind is None:
            if record.get("schemaClass") not in ("Reaction", "CatalystActivity"):
                raise InvalidRequest("Unsupported Reactome source class")
            continue
        locations = tuple(
            MolecularLocation(id="GO:" + c["accession"], label=c["displayName"])
            for c in record.get("compartment", [])
        )
        species = record.get("species", [])
        if isinstance(species, dict):
            species = [species]
        taxons = {s["taxId"] for s in species}
        if len(taxons) > 1:
            raise InvalidRequest("Mixed-species physical states are outside this projection")
        molecular = record.get("referenceEntity")
        identity = (
            None if molecular is None else molecular["databaseName"] + ":" + molecular["identifier"]
        )
        attributes = [
            MolecularAttribute(name=key, value=str(record[key]))
            for key in ("startCoordinate", "endCoordinate")
            if key in record
        ]
        attributes.extend(
            MolecularAttribute(
                name="hasModifiedResidue",
                value=(
                    f"Reactome:{r['dbId']} | {r['displayName']} | "
                    f"coordinate={r.get('coordinate', 'unknown')}"
                ),
            )
            for r in record.get("hasModifiedResidue", [])
        )
        states.append(
            MolecularState(
                id="state." + str(db_id),
                label=record["displayName"],
                source_id=source_ids[db_id],
                external_id=record["stIdVersion"],
                kind=kind,
                species="NCBITaxon:" + next(iter(taxons)) if taxons else None,
                locations=locations,
                molecular_identity=identity,
                attributes=tuple(attributes),
                components=terms(record["hasComponent"]) if "hasComponent" in record else (),
                alternatives=tuple(state_id(v) for v in record.get("hasMember", [])),
            )
        )
    reactions = []
    for db_id in catalog["reaction_db_ids"]:
        record = records[db_id]
        if record.get("schemaClass") != "Reaction" or type(record.get("isInferred")) is not bool:
            raise InvalidRequest("Selected event is not a supported explicit reaction")
        species = record.get("species", [])
        if {"NCBITaxon:" + s["taxId"] for s in species} != {catalog["context"]["species"]}:
            raise InvalidRequest("Reactome reaction species differs from declared context")
        modifiers = []
        for value in record.get("catalystActivity", []):
            catalyst = resolve(value)
            modifiers.append(
                ProcessModifier(
                    state_id=state_id(catalyst["physicalEntity"]),
                    role="catalyst",
                    activity="GO:" + catalyst["activity"]["accession"],
                )
            )
        relations = tuple(
            ProcessSourceRelation(
                predicate=key, target="Reactome:" + str(v["dbId"]), label=v["displayName"]
            )
            for key in ("regulatedBy", "regulationReference")
            for v in record.get(key, [])
        )
        evidence = ["https://reactome.org/content/detail/" + record["stIdVersion"]]
        evidence.extend(
            "https://pubmed.ncbi.nlm.nih.gov/" + str(v["pubMedIdentifier"]) + "/"
            for v in record.get("literatureReference", [])
            if "pubMedIdentifier" in v
        )
        reactions.append(
            BiologicalReaction(
                id="reaction." + str(db_id),
                label=record["displayName"],
                source_id=source_ids[db_id],
                external_id=record["stIdVersion"],
                source_class="Reaction",
                inferred=record["isInferred"],
                reversible=record.get("isReversible"),
                inputs=terms(record["input"]),
                outputs=terms(record["output"]),
                modifiers=tuple(modifiers),
                unexpanded_relations=relations,
                evidence=tuple(evidence),
            )
        )
    return BiologicalProcessNetwork(
        id=catalog["id"],
        version=catalog["version"],
        title=catalog["title"],
        context=ScientificContext.model_validate(catalog["context"]),
        sources=sources,
        states=tuple(states),
        reactions=tuple(reactions),
        limitations=tuple(catalog["limitations"]),
    ), raw_sources
