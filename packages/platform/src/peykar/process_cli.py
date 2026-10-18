"""Explicit offline biological process admission and declarations."""

import json
import os
from pathlib import Path

import typer

from peykar.biology.processes import ProcessService
from peykar.biology.reactome_processes import read_bundle
from peykar.simulation.service import SimulationService
from peykar_core.contracts import MolecularStateFrameSpec, ProcessOwnershipSpec, Reference

process_app = typer.Typer(
    no_args_is_help=True,
    help="Multiparty biological process declarations; no implicit solver execution",
)


def service() -> ProcessService:
    svc = SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar")))
    return ProcessService(svc.db, svc.artifacts)


@process_app.command("admit-reactome")
def admit(catalog: Path) -> None:
    network, sources = read_bundle(catalog)
    typer.echo(service().register(network, sources).model_dump_json(indent=2))


@process_app.command("list")
def discover() -> None:
    for network in service().discover(50):
        typer.echo(network.model_dump_json())


@process_app.command("show")
def show(id: str, version: str) -> None:
    typer.echo(service().get(Reference(id=id, version=version)).model_dump_json(indent=2))


@process_app.command("stoichiometry")
def stoichiometry(id: str, version: str) -> None:
    typer.echo(json.dumps(service().stoichiometry(Reference(id=id, version=version)), indent=2))


@process_app.command("ownership")
def own(spec: Path) -> None:
    typer.echo(
        service()
        .own(ProcessOwnershipSpec.model_validate_json(spec.read_bytes()))
        .model_dump_json(indent=2)
    )


@process_app.command("frame")
def frame(spec: Path) -> None:
    typer.echo(
        service()
        .frame(MolecularStateFrameSpec.model_validate_json(spec.read_bytes()))
        .model_dump_json(indent=2)
    )
