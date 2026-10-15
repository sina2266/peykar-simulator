"""Explicit local output import and frozen repeated-seed comparison commands."""

import json
import os
from pathlib import Path
from typing import Annotated, Any

import typer
from pydantic import TypeAdapter

from peykar.data_fabric.compucell_import import CompuCellImporter
from peykar.data_fabric.csv_import import MAX_CSV_BYTES
from peykar.data_fabric.service import DatasetService
from peykar.simulation.service import SimulationService
from peykar.validation.repeated_seeds import RepeatedSeedService
from peykar_core.contracts import CompuCellImportSpec, Reference, RepeatedSeedProtocol
from peykar_core.errors import InvalidRequest

compucell_app = typer.Typer(help="Import local mapped scalar exports; no external engine execution")


def _datasets() -> DatasetService:
    service = SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar")))
    return DatasetService(service.db, service.artifacts)


def _emit(value: Any) -> None:
    typer.echo(
        json.dumps(value.model_dump(mode="json"), indent=2, ensure_ascii=False, allow_nan=False)
    )


@compucell_app.command("import-output")
def import_output(
    file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
    spec: Annotated[Path, typer.Option(exists=True, dir_okay=False)],
) -> None:
    """Preserve raw bytes and apply an explicit scalar/MCS mapping from JSON."""
    with file.open("rb") as stream:
        raw = stream.read(MAX_CSV_BYTES + 1)
    if len(raw) > MAX_CSV_BYTES:
        raise InvalidRequest("Scalar export exceeds the 20 MiB limit")
    _emit(
        CompuCellImporter(_datasets()).import_output(
            CompuCellImportSpec.model_validate_json(spec.read_bytes()), raw
        )
    )


@compucell_app.command("lineage")
def lineage(dataset_id: str, version: str) -> None:
    _emit(CompuCellImporter(_datasets()).lineage(Reference(id=dataset_id, version=version)))


@compucell_app.command("freeze-seeds")
def freeze(file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)]) -> None:
    _emit(
        RepeatedSeedService(_datasets()).freeze(
            RepeatedSeedProtocol.model_validate_json(file.read_bytes())
        )
    )


@compucell_app.command("compare-seeds")
def compare(
    protocol_id: str,
    version: str,
    candidates: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
) -> None:
    """Compare a JSON array of dataset references with all frozen seed references."""
    _emit(
        RepeatedSeedService(_datasets()).compare(
            Reference(id=protocol_id, version=version),
            TypeAdapter(tuple[Reference, ...]).validate_json(candidates.read_bytes()),
        )
    )


@compucell_app.command("seed-report")
def report(report_id: str) -> None:
    _emit(RepeatedSeedService(_datasets()).get_report(report_id))
