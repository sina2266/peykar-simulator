"""Frozen declared splits and content-addressed, run-bound evaluation records."""

from peykar.data_fabric.service import DatasetService
from peykar.simulation.service import SimulationService
from peykar.validation.evaluation import evaluate
from peykar_core.contracts import (
    DatasetSnapshot,
    DatasetSpec,
    EvaluationSetSpec,
    FrozenEvaluationSet,
    Observation,
    Quantity,
    Reference,
    ValidationRecord,
)
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import digest
from peykar_core.units import convert


class EvaluationService:
    def __init__(self, service: SimulationService) -> None:
        self.service = service
        self.datasets = DatasetService(service.db, service.artifacts)

    def freeze(self, spec: EvaluationSetSpec) -> FrozenEvaluationSet:
        calibration: list[DatasetSnapshot] = []
        evaluation: list[DatasetSnapshot] = []
        for references, destination, is_calibration in (
            (spec.calibration, calibration, True),
            (spec.evaluation, evaluation, False),
        ):
            for reference in references:
                dataset, observation = self.datasets.get(reference)
                if dataset.context != spec.context or observation.endpoint != spec.endpoint:
                    raise InvalidRequest("Dataset endpoint or context differs from evaluation set")
                if (dataset.role == "calibration") != is_calibration:
                    raise InvalidRequest("Dataset role disagrees with its declared split")
                if not dataset.independence_group.strip():
                    raise InvalidRequest("Evaluation sets require nonempty independence groups")
                convert(spec.tolerance, observation.values.unit)
                destination.append(
                    DatasetSnapshot(
                        reference=reference,
                        spec_digest=digest(dataset),
                        artifact_digest=dataset.artifact_digest,
                        independence_group=dataset.independence_group,
                    )
                )
        if {s.independence_group for s in calibration} & {s.independence_group for s in evaluation}:
            raise InvalidRequest("Calibration and evaluation independence groups overlap")
        frozen = FrozenEvaluationSet(
            spec=spec, calibration=tuple(calibration), evaluation=tuple(evaluation)
        )
        self.service.db.register("evaluation_set", spec.id, spec.version, frozen.model_dump_json())
        return frozen

    def get_set(self, reference: Reference) -> FrozenEvaluationSet:
        return FrozenEvaluationSet.model_validate_json(
            self.service.db.object("evaluation_set", reference.id, reference.version)
        )

    def _record(
        self,
        run_id: str,
        dataset: DatasetSpec,
        observation: Observation,
        tolerance: Quantity,
        calibration_groups: tuple[str, ...],
        evaluation_set: Reference | None = None,
        set_digest: str | None = None,
    ) -> ValidationRecord:
        manifest = self.service.db.manifest(run_id)
        result = self.service.result(run_id)
        report = evaluate(
            manifest, result, dataset, observation, tolerance, frozenset(calibration_groups)
        )
        # Empty groups cannot prove that no data were used to tune this model.
        report = report.model_copy(
            update={
                "limitations": report.limitations
                + ("Declared splits do not prove that model tuning avoided evaluation data.",)
            }
        )
        record = ValidationRecord(
            id="pending",
            run_id=run_id,
            manifest_digest=digest(manifest),
            result_digest=digest(result),
            dataset_digest=digest(dataset),
            observation_digest=dataset.artifact_digest,
            report=report,
            calibration_groups=tuple(sorted(set(calibration_groups))),
            evaluation_set=evaluation_set,
            evaluation_set_digest=set_digest,
        )
        return record.model_copy(
            update={"id": "report." + digest(record.model_dump(mode="json", exclude={"id"}))}
        )

    def evaluate_run(
        self,
        run_id: str,
        dataset: Reference,
        tolerance: Quantity,
        calibration_groups: tuple[str, ...] = (),
    ) -> ValidationRecord:
        metadata, observation = self.datasets.get(dataset)
        record = self._record(run_id, metadata, observation, tolerance, calibration_groups)
        self._save((record,))
        return record

    def evaluate_set(self, run_id: str, reference: Reference) -> tuple[ValidationRecord, ...]:
        frozen = self.get_set(reference)
        resolved = {}
        for snapshot in frozen.calibration + frozen.evaluation:
            dataset, observation = self.datasets.get(snapshot.reference)
            if (
                digest(dataset) != snapshot.spec_digest
                or dataset.artifact_digest != snapshot.artifact_digest
                or dataset.independence_group != snapshot.independence_group
            ):
                raise Conflict("Frozen dataset snapshot changed")
            resolved[(snapshot.reference.id, snapshot.reference.version)] = (dataset, observation)
        groups = tuple(snapshot.independence_group for snapshot in frozen.calibration)
        records = tuple(
            self._record(
                run_id,
                *resolved[(snapshot.reference.id, snapshot.reference.version)],
                frozen.spec.tolerance,
                groups,
                reference,
                digest(frozen),
            )
            for snapshot in frozen.evaluation
        )
        self._save(records)
        return records

    def _save(self, records: tuple[ValidationRecord, ...]) -> None:
        self.service.db.register_many(
            tuple(
                ("validation_record", record.id, "1.0.0", record.model_dump_json())
                for record in records
            )
        )

    def get_report(self, id: str) -> ValidationRecord:
        record = ValidationRecord.model_validate_json(
            self.service.db.object("validation_record", id, "1.0.0")
        )
        expected = "report." + digest(record.model_dump(mode="json", exclude={"id"}))
        if record.id != id or expected != id:
            raise Conflict("Validation record identifier does not match its contents")
        return record
