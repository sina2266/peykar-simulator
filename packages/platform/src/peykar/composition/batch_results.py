"""Terminal cohort statistics with exact child lineage and explicit missing members."""

import csv
import io
import json
import statistics

from peykar.composition.batches import TERMINAL, BatchService
from peykar_core.contracts import (
    BatchGroupSummary,
    BatchMemberEvidence,
    BatchReport,
    EndpointStatistics,
    FrozenBatchMember,
    Quantity,
    RunManifest,
)
from peykar_core.errors import Conflict
from peykar_core.hashing import digest
from peykar_core.units import convert


def terminal_values(
    batches: BatchService, member: FrozenBatchMember, run_id: str
) -> dict[str, Quantity]:
    template = member.manifest
    if isinstance(template, RunManifest):
        actual = batches.db.manifest(run_id)
        if (
            actual.model_copy(update={"run_id": template.run_id, "created_at": template.created_at})
            != template
        ):
            raise Conflict("Batch child manifest differs from its frozen template")
        result = batches.service.result(run_id)
        if result.seed != template.plan.seed:
            raise Conflict("Batch child seed differs from its frozen allocation")
        return {s.name: Quantity(value=s.values[-1], unit=s.unit) for s in result.outputs}
    actual_native = batches.native.manifest(run_id)
    if (
        actual_native.model_copy(
            update={"run_id": template.run_id, "created_at": template.created_at}
        )
        != template
    ):
        raise Conflict("Native batch child differs from its frozen template")
    native_result = batches.native.result(run_id)
    if native_result.manifest_digest != digest(actual_native):
        raise Conflict("Native result manifest differs")
    blobs = {n: batches.service.artifacts.get(h) for n, h in native_result.artifacts.items()}
    try:
        metadata = json.loads(blobs["quantities.json"])
        rows = list(csv.DictReader(io.StringIO(blobs["native.csv"].decode("utf-8-sig"))))
        if template.profile_snapshot.kind == "copasi_sbml":
            units = {s["name"]: s["unit"] for s in metadata["species"]}
        else:
            units = {n: q["unit"] for n, q in metadata["quantities"].items()}
        return {
            n: Quantity(value=float(value), unit=units[n])
            for n, value in rows[-1].items()
            if n not in ("Time", "IndividualId")
        }
    except (ValueError, TypeError, KeyError, IndexError) as error:
        raise Conflict("Native terminal outputs lack unambiguous quantity units") from error


def report(batches: BatchService, id: str) -> BatchReport:
    record = batches.record(id)
    if record.status not in TERMINAL:
        raise Conflict("Batch summary requires a terminal cohort; inspect member status meanwhile")
    frozen = batches.get(record.batch)
    if digest(frozen) != record.batch_digest:
        raise Conflict("Batch snapshot checksum differs")
    templates = {m.id: m for m in frozen.members}
    if {m.member_id for m in record.members} != set(templates):
        raise Conflict("Batch membership differs from its frozen specification")
    groups: dict[tuple[str, int], list[tuple[str, dict[str, Quantity]]]] = {}
    evidence = []
    for member in record.members:
        template = templates[member.member_id]
        if (member.queue == "reference") != isinstance(template.manifest, RunManifest):
            raise Conflict("Batch member queue differs from its frozen specification")
        key = (template.case_id, template.grid_index)
        groups.setdefault(key, [])
        child = (
            (
                batches.db.run(member.run_id)
                if member.queue == "reference"
                else batches.native.run(member.run_id)
            )
            if member.run_id
            else None
        )
        if child and member.status == "succeeded":
            values = terminal_values(batches, template, str(member.run_id))
            result = (
                batches.service.result(str(member.run_id))
                if member.queue == "reference"
                else batches.native.result(str(member.run_id))
            )
            if digest(result) != child.result_digest:
                raise Conflict("Batch child result checksum differs")
            groups[key].append((member.member_id, values))
        evidence.append(
            BatchMemberEvidence(
                member_id=member.member_id,
                queue=member.queue,
                run_id=member.run_id,
                status=member.status,
                manifest_digest=child.manifest_digest if child else None,
                result_digest=child.result_digest if child else None,
            )
        )
    summaries = []
    for (case, grid), samples in groups.items():
        group = [m for m in frozen.members if (m.case_id, m.grid_index) == (case, grid)]
        successful = {id for id, _ in samples}
        endpoints = []
        if samples:
            names = set(samples[0][1])
            if any(set(values) != names for _, values in samples):
                raise Conflict("Replicates expose different terminal quantities")
            for name in sorted(names):
                unit = samples[0][1][name].unit
                numbers = [convert(v[name], unit) for _, v in samples]
                try:
                    endpoints.append(
                        EndpointStatistics(
                            output=name,
                            unit=unit,
                            samples=len(numbers),
                            mean=statistics.mean(numbers),
                            minimum=min(numbers),
                            maximum=max(numbers),
                            sample_standard_deviation=statistics.stdev(numbers)
                            if len(numbers) > 1
                            else None,
                        )
                    )
                except (ValueError, ArithmeticError) as error:
                    raise Conflict(
                        "Cohort statistics exceed the finite numerical contract"
                    ) from error
        summaries.append(
            BatchGroupSummary(
                case_id=case,
                grid_index=grid,
                parameters=group[0].parameters,
                expected_replicates=len(group),
                successful_replicates=len(samples),
                excluded_member_ids=tuple(m.id for m in group if m.id not in successful),
                complete=len(samples) == len(group),
                endpoints=tuple(endpoints),
            )
        )
    return BatchReport(
        run=record,
        groups=tuple(summaries),
        members=tuple(evidence),
        limitations=(
            "Terminal statistics stay within each case/grid; contexts are never pooled.",
            "Sample standard deviation uses n-1 and is absent with fewer than two successes.",
            "Excluded failures/cancellations/skips are disclosed; incomplete groups may be biased.",
            "Seeded replicas describe model variability, not independent biological evidence.",
            "Native drivers lack stochastic seed control; one native replicate is admitted.",
            "No confidence interval, parameter uncertainty, sensitivity or validity is inferred.",
        ),
    )
