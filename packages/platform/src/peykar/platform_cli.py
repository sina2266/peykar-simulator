"""Operator access to shared library, graph and experiment services."""

import os
from pathlib import Path
from typing import Annotated

import typer

from peykar.biology.persistent_graph import GraphService
from peykar.composition.experiments import ExperimentService
from peykar.model_import.libraries import LibraryService
from peykar.model_import.native_runtimes import NativeRuntimeService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    BiologicalGraphSpec,
    ExperimentSpec,
    LibrarySourceReceipt,
    LibrarySourceSpec,
    NativeRuntimeEvidence,
    Reference,
)

library_app = typer.Typer(help="Inspect pinned local source; no automatic build or execution")
graph_app = typer.Typer(help="Persist and query evidence-linked biological graphs")
experiment_app = typer.Typer(help="Freeze and run explicit multi-model experiment workflows")
runtime_app = typer.Typer(help="Import/discover actual native execution evidence; no installation")


def _service() -> SimulationService:
    return SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar")))


@library_app.command("inspect")
def inspect_library(
    spec: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
    source: Annotated[Path, typer.Option(exists=True, file_okay=False)],
) -> None:
    service = _service()
    receipt = LibraryService(service.db, service.artifacts).inspect(
        LibrarySourceSpec.model_validate_json(spec.read_bytes()), source
    )
    typer.echo(receipt.model_dump_json(indent=2))


@library_app.command("list")
def libraries() -> None:
    service = _service()
    for receipt in LibraryService(service.db, service.artifacts).discover():
        typer.echo(receipt.model_dump_json())


@runtime_app.command("list")
def runtimes(engine: str | None = None) -> None:
    service = _service()
    for receipt in NativeRuntimeService(service.db, service.artifacts).discover(engine):
        typer.echo(receipt.model_dump_json())


@runtime_app.command("import-evidence")
def import_runtime_evidence(
    file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
    bundle: Annotated[Path, typer.Option(exists=True, file_okay=False)],
) -> None:
    service = _service()
    receipt = NativeRuntimeEvidence.model_validate_json(file.read_bytes())
    typer.echo(NativeRuntimeService(service.db, service.artifacts).import_evidence(receipt, bundle))


@graph_app.command("register")
def register_graph(file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)]) -> None:
    typer.echo(
        GraphService(_service().db).register(
            BiologicalGraphSpec.model_validate_json(file.read_bytes())
        )
    )


@library_app.command("import-receipt")
def import_receipt(
    file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
    bundle: Annotated[Path, typer.Option(exists=True, file_okay=False)],
) -> None:
    """Import an operator inspection bundle without Git/network/scientific dependencies."""
    receipt = LibrarySourceReceipt.model_validate_json(file.read_bytes())
    service = _service()
    raw = {
        name: (bundle / checksum).read_bytes() for name, checksum in receipt.license_digests.items()
    }
    result = LibraryService(service.db, service.artifacts).import_receipt(
        receipt, (bundle / receipt.inventory_digest).read_bytes(), raw
    )
    typer.echo(result.model_dump_json(indent=2))


@graph_app.command("show")
def graph(id: str, version: str) -> None:
    typer.echo(
        GraphService(_service().db).get(Reference(id=id, version=version)).model_dump_json(indent=2)
    )


@experiment_app.command("register")
def register_experiment(file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)]) -> None:
    typer.echo(
        ExperimentService(_service())
        .freeze(ExperimentSpec.model_validate_json(file.read_bytes()))
        .model_dump_json(indent=2)
    )


@experiment_app.command("submit")
def submit(id: str, version: str, idempotency_key: str | None = None) -> None:
    typer.echo(
        ExperimentService(_service())
        .submit(Reference(id=id, version=version), idempotency_key)
        .model_dump_json(indent=2)
    )


@experiment_app.command("status")
def status(run_id: str) -> None:
    typer.echo(ExperimentService(_service()).record(run_id).model_dump_json(indent=2))


@experiment_app.command("cancel")
def cancel(run_id: str) -> None:
    typer.echo(ExperimentService(_service()).cancel(run_id).model_dump_json(indent=2))


@experiment_app.command("results")
def results(run_id: str) -> None:
    import json

    typer.echo(json.dumps(ExperimentService(_service()).results(run_id), indent=2))
