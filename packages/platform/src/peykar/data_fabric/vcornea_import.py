"""Native output from the pinned vCornea writer, without executing its source.

Source: Local/Project/*/Simulation/vCornea_v2Steppables.py at the pinned commit.
Time is already expressed in hours; Parquet Height is a lattice coordinate.
"""

import csv
import io
import json
import math
from typing import Any, Literal

from peykar.data_fabric.service import DatasetService
from peykar.infrastructure.artifacts import ArtifactStore
from peykar.infrastructure.database import Database
from peykar_core.contracts import (
    DatasetSpec,
    Observation,
    Reference,
    Series,
    VCorneaOutputArtifact,
    VCorneaOutputLineage,
    VCorneaOutputSpec,
    VCorneaScalarLineage,
)
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, canonical_json, digest

UPSTREAM_COMMIT = "f5c03afb9c4c7d909494146c5db608389e957070"
CELL_COLUMNS = ("Superficial", "Wing", "Basal", "Stem", "Time")
THICKNESS_COLUMNS = ("Time", "Bin", "Height")
MAX_NATIVE_BYTES = 100 * 1024 * 1024
MAX_NATIVE_SAMPLES = 100_001


def _bytes(raw: bytes) -> None:
    if not raw or len(raw) > MAX_NATIVE_BYTES:
        raise InvalidRequest("Native output must contain 1 to 104857600 bytes")


class VCorneaImporter:
    def __init__(self, db: Database, artifacts: ArtifactStore) -> None:
        self.db, self.artifacts = db, artifacts

    def import_cell_counts(self, spec: VCorneaOutputSpec, raw: bytes) -> VCorneaOutputLineage:
        if spec.kind != "cell_count":
            raise InvalidRequest("Cell-count CSV requires cell_count output metadata")
        _bytes(raw)
        times: list[float] = []
        counts: list[list[float]] = [[] for _ in range(4)]
        try:
            text = raw.decode("utf-8-sig")
            if "\x00" in text:
                raise ValueError("NUL byte")
            reader = csv.reader(io.StringIO(text, newline=""), strict=True)
            if tuple(next(reader)) != CELL_COLUMNS:
                raise ValueError("Native cell-count columns differ")
            for row in reader:
                if len(row) != 5 or len(times) >= MAX_NATIVE_SAMPLES:
                    raise ValueError("Ragged row or sample limit")
                values = tuple(float(value) for value in row)
                if any(not math.isfinite(value) for value in values):
                    raise ValueError("Nonfinite native value")
                time = values[4]
                if time < 0 or not time.is_integer():
                    raise ValueError("Cell-count Time must contain nonnegative whole hours")
                if any(
                    value < 0 or value > 2**53 - 1 or not value.is_integer() for value in values[:4]
                ):
                    raise ValueError("Native counts must contain nonnegative exact integers")
                times.append(time)
                for column, value in zip(counts, values[:4], strict=True):
                    column.append(value)
            output = VCorneaOutputArtifact(
                reference=Reference(id=spec.id, version=spec.version),
                kind="cell_count",
                origin=spec.origin,
                time=Series(name="Time", unit="hour", values=tuple(times)),
                outputs=tuple(
                    Series(name=name, unit="count", values=tuple(values))
                    for name, values in zip(CELL_COLUMNS[:4], counts, strict=True)
                ),
                seed=spec.seed,
                limitations=self._limitations(spec),
            )
        except (UnicodeDecodeError, csv.Error, ValueError, StopIteration) as exc:
            raise InvalidRequest(
                "Invalid native vCornea CSV: four count columns then hourly Time required"
            ) from exc
        return self._save(spec, output, raw, len(times), CELL_COLUMNS)

    def import_thickness(
        self, spec: VCorneaOutputSpec, raw: bytes, decoded: bytes
    ) -> VCorneaOutputLineage:
        if spec.kind != "thickness":
            raise InvalidRequest("Thickness Parquet requires thickness output metadata")
        _bytes(raw)
        _bytes(decoded)
        if raw[:4] != b"PAR1" or raw[-4:] != b"PAR1":
            raise InvalidRequest("Expected an ordinary native Parquet file")
        try:
            document: dict[str, Any] = json.loads(decoded)
            if set(document) != {
                "schema_version",
                "format",
                "schema_reference_commit",
                "raw_digest",
                "normalizer_digest",
                "columns",
                "rows",
            }:
                raise ValueError("Decoded descriptor fields differ")
            if (
                document["schema_version"] != "1.0.0"
                or document["format"] != "vcornea.thickness/1.0.0"
                or document["schema_reference_commit"] != UPSTREAM_COMMIT
                or document["raw_digest"] != bytes_digest(raw)
                or document["normalizer_digest"] != spec.normalizer_digest
                or document["columns"] != list(THICKNESS_COLUMNS)
            ):
                raise ValueError("Decoded native output identity differs")
            rows = document["rows"]
            if not isinstance(rows, list) or not 20 <= len(rows) <= MAX_NATIVE_SAMPLES * 10:
                raise ValueError("Decoded thickness row limit")
            times: list[float] = []
            heights: list[list[float]] = [[] for _ in range(10)]
            for index, row in enumerate(rows):
                if not isinstance(row, list) or len(row) != 3:
                    raise ValueError("Thickness row must have Time,Bin,Height")
                time, bin_index, height = row
                if (
                    type(time) not in (int, float)
                    or type(bin_index) is not int
                    or type(height) not in (int, float)
                    or not math.isfinite(time)
                    or not math.isfinite(height)
                    or time < 0
                    or not float(time).is_integer()
                    or height < 0
                    or bin_index != index % 10
                ):
                    raise ValueError("Invalid native time, bin order or height")
                if bin_index == 0:
                    times.append(float(time))
                elif time != times[-1]:
                    raise ValueError("Every time group must contain all ten bins")
                heights[bin_index].append(float(height))
            output = VCorneaOutputArtifact(
                reference=Reference(id=spec.id, version=spec.version),
                kind="thickness",
                origin=spec.origin,
                time=Series(name="Time", unit="hour", values=tuple(times)),
                outputs=tuple(
                    Series(name=f"bin.{index}", unit="lattice_y_coordinate", values=tuple(values))
                    for index, values in enumerate(heights)
                ),
                seed=spec.seed,
                limitations=self._limitations(spec)
                + (
                    "Height is mean top-cell yCOM, not calibrated physical tissue thickness.",
                    "Parquet decoding is delegated to the recorded trusted runtime normalizer.",
                ),
            )
        except (ValueError, TypeError, KeyError, OverflowError) as exc:
            raise InvalidRequest("Invalid pinned vCornea thickness normalization") from exc
        return self._save(spec, output, raw, len(rows), THICKNESS_COLUMNS, decoded)

    @staticmethod
    def _limitations(spec: VCorneaOutputSpec) -> tuple[str, ...]:
        return (
            "Native schema/hours follow the pinned writer; biological validity is unestablished.",
            "Source/runtime/context/license metadata are supplied; output bytes do not prove them.",
            "Upstream output has no seed; supplied seeds require separate runtime evidence.",
            "No seed was supplied; seed remains unknown."
            if spec.seed is None
            else "Seed is recorded in supplied runtime evidence, not inferred from a filename.",
        )

    def _save(
        self,
        spec: VCorneaOutputSpec,
        output: VCorneaOutputArtifact,
        raw: bytes,
        rows: int,
        columns: tuple[str, ...],
        decoded: bytes | None = None,
    ) -> VCorneaOutputLineage:
        normalized = canonical_json(output).encode()
        lineage = VCorneaOutputLineage(
            reference=output.reference,
            raw_digest=bytes_digest(raw),
            normalized_digest=bytes_digest(normalized),
            decoded_digest=bytes_digest(decoded) if decoded is not None else None,
            spec=spec,
            rows=rows,
            columns=columns,
            quality_checks=(
                "Exact pinned native columns, finite numeric values and aligned increasing times",
                "Time retained in hours; no implicit MCS conversion, interpolation or imputation",
                "Exact raw and canonical normalized bytes preserved under separate SHA-256 hashes",
            ),
            limitations=output.limitations,
        )
        self.artifacts.put(raw)
        if decoded is not None:
            self.artifacts.put(decoded)
        self.artifacts.put(normalized)
        self.db.register_many(
            (
                ("vcornea_output", spec.id, spec.version, output.model_dump_json()),
                ("vcornea_output_lineage", spec.id, spec.version, lineage.model_dump_json()),
            )
        )
        return lineage

    def get_lineage(self, reference: Reference) -> VCorneaOutputLineage:
        lineage = VCorneaOutputLineage.model_validate_json(
            self.db.object("vcornea_output_lineage", reference.id, reference.version)
        )
        if (
            lineage.reference != reference
            or lineage.spec.id != reference.id
            or lineage.spec.version != reference.version
        ):
            raise Conflict("Native output lineage differs from its reference")
        self.artifacts.get(lineage.raw_digest)
        if lineage.decoded_digest is not None:
            self.artifacts.get(lineage.decoded_digest)
        return lineage

    def get_output(self, reference: Reference) -> VCorneaOutputArtifact:
        lineage = self.get_lineage(reference)
        output = VCorneaOutputArtifact.model_validate_json(
            self.db.object("vcornea_output", reference.id, reference.version)
        )
        if (
            output.reference != reference
            or output.kind != lineage.spec.kind
            or output.origin != lineage.spec.origin
            or output.seed != lineage.spec.seed
            or bytes_digest(canonical_json(output).encode()) != lineage.normalized_digest
        ):
            raise Conflict("Native output differs from its immutable lineage")
        self.artifacts.get(lineage.normalized_digest)
        return output

    def extract_scalar(
        self,
        reference: Reference,
        dataset: Reference,
        column: Literal["Superficial", "Wing", "Basal", "Stem"],
        independence_group: str,
    ) -> VCorneaScalarLineage:
        """Expose one unchanged native count as a benchmark observation."""
        if not independence_group.strip() or len(independence_group) > 1000:
            raise InvalidRequest("Scalar extraction requires a declared independence group")
        output = self.get_output(reference)
        native = self.get_lineage(reference)
        if output.kind != "cell_count" or column not in CELL_COLUMNS[:4]:
            raise InvalidRequest("Scalar extraction supports only the four native count columns")
        series = next(series for series in output.outputs if series.name == column)
        endpoint = f"vcornea.{column.lower()}_count"
        observation = Observation(
            dataset=dataset,
            endpoint=endpoint,
            time=output.time,
            values=Series(name=endpoint, unit="count", values=series.values),
        )
        normalized = canonical_json(observation).encode()
        metadata = DatasetSpec(
            id=dataset.id,
            version=dataset.version,
            title=f"{native.spec.title}: {column} count",
            origin=native.spec.origin,
            role="benchmark",
            context=native.spec.context,
            artifact_digest=bytes_digest(normalized),
            license=native.spec.license,
            source=native.spec.source,
            independence_group=independence_group,
        )
        datasets = DatasetService(self.db, self.artifacts)
        datasets.validate(metadata, observation)
        lineage = VCorneaScalarLineage(
            dataset=dataset,
            native_output=reference,
            native_lineage_digest=digest(native),
            normalized_digest=metadata.artifact_digest,
            column=column,
            limitations=output.limitations
            + ("One unchanged count column; no aggregation or independent experimental evidence.",),
        )
        self.artifacts.put(normalized)
        datasets.register_import(metadata, "vcornea_scalar_lineage", lineage.model_dump_json())
        return lineage

    def get_scalar_lineage(self, reference: Reference) -> VCorneaScalarLineage:
        lineage = VCorneaScalarLineage.model_validate_json(
            self.db.object("vcornea_scalar_lineage", reference.id, reference.version)
        )
        native = self.get_lineage(lineage.native_output)
        dataset, _ = DatasetService(self.db, self.artifacts).get(reference)
        if (
            lineage.dataset != reference
            or lineage.normalized_digest != dataset.artifact_digest
            or lineage.native_lineage_digest != digest(native)
        ):
            raise Conflict("Extracted scalar differs from its pinned native output")
        return lineage
