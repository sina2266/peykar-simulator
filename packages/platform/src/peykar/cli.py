"""Local operator CLI. The API has its own authentication boundary."""

import json
import os
import time
from pathlib import Path
from typing import Annotated, Any

import typer

from peykar.compucell_cli import compucell_app
from peykar.data_fabric.csv_import import MAX_CSV_BYTES, import_csv
from peykar.data_fabric.service import DatasetService
from peykar.examples import bootstrap, demo
from peykar.knowledge_gap_cli import knowledge_gap_app
from peykar.model_import.cli import external_artifact_app
from peykar.native_cli import native_app
from peykar.platform_cli import experiment_app, graph_app, library_app, runtime_app
from peykar.simulation.service import SimulationService
from peykar.simulation.worker import Worker
from peykar.validation.service import EvaluationService
from peykar.vcornea_cli import vcornea_app
from peykar_core.contracts import (
    CSVImportSpec,
    EvaluationSetSpec,
    ModelPackage,
    Reference,
    SimulationPlan,
)
from peykar_core.errors import InvalidRequest, PeykarError

app = typer.Typer(no_args_is_help=True, help="Peykar biological modeling and simulation platform")
app.add_typer(external_artifact_app, name="external-artifacts")
app.add_typer(compucell_app, name="compucell")
app.add_typer(knowledge_gap_app, name="knowledge-gaps")
app.add_typer(vcornea_app, name="vcornea")
app.add_typer(library_app, name="libraries")
app.add_typer(graph_app, name="biology")
app.add_typer(experiment_app, name="experiments")
app.add_typer(runtime_app, name="runtimes")
app.add_typer(native_app, name="native")


def local_service() -> SimulationService:
    return SimulationService(Path(os.getenv("PEYKAR_DATA_DIR", ".peykar")))


def emit(value: Any) -> None:
    if hasattr(value, "model_dump"):
        value = value.model_dump(mode="json")
    typer.echo(json.dumps(value, indent=2, ensure_ascii=False, allow_nan=False))


@app.command("init")
def initialize() -> None:
    """Create local state and register both synthetic reference models."""
    bootstrap(local_service())
    emit({"status": "initialized", "models": ["reference.decay", "reference.logistic"]})


@app.command("demo")
def run_demo() -> None:
    """Run analytical decay verification end to end; no external service needed."""
    emit(demo(local_service()))


@app.command("models")
def models() -> None:
    emit([m.model_dump(mode="json") for m in local_service().models.discover()])


@app.command("register-model")
def register_model(file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)]) -> None:
    model = ModelPackage.model_validate_json(file.read_bytes())
    emit({"digest": local_service().models.register(model)})


@app.command("submit")
def submit(
    file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
    idempotency_key: str | None = None,
) -> None:
    emit(
        local_service().submit(
            SimulationPlan.model_validate_json(file.read_bytes()), idempotency_key
        )
    )


@app.command("status")
def status(run_id: str) -> None:
    emit(local_service().db.run(run_id))


@app.command("result")
def result(run_id: str) -> None:
    emit(local_service().result(run_id))


@app.command("cancel")
def cancel(run_id: str) -> None:
    emit(local_service().db.cancel(run_id))


@app.command("import-csv")
def csv_dataset(
    file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)],
    spec: Annotated[Path, typer.Option(exists=True, dir_okay=False)],
) -> None:
    """Import UTF-8 scalar observations and preserve the exact raw CSV bytes."""
    with file.open("rb") as stream:
        raw = stream.read(MAX_CSV_BYTES + 1)
    if len(raw) > MAX_CSV_BYTES:
        raise InvalidRequest("CSV exceeds the 20 MiB limit")
    svc = local_service()
    emit(
        import_csv(
            DatasetService(svc.db, svc.artifacts),
            CSVImportSpec.model_validate_json(spec.read_bytes()),
            raw,
        )
    )


@app.command("freeze-evaluation")
def freeze_evaluation(file: Annotated[Path, typer.Argument(exists=True, dir_okay=False)]) -> None:
    """Freeze dataset versions, declared groups, endpoint and tolerance before evaluation."""
    emit(
        EvaluationService(local_service()).freeze(
            EvaluationSetSpec.model_validate_json(file.read_bytes())
        )
    )


@app.command("evaluate")
def evaluate(run_id: str, set_id: str, set_version: str) -> None:
    records = EvaluationService(local_service()).evaluate_set(
        run_id, Reference(id=set_id, version=set_version)
    )
    emit([record.model_dump(mode="json") for record in records])


@app.command("report")
def report(report_id: str) -> None:
    emit(EvaluationService(local_service()).get_report(report_id))


@app.command("recover")
def recover(age_seconds: Annotated[float, typer.Option(min=60)] = 300) -> None:
    """Fail stale runs explicitly. No implicit rerun or silent state replacement."""
    emit({"failed_stale_runs": local_service().db.recover_stale(age_seconds)})


@app.command("worker")
def worker(once: bool = False, poll_seconds: Annotated[float, typer.Option(min=0.1)] = 1.0) -> None:
    """Run the local trusted queue worker in this process."""
    process = Worker(local_service())
    while True:
        record = process.tick()
        if record:
            emit(record)
        if once:
            break
        if record is None:
            time.sleep(poll_seconds)


@app.command("serve")
def serve(
    host: str = "127.0.0.1", port: Annotated[int, typer.Option(min=1, max=65535)] = 8000
) -> None:
    """Serve authenticated HTTP API; requires PEYKAR_API_TOKEN."""
    import uvicorn

    from peykar.api.app import create_app

    uvicorn.run(create_app(), host=host, port=port)


def main() -> None:
    try:
        app()
    except PeykarError as exc:
        typer.echo(str(exc), err=True)
        raise SystemExit(1) from None
