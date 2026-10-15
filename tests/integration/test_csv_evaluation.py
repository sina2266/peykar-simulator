import math

import pytest
from fastapi.testclient import TestClient
from typer.testing import CliRunner

from peykar.api.app import create_app
from peykar.cli import app
from peykar.data_fabric.csv_import import import_csv
from peykar.data_fabric.service import DatasetService
from peykar.examples import SYNTHETIC
from peykar.simulation.worker import Worker
from peykar.validation.service import EvaluationService
from peykar_core.contracts import CSVImportSpec, EvaluationSetSpec, Quantity, Reference
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import bytes_digest


def csv_spec(id="csv.decay", role="benchmark", group="csv-analytic-v1"):
    return CSVImportSpec(
        id=id,
        version="1.0.0",
        title="Synthetic analytic decay",
        origin="synthetic",
        role=role,
        context=SYNTHETIC,
        license="CC0-1.0",
        source="Test analytical formula",
        independence_group=group,
        endpoint="concentration",
        time_column="seconds",
        value_column="mol_per_liter",
        time_unit="second",
        value_unit="mole/liter",
    )


def raw_csv():
    return (
        "seconds,mol_per_liter\r\n" + "".join(f"{i},{math.exp(-0.1 * i)}\r\n" for i in range(11))
    ).encode()


def freeze_spec(reference, calibration=()):
    return EvaluationSetSpec(
        id="decay.holdout",
        version="1.0.0",
        context=SYNTHETIC,
        endpoint="concentration",
        calibration=calibration,
        evaluation=(reference,),
        tolerance=Quantity(value=1e-12, unit="mole/liter"),
    )


def test_csv_preserves_exact_raw_bytes_and_atomic_lineage(service):
    datasets = DatasetService(service.db, service.artifacts)
    raw = b"\xef\xbb\xbf" + raw_csv()
    lineage = import_csv(datasets, csv_spec(), raw)
    assert lineage.rows == 11 and lineage.raw_digest == bytes_digest(raw)
    assert service.artifacts.get(lineage.raw_digest) == raw
    assert datasets.lineage(lineage.dataset) == lineage
    assert import_csv(datasets, csv_spec(), raw) == lineage
    with pytest.raises(Conflict):
        import_csv(datasets, csv_spec(), raw_csv())  # Same observations, different raw file.
    assert datasets.lineage(lineage.dataset) == lineage


@pytest.mark.parametrize(
    "raw",
    [
        b"",
        b"seconds,seconds\n0,1\n1,2\n",
        b"wrong,value\n0,1\n1,2\n",
        b"seconds,mol_per_liter\n0,nan\n1,2\n",
        b"seconds,mol_per_liter\n0,1\n1,inf\n",
        b"seconds,mol_per_liter\n0,1\n0,2\n",
        b"seconds,mol_per_liter\n0,1\n",
        b"seconds,mol_per_liter\n0,1\n\n1,2\n",
        b"seconds,mol_per_liter\n0,1,3\n1,2\n",
        b"seconds,mol_per_liter\n0,\xff\n1,2\n",
    ],
)
def test_csv_rejects_invalid_data_without_registry_writes(service, raw):
    with pytest.raises(InvalidRequest):
        import_csv(DatasetService(service.db, service.artifacts), csv_spec(), raw)
    with pytest.raises(NotFound):
        service.db.object("dataset", "csv.decay", "1.0.0")


def test_related_objects_rollback_together(service):
    db = service.db
    db.register("example", "exists", "1.0.0", '{"value":1}')
    with pytest.raises(Conflict):
        db.register_many(
            (
                ("example", "new", "1.0.0", '{"value":2}'),
                ("example", "exists", "1.0.0", '{"value":3}'),
            )
        )
    with pytest.raises(NotFound):
        db.object("example", "new", "1.0.0")


def test_frozen_evaluation_is_run_bound_retrievable_and_idempotent(service, plan):
    lineage = import_csv(DatasetService(service.db, service.artifacts), csv_spec(), raw_csv())
    evaluations = EvaluationService(service)
    frozen = evaluations.freeze(freeze_spec(lineage.dataset))
    ref = Reference(id=frozen.spec.id, version=frozen.spec.version)
    assert evaluations.get_set(ref) == frozen
    records = []
    for _ in range(2):
        run = service.submit(plan)
        assert Worker(service).tick().status == "succeeded"
        (record,) = evaluations.evaluate_set(run.id, ref)
        assert record.report.passed and record.report.evidence_class == "synthetic_verification"
        assert record.run_id == run.id and record.manifest_digest == run.manifest_digest
        assert evaluations.get_report(record.id) == record
        assert evaluations.evaluate_set(run.id, ref) == (record,)
        records.append(record)
    assert records[0].id != records[1].id
    with pytest.raises(Conflict):
        evaluations.freeze(
            frozen.spec.model_copy(update={"tolerance": Quantity(value=0.1, unit="mole/liter")})
        )


def test_frozen_evaluation_rejects_calibration_leakage(service):
    datasets = DatasetService(service.db, service.artifacts)
    calibration = import_csv(
        datasets, csv_spec("csv.train", "calibration", "same-cohort"), raw_csv()
    )
    evaluation = import_csv(datasets, csv_spec(group="same-cohort"), raw_csv())
    evaluator = EvaluationService(service)
    with pytest.raises(InvalidRequest, match="overlap"):
        evaluator.freeze(freeze_spec(evaluation.dataset, (calibration.dataset,)))
    with pytest.raises(InvalidRequest, match="role"):
        evaluator.freeze(freeze_spec(calibration.dataset))


def test_csv_cli_and_frozen_evaluation_commands(tmp_path, monkeypatch):
    monkeypatch.setenv("PEYKAR_DATA_DIR", str(tmp_path / "state"))
    spec_file, data_file = tmp_path / "import.json", tmp_path / "data.csv"
    spec_file.write_text(csv_spec().model_dump_json())
    data_file.write_bytes(raw_csv())
    runner = CliRunner()
    result = runner.invoke(app, ["import-csv", str(data_file), "--spec", str(spec_file)])
    assert result.exit_code == 0, result.output
    frozen_file = tmp_path / "set.json"
    frozen_file.write_text(
        freeze_spec(Reference(id="csv.decay", version="1.0.0")).model_dump_json()
    )
    result = runner.invoke(app, ["freeze-evaluation", str(frozen_file)])
    assert result.exit_code == 0, result.output


def test_api_csv_evaluation_and_legacy_report_location(service, plan):
    token = "admin-token-at-least-24-characters"
    reader = "reader-token-at-least-24-characters"
    with TestClient(create_app(service.root, token, reader)) as client:
        headers = {"Authorization": f"Bearer {token}"}
        reader_headers = {"Authorization": f"Bearer {reader}"}
        body = {"spec": csv_spec().model_dump(mode="json"), "csv_text": raw_csv().decode()}
        assert (
            client.post("/v1/datasets/import-csv", json=body, headers=reader_headers).status_code
            == 403
        )
        imported = client.post("/v1/datasets/import-csv", json=body, headers=headers)
        assert imported.status_code == 201, imported.text
        assert (
            client.get(
                "/v1/datasets/csv.decay/versions/1.0.0/lineage", headers=reader_headers
            ).status_code
            == 200
        )
        spec = freeze_spec(Reference(id="csv.decay", version="1.0.0"))
        assert (
            client.post(
                "/v1/validation/sets", json=spec.model_dump(mode="json"), headers=headers
            ).status_code
            == 201
        )
        run = service.submit(plan)
        Worker(service).tick()
        evaluation = client.post(
            "/v1/validation/evaluations",
            headers=headers,
            json={
                "run_id": run.id,
                "evaluation_set": {"id": spec.id, "version": spec.version},
            },
        )
        assert evaluation.status_code == 201, evaluation.text
        legacy = client.post(
            "/v1/validation/runs",
            headers=headers,
            json={
                "run_id": run.id,
                "dataset": {"id": "csv.decay", "version": "1.0.0"},
                "tolerance": {"value": 1e-12, "unit": "mole/liter"},
            },
        )
        assert legacy.status_code == 201 and legacy.json()["passed"]
        report = client.get(legacy.headers["Location"], headers=reader_headers)
        assert report.status_code == 200 and report.json()["run_id"] == run.id
