"""Terminal cohort statistics with exact child lineage and explicit missing members."""

import statistics

from peykar.composition.batches import TERMINAL, BatchService
from peykar.simulation.terminal_values import terminal_values
from peykar_core.contracts import (
    BatchGroupSummary,
    BatchMemberEvidence,
    BatchReport,
    EndpointStatistics,
    Quantity,
    RunManifest,
)
from peykar_core.errors import Conflict
from peykar_core.hashing import digest
from peykar_core.units import convert


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
            values = terminal_values(batches.service, template.manifest, str(member.run_id))
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
