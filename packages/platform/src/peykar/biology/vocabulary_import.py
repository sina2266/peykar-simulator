"""Bounded declarative vocabulary readers. No imports, URLs or code are executed."""

import io
import json
from collections import defaultdict
from typing import Any

from pydantic import ValidationError

from peykar_core.contracts import VocabularyRelation, VocabularySourceSpec, VocabularyTerm
from peykar_core.errors import InvalidRequest
from peykar_core.hashing import bytes_digest

MAX_VOCABULARY_BYTES = 64 * 1024**2


def _obo(raw: bytes) -> tuple[tuple[VocabularyTerm, ...], dict[str, tuple[str, ...]]]:
    header: dict[str, list[str]] = defaultdict(list)
    fields: dict[str, list[str]] = defaultdict(list)
    terms = []
    stanza = "header"

    def finish() -> None:
        if stanza != "Term":
            return
        if len(fields["id"]) != 1 or len(fields["name"]) != 1 or len(fields["def"]) > 1:
            raise InvalidRequest("OBO terms require one ID/name and at most one definition")
        if (
            len(fields.get("namespace", [])) > 1
            or len(fields.get("is_obsolete", [])) > 1
            or any(
                value.split(maxsplit=1)[0] not in ("true", "false")
                for value in fields.get("is_obsolete", [])
            )
        ):
            raise InvalidRequest("OBO namespace/obsolete declarations must be unambiguous")
        terms.append(
            VocabularyTerm(
                id=fields["id"][0],
                label=fields["name"][0],
                namespace=fields.get("namespace", [""])[0],
                definition=fields["def"][0] if fields["def"] else None,
                aliases=tuple(fields["alt_id"]),
                synonyms=tuple(fields["synonym"]),
                xrefs=tuple(fields["xref"]),
                parents=tuple(value.split()[0] for value in fields["is_a"]),
                relationships=tuple(
                    VocabularyRelation(
                        relation=value.split(maxsplit=1)[0], target=value.split(maxsplit=1)[1]
                    )
                    for value in fields["relationship"]
                ),
                obsolete=fields.get("is_obsolete", ["false"])[0].split(maxsplit=1)[0] == "true",
                obsolete_declaration=fields.get("is_obsolete", [None])[0],
                replaced_by=tuple(fields["replaced_by"]),
                consider=tuple(fields["consider"]),
            )
        )
        if len(terms) > 100000:
            raise InvalidRequest("Vocabulary exceeds the 100000-term local profile")

    with io.TextIOWrapper(io.BytesIO(raw), encoding="utf-8-sig", errors="strict") as stream:
        for source in stream:
            if len(source) > 65536 or "\x00" in source:
                raise InvalidRequest("OBO line exceeds the bounded text profile")
            line = source.strip()
            if not line or line.startswith("!"):
                continue
            if line.startswith("["):
                finish()
                if line not in ("[Term]", "[Typedef]", "[Instance]"):
                    raise InvalidRequest("Unsupported OBO stanza")
                stanza = line[1:-1]
                fields = defaultdict(list)
            else:
                key, separator, value = line.partition(": ")
                if not separator or not key or not value or line.endswith("\\"):
                    raise InvalidRequest("Unsupported or malformed OBO field")
                (header if stanza == "header" else fields)[key].append(value)
        finish()
    if header.get("format-version") != ["1.2"] or len(header.get("data-version", [])) != 1:
        raise InvalidRequest("OBO import requires format 1.2 and one pinned data-version")
    header["peykar:normalized_scope"] = [
        "Term declarations only; no Typedef/Instance execution, remote imports or OWL reasoning."
    ]
    return tuple(terms), {key: tuple(values) for key, values in header.items()}


def _unique_json(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    values: dict[str, Any] = {}
    for key, value in pairs:
        if key in values:
            raise ValueError("Duplicate JSON key")
        values[key] = value
    return values


def _json(
    spec: VocabularySourceSpec, raw: bytes
) -> tuple[tuple[VocabularyTerm, ...], dict[str, tuple[str, ...]]]:
    data = json.loads(
        raw.decode("utf-8-sig"),
        object_pairs_hook=_unique_json,
        parse_constant=lambda value: (_ for _ in ()).throw(ValueError(value)),
    )
    if spec.connector == "hgnc_json":
        docs = data["response"]["docs"]
        if data["response"]["numFound"] != len(docs):
            raise InvalidRequest(
                "HGNC paginated/truncated responses require explicit complete source data"
            )
        terms = tuple(
            VocabularyTerm(
                id=d["hgnc_id"],
                label=d["symbol"],
                namespace="HGNC",
                species="NCBITaxon:9606",
                definition=d["name"],
                obsolete=d["status"] != "Approved",
                synonyms=tuple(
                    f"{key}: {value}"
                    for key in ("alias_symbol", "alias_name", "prev_symbol", "prev_name")
                    for value in d.get(key, [])
                ),
                xrefs=tuple("UniProtKB:" + value for value in d.get("uniprot_ids", []))
                + tuple("Ensembl:" + d[key] for key in ("ensembl_gene_id",) if d.get(key)),
            )
            for d in docs
        )
        return terms, {
            "entry_modified_dates": tuple(sorted({d["date_modified"] for d in docs})),
            "revision_scope": ("checksum-pinned HGNC response; not a full database release",),
        }
    if spec.connector == "uniprot_json":
        names = data["proteinDescription"].get("recommendedName", {}).get("fullName", {})
        term = VocabularyTerm(
            id="UniProtKB:" + data["primaryAccession"],
            label=names["value"],
            namespace="UniProtKB",
            species="NCBITaxon:" + str(data["organism"]["taxonId"]),
            aliases=tuple("UniProtKB:" + value for value in data.get("secondaryAccessions", [])),
            synonyms=tuple(
                g["geneName"]["value"] for g in data.get("genes", []) if "geneName" in g
            ),
            xrefs=tuple(
                d["database"] + "|" + d["id"] for d in data.get("uniProtKBCrossReferences", [])
            ),
        )
        return (term,), {
            "entry_version": (str(data["entryAudit"]["entryVersion"]),),
            "sequence_version": (str(data["entryAudit"]["sequenceVersion"]),),
            "annotation_updated": (data["entryAudit"]["lastAnnotationUpdateDate"],),
        }
    if data["schemaClass"] != "Pathway":
        raise InvalidRequest("Reactome identifier connector currently admits Pathway records only")
    species = data.get("species", [])
    taxon = "NCBITaxon:" + str(species[0]["taxId"]) if len(species) == 1 else None
    term = VocabularyTerm(
        id="Reactome:" + data["stId"],
        label=data["displayName"],
        namespace="Reactome",
        species=taxon,
        synonyms=tuple(data.get("name", [])),
        definition="\n".join(s["text"] for s in data.get("summation", [])),
        xrefs=tuple("Reactome:" + d["stId"] for d in data.get("hasEvent", []) if "stId" in d),
    )
    return (term,), {
        "stable_id_version": (data["stIdVersion"],),
        "species_names": tuple(d["displayName"] for d in species),
    }


def parse_vocabulary(
    spec: VocabularySourceSpec, raw: bytes
) -> tuple[tuple[VocabularyTerm, ...], dict[str, tuple[str, ...]]]:
    if not raw or len(raw) > MAX_VOCABULARY_BYTES or bytes_digest(raw) != spec.raw_digest:
        raise InvalidRequest("Vocabulary requires 1..64 MiB of exact checksum-pinned source bytes")
    domains = {"hgnc_json": "gene", "uniprot_json": "protein", "reactome_json": "pathway"}
    if spec.connector in domains and spec.domain != domains[spec.connector]:
        raise InvalidRequest("Vocabulary connector and declared biological domain differ")
    try:
        terms, metadata = _obo(raw) if spec.connector == "obo" else _json(spec, raw)
        expected = {
            "obo": "data-version",
            "uniprot_json": "entry_version",
            "reactome_json": "stable_id_version",
        }
        if spec.connector in expected and metadata.get(expected[spec.connector]) != (
            spec.revision,
        ):
            raise InvalidRequest("Vocabulary revision differs from the original source metadata")
        if not terms or len(terms) > 100000:
            raise InvalidRequest("Vocabulary requires 1..100000 terms")
        return terms, metadata
    except (
        ValueError,
        KeyError,
        TypeError,
        IndexError,
        UnicodeError,
        RecursionError,
        ValidationError,
    ) as error:
        raise InvalidRequest("Invalid or unsupported declarative vocabulary source") from error
