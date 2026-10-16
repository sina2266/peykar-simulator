import json
import runpy
from pathlib import Path

import pytest
from pydantic import ValidationError
from typer.testing import CliRunner

from peykar.data_fabric.csv_import import import_csv
from peykar.data_fabric.service import DatasetService
from peykar.data_fabric.vcornea_import import UPSTREAM_COMMIT, VCorneaImporter
from peykar.examples import SYNTHETIC
from peykar.validation.repeated_seeds import RepeatedSeedService
from peykar.vcornea_cli import vcornea_app
from peykar_core.contracts import (
    CSVImportSpec,
    Quantity,
    Reference,
    RepeatedSeedProtocol,
    VCorneaOutputSpec,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import bytes_digest
from peykar_core.units import convert

ROOT = Path(__file__).resolve().parents[2]
NORMALIZER = ROOT / "scripts/vcornea_normalize.py"


def spec(**updates):
    values = {
        "id": "vcornea.native-fixture",
        "version": "1.0.0",
        "title": "Invented native-schema infrastructure fixture; no engine executed",
        "kind": "cell_count",
        "origin": "synthetic",
        "context": SYNTHETIC,
        "license": "LicenseRef-Peykar-Noncommercial-1.1.0",
        "source": "Locally invented values matching the inspected upstream writer schema",
        "upstream_commit": UPSTREAM_COMMIT,
        "variant": "paper_version",
        "source_tree_digest": bytes_digest(b"synthetic source descriptor"),
    }
    return VCorneaOutputSpec(**(values | updates))


def counts():
    return b"Superficial,Wing,Basal,Stem,Time\r\n10,20,30,4,0.0\r\n11,21,31,4,1.0\r\n"


def thickness():
    # This stub tests the management-side trusted-decoder boundary only.
    # Actual Parquet decoding is a separate isolated-runtime integration check.
    raw = b"PAR1-synthetic-reader-boundary-fixture-PAR1"
    records = [(float(time), index, 10.0 + index) for time in (0, 1) for index in range(10)]
    document = runpy.run_path(str(NORMALIZER))["normalized_document"](raw, records)
    return raw, document


def test_native_counts_keep_hours_exact_bytes_all_four_series_and_unknown_seed(service):
    importer = VCorneaImporter(service.db, service.artifacts)
    raw = b"\xef\xbb\xbf" + counts()
    metadata = spec()
    lineage = importer.import_cell_counts(metadata, raw)
    output = importer.get_output(lineage.reference)
    assert output.time.unit == "hour" and output.time.values == (0.0, 1.0)
    assert convert(Quantity(value=output.time.values[1], unit=output.time.unit), "second") == 3600
    assert [series.name for series in output.outputs] == ["Superficial", "Wing", "Basal", "Stem"]
    assert output.outputs[2].values == (30.0, 31.0)
    assert all(series.unit == "count" for series in output.outputs)
    assert output.seed is None and output.origin == "synthetic"
    assert output.validity_status == "unvalidated"
    assert lineage.raw_digest == bytes_digest(raw)
    assert service.artifacts.get(lineage.raw_digest) == raw
    assert importer.get_lineage(lineage.reference) == lineage
    assert importer.import_cell_counts(metadata, raw) == lineage
    with pytest.raises(Conflict):
        importer.import_cell_counts(metadata, counts())
    with pytest.raises(Conflict):
        importer.import_cell_counts(spec(source_tree_digest="b" * 64), raw)
    assert importer.get_lineage(lineage.reference) == lineage


@pytest.mark.parametrize(
    "raw",
    [
        b"Time,Superficial,Wing,Basal,Stem\n0,1,2,3,4\n1,1,2,3,4\n",
        b"Superficial,Wing,Basal,Stem,Time\n1,2,3,4,0\n1,2,3,4,0\n",
        b"Superficial,Wing,Basal,Stem,Time\n1,2,3,4,0\n1,2,3,4,0.1\n",
        b"Superficial,Wing,Basal,Stem,Time\n1,2,3,4,0\n1,2,3,-1,1\n",
        b"Superficial,Wing,Basal,Stem,Time\n1,2,3,4,0\n1,2,3,1.5,1\n",
        b"Superficial,Wing,Basal,Stem,Time\n1,2,3,4,0\n1,2,3,NaN,1\n",
        b"Superficial,Wing,Basal,Stem,Time\n1,2,3,4,0\n1,2,3,4,1,extra\n",
        b"Superficial,Wing,Basal,Stem,Time\n1,2,3,4,0\n",
        b"Superficial,Wing,Basal,Stem,Time\n1,2,3,4,0\n\n1,2,3,4,1\n",
        b"Superficial,Wing,Basal,Stem,Time\n1,2,3,4,0\n1,2,3,\xff,1\n",
    ],
)
def test_native_counts_reject_changed_schema_and_invalid_values_without_registration(service, raw):
    importer = VCorneaImporter(service.db, service.artifacts)
    with pytest.raises(InvalidRequest):
        importer.import_cell_counts(spec(), raw)
    with pytest.raises(NotFound):
        service.db.object("vcornea_output", "vcornea.native-fixture", "1.0.0")
    with pytest.raises(NotFound):
        service.db.object("vcornea_output_lineage", "vcornea.native-fixture", "1.0.0")


def test_native_metadata_preserves_archive_identity_and_requires_seed_evidence(service):
    metadata = spec(
        upstream_commit=None,
        source_archive_doi="10.5281/zenodo.16884414",
        source_archive_digest="a" * 64,
        seed=42,
        seed_evidence="Synthetic fixture declaration, not an actual engine seed claim",
    )
    importer = VCorneaImporter(service.db, service.artifacts)
    lineage = importer.import_cell_counts(metadata, counts())
    assert lineage.spec.upstream_commit is None
    assert lineage.spec.source_archive_doi == "10.5281/zenodo.16884414"
    assert importer.get_output(lineage.reference).seed == 42
    with pytest.raises(ValidationError, match="requires explicit"):
        spec(seed=42)
    with pytest.raises(ValidationError, match="pinned commit or exact archive"):
        spec(upstream_commit=None)
    with pytest.raises(ValidationError, match="both exact DOI"):
        spec(source_archive_doi="10.5281/zenodo.16884414")
    with pytest.raises(ValidationError):
        spec(upstream_commit="unreviewed-source-revision")


def test_native_thickness_keeps_lattice_coordinates_and_decoder_lineage(service):
    importer = VCorneaImporter(service.db, service.artifacts)
    raw, document = thickness()
    metadata = spec(kind="thickness", normalizer_digest=document["normalizer_digest"])
    decoded = json.dumps(document).encode()
    lineage = importer.import_thickness(metadata, raw, decoded)
    output = importer.get_output(lineage.reference)
    assert output.time.unit == "hour" and output.time.values == (0.0, 1.0)
    assert len(output.outputs) == 10 and output.outputs[9].values == (19.0, 19.0)
    assert all(series.unit == "lattice_y_coordinate" for series in output.outputs)
    assert lineage.columns == ("Time", "Bin", "Height") and lineage.rows == 20
    assert lineage.decoded_digest == bytes_digest(decoded)
    assert service.artifacts.get(lineage.raw_digest) == raw
    assert service.artifacts.get(lineage.decoded_digest) == decoded
    assert importer.import_thickness(metadata, raw, decoded) == lineage


@pytest.mark.parametrize(
    "fault", ["raw", "normalizer", "columns", "bin", "time", "height", "missing"]
)
def test_native_thickness_rejects_reader_mismatch_and_invalid_geometry_rows(service, fault):
    importer = VCorneaImporter(service.db, service.artifacts)
    raw, document = thickness()
    document = json.loads(json.dumps(document))
    metadata = spec(kind="thickness", normalizer_digest=document["normalizer_digest"])
    if fault == "raw":
        raw = b"PAR1-different-PAR1"
    elif fault == "normalizer":
        document["normalizer_digest"] = "b" * 64
    elif fault == "columns":
        document["columns"] = ["Time", "Bin", "ThicknessMicrometers"]
    elif fault == "bin":
        document["rows"][5][1] = 4
    elif fault == "time":
        document["rows"][15][0] = 2
    elif fault == "height":
        document["rows"][15][2] = -1
    else:
        document["rows"].pop()
    with pytest.raises(InvalidRequest):
        importer.import_thickness(metadata, raw, json.dumps(document).encode())
    with pytest.raises(NotFound):
        importer.get_lineage(Reference(id=metadata.id, version=metadata.version))


def test_native_cli_import_and_retrieve_without_engine_execution(tmp_path, monkeypatch):
    monkeypatch.setenv("PEYKAR_DATA_DIR", str(tmp_path / "state"))
    file, metadata = tmp_path / "cell_count_11.csv", tmp_path / "native.json"
    file.write_bytes(counts())
    metadata.write_text(spec().model_dump_json())
    runner = CliRunner()
    result = runner.invoke(vcornea_app, ["import-cell-counts", str(file), "--spec", str(metadata)])
    assert result.exit_code == 0, result.output
    for command in ("output", "lineage"):
        result = runner.invoke(vcornea_app, [command, "vcornea.native-fixture", "1.0.0"])
        assert result.exit_code == 0, result.output
    assert json.loads(result.output)["spec"]["seed"] is None


def test_native_scalar_extraction_is_unchanged_pinned_and_exclusive_across_importers(service):
    importer = VCorneaImporter(service.db, service.artifacts)
    native = importer.import_cell_counts(spec(), counts())
    reference = Reference(id="vcornea.stem-benchmark", version="1.0.0")
    scalar = importer.extract_scalar(
        native.reference, reference, "Stem", "synthetic-native-fixture"
    )
    datasets = DatasetService(service.db, service.artifacts)
    metadata, observation = datasets.get(reference)
    assert observation.endpoint == "vcornea.stem_count"
    assert observation.time.unit == "hour" and observation.time.values == (0.0, 1.0)
    assert observation.values.unit == "count" and observation.values.values == (4.0, 4.0)
    assert metadata.origin == "synthetic" and metadata.role == "benchmark"
    assert importer.get_scalar_lineage(reference) == scalar
    assert (
        importer.extract_scalar(native.reference, reference, "Stem", "synthetic-native-fixture")
        == scalar
    )
    with pytest.raises(Conflict):
        importer.extract_scalar(native.reference, reference, "Basal", "synthetic-native-fixture")
    csv_spec = CSVImportSpec(
        **metadata.model_dump(exclude={"artifact_digest"}),
        endpoint=observation.endpoint,
        time_column="Time",
        value_column="vcornea.stem_count",
        time_unit="hour",
        value_unit="count",
    )
    with pytest.raises(Conflict, match="different import origin"):
        import_csv(datasets, csv_spec, b"Time,vcornea.stem_count\n0,4\n1,4\n")
    assert importer.get_scalar_lineage(reference) == scalar


def test_native_repeated_seed_protocol_requires_recorded_seed_and_runtime(service):
    importer = VCorneaImporter(service.db, service.artifacts)
    datasets = DatasetService(service.db, service.artifacts)
    evaluator = RepeatedSeedService(datasets)
    references, candidates = [], []
    for seed in (7, 11):
        for cohort, destination in (("reference", references), ("candidate", candidates)):
            metadata = spec(
                id=f"native.{cohort}.{seed}",
                seed=seed,
                seed_evidence="Synthetic recorded-seed fixture, no engine executed",
                runtime_digest="a" * 64,
            )
            native = importer.import_cell_counts(metadata, counts())
            dataset = Reference(id=f"native.stem.{cohort}.{seed}", version="1.0.0")
            importer.extract_scalar(native.reference, dataset, "Stem", "synthetic-fixture")
            destination.append(dataset)
    protocol = RepeatedSeedProtocol(
        id="native.seed-protocol",
        version="1.0.0",
        context=SYNTHETIC,
        endpoint="vcornea.stem_count",
        references=tuple(references),
        tolerance=Quantity(value=0.0, unit="count"),
    )
    frozen = evaluator.freeze(protocol)
    report = evaluator.compare(
        Reference(id=frozen.spec.id, version=frozen.spec.version), tuple(candidates)
    )
    assert report.passed and report.evidence_class == "synthetic_verification"
    assert [comparison.seed for comparison in report.comparisons] == [7, 11]
    assert evaluator.get_report(report.id) == report
    unknown = importer.import_cell_counts(spec(id="native.unknown-seed"), counts())
    unknown_dataset = Reference(id="native.unknown-stem", version="1.0.0")
    importer.extract_scalar(unknown.reference, unknown_dataset, "Stem", "unknown-seed-fixture")
    with pytest.raises(InvalidRequest, match="recorded seed evidence and runtime"):
        evaluator.freeze(
            protocol.model_copy(update={"references": (references[0], unknown_dataset)})
        )
    without_runtime = importer.import_cell_counts(
        spec(id="native.no-runtime", seed=13, seed_evidence="Synthetic fixture seed"), counts()
    )
    missing_runtime = Reference(id="native.no-runtime-stem", version="1.0.0")
    importer.extract_scalar(without_runtime.reference, missing_runtime, "Stem", "fixture")
    with pytest.raises(InvalidRequest, match="recorded seed evidence and runtime"):
        evaluator.freeze(
            protocol.model_copy(update={"references": (references[0], missing_runtime)})
        )
