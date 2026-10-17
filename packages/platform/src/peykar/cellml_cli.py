"""CellML operator commands; no automatic installation or remote imports."""

import json
import os
from pathlib import Path
from typing import Annotated

import typer

from peykar.model_import.cellml import CellMLService
from peykar.simulation.cellml_operator import execute_cellml
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    CellMLExecutionReceipt,
    CellMLModelSpec,
    CellMLRunControls,
    Reference,
)

cellml_app = typer.Typer(no_args_is_help=True)


def service() -> CellMLService:
    svc = SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar")))
    return CellMLService(svc.db, svc.artifacts)


def emit(value: object) -> None:
    typer.echo(json.dumps(value, indent=2, ensure_ascii=False))


@cellml_app.command("register-model")
def register_model(spec: Path, model: Path) -> None:
    with model.open("rb") as stream:
        raw = stream.read(10 * 1024**2 + 1)
    emit(
        service()
        .admit(CellMLModelSpec.model_validate_json(spec.read_bytes()), raw)
        .model_dump(mode="json")
    )


@cellml_app.command("models")
def models(
    limit: Annotated[int, typer.Option(min=1, max=50)] = 20,
    offset: Annotated[int, typer.Option(min=0)] = 0,
) -> None:
    emit([m.model_dump(mode="json") for m in service().discover(limit, offset)])


@cellml_app.command("run")
def run(
    id: str,
    version: str,
    controls: Path,
    catalog: Path,
    output: Path,
    receipt_id: str,
    wall_seconds: Annotated[float, typer.Option(min=1, max=600)] = 120,
) -> None:
    emit(
        execute_cellml(
            service(),
            Reference(id=id, version=version),
            CellMLRunControls.model_validate_json(controls.read_bytes()),
            catalog,
            output,
            receipt_id,
            wall_seconds,
        ).model_dump(mode="json")
    )


@cellml_app.command("import-execution")
def import_execution(receipt: Path, bundle: Path) -> None:
    emit(
        {
            "digest": service().import_execution(
                CellMLExecutionReceipt.model_validate_json(receipt.read_bytes()), bundle
            )
        }
    )


@cellml_app.command("executions")
def executions(
    limit: Annotated[int, typer.Option(min=1, max=50)] = 20,
    offset: Annotated[int, typer.Option(min=0)] = 0,
) -> None:
    emit([m.model_dump(mode="json") for m in service().executions(limit, offset)])
