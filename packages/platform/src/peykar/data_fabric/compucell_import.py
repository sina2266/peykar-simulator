"""Local scalar exports with explicit Monte Carlo step mapping and raw provenance.

This does not discover native CompuCell3D formats or execute an external engine.
"""

import csv
import io
import math

from peykar.data_fabric.csv_import import MAX_CSV_BYTES, MAX_ROWS
from peykar.data_fabric.service import DatasetService
from peykar_core.contracts import (
    CompuCellImportLineage,
    CompuCellImportSpec,
    DatasetSpec,
    Observation,
    Reference,
    Series,
)
from peykar_core.errors import InvalidRequest
from peykar_core.hashing import bytes_digest, canonical_json
from peykar_core.units import convert


class CompuCellImporter:
    def __init__(self, datasets: DatasetService) -> None:
        self.datasets = datasets

    def import_output(self, spec: CompuCellImportSpec, raw: bytes) -> CompuCellImportLineage:
        if not raw or len(raw) > MAX_CSV_BYTES:
            raise InvalidRequest("Scalar export must contain 1 to 20971520 bytes")
        mapping = spec.dataset
        if mapping.time_column == mapping.value_column:
            raise InvalidRequest("Step and value columns must differ")
        convert(spec.time_per_step, "second")
        per_step = convert(spec.time_per_step, mapping.time_unit)
        times: list[float] = []
        values: list[float] = []
        try:
            text = raw.decode("utf-8-sig")
            if "\x00" in text:
                raise ValueError("NUL byte")
            reader = csv.reader(
                io.StringIO(text, newline=""), delimiter=spec.delimiter, strict=True
            )
            headers = next(reader)
            if len(headers) != len(set(headers)) or any(not name.strip() for name in headers):
                raise ValueError("Duplicate or blank headers")
            ti, vi = headers.index(mapping.time_column), headers.index(mapping.value_column)
            for row in reader:
                if len(times) >= MAX_ROWS or len(row) != len(headers):
                    raise ValueError("Too many rows or ragged row")
                step, value = float(row[ti]), float(row[vi])
                if (
                    not math.isfinite(step)
                    or not step.is_integer()
                    or not 0 <= step <= 2**32 - 1
                    or not math.isfinite(value)
                ):
                    raise ValueError("Invalid step or scalar value")
                time = step * per_step
                if not math.isfinite(time):
                    raise ValueError("Time mapping overflow")
                times.append(time)
                values.append(value)
        except (UnicodeDecodeError, csv.Error, ValueError, StopIteration) as exc:
            raise InvalidRequest(
                "Invalid scalar export: UTF-8, unique named columns, finite values and "
                "integer Monte Carlo steps in [0, 4294967295] required"
            ) from exc
        reference = Reference(id=mapping.id, version=mapping.version)
        observation = Observation(
            dataset=reference,
            endpoint=mapping.endpoint,
            time=Series(name="time", unit=mapping.time_unit, values=tuple(times)),
            values=Series(name=mapping.endpoint, unit=mapping.value_unit, values=tuple(values)),
        )
        normalized = canonical_json(observation).encode()
        metadata = mapping.model_dump(
            exclude={"endpoint", "time_column", "value_column", "time_unit", "value_unit"}
        )
        dataset = DatasetSpec(**metadata, artifact_digest=bytes_digest(normalized))
        self.datasets.validate(dataset, observation)
        lineage = CompuCellImportLineage(
            dataset=reference,
            raw_digest=bytes_digest(raw),
            normalized_digest=dataset.artifact_digest,
            import_spec=spec,
            rows=len(times),
            quality_checks=(
                "Explicit delimiter and named columns; no dropped, sorted or imputed rows",
                "Finite scalars and strictly increasing nonnegative integer Monte Carlo steps",
                "Time equals step multiplied by the declared physical time per step",
                "Exact raw bytes and normalized observation stored as separate SHA-256 artifacts",
            ),
            limitations=(
                "This is an operator-mapped scalar export, not a universal native-format parser.",
                "Time mapping, seed, source/runtime digests and permissions are declarations.",
                "No external code executed; no reproduction or biological validity proved.",
            ),
        )
        self.datasets.artifacts.put(raw)
        self.datasets.artifacts.put(normalized)
        self.datasets.register_import(dataset, "compucell_lineage", lineage.model_dump_json())
        return lineage

    def lineage(self, reference: Reference) -> CompuCellImportLineage:
        lineage = CompuCellImportLineage.model_validate_json(
            self.datasets.db.object("compucell_lineage", reference.id, reference.version)
        )
        spec, observation = self.datasets.get(reference)
        mapping = lineage.import_spec.dataset
        if (
            lineage.dataset != reference
            or lineage.normalized_digest != spec.artifact_digest
            or mapping.id != reference.id
            or mapping.version != reference.version
            or mapping.context != spec.context
            or mapping.endpoint != observation.endpoint
        ):
            raise InvalidRequest("CompuCell scalar lineage differs from its dataset")
        self.datasets.artifacts.get(lineage.raw_digest)
        return lineage
