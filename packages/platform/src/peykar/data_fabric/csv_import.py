"""Strict two-column observation import with byte-level raw provenance."""

import csv
import io
import math

from peykar.data_fabric.service import DatasetService
from peykar_core.contracts import (
    CSVImportSpec,
    DatasetLineage,
    DatasetSpec,
    Observation,
    Reference,
    Series,
)
from peykar_core.errors import InvalidRequest
from peykar_core.hashing import bytes_digest, canonical_json

MAX_CSV_BYTES = 20 * 1024 * 1024
MAX_ROWS = 100_001


def import_csv(service: DatasetService, spec: CSVImportSpec, raw: bytes) -> DatasetLineage:
    if not raw or len(raw) > MAX_CSV_BYTES:
        raise InvalidRequest("CSV must contain 1 to 20971520 bytes")
    if spec.time_column == spec.value_column:
        raise InvalidRequest("Time and value columns must differ")
    times: list[float] = []
    values: list[float] = []
    try:
        text = raw.decode("utf-8-sig")
        if "\x00" in text:
            raise ValueError("NUL byte")
        reader = csv.reader(io.StringIO(text, newline=""), strict=True)
        headers = next(reader)
        if len(headers) != len(set(headers)) or any(not name.strip() for name in headers):
            raise ValueError("Duplicate or blank column names")
        if spec.time_column not in headers or spec.value_column not in headers:
            raise ValueError("Required column missing")
        ti, vi = headers.index(spec.time_column), headers.index(spec.value_column)
        for row in reader:
            if len(times) >= MAX_ROWS:
                raise ValueError("Too many observation rows")
            if len(row) != len(headers):
                raise ValueError("Ragged or empty observation row")
            t, v = float(row[ti]), float(row[vi])
            if not math.isfinite(t) or not math.isfinite(v):
                raise ValueError("Nonfinite observation")
            times.append(t)
            values.append(v)
    except (UnicodeDecodeError, csv.Error, ValueError, StopIteration) as exc:
        raise InvalidRequest(
            "Invalid CSV: UTF-8, unique headers and finite numeric rows required"
        ) from exc
    reference = Reference(id=spec.id, version=spec.version)
    observation = Observation(
        dataset=reference,
        endpoint=spec.endpoint,
        time=Series(name="time", unit=spec.time_unit, values=tuple(times)),
        values=Series(name=spec.endpoint, unit=spec.value_unit, values=tuple(values)),
    )
    normalized = canonical_json(observation).encode()
    metadata = spec.model_dump(
        exclude={"endpoint", "time_column", "value_column", "time_unit", "value_unit"}
    )
    dataset = DatasetSpec(**metadata, artifact_digest=bytes_digest(normalized))
    service.validate(dataset, observation)
    lineage = DatasetLineage(
        dataset=reference,
        raw_digest=bytes_digest(raw),
        normalized_digest=dataset.artifact_digest,
        import_spec=spec,
        rows=len(times),
        quality_checks=(
            "UTF-8 CSV with unique headers and consistent row widths",
            "Finite numeric samples; no dropped or imputed rows",
            "Strictly increasing times; aligned values; compatible time units",
            "Origin and independence group are submitter declarations, not verified evidence",
        ),
    )
    service.artifacts.put(raw)
    service.artifacts.put(normalized)
    service.register_import(dataset, "dataset_lineage", lineage.model_dump_json())
    return lineage
