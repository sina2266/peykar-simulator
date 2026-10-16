"""Local operator commands for declared knowledge gaps."""

import json
import os
from pathlib import Path
from typing import Annotated

import typer

from peykar.biology.knowledge_gaps import KnowledgeGapService, Severity
from peykar.simulation.service import SimulationService
from peykar_core.contracts import KnowledgeGap

knowledge_gap_app = typer.Typer(help="Register and inspect explicit immutable scientific unknowns")


def _service() -> KnowledgeGapService:
    service = SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar")))
    return KnowledgeGapService(service.db)


@knowledge_gap_app.command("register")
def register(file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)]) -> None:
    """Store a declared gap after resolving its exact affected model versions."""
    checksum = _service().register(KnowledgeGap.model_validate_json(file.read_bytes()))
    typer.echo(json.dumps({"digest": checksum}, indent=2))


@knowledge_gap_app.command("show")
def show(id: str) -> None:
    typer.echo(_service().get(id).model_dump_json(indent=2))


@knowledge_gap_app.command("list")
def discover(
    species: str | None = None,
    biological_system: str | None = None,
    condition: str | None = None,
    severity: Severity | None = None,
    model_id: str | None = None,
    model_version: str | None = None,
    limit: Annotated[int, typer.Option(min=1, max=200)] = 100,
    offset: Annotated[int, typer.Option(min=0)] = 0,
) -> None:
    gaps = _service().discover(
        species=species,
        biological_system=biological_system,
        condition=condition,
        severity=severity,
        model_id=model_id,
        model_version=model_version,
        limit=limit,
        offset=offset,
    )
    typer.echo(
        json.dumps([gap.model_dump(mode="json") for gap in gaps], indent=2, ensure_ascii=False)
    )
