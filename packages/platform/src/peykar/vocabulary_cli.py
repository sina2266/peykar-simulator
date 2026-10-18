"""Explicit offline vocabulary admission and contextual annotation commands."""

import os
from pathlib import Path
from typing import Annotated

import typer

from peykar.biology.vocabularies import VocabularyService
from peykar.biology.vocabulary_import import MAX_VOCABULARY_BYTES
from peykar.simulation.service import SimulationService
from peykar_core.contracts import EntityVocabularyMappingSpec, Reference, VocabularySourceSpec

vocabulary_app = typer.Typer(
    no_args_is_help=True, help="Pinned offline vocabulary snapshots and contextual mappings"
)


def service() -> VocabularyService:
    svc = SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar")))
    return VocabularyService(svc.db, svc.artifacts)


@vocabulary_app.command("admit")
def admit(spec: Path, source: Path) -> None:
    with source.open("rb") as stream:
        raw = stream.read(MAX_VOCABULARY_BYTES + 1)
    typer.echo(
        service()
        .admit(VocabularySourceSpec.model_validate_json(spec.read_bytes()), raw)
        .model_dump_json(indent=2)
    )


@vocabulary_app.command("list")
def discover() -> None:
    for package in service().discover(50):
        typer.echo(package.model_dump_json())


@vocabulary_app.command("search")
def search(
    id: str, version: str, query: str = "", limit: Annotated[int, typer.Option(min=1, max=50)] = 20
) -> None:
    for record in service().search(Reference(id=id, version=version), query, limit=limit):
        typer.echo(record.model_dump_json())


@vocabulary_app.command("term")
def term(id: str, version: str, term_id: str) -> None:
    typer.echo(service().term(Reference(id=id, version=version), term_id).model_dump_json(indent=2))


@vocabulary_app.command("map-entity")
def map_entity(spec: Path) -> None:
    typer.echo(
        service()
        .map_entity(EntityVocabularyMappingSpec.model_validate_json(spec.read_bytes()))
        .model_dump_json(indent=2)
    )


@vocabulary_app.command("conflicts")
def conflicts() -> None:
    for record in service().conflicts(50):
        typer.echo(record.model_dump_json())
