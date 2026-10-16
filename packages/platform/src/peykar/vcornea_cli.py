"""Import native vCornea outputs without executing the external model."""

import os
from pathlib import Path
from typing import Annotated, Literal

import typer

from peykar.data_fabric.vcornea_import import MAX_NATIVE_BYTES, VCorneaImporter
from peykar.simulation.service import SimulationService
from peykar_core.contracts import Reference, VCorneaOutputSpec
from peykar_core.errors import InvalidRequest

vcornea_app = typer.Typer(help="Import pinned native vCornea cell counts and decoded thickness")


def _importer() -> VCorneaImporter:
    service = SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar")))
    return VCorneaImporter(service.db, service.artifacts)


def _read(file: Path) -> bytes:
    with file.open("rb") as stream:
        raw = stream.read(MAX_NATIVE_BYTES + 1)
    if len(raw) > MAX_NATIVE_BYTES:
        raise InvalidRequest("Native output exceeds the 100 MiB limit")
    return raw


@vcornea_app.command("import-cell-counts")
def cell_counts(
    file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
    spec: Annotated[Path, typer.Option(exists=True, dir_okay=False)],
) -> None:
    lineage = _importer().import_cell_counts(
        VCorneaOutputSpec.model_validate_json(spec.read_bytes()), _read(file)
    )
    typer.echo(lineage.model_dump_json(indent=2))


@vcornea_app.command("import-thickness")
def thickness(
    file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
    spec: Annotated[Path, typer.Option(exists=True, dir_okay=False)],
    decoded: Annotated[Path, typer.Option(exists=True, dir_okay=False)],
) -> None:
    """Consume a native Parquet file plus JSON from the recorded runtime normalizer."""
    lineage = _importer().import_thickness(
        VCorneaOutputSpec.model_validate_json(spec.read_bytes()), _read(file), _read(decoded)
    )
    typer.echo(lineage.model_dump_json(indent=2))


@vcornea_app.command("output")
def output(id: str, version: str) -> None:
    typer.echo(_importer().get_output(Reference(id=id, version=version)).model_dump_json(indent=2))


@vcornea_app.command("lineage")
def lineage(id: str, version: str) -> None:
    typer.echo(_importer().get_lineage(Reference(id=id, version=version)).model_dump_json(indent=2))


@vcornea_app.command("extract-count")
def extract_count(
    source_id: str,
    source_version: str,
    dataset_id: str,
    dataset_version: str,
    column: Annotated[Literal["Superficial", "Wing", "Basal", "Stem"], typer.Option()],
    independence_group: Annotated[str, typer.Option()],
) -> None:
    """Pin one unchanged native cell-count column as a benchmark dataset."""
    result = _importer().extract_scalar(
        Reference(id=source_id, version=source_version),
        Reference(id=dataset_id, version=dataset_version),
        column,
        independence_group,
    )
    typer.echo(result.model_dump_json(indent=2))


@vcornea_app.command("scalar-lineage")
def scalar_lineage(id: str, version: str) -> None:
    typer.echo(
        _importer().get_scalar_lineage(Reference(id=id, version=version)).model_dump_json(indent=2)
    )
