"""Versioned, JSON-compatible scientific boundary contracts.

Tuples avoid hidden mutable collections. Foreign keys are checked by application services.
Model validity is contextual evidence, never a global quality score.
"""

from enum import StrEnum
from pathlib import PurePosixPath
from typing import Annotated, Literal, Self

from pydantic import BaseModel, ConfigDict, Field, FiniteFloat, StrictInt, model_validator

Identifier = Annotated[str, Field(pattern=r"^[a-z][a-z0-9_.-]{0,79}$")]
Version = Annotated[str, Field(pattern=r"^\d+\.\d+\.\d+$")]
Sha256 = Annotated[str, Field(pattern=r"^[a-f0-9]{64}$")]
Number = Annotated[FiniteFloat, Field(strict=True)]
Positive = Annotated[Number, Field(gt=0)]
NonNegative = Annotated[Number, Field(ge=0)]


class Contract(BaseModel):
    model_config = ConfigDict(extra="forbid", frozen=True, allow_inf_nan=False)
    schema_version: Literal["1.0.0"] = "1.0.0"


class Quantity(Contract):
    value: Number
    unit: Annotated[str, Field(min_length=1, max_length=80)]


class Reference(Contract):
    id: Identifier
    version: Version


class ScientificContext(Contract):
    species: Annotated[str, Field(min_length=1, max_length=100)]
    biological_system: Annotated[str, Field(min_length=1, max_length=200)]
    conditions: tuple[str, ...] = ()


class ValidityProfile(Contract):
    status: Literal["unvalidated", "reference_verified", "context_validated"] = "unvalidated"
    context: ScientificContext
    endpoints: tuple[str, ...] = ()
    evidence: tuple[str, ...] = ()
    limitations: tuple[str, ...]

    @model_validator(mode="after")
    def evidence_required(self) -> Self:
        if self.status != "unvalidated" and (not self.evidence or not self.endpoints):
            raise ValueError("Validation requires endpoint-specific evidence")
        if not self.limitations:
            raise ValueError("Explicit limitations are required")
        return self


class ScientificPort(Contract):
    name: Identifier
    direction: Literal["input", "output"]
    unit: str
    semantic_id: str
    scale: Literal["molecular", "cellular", "tissue", "organ", "organism", "population"]
    shape: Literal["scalar", "field", "agents"] = "scalar"
    frame: str | None = None
    ownership: Literal["read", "write"] = "read"


class ParameterSpec(Contract):
    name: Identifier
    default: Quantity
    minimum: Number | None = None
    maximum: Number | None = None

    @model_validator(mode="after")
    def bounds(self) -> Self:
        if self.minimum is not None and self.default.value < self.minimum:
            raise ValueError("Default below minimum")
        if self.maximum is not None and self.default.value > self.maximum:
            raise ValueError("Default above maximum")
        if self.minimum is not None and self.maximum is not None and self.minimum > self.maximum:
            raise ValueError("Invalid parameter range")
        return self


class ModelPackage(Contract):
    id: Identifier
    version: Version
    title: str
    engine: Identifier
    engine_version: Version
    license: Annotated[str, Field(min_length=1, max_length=100)]
    source: str
    source_digest: Sha256
    capabilities: tuple[Identifier, ...]
    context: ScientificContext
    validity: ValidityProfile
    parameters: tuple[ParameterSpec, ...]
    ports: tuple[ScientificPort, ...] = ()
    dependencies: tuple[Reference, ...] = ()

    @model_validator(mode="after")
    def uniqueness(self) -> Self:
        for names in ([p.name for p in self.parameters], [p.name for p in self.ports]):
            if len(names) != len(set(names)):
                raise ValueError("Duplicate parameter or port name")
        if self.context != self.validity.context:
            raise ValueError("Model and validity contexts must agree")
        return self


class ParameterValue(Contract):
    name: Identifier
    quantity: Quantity


class Intervention(Contract):
    kind: Literal["parameter_set"] = "parameter_set"
    at: Quantity
    parameter: Identifier
    quantity: Quantity


class ExecutionBudget(Contract):
    max_samples: Annotated[StrictInt, Field(ge=2, le=100_001)] = 10_001
    wall_seconds: Annotated[Positive, Field(le=3600)] = 60.0


class SimulationPlan(Contract):
    model: Reference
    context: ScientificContext
    duration: Quantity
    sample_interval: Quantity
    parameters: tuple[ParameterValue, ...] = ()
    interventions: tuple[Intervention, ...] = ()
    seed: Annotated[StrictInt, Field(ge=0, le=2**32 - 1)] = 42
    budget: ExecutionBudget = ExecutionBudget()
    purpose: Annotated[str, Field(min_length=1, max_length=1000)]

    @model_validator(mode="after")
    def unique_parameters(self) -> Self:
        names = [p.name for p in self.parameters]
        if len(names) != len(set(names)):
            raise ValueError("Duplicate parameter override")
        return self


class Scenario(Contract):
    id: Identifier
    version: Version
    plan: SimulationPlan
    parent: Reference | None = None
    rationale: str


class RuntimeEnvironment(Contract):
    platform_version: str
    python_version: str
    engine_id: str
    engine_version: str
    engine_digest: Sha256
    dependencies: tuple[str, ...]
    lock_digest: Sha256 | None = None


class RunManifest(Contract):
    run_id: str
    created_at: str
    plan: SimulationPlan
    model_snapshot: ModelPackage
    model_digest: Sha256
    dependency_snapshots: tuple[ModelPackage, ...] = ()
    environment: RuntimeEnvironment


class RunStatus(StrEnum):
    QUEUED = "queued"
    RUNNING = "running"
    SUCCEEDED = "succeeded"
    FAILED = "failed"
    CANCELLED = "cancelled"


class RunRecord(Contract):
    id: str
    manifest_digest: Sha256
    status: RunStatus
    created_at: str
    updated_at: str
    cancel_requested: bool = False
    result_digest: Sha256 | None = None
    error: str | None = None


class Series(Contract):
    name: str
    unit: str
    values: tuple[Number, ...]


class SimulationResult(Contract):
    origin: Literal["simulation"] = "simulation"
    time: Series
    outputs: tuple[Series, ...]
    seed: int
    validity_status: Literal["unvalidated"] = "unvalidated"
    limitations: tuple[str, ...]

    @model_validator(mode="after")
    def alignment(self) -> Self:
        t = self.time.values
        if len(t) < 2 or any(b <= a for a, b in zip(t, t[1:], strict=False)):
            raise ValueError("Time must be strictly increasing with at least two samples")
        if not self.outputs or any(len(o.values) != len(t) for o in self.outputs):
            raise ValueError("Output lengths must match time")
        if len({o.name for o in self.outputs}) != len(self.outputs):
            raise ValueError("Duplicate output name")
        return self


class DatasetSpec(Contract):
    id: Identifier
    version: Version
    title: str
    origin: Literal["experimental", "synthetic", "simulation"]
    role: Literal["calibration", "validation", "benchmark"]
    context: ScientificContext
    artifact_digest: Sha256
    license: str
    source: str
    independence_group: str


class Observation(Contract):
    dataset: Reference
    endpoint: str
    time: Series
    values: Series


class ValidationReport(Contract):
    model: Reference
    dataset: Reference
    endpoint: str
    n: int
    rmse: NonNegative
    max_absolute_error: NonNegative
    unit: str
    tolerance: NonNegative
    passed: bool
    evidence_class: Literal["synthetic_verification", "independent_experimental_validation"]
    context: ScientificContext
    limitations: tuple[str, ...]


class CSVImportSpec(Contract):
    id: Identifier
    version: Version
    title: str
    origin: Literal["experimental", "synthetic", "simulation"]
    role: Literal["calibration", "validation", "benchmark"]
    context: ScientificContext
    license: Annotated[str, Field(min_length=1)]
    source: Annotated[str, Field(min_length=1)]
    independence_group: Annotated[str, Field(min_length=1)]
    endpoint: Identifier
    time_column: Annotated[str, Field(min_length=1)]
    value_column: Annotated[str, Field(min_length=1)]
    time_unit: str
    value_unit: str


class DatasetLineage(Contract):
    dataset: Reference
    raw_digest: Sha256
    normalized_digest: Sha256
    import_spec: CSVImportSpec
    importer: Literal["peykar.csv/1.0.0"] = "peykar.csv/1.0.0"
    rows: Annotated[StrictInt, Field(ge=2, le=100_001)]
    quality_checks: tuple[str, ...]


class EvaluationSetSpec(Contract):
    id: Identifier
    version: Version
    context: ScientificContext
    endpoint: Identifier
    calibration: tuple[Reference, ...] = ()
    evaluation: Annotated[tuple[Reference, ...], Field(min_length=1, max_length=200)]
    tolerance: Quantity

    @model_validator(mode="after")
    def split(self) -> Self:
        all_refs = self.calibration + self.evaluation
        keys = [(r.id, r.version) for r in all_refs]
        if len(set(keys)) != len(keys):
            raise ValueError("Duplicate or overlapping dataset references")
        if len(self.calibration) > 200:
            raise ValueError("At most 200 calibration datasets are supported")
        if self.tolerance.value < 0:
            raise ValueError("Tolerance must be nonnegative")
        return self


class DatasetSnapshot(Contract):
    reference: Reference
    spec_digest: Sha256
    artifact_digest: Sha256
    independence_group: str


class FrozenEvaluationSet(Contract):
    spec: EvaluationSetSpec
    calibration: tuple[DatasetSnapshot, ...]
    evaluation: tuple[DatasetSnapshot, ...]


class ValidationRecord(Contract):
    id: Identifier
    run_id: str
    manifest_digest: Sha256
    result_digest: Sha256
    dataset_digest: Sha256
    observation_digest: Sha256
    report: ValidationReport
    calibration_groups: tuple[str, ...] = ()
    evaluation_set: Reference | None = None
    evaluation_set_digest: Sha256 | None = None


class BiologicalEntity(Contract):
    id: str
    label: str
    kind: str
    species: str
    ontology_id: str | None = None


class BiologicalRelationship(Contract):
    source: str
    target: str
    relation: Literal["is_a", "part_of", "located_in", "regulates", "interacts_with"]
    context: ScientificContext
    evidence: tuple[str, ...]


class BiologicalGraphSpec(Contract):
    id: Identifier
    version: Version
    title: Annotated[str, Field(min_length=1, max_length=500)]
    context: ScientificContext
    entities: Annotated[tuple[BiologicalEntity, ...], Field(min_length=1, max_length=5000)]
    relationships: Annotated[tuple[BiologicalRelationship, ...], Field(max_length=20000)] = ()


class LibrarySourceSpec(Contract):
    id: Identifier
    version: Version
    title: Annotated[str, Field(min_length=1, max_length=500)]
    repository: Annotated[str, Field(pattern=r"^https://github.com/[\w.-]+/[\w.-]+$")]
    commit: Annotated[str, Field(pattern=r"^[a-f0-9]{40}$")]
    license: Annotated[str, Field(min_length=1, max_length=200)]
    license_files: Annotated[tuple[str, ...], Field(min_length=1, max_length=50)]
    capabilities: Annotated[tuple[Identifier, ...], Field(min_length=1, max_length=50)]
    limitations: Annotated[tuple[str, ...], Field(min_length=1, max_length=50)]


class LibrarySourceReceipt(Contract):
    spec: LibrarySourceSpec
    inventory_digest: Sha256
    license_digests: dict[str, Sha256]
    tracked_files: Annotated[StrictInt, Field(ge=1)]
    tracked_bytes: Annotated[StrictInt, Field(ge=0)]
    unresolved_submodules: tuple[str, ...] = ()
    lfs_pointers: tuple[str, ...] = ()
    state: Literal["source_inspected"] = "source_inspected"
    runtime_status: Literal["not_assessed_by_source_inspection"] = (
        "not_assessed_by_source_inspection"
    )


class NativeRuntimeEvidence(Contract):
    """An operator's retained execution evidence; never an executable API request."""

    id: Identifier
    version: Version
    title: Annotated[str, Field(min_length=1, max_length=500)]
    engine: Identifier
    profile: Identifier
    sources: Annotated[tuple[Reference, ...], Field(min_length=1, max_length=20)]
    image_digest: Sha256
    execution_digest: Sha256
    artifact_digests: Annotated[dict[str, Sha256], Field(min_length=1, max_length=2000)]
    scope: Literal["operator_native_probe"] = "operator_native_probe"
    managed_queue_supported: Literal[False] = False
    scientific_validity: Literal["not_established"] = "not_established"
    limitations: Annotated[tuple[str, ...], Field(min_length=1, max_length=50)]


class ExperimentNode(Contract):
    id: Identifier
    plan: SimulationPlan


class EndpointBinding(Contract):
    source_node: Identifier
    source_port: Identifier
    target_node: Identifier
    target_port: Identifier
    target_parameter: Identifier
    mode: Literal["terminal_to_initial"] = "terminal_to_initial"
    evidence: Annotated[tuple[str, ...], Field(min_length=1, max_length=50)]


class ExperimentSpec(Contract):
    id: Identifier
    version: Version
    title: Annotated[str, Field(min_length=1, max_length=500)]
    nodes: Annotated[tuple[ExperimentNode, ...], Field(min_length=1, max_length=32)]
    bindings: Annotated[tuple[EndpointBinding, ...], Field(max_length=128)] = ()
    limitations: Annotated[tuple[str, ...], Field(min_length=1, max_length=50)]


class FrozenExperiment(Contract):
    spec: ExperimentSpec
    models: tuple[ModelPackage, ...]
    order: tuple[Identifier, ...]
    execution_mode: Literal["sequential_endpoint_transfer"] = "sequential_endpoint_transfer"


class ExperimentNodeRecord(Contract):
    node_id: Identifier
    run_id: str | None = None
    status: Literal["pending", "queued", "running", "succeeded", "failed", "cancelled", "skipped"]


class ExperimentRunRecord(Contract):
    id: str
    experiment: Reference
    experiment_digest: Sha256
    status: RunStatus
    cancel_requested: bool
    error: str | None = None
    created_at: str
    updated_at: str
    nodes: tuple[ExperimentNodeRecord, ...]


class KnowledgeGap(Contract):
    id: Identifier
    description: str
    context: ScientificContext
    affected_models: tuple[Reference, ...]
    severity: Literal["blocking", "limitation", "informational"]


class VCorneaOutputSpec(Contract):
    """Metadata for the pinned upstream native output, separate from a Peykar run."""

    id: Identifier
    version: Version
    title: Annotated[str, Field(min_length=1, max_length=500)]
    kind: Literal["cell_count", "thickness"]
    origin: Literal["simulation", "synthetic"]
    context: ScientificContext
    license: Annotated[str, Field(min_length=1, max_length=200)]
    source: Annotated[str, Field(min_length=1, max_length=2000)]
    upstream_commit: Literal["f5c03afb9c4c7d909494146c5db608389e957070"] | None = None
    source_archive_doi: Literal["10.5281/zenodo.16884414"] | None = None
    source_archive_digest: Sha256 | None = None
    variant: Literal["paper_version", "paper_version_STEM_ONLY"]
    source_tree_digest: Sha256
    runtime_digest: Sha256 | None = None
    parameter_digest: Sha256 | None = None
    normalizer_digest: Sha256 | None = None
    seed: Annotated[StrictInt, Field(ge=0, le=2**32 - 1)] | None = None
    seed_evidence: Annotated[str, Field(min_length=1, max_length=2000)] | None = None

    @model_validator(mode="after")
    def recorded_seed_and_reader(self) -> Self:
        if self.upstream_commit is None and self.source_archive_doi is None:
            raise ValueError("Native output requires its pinned commit or exact archive DOI")
        if (self.source_archive_doi is None) != (self.source_archive_digest is None):
            raise ValueError("Archive provenance requires both exact DOI and archive digest")
        if self.seed is not None and (self.seed_evidence is None or not self.seed_evidence.strip()):
            raise ValueError("An imported seed requires explicit runtime or sidecar evidence")
        if self.seed is None and self.seed_evidence is not None:
            raise ValueError("Seed evidence cannot supply an unavailable seed")
        if self.kind == "thickness" and self.normalizer_digest is None:
            raise ValueError("Parquet output requires the trusted normalizer digest")
        return self


class VCorneaOutputArtifact(Contract):
    reference: Reference
    kind: Literal["cell_count", "thickness"]
    origin: Literal["simulation", "synthetic"]
    time: Series
    outputs: tuple[Series, ...]
    seed: Annotated[StrictInt, Field(ge=0, le=2**32 - 1)] | None = None
    validity_status: Literal["unvalidated"] = "unvalidated"
    limitations: tuple[str, ...]

    @model_validator(mode="after")
    def native_series(self) -> Self:
        times = self.time.values
        if (
            self.time.unit != "hour"
            or len(times) < 2
            or len(times) > 100_001
            or times[0] < 0
            or any(b <= a for a, b in zip(times, times[1:], strict=False))
        ):
            raise ValueError("Native output requires increasing nonnegative hourly time values")
        names = (
            ("Superficial", "Wing", "Basal", "Stem")
            if self.kind == "cell_count"
            else tuple(f"bin.{index}" for index in range(10))
        )
        expected_unit = "count" if self.kind == "cell_count" else "lattice_y_coordinate"
        if tuple(series.name for series in self.outputs) != names:
            raise ValueError("Output series differ from the pinned upstream schema")
        for series in self.outputs:
            if series.unit != expected_unit or len(series.values) != len(times):
                raise ValueError("Native output units or alignment differ from the upstream schema")
            if any(value < 0 for value in series.values):
                raise ValueError("Native cell counts and height coordinates cannot be negative")
            if self.kind == "cell_count" and any(not value.is_integer() for value in series.values):
                raise ValueError("Native cell counts must be nonnegative integers")
        return self


class VCorneaOutputLineage(Contract):
    reference: Reference
    raw_digest: Sha256
    normalized_digest: Sha256
    decoded_digest: Sha256 | None = None
    spec: VCorneaOutputSpec
    importer: Literal["peykar.vcornea-native/1.0.0"] = "peykar.vcornea-native/1.0.0"
    rows: Annotated[StrictInt, Field(ge=2, le=1_000_010)]
    columns: tuple[str, ...]
    quality_checks: tuple[str, ...]
    limitations: tuple[str, ...]


class VCorneaScalarLineage(Contract):
    dataset: Reference
    native_output: Reference
    native_lineage_digest: Sha256
    normalized_digest: Sha256
    column: Literal["Superficial", "Wing", "Basal", "Stem"]
    importer: Literal["peykar.vcornea-scalar/1.0.0"] = "peykar.vcornea-scalar/1.0.0"
    limitations: tuple[str, ...]


class CompuCellImportSpec(Contract):
    """Operator-declared scalar export mapping, never a native-format assumption."""

    dataset: CSVImportSpec
    delimiter: Literal[",", "\t", ";"]
    time_per_step: Quantity
    time_mapping_evidence: Annotated[str, Field(min_length=1, max_length=2000)]
    seed: Annotated[StrictInt, Field(ge=0, le=2**32 - 1)]
    source_digest: Sha256
    runtime_digest: Sha256
    engine_version: Version

    @model_validator(mode="after")
    def simulation_mapping(self) -> Self:
        if self.dataset.origin == "experimental":
            raise ValueError("Imported engine output cannot have experimental origin")
        if self.time_per_step.value <= 0:
            raise ValueError("Time per Monte Carlo step must be positive")
        if not self.time_mapping_evidence.strip():
            raise ValueError("Time mapping requires explicit evidence or a synthetic declaration")
        return self


class CompuCellImportLineage(Contract):
    dataset: Reference
    raw_digest: Sha256
    normalized_digest: Sha256
    import_spec: CompuCellImportSpec
    importer: Literal["peykar.compucell-scalar/1.0.0"] = "peykar.compucell-scalar/1.0.0"
    rows: Annotated[StrictInt, Field(ge=2, le=100_001)]
    quality_checks: tuple[str, ...]
    limitations: tuple[str, ...]


class RepeatedSeedProtocol(Contract):
    id: Identifier
    version: Version
    context: ScientificContext
    endpoint: Identifier
    references: Annotated[tuple[Reference, ...], Field(min_length=2, max_length=200)]
    tolerance: Quantity

    @model_validator(mode="after")
    def unique_references(self) -> Self:
        if len(set(self.references)) != len(self.references):
            raise ValueError("Repeated-seed references must be unique")
        if self.tolerance.value < 0:
            raise ValueError("Tolerance must be nonnegative")
        return self


class SeededDataset(Contract):
    snapshot: DatasetSnapshot
    seed: Annotated[StrictInt, Field(ge=0, le=2**32 - 1)]
    lineage_digest: Sha256


class FrozenSeedProtocol(Contract):
    spec: RepeatedSeedProtocol
    references: tuple[SeededDataset, ...]


class SeedComparison(Contract):
    seed: Annotated[StrictInt, Field(ge=0, le=2**32 - 1)]
    reference: Reference
    candidate: Reference
    candidate_digest: Sha256
    candidate_lineage_digest: Sha256
    n: Annotated[StrictInt, Field(ge=2, le=100_001)]
    rmse: NonNegative
    max_absolute_error: NonNegative
    passed: bool


class RepeatedSeedReport(Contract):
    id: Identifier
    protocol: Reference
    protocol_digest: Sha256
    comparisons: tuple[SeedComparison, ...]
    unit: str
    tolerance: NonNegative
    passed: bool
    evidence_class: Literal["synthetic_verification", "simulation_reference_comparison"]
    limitations: tuple[str, ...]


class ArtifactFilePin(Contract):
    """A portable relative file name and its expected exact-byte SHA-256."""

    path: Annotated[str, Field(min_length=1, max_length=255)]
    digest: Sha256

    @model_validator(mode="after")
    def relative_path(self) -> Self:
        path = PurePosixPath(self.path)
        if (
            path.is_absolute()
            or str(path) != self.path
            or not path.parts
            or any(part in (".", "..") for part in path.parts)
            or "\\" in self.path
            or "\x00" in self.path
        ):
            raise ValueError("Artifact paths must be normalized relative POSIX file paths")
        return self


class ExternalArtifactSpec(Contract):
    """Offline admission inventory; it does not authorize execution or redistribution."""

    id: Identifier
    version: Version
    source: Annotated[str, Field(min_length=1, max_length=2000)]
    license: Annotated[str, Field(min_length=1, max_length=1000)]
    artifact: ArtifactFilePin
    dependencies: Annotated[tuple[ArtifactFilePin, ...], Field(max_length=200)] = ()
    runtime_lock: ArtifactFilePin

    @model_validator(mode="after")
    def unique_files(self) -> Self:
        paths = [pin.path for pin in (self.artifact, *self.dependencies, self.runtime_lock)]
        if len(paths) != len(set(paths)):
            raise ValueError("Artifact, dependency and runtime-lock paths must be distinct")
        if not self.source.strip() or not self.license.strip():
            raise ValueError("Source and license declarations must not be blank")
        return self
