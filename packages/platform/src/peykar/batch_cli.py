"""Local operator commands for immutable job cohorts and terminal statistics."""

import os
from pathlib import Path
from typing import Annotated

import typer

from peykar.composition.batch_results import report
from peykar.composition.batches import BatchService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import BatchSpec, Reference

batch_app = typer.Typer(help="Frozen parameter sweeps and bounded reference/native job batches")


def service() -> BatchService:
    return BatchService(SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar"))))


@batch_app.command("register")
def register(file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)]) -> None:
    typer.echo(
        service().freeze(BatchSpec.model_validate_json(file.read_bytes())).model_dump_json(indent=2)
    )


@batch_app.command("submit")
def submit(id: str, version: str, idempotency_key: str | None = None) -> None:
    typer.echo(
        service()
        .submit(Reference(id=id, version=version), idempotency_key)
        .model_dump_json(indent=2)
    )


@batch_app.command("status")
def status(id: str) -> None:
    typer.echo(service().record(id).model_dump_json(indent=2))


@batch_app.command("results")
def results(id: str) -> None:
    typer.echo(report(service(), id).model_dump_json(indent=2))


@batch_app.command("cancel")
def cancel(id: str) -> None:
    typer.echo(service().cancel(id).model_dump_json(indent=2))


@batch_app.command("advance")
def advance() -> None:
    """Reconcile pending cohorts; ordinary Worker already does this each tick."""
    service().advance_pending()
    typer.echo("Pending batches reconciled")
