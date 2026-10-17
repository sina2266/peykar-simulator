"""Operator admission, durable jobs and an explicitly allowlisted native agent."""

import os
import time
from pathlib import Path
from typing import Annotated

import typer

from peykar.model_import.native_models import NativeModelService
from peykar.simulation.native_agent import NativeAgent, NativeAgentClient
from peykar.simulation.native_jobs import NativeJobService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import NativeExecutionProfile, NativeModelSpec, NativeRunSpec

native_app = typer.Typer(help="Declarative native models and explicitly admitted external agents")


def service() -> SimulationService:
    return SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar")))


@native_app.command("register-model")
def register_model(
    spec: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
    model: Annotated[Path, typer.Option(exists=True, dir_okay=False)],
) -> None:
    svc = service()
    package = NativeModelService(svc.db, svc.artifacts).admit(
        NativeModelSpec.model_validate_json(spec.read_bytes()), model.read_bytes()
    )
    typer.echo(package.model_dump_json(indent=2))


@native_app.command("register-profile")
def register_profile(file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)]) -> None:
    svc = service()
    typer.echo(
        NativeModelService(svc.db, svc.artifacts).register_profile(
            NativeExecutionProfile.model_validate_json(file.read_bytes())
        )
    )


@native_app.command("profiles")
def profiles() -> None:
    svc = service()
    for profile in NativeModelService(svc.db, svc.artifacts).profiles():
        typer.echo(profile.model_dump_json())


@native_app.command("models")
def models() -> None:
    svc = service()
    for model in NativeModelService(svc.db, svc.artifacts).discover():
        typer.echo(model.model_dump_json())


@native_app.command("submit")
def submit(
    file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
    idempotency_key: str | None = None,
) -> None:
    typer.echo(
        NativeJobService(service())
        .submit(NativeRunSpec.model_validate_json(file.read_bytes()), idempotency_key)
        .model_dump_json(indent=2)
    )


@native_app.command("status")
def status(id: str) -> None:
    typer.echo(NativeJobService(service()).run(id).model_dump_json(indent=2))


@native_app.command("result")
def result(id: str) -> None:
    typer.echo(NativeJobService(service()).result(id).model_dump_json(indent=2))


@native_app.command("cancel")
def cancel(id: str) -> None:
    typer.echo(NativeJobService(service()).cancel(id).model_dump_json(indent=2))


@native_app.command("recover")
def recover() -> None:
    typer.echo({"failed": NativeJobService(service()).recover_stale()})


@native_app.command("agent")
def agent(
    catalog: Annotated[Path, typer.Option(exists=True, dir_okay=False)],
    output: Annotated[Path, typer.Option()],
    base_url: str = "http://127.0.0.1:8000",
    loop: bool = False,
    poll_seconds: Annotated[float, typer.Option(min=0.2, max=30)] = 2,
) -> None:
    client = NativeAgentClient(base_url, os.getenv("PEYKAR_API_TOKEN", ""))
    worker = NativeAgent(client, catalog, output)
    while True:
        try:
            id = worker.run_once()
        except Exception as error:
            if not loop:
                raise
            # Failed jobs are already terminal/fenced; continue with other jobs.
            # Print no exception text that might contain credentials or model data.
            typer.echo({"native_agent_error": type(error).__name__}, err=True)
            time.sleep(poll_seconds)
            continue
        if id:
            typer.echo({"run_id": id})
        if not loop:
            break
        if id is None:
            time.sleep(poll_seconds)
