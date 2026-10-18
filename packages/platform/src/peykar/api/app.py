import hmac
import os
from collections.abc import AsyncIterator
from contextlib import asynccontextmanager
from pathlib import Path
from typing import Annotated, Any

from fastapi import (
    APIRouter,
    Depends,
    FastAPI,
    Header,
    HTTPException,
    Query,
    Request,
    Response,
    Security,
)
from fastapi.responses import JSONResponse
from fastapi.security import HTTPAuthorizationCredentials, HTTPBearer

from peykar import __version__
from peykar.api.batch_routes import install_batch_routes
from peykar.api.cellml_routes import install_cellml_routes
from peykar.api.dashboard import install_dashboard
from peykar.api.hybrid_routes import install_hybrid_routes
from peykar.api.middleware import BodyLimitMiddleware
from peykar.api.native_routes import install_native_routes
from peykar.api.platform_routes import install_platform_routes
from peykar.api.process_routes import install_process_routes
from peykar.api.vocabulary_routes import install_vocabulary_routes
from peykar.api.workspace_routes import install_workspace_routes
from peykar.biology.knowledge_gaps import KnowledgeGapService, Severity
from peykar.data_fabric.csv_import import import_csv
from peykar.data_fabric.service import DatasetService
from peykar.data_fabric.vcornea_import import VCorneaImporter
from peykar.governance.provenance import provenance
from peykar.simulation.service import SimulationService
from peykar.validation.service import EvaluationService
from peykar_core.contracts import (
    Contract,
    CSVImportSpec,
    DatasetLineage,
    DatasetSpec,
    EvaluationSetSpec,
    FrozenEvaluationSet,
    Identifier,
    KnowledgeGap,
    ModelPackage,
    Observation,
    Quantity,
    Reference,
    RunManifest,
    RunRecord,
    Scenario,
    SimulationPlan,
    SimulationResult,
    ValidationRecord,
    ValidationReport,
    VCorneaOutputArtifact,
    VCorneaOutputLineage,
    VCorneaOutputSpec,
    Version,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound, PeykarError, Unsupported


class DatasetRegistration(Contract):
    spec: DatasetSpec
    observation: Observation


class EvaluationRequest(Contract):
    run_id: str
    dataset: Reference
    tolerance: Quantity
    calibration_groups: tuple[str, ...] = ()


class CSVImportRequest(Contract):
    spec: CSVImportSpec
    csv_text: str


class SetEvaluationRequest(Contract):
    run_id: str
    evaluation_set: Reference


class VCorneaCellCountRequest(Contract):
    spec: VCorneaOutputSpec
    csv_text: str


def create_app(
    data_dir: Path | None = None,
    api_token: str | None = None,
    read_token: str | None = None,
) -> FastAPI:
    admin_secret = api_token if api_token is not None else os.getenv("PEYKAR_API_TOKEN", "")
    reader_secret = read_token if read_token is not None else os.getenv("PEYKAR_READ_TOKEN", "")
    if len(admin_secret) < 24 or (reader_secret and len(reader_secret) < 24):
        raise RuntimeError(
            "Set PEYKAR_API_TOKEN (and optional PEYKAR_READ_TOKEN) to 24+ characters"
        )
    if reader_secret and hmac.compare_digest(admin_secret.encode(), reader_secret.encode()):
        raise RuntimeError("Reader and administrator tokens must differ")
    root = data_dir or Path(os.getenv("PEYKAR_DATA_DIR", ".peykar"))

    @asynccontextmanager
    async def lifespan(application: FastAPI) -> AsyncIterator[None]:
        application.state.service = SimulationService(root)
        yield

    app = FastAPI(
        title="Peykar",
        version=__version__,
        lifespan=lifespan,
        description="Biological Modeling & Simulation Platform. Research use only.",
    )
    app.add_middleware(BodyLimitMiddleware)
    bearer = HTTPBearer(auto_error=False)

    def role(
        credentials: Annotated[HTTPAuthorizationCredentials | None, Security(bearer)],
    ) -> str:
        if credentials and hmac.compare_digest(
            credentials.credentials.encode(), admin_secret.encode()
        ):
            return "admin"
        if (
            credentials
            and reader_secret
            and hmac.compare_digest(credentials.credentials.encode(), reader_secret.encode())
        ):
            return "reader"
        raise HTTPException(
            401, "Valid bearer token required", headers={"WWW-Authenticate": "Bearer"}
        )

    def write_access(access: Annotated[str, Depends(role)]) -> None:
        if access != "admin":
            raise HTTPException(403, "Administrator access required")

    def service(request: Request) -> SimulationService:
        value: SimulationService = request.app.state.service
        return value

    @app.exception_handler(PeykarError)
    async def domain_error(request: Request, exc: PeykarError) -> JSONResponse:
        code = 422
        if isinstance(exc, NotFound):
            code = 404
        elif isinstance(exc, Conflict):
            code = 409
        elif isinstance(exc, Unsupported):
            code = 501
        return JSONResponse({"detail": str(exc), "type": type(exc).__name__}, status_code=code)

    @app.get("/healthz", tags=["operations"])
    def health() -> dict[str, str]:
        return {"status": "ok", "version": __version__}

    read = APIRouter(prefix="/v1", dependencies=[Depends(role)])
    write = APIRouter(prefix="/v1", dependencies=[Depends(write_access)])
    install_native_routes(read, write, service)
    install_cellml_routes(read, write, service)
    install_vocabulary_routes(read, write, service)
    install_process_routes(read, write, service)
    install_batch_routes(read, write, service)
    install_hybrid_routes(read, write, service)

    @read.get("/engines", tags=["models"])
    def engines(svc: Annotated[SimulationService, Depends(service)]) -> tuple[dict[str, str], ...]:
        return svc.engines.describe()

    @read.get("/models", tags=["models"])
    def models(
        svc: Annotated[SimulationService, Depends(service)],
        capability: str | None = None,
        species: str | None = None,
        limit: Annotated[int, Query(ge=1, le=200)] = 100,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[ModelPackage, ...]:
        return svc.models.discover(capability, species, limit, offset)

    @read.get("/models/{id}/versions/{version}", tags=["models"])
    def model(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> ModelPackage:
        return svc.models.get(Reference(id=id, version=version))

    @write.post("/models", status_code=201, tags=["models"])
    def register_model(
        body: ModelPackage, svc: Annotated[SimulationService, Depends(service)]
    ) -> dict[str, str]:
        if body.validity.status != "unvalidated":
            raise InvalidRequest("API registration cannot self-certify scientific validity")
        return {"digest": svc.models.register(body)}

    @read.get("/knowledge-gaps", tags=["knowledge"])
    def knowledge_gaps(
        svc: Annotated[SimulationService, Depends(service)],
        species: Annotated[str | None, Query(min_length=1, max_length=100)] = None,
        biological_system: Annotated[str | None, Query(min_length=1, max_length=200)] = None,
        condition: Annotated[str | None, Query(min_length=1, max_length=1000)] = None,
        severity: Severity | None = None,
        model_id: Identifier | None = None,
        model_version: Version | None = None,
        limit: Annotated[int, Query(ge=1, le=200)] = 100,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[KnowledgeGap, ...]:
        return KnowledgeGapService(svc.db).discover(
            species=species,
            biological_system=biological_system,
            condition=condition,
            severity=severity,
            model_id=model_id,
            model_version=model_version,
            limit=limit,
            offset=offset,
        )

    @read.get("/knowledge-gaps/{id}", tags=["knowledge"])
    def knowledge_gap(
        id: Identifier, svc: Annotated[SimulationService, Depends(service)]
    ) -> KnowledgeGap:
        return KnowledgeGapService(svc.db).get(id)

    @write.post("/knowledge-gaps", status_code=201, tags=["knowledge"])
    def register_knowledge_gap(
        body: KnowledgeGap,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> dict[str, str]:
        checksum = KnowledgeGapService(svc.db).register(body)
        response.headers["Location"] = f"/v1/knowledge-gaps/{body.id}"
        return {"digest": checksum}

    @write.post("/simulations/validate", tags=["simulations"])
    def check(
        body: SimulationPlan, svc: Annotated[SimulationService, Depends(service)]
    ) -> dict[str, str]:
        svc.validate(body)
        return {"status": "execution_valid", "scientific_validity": "not_established"}

    @write.post("/vcornea/outputs/cell-counts", status_code=201, tags=["vcornea"])
    def import_vcornea_counts(
        body: VCorneaCellCountRequest,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> VCorneaOutputLineage:
        try:
            raw = body.csv_text.encode("utf-8")
        except UnicodeEncodeError as exc:
            raise InvalidRequest("Native CSV text must be valid UTF-8") from exc
        lineage = VCorneaImporter(svc.db, svc.artifacts).import_cell_counts(body.spec, raw)
        response.headers["Location"] = (
            f"/v1/vcornea/outputs/{body.spec.id}/versions/{body.spec.version}"
        )
        return lineage

    @read.get("/vcornea/outputs/{id}/versions/{version}", tags=["vcornea"])
    def vcornea_output(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> VCorneaOutputArtifact:
        return VCorneaImporter(svc.db, svc.artifacts).get_output(Reference(id=id, version=version))

    @read.get("/vcornea/outputs/{id}/versions/{version}/lineage", tags=["vcornea"])
    def vcornea_lineage(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> VCorneaOutputLineage:
        return VCorneaImporter(svc.db, svc.artifacts).get_lineage(Reference(id=id, version=version))

    @write.post("/simulations/estimate", tags=["simulations"])
    def estimate(
        body: SimulationPlan, svc: Annotated[SimulationService, Depends(service)]
    ) -> dict[str, int | float | str]:
        return svc.estimate(body)

    @write.post("/simulations", status_code=202, tags=["simulations"])
    def submit(
        body: SimulationPlan,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
        idempotency_key: Annotated[str | None, Header(max_length=128)] = None,
    ) -> RunRecord:
        run = svc.submit(body, idempotency_key)
        response.headers["Location"] = f"/v1/simulations/{run.id}"
        if run.status in ("queued", "running"):
            response.headers["Retry-After"] = "1"
        return run

    @read.get("/simulations", tags=["simulations"])
    def runs(
        svc: Annotated[SimulationService, Depends(service)],
        limit: Annotated[int, Query(ge=1, le=200)] = 100,
        offset: Annotated[int, Query(ge=0)] = 0,
    ) -> tuple[RunRecord, ...]:
        return svc.db.runs(limit, offset)

    @read.get("/simulations/{id}", tags=["simulations"])
    def status(id: str, svc: Annotated[SimulationService, Depends(service)]) -> RunRecord:
        return svc.db.run(id)

    @read.get("/simulations/{id}/manifest", tags=["provenance"])
    def manifest(id: str, svc: Annotated[SimulationService, Depends(service)]) -> RunManifest:
        return svc.db.manifest(id)

    @read.get("/simulations/{id}/results", tags=["simulations"])
    def results(id: str, svc: Annotated[SimulationService, Depends(service)]) -> SimulationResult:
        return svc.result(id)

    @read.get("/simulations/{id}/events", tags=["provenance"])
    def events(
        id: str, svc: Annotated[SimulationService, Depends(service)]
    ) -> list[dict[str, Any]]:
        return svc.db.events(id)

    @read.get("/simulations/{id}/provenance", tags=["provenance"])
    def prov(id: str, svc: Annotated[SimulationService, Depends(service)]) -> dict[str, Any]:
        return provenance(svc, id)

    @write.post("/simulations/{id}/cancel", tags=["simulations"])
    def cancel(id: str, svc: Annotated[SimulationService, Depends(service)]) -> RunRecord:
        return svc.db.cancel(id)

    @write.post("/scenarios", status_code=201, tags=["scenarios"])
    def scenario(
        body: Scenario, svc: Annotated[SimulationService, Depends(service)]
    ) -> dict[str, str]:
        return {"digest": svc.register_scenario(body)}

    @read.get("/scenarios/{id}/versions/{version}", tags=["scenarios"])
    def get_scenario(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> Scenario:
        return Scenario.model_validate_json(svc.db.object("scenario", id, version))

    @write.post("/datasets", status_code=201, tags=["datasets"])
    def dataset(
        body: DatasetRegistration, svc: Annotated[SimulationService, Depends(service)]
    ) -> dict[str, str]:
        return {
            "digest": DatasetService(svc.db, svc.artifacts).register(body.spec, body.observation)
        }

    @read.get("/datasets/{id}/versions/{version}", tags=["datasets"])
    def get_dataset(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> DatasetRegistration:
        spec, obs = DatasetService(svc.db, svc.artifacts).get(Reference(id=id, version=version))
        return DatasetRegistration(spec=spec, observation=obs)

    @write.post("/datasets/import-csv", status_code=201, tags=["datasets"])
    def csv_dataset(
        body: CSVImportRequest, svc: Annotated[SimulationService, Depends(service)]
    ) -> DatasetLineage:
        try:
            raw = body.csv_text.encode("utf-8")
        except UnicodeEncodeError as exc:
            raise InvalidRequest("CSV text must be valid UTF-8") from exc
        return import_csv(DatasetService(svc.db, svc.artifacts), body.spec, raw)

    @read.get("/datasets/{id}/versions/{version}/lineage", tags=["datasets"])
    def dataset_lineage(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> DatasetLineage:
        return DatasetService(svc.db, svc.artifacts).lineage(Reference(id=id, version=version))

    @write.post("/validation/runs", status_code=201, tags=["validation"])
    def validation(
        body: EvaluationRequest,
        response: Response,
        svc: Annotated[SimulationService, Depends(service)],
    ) -> ValidationReport:
        record = EvaluationService(svc).evaluate_run(
            body.run_id, body.dataset, body.tolerance, body.calibration_groups
        )
        response.headers["Location"] = f"/v1/validation/reports/{record.id}"
        return record.report

    @read.get("/validation/reports/{id}", tags=["validation"])
    def validation_report(
        id: Identifier, svc: Annotated[SimulationService, Depends(service)]
    ) -> ValidationRecord:
        return EvaluationService(svc).get_report(id)

    @write.post("/validation/sets", status_code=201, tags=["validation"])
    def freeze_set(
        body: EvaluationSetSpec, svc: Annotated[SimulationService, Depends(service)]
    ) -> FrozenEvaluationSet:
        return EvaluationService(svc).freeze(body)

    @read.get("/validation/sets/{id}/versions/{version}", tags=["validation"])
    def get_set(
        id: Identifier, version: Version, svc: Annotated[SimulationService, Depends(service)]
    ) -> FrozenEvaluationSet:
        return EvaluationService(svc).get_set(Reference(id=id, version=version))

    @write.post("/validation/evaluations", status_code=201, tags=["validation"])
    def evaluate_set(
        body: SetEvaluationRequest, svc: Annotated[SimulationService, Depends(service)]
    ) -> tuple[ValidationRecord, ...]:
        return EvaluationService(svc).evaluate_set(body.run_id, body.evaluation_set)

    install_platform_routes(read, write, service)
    install_workspace_routes(read, service, role)
    app.include_router(read)
    app.include_router(write)
    install_dashboard(app)
    return app
