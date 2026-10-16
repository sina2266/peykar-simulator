import json
from concurrent.futures import ThreadPoolExecutor
from threading import Barrier

import pytest
from pydantic import ValidationError
from typer.testing import CliRunner

from peykar.compucell_cli import compucell_app
from peykar.data_fabric.compucell_import import CompuCellImporter
from peykar.data_fabric.csv_import import import_csv
from peykar.data_fabric.service import DatasetService
from peykar.examples import SYNTHETIC
from peykar.validation.repeated_seeds import RepeatedSeedService
from peykar_core.contracts import (
    CompuCellImportSpec,
    CSVImportSpec,
    Quantity,
    Reference,
    RepeatedSeedProtocol,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import bytes_digest


def mapping(id="scalar.reference7", seed=7, origin="synthetic", time_unit="second"):
    return CompuCellImportSpec(
        dataset=CSVImportSpec(
            id=id,
            version="1.0.0",
            title="Synthetic scalar export fixture, no external model execution",
            origin=origin,
            role="benchmark",
            context=SYNTHETIC,
            license="CC0-1.0",
            source="Locally invented infrastructure fixture",
            independence_group="synthetic-scalar-fixture",
            endpoint="signal",
            time_column="mcs",
            value_column="scalar",
            time_unit=time_unit,
            value_unit="dimensionless",
        ),
        delimiter="\t",
        time_per_step=Quantity(value=0.5, unit="second"),
        time_mapping_evidence="Synthetic choice for a fixture; not a biological time mapping",
        seed=seed,
        source_digest=bytes_digest(b"synthetic fixture source descriptor"),
        runtime_digest=bytes_digest(b"synthetic fixture runtime descriptor"),
        engine_version="4.7.0",
    )


def data(values=(10, 11, 12), steps=(0, 10, 20)):
    return (
        b"mcs\tscalar\tuninterpreted\r\n"
        + "".join(
            f"{step}\t{value}\tkept in raw file\r\n"
            for step, value in zip(steps, values, strict=True)
        ).encode()
    )


def import_seed(service, id, seed, values=(10, 11, 12), **kwargs):
    importer = CompuCellImporter(DatasetService(service.db, service.artifacts))
    return importer.import_output(mapping(id=id, seed=seed, **kwargs), data(values)).dataset


def protocol(references, tolerance=0.2):
    return RepeatedSeedProtocol(
        id="scalar.seed-regression",
        version="1.0.0",
        context=SYNTHETIC,
        endpoint="signal",
        references=references,
        tolerance=Quantity(value=tolerance, unit="dimensionless"),
    )


def test_scalar_import_preserves_raw_mapping_and_immutable_versions(service):
    datasets = DatasetService(service.db, service.artifacts)
    importer = CompuCellImporter(datasets)
    raw = b"\xef\xbb\xbf" + data()
    lineage = importer.import_output(mapping(), raw)
    assert importer.lineage(lineage.dataset) == lineage
    assert service.artifacts.get(lineage.raw_digest) == raw
    spec, observation = datasets.get(lineage.dataset)
    assert observation.time.values == (0.0, 5.0, 10.0)
    assert observation.values.values == (10.0, 11.0, 12.0)
    assert spec.origin == "synthetic" and lineage.import_spec.seed == 7
    assert importer.import_output(mapping(), raw) == lineage
    with pytest.raises(Conflict):
        importer.import_output(mapping(), data())  # Same scalar values, different exact bytes.
    assert importer.lineage(lineage.dataset) == lineage


@pytest.mark.parametrize(
    "raw",
    [
        b"mcs\tscalar\n0\t1\n0\t2\n",
        b"mcs\tscalar\n0\t1\n1.5\t2\n",
        b"mcs\tscalar\n-1\t1\n1\t2\n",
        b"mcs\tscalar\n0\t1\n4294967296\t2\n",
        b"mcs\tscalar\n0\t1\n1\tNaN\n",
        b"mcs\tscalar\n0\t1\n",
        b"mcs\tscalar\n0\t1\n1\t2\textra\n",
        b"mcs\tmcs\n0\t1\n1\t2\n",
        b"mcs\tscalar\n0\t1\n1\t\xff\n",
        b"mcs\tscalar\n0\t1\n\n1\t2\n",
    ],
)
def test_scalar_import_rejects_bad_steps_rows_and_scalars_atomically(service, raw):
    importer = CompuCellImporter(DatasetService(service.db, service.artifacts))
    with pytest.raises(InvalidRequest):
        importer.import_output(mapping(), raw)
    with pytest.raises(NotFound):
        service.db.object("dataset", "scalar.reference7", "1.0.0")
    with pytest.raises(NotFound):
        service.db.object("compucell_lineage", "scalar.reference7", "1.0.0")


def test_scalar_mapping_cannot_imply_experimental_origin_or_invent_time_units(service):
    with pytest.raises(ValidationError, match="experimental origin"):
        mapping(origin="experimental")
    importer = CompuCellImporter(DatasetService(service.db, service.artifacts))
    spec = mapping().model_copy(update={"time_per_step": Quantity(value=1.0, unit="meter")})
    with pytest.raises(InvalidRequest, match="Cannot convert"):
        importer.import_output(spec, data())


def test_frozen_seed_protocol_compares_all_seeds_in_units_and_records_failures(service):
    references = tuple(import_seed(service, f"ref.seed{seed}", seed) for seed in (7, 11))
    candidates = (
        import_seed(service, "candidate.seed11", 11, (10, 11.1, 12.1), time_unit="minute"),
        import_seed(service, "candidate.seed7", 7, (10, 11.1, 12.1)),
    )
    service_ = RepeatedSeedService(DatasetService(service.db, service.artifacts))
    frozen = service_.freeze(protocol(references))
    reference = Reference(id=frozen.spec.id, version=frozen.spec.version)
    assert service_.get_protocol(reference) == frozen
    report = service_.compare(reference, candidates)
    assert report.passed and report.evidence_class == "synthetic_verification"
    assert [item.seed for item in report.comparisons] == [7, 11]
    assert report.comparisons[0].max_absolute_error == pytest.approx(0.1)
    assert service_.compare(reference, tuple(reversed(candidates))) == report
    assert service_.get_report(report.id) == report
    bad = import_seed(service, "candidate.bad7", 7, (10, 11, 15))
    failed = service_.compare(reference, (bad, candidates[0]))
    assert not failed.passed and not failed.comparisons[0].passed
    with pytest.raises(Conflict):
        service_.freeze(protocol(references, tolerance=1.0))


def test_seed_protocol_rejects_missing_duplicate_and_mismatched_seeds(service):
    references = tuple(import_seed(service, f"ref.seed{seed}", seed) for seed in (7, 11))
    service_ = RepeatedSeedService(DatasetService(service.db, service.artifacts))
    frozen = service_.freeze(protocol(references))
    reference = Reference(id=frozen.spec.id, version=frozen.spec.version)
    candidate7 = import_seed(service, "candidate.seed7", 7)
    other7 = import_seed(service, "candidate.other7", 7)
    other13 = import_seed(service, "candidate.seed13", 13)
    with pytest.raises(InvalidRequest, match="exactly one"):
        service_.compare(reference, (candidate7,))
    with pytest.raises(InvalidRequest, match="distinct seeds"):
        service_.compare(reference, (candidate7, other7))
    with pytest.raises(InvalidRequest, match="exactly match"):
        service_.compare(reference, (candidate7, other13))
    with pytest.raises(InvalidRequest, match="distinct seeds"):
        service_.freeze(protocol((candidate7, other7)))


def test_seed_protocol_rejects_changed_grids_and_mapping(service):
    references = tuple(import_seed(service, f"ref.seed{seed}", seed) for seed in (7, 11))
    service_ = RepeatedSeedService(DatasetService(service.db, service.artifacts))
    service_.freeze(protocol(references))
    reference = Reference(id="scalar.seed-regression", version="1.0.0")
    candidate7 = import_seed(service, "candidate.seed7", 7)
    importer = CompuCellImporter(DatasetService(service.db, service.artifacts))
    bad = importer.import_output(mapping(id="bad.grid11", seed=11), data(steps=(0, 10, 21)))
    with pytest.raises(InvalidRequest, match="time grids"):
        service_.compare(reference, (candidate7, bad.dataset))
    bad_spec = mapping(id="bad.mapping11", seed=11).model_copy(
        update={"time_per_step": Quantity(value=0.25, unit="second")}
    )
    bad = importer.import_output(bad_spec, data())
    with pytest.raises(InvalidRequest, match="cohort"):
        service_.compare(reference, (candidate7, bad.dataset))


def test_seed_comparison_accepts_equivalent_units_despite_conversion_roundoff(service):
    datasets = DatasetService(service.db, service.artifacts)
    importer = CompuCellImporter(datasets)
    references, candidates = [], []
    for seed in (7, 11):
        for prefix, time_unit, destination in (
            ("reference", "second", references),
            ("candidate", "minute", candidates),
        ):
            spec = mapping(id=f"{prefix}.seed{seed}", seed=seed, time_unit=time_unit)
            destination.append(importer.import_output(spec, data(steps=(0, 13, 29))).dataset)
    seed_service = RepeatedSeedService(datasets)
    frozen = seed_service.freeze(protocol(tuple(references)))
    report = seed_service.compare(
        Reference(id=frozen.spec.id, version=frozen.spec.version), tuple(candidates)
    )
    assert report.passed
    assert all(item.rmse == 0 for item in report.comparisons)


def test_seed_commands_import_freeze_compare_and_read(tmp_path, monkeypatch):
    monkeypatch.setenv("PEYKAR_DATA_DIR", str(tmp_path / "state"))
    runner = CliRunner()
    raw_file = tmp_path / "output.tsv"
    raw_file.write_bytes(data())
    references = []
    for seed in (7, 11):
        spec = mapping(id=f"cli.seed{seed}", seed=seed)
        spec_file = tmp_path / f"seed{seed}.json"
        spec_file.write_text(spec.model_dump_json())
        result = runner.invoke(
            compucell_app, ["import-output", str(raw_file), "--spec", str(spec_file)]
        )
        assert result.exit_code == 0, result.output
        references.append(Reference(**json.loads(result.stdout)["dataset"]))
    protocol_file = tmp_path / "protocol.json"
    protocol_file.write_text(protocol(tuple(references)).model_dump_json())
    result = runner.invoke(compucell_app, ["freeze-seeds", str(protocol_file)])
    assert result.exit_code == 0, result.output
    candidate_file = tmp_path / "candidates.json"
    candidate_file.write_text(json.dumps([r.model_dump(mode="json") for r in references]))
    result = runner.invoke(
        compucell_app,
        ["compare-seeds", "scalar.seed-regression", "1.0.0", str(candidate_file)],
    )
    assert result.exit_code == 0, result.output
    report_id = json.loads(result.stdout)["id"]
    result = runner.invoke(compucell_app, ["seed-report", report_id])
    assert result.exit_code == 0 and json.loads(result.stdout)["passed"], result.output


def csv_equivalent(spec):
    return spec.dataset.model_copy(update={"time_column": "seconds"})


@pytest.mark.parametrize("first", ["csv", "compucell"])
def test_cross_importer_provenance_collision_preserves_legacy_versions(service, first):
    datasets = DatasetService(service.db, service.artifacts)
    importer = CompuCellImporter(datasets)
    spec = mapping()
    csv_raw = b"seconds,scalar\n0,10\n5,11\n10,12\n"
    if first == "csv":
        original = import_csv(datasets, csv_equivalent(spec), csv_raw)
    else:
        original = importer.import_output(spec, data())
    # Simulate the pre-identity registry without changing the historical lineage.
    with datasets.db.connection(immediate=True) as db:
        db.execute("DELETE FROM objects WHERE kind='dataset_import'")
    if first == "csv":
        with pytest.raises(Conflict, match="different import origin"):
            importer.import_output(spec, data())
        assert import_csv(datasets, csv_equivalent(spec), csv_raw) == original
        assert datasets.lineage(original.dataset) == original
    else:
        with pytest.raises(Conflict, match="different import origin"):
            import_csv(datasets, csv_equivalent(spec), csv_raw)
        assert importer.import_output(spec, data()) == original
        assert importer.lineage(original.dataset) == original
    identity = json.loads(datasets.db.object("dataset_import", spec.dataset.id, "1.0.0"))
    assert identity["lineage_kind"] == (
        "dataset_lineage" if first == "csv" else "compucell_lineage"
    )


def test_concurrent_cross_importer_collision_has_one_atomic_winner(service, monkeypatch):
    datasets = DatasetService(service.db, service.artifacts)
    importer = CompuCellImporter(datasets)
    spec = mapping()
    barrier = Barrier(2)
    lookup = datasets.db.object

    def synchronized_lookup(kind, id, version):
        try:
            return lookup(kind, id, version)
        except NotFound:
            if kind == "dataset" and id == spec.dataset.id:
                barrier.wait(timeout=5)
            raise

    monkeypatch.setattr(datasets.db, "object", synchronized_lookup)

    def attempt(kind):
        try:
            if kind == "csv":
                import_csv(
                    datasets,
                    csv_equivalent(spec),
                    b"seconds,scalar\n0,10\n5,11\n10,12\n",
                )
            else:
                importer.import_output(spec, data())
        except Conflict:
            return kind, False
        return kind, True

    with ThreadPoolExecutor(max_workers=2) as pool:
        results = tuple(pool.map(attempt, ("csv", "compucell")))
    assert sum(succeeded for _, succeeded in results) == 1
    winner = next(kind for kind, succeeded in results if succeeded)
    losing_kind = "compucell_lineage" if winner == "csv" else "dataset_lineage"
    with pytest.raises(NotFound):
        lookup(losing_kind, spec.dataset.id, "1.0.0")
    identity = json.loads(lookup("dataset_import", spec.dataset.id, "1.0.0"))
    assert identity["lineage_kind"] == (
        "dataset_lineage" if winner == "csv" else "compucell_lineage"
    )
