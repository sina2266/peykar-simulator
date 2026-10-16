"""Frozen seed-paired scalar regression; no inferred stochastic or biological validity."""

import math
from dataclasses import dataclass

from peykar.data_fabric.compucell_import import CompuCellImporter
from peykar.data_fabric.service import DatasetService
from peykar.data_fabric.vcornea_import import VCorneaImporter
from peykar_core.contracts import (
    DatasetSnapshot,
    DatasetSpec,
    FrozenSeedProtocol,
    Observation,
    Quantity,
    Reference,
    RepeatedSeedProtocol,
    RepeatedSeedReport,
    SeedComparison,
    SeededDataset,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import digest
from peykar_core.units import convert


@dataclass(frozen=True)
class _SeedIdentity:
    seed: int
    source_digest: str
    runtime_digest: str
    engine_version: str
    origin: str
    seconds_per_step: float
    lineage_digest: str


class RepeatedSeedService:
    def __init__(self, datasets: DatasetService) -> None:
        self.datasets = datasets
        self.importer = CompuCellImporter(datasets)

    def _seed_identity(self, reference: Reference) -> _SeedIdentity:
        try:
            lineage = self.importer.lineage(reference)
        except NotFound:
            native_importer = VCorneaImporter(self.datasets.db, self.datasets.artifacts)
            scalar = native_importer.get_scalar_lineage(reference)
            native = native_importer.get_lineage(scalar.native_output)
            if native.spec.seed is None or native.spec.runtime_digest is None:
                raise InvalidRequest(
                    "Native seed protocols require recorded seed evidence and runtime digest"
                ) from None
            return _SeedIdentity(
                seed=native.spec.seed,
                source_digest=digest(
                    {
                        "source_tree_digest": native.spec.source_tree_digest,
                        "variant": native.spec.variant,
                        "parameter_digest": native.spec.parameter_digest,
                    }
                ),
                runtime_digest=native.spec.runtime_digest,
                engine_version="4.7.0",
                origin=native.spec.origin,
                seconds_per_step=360.0,  # Pinned writer: ten MCS per hour.
                lineage_digest=digest(scalar),
            )
        return _SeedIdentity(
            seed=lineage.import_spec.seed,
            source_digest=lineage.import_spec.source_digest,
            runtime_digest=lineage.import_spec.runtime_digest,
            engine_version=lineage.import_spec.engine_version,
            origin=lineage.import_spec.dataset.origin,
            seconds_per_step=convert(lineage.import_spec.time_per_step, "second"),
            lineage_digest=digest(lineage),
        )

    def _resolve(
        self, reference: Reference, protocol: RepeatedSeedProtocol
    ) -> tuple[DatasetSpec, Observation, _SeedIdentity, SeededDataset]:
        spec, observation = self.datasets.get(reference)
        lineage = self._seed_identity(reference)
        if spec.context != protocol.context or observation.endpoint != protocol.endpoint:
            raise InvalidRequest("Scalar endpoint or context differs from seed protocol")
        if spec.role != "benchmark" or spec.origin == "experimental":
            raise InvalidRequest("Seed comparisons require simulation or synthetic benchmark data")
        convert(protocol.tolerance, observation.values.unit)
        snapshot = SeededDataset(
            snapshot=DatasetSnapshot(
                reference=reference,
                spec_digest=digest(spec),
                artifact_digest=spec.artifact_digest,
                independence_group=spec.independence_group,
            ),
            seed=lineage.seed,
            lineage_digest=lineage.lineage_digest,
        )
        return spec, observation, lineage, snapshot

    @staticmethod
    def _check_cohort(lineages: tuple[_SeedIdentity, ...]) -> None:
        identities = {
            (
                item.source_digest,
                item.runtime_digest,
                item.engine_version,
                item.origin,
            )
            for item in lineages
        }
        steps = tuple(item.seconds_per_step for item in lineages)
        if len(identities) != 1 or any(
            not math.isclose(step, steps[0], rel_tol=1e-12, abs_tol=0.0) for step in steps
        ):
            raise InvalidRequest(
                "A seed cohort must share source, runtime, origin and time mapping"
            )
        seeds = [item.seed for item in lineages]
        if len(set(seeds)) != len(seeds):
            raise InvalidRequest("Repeated-seed protocol requires distinct seeds")

    def freeze(self, spec: RepeatedSeedProtocol) -> FrozenSeedProtocol:
        resolved = tuple(self._resolve(reference, spec) for reference in spec.references)
        self._check_cohort(tuple(item[2] for item in resolved))
        frozen = FrozenSeedProtocol(
            spec=spec,
            references=tuple(sorted((item[3] for item in resolved), key=lambda item: item.seed)),
        )
        self.datasets.db.register("seed_protocol", spec.id, spec.version, frozen.model_dump_json())
        return frozen

    def get_protocol(self, reference: Reference) -> FrozenSeedProtocol:
        return FrozenSeedProtocol.model_validate_json(
            self.datasets.db.object("seed_protocol", reference.id, reference.version)
        )

    def compare(self, protocol: Reference, candidates: tuple[Reference, ...]) -> RepeatedSeedReport:
        frozen = self.get_protocol(protocol)
        if len(candidates) != len(frozen.references) or len(set(candidates)) != len(candidates):
            raise InvalidRequest("Provide exactly one unique candidate for every frozen seed")
        reference_data = {}
        for snapshot in frozen.references:
            resolved = self._resolve(snapshot.snapshot.reference, frozen.spec)
            if resolved[3] != snapshot:
                raise Conflict("Frozen seed reference snapshot changed")
            reference_data[snapshot.seed] = resolved
        candidate_data = tuple(self._resolve(reference, frozen.spec) for reference in candidates)
        self._check_cohort(tuple(item[2] for item in candidate_data))
        if {item[3].seed for item in candidate_data} != set(reference_data):
            raise InvalidRequest("Candidate seeds must exactly match the frozen reference seeds")
        unit = frozen.spec.tolerance.unit
        tolerance = frozen.spec.tolerance.value
        comparisons = []
        synthetic = False
        for candidate_spec, candidate, candidate_lineage, candidate_snapshot in sorted(
            candidate_data, key=lambda item: item[3].seed
        ):
            reference_spec, reference, reference_lineage, _ = reference_data[
                candidate_snapshot.seed
            ]
            if not math.isclose(
                candidate_lineage.seconds_per_step,
                reference_lineage.seconds_per_step,
                rel_tol=1e-12,
                abs_tol=0.0,
            ):
                raise InvalidRequest("Candidate and reference Monte Carlo time mappings differ")
            reference_times = tuple(
                convert(Quantity(value=value, unit=reference.time.unit), "second")
                for value in reference.time.values
            )
            candidate_times = tuple(
                convert(Quantity(value=value, unit=candidate.time.unit), "second")
                for value in candidate.time.values
            )
            if len(reference_times) != len(candidate_times) or any(
                not math.isclose(a, b, rel_tol=1e-12, abs_tol=0.0)
                for a, b in zip(reference_times, candidate_times, strict=False)
            ):
                raise InvalidRequest("Seed comparisons require identical physical time grids")
            reference_values = tuple(
                convert(Quantity(value=value, unit=reference.values.unit), unit)
                for value in reference.values.values
            )
            candidate_values = tuple(
                convert(Quantity(value=value, unit=candidate.values.unit), unit)
                for value in candidate.values.values
            )
            errors = tuple(
                abs(a - b) for a, b in zip(reference_values, candidate_values, strict=True)
            )
            maximum = max(errors)
            # Scaling avoids overflow in sum(error**2) for finite large values.
            rmse = (
                maximum * math.sqrt(sum((error / maximum) ** 2 for error in errors) / len(errors))
                if maximum
                else 0.0
            )
            if not math.isfinite(maximum) or not math.isfinite(rmse):
                raise InvalidRequest("Scalar comparison overflow")
            comparisons.append(
                SeedComparison(
                    seed=candidate_snapshot.seed,
                    reference=reference.dataset,
                    candidate=candidate.dataset,
                    candidate_digest=candidate_spec.artifact_digest,
                    candidate_lineage_digest=candidate_lineage.lineage_digest,
                    n=len(errors),
                    rmse=rmse,
                    max_absolute_error=maximum,
                    passed=maximum <= tolerance,
                )
            )
            synthetic |= (
                reference_spec.origin == "synthetic" or candidate_spec.origin == "synthetic"
            )
        report = RepeatedSeedReport(
            id="pending",
            protocol=protocol,
            protocol_digest=digest(frozen),
            comparisons=tuple(comparisons),
            unit=unit,
            tolerance=tolerance,
            passed=all(item.passed for item in comparisons),
            evidence_class=(
                "synthetic_verification" if synthetic else "simulation_reference_comparison"
            ),
            limitations=(
                "Every sample of every paired seed must pass the frozen scalar tolerance.",
                "Seed/runtime evidence is recorded or declared; comparison runs no external code.",
                "No confidence intervals, distribution equivalence or biological validation.",
                "Time conversion roundoff permitted at relative tolerance 1e-12; no resampling.",
                "Local freezing provides immutability, not independent preregistration authority.",
            ),
        )
        report = report.model_copy(
            update={"id": "seed-report." + digest(report.model_dump(mode="json", exclude={"id"}))}
        )
        self.datasets.db.register("seed_report", report.id, "1.0.0", report.model_dump_json())
        return report

    def get_report(self, report_id: str) -> RepeatedSeedReport:
        report = RepeatedSeedReport.model_validate_json(
            self.datasets.db.object("seed_report", report_id, "1.0.0")
        )
        expected = "seed-report." + digest(report.model_dump(mode="json", exclude={"id"}))
        if expected != report_id or report.id != report_id:
            raise Conflict("Seed comparison report identifier differs from its content")
        return report
