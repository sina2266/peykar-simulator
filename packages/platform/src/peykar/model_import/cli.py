"""Operator-only, offline artifact inventory admission."""

import json
import os
from pathlib import Path
from typing import Annotated

import typer

from peykar.model_import.external_artifacts import ExternalArtifactService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import ExternalArtifactSpec, Reference
from peykar_core.errors import InvalidRequest
from peykar_core.hashing import digest

external_artifact_app = typer.Typer(help="Verify and preserve declared external artifact bytes.")


def _service() -> ExternalArtifactService:
    service = SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar")))
    return ExternalArtifactService(service.db, service.artifacts)


def _emit(spec: ExternalArtifactSpec) -> None:
    typer.echo(
        json.dumps(
            {
                "spec": spec.model_dump(mode="json"),
                "spec_digest": digest(spec),
                "verified_files": len(spec.dependencies) + 2,
                "verification": "sha256_all_declared_files",
                "runtime_reproduced": False,
                "code_executed": False,
                "source_fetched": False,
                "scientific_validity": "not_established_by_import",
            },
            indent=2,
            ensure_ascii=False,
        )
    )


@external_artifact_app.command("import")
def import_artifact(
    file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
    directory: Annotated[Path, typer.Option(exists=True, file_okay=False)],
) -> None:
    """Verify an inventory's source, dependencies and runtime lock before admission."""
    with file.open("rb") as stream:
        payload = stream.read(2_000_001)
    if len(payload) > 2_000_000:
        raise InvalidRequest("External artifact descriptor exceeds 2 MB")
    _emit(_service().admit(ExternalArtifactSpec.model_validate_json(payload), directory))


@external_artifact_app.command("verify")
def verify_artifact(id: str, version: str) -> None:
    """Recheck every stored artifact against an immutable admitted inventory."""
    _emit(_service().get(Reference(id=id, version=version)))
