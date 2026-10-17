"""Operator CLI for native interfaces and explicit mixed scalar flows."""

import os
from pathlib import Path
from typing import Annotated

import typer

from peykar.composition.hybrid_flows import HybridFlowService
from peykar.model_import.native_interfaces import NativeInterfaceService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import HybridFlowSpec, NativeInterfaceSpec, Reference

hybrid_app = typer.Typer(help="Explicit native/reference scalar DAG flows")


def service() -> SimulationService:
    return SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar")))


@hybrid_app.command("interface")
def interface(file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)]) -> None:
    typer.echo(
        NativeInterfaceService(service()).register(
            NativeInterfaceSpec.model_validate_json(file.read_bytes())
        )
    )


@hybrid_app.command("register")
def register(file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)]) -> None:
    typer.echo(
        HybridFlowService(service())
        .freeze(HybridFlowSpec.model_validate_json(file.read_bytes()))
        .model_dump_json(indent=2)
    )


@hybrid_app.command("submit")
def submit(id: str, version: str, idempotency_key: str | None = None) -> None:
    typer.echo(
        HybridFlowService(service())
        .submit(Reference(id=id, version=version), idempotency_key)
        .model_dump_json(indent=2)
    )


@hybrid_app.command("status")
def status(id: str) -> None:
    typer.echo(HybridFlowService(service()).record(id).model_dump_json(indent=2))


@hybrid_app.command("results")
def results(id: str) -> None:
    import json

    typer.echo(json.dumps(HybridFlowService(service()).results(id), indent=2))


@hybrid_app.command("cancel")
def cancel(id: str) -> None:
    typer.echo(HybridFlowService(service()).cancel(id).model_dump_json(indent=2))
