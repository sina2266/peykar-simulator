"""Explicit offline operator: IOP model 1.1 development and its tranche-2 sealed test.

Training data of model 1.1 are the 72 tranche-1 records of dataset 2026-10-28: the
47 development records (public and privately kept) and the 25 sealed records that
the one-shot test of model 1.0.0 consumed. ``--development`` runs leave-one-study-out
over them; linked studies (tranche-2 author-overlap file, development clusters) leave
training together. ``--model 1.0.0`` runs the frozen 1.0.0 harness unchanged as the
reference. ``--sealed`` refuses to run unless every hash pinned by the frozen 1.1
protocol matches, fits on all 72 training records (minus those linked to a sealed
study) and scores each tranche-2 sealed study exactly once with model 1.1 and, in
the same run, with the frozen model 1.0.0 as the declared reference (H5). Reports go
to a new
directory. No network access, model registration or biological-validity claim is
involved. The 1.0.0 operator stays unchanged because its protocol pins it.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib
import json
import platform
import sys
import time
from datetime import UTC, datetime
from pathlib import Path
from typing import Any

from peykar.validation.eye_iop_data import PressureSeries, to_pressure_series

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/iop-glaucoma-surgery-2026-10-28"
TRAINING_DIRS = (
    DATASET / "development/records",
    # Records derived from NC/ND/SA-licensed sources stay outside the repository.
    ROOT / ".peykar/iop-development-private/records",
    # Tranche-1 sealed records, consumed by the one-shot test of model 1.0.0.
    ROOT / ".peykar/iop-sealed-2026-10-28/records",
)
TRANCHE1_RESERVATION = DATASET / "split-reservation-1.0.0.json"
SEALED = ROOT / ".peykar/iop-tranche2-sealed-2026-10-28/records"
RESERVATION = DATASET / "tranche-2/split-reservation-1.0.0.json"
# Declared author-overlap rule over the 72 training studies and the tranche-2 admits.
OVERLAP = DATASET / "tranche-2/author-overlap-1.0.0.json"
HARNESSES = {
    "1.0.0": "peykar.validation.eye_iop_evaluation",
    "1.1.0": "peykar.validation.eye_iop_evaluation_1_1",
}
MODELS = {
    "1.0.0": "peykar.model_learning.iop_population",
    "1.1.0": "peykar.model_learning.iop_population_1_1",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def training_paths() -> list[Path]:
    return sorted(
        (p for directory in TRAINING_DIRS for p in directory.glob("*.json")), key=lambda p: p.name
    )


def load_training() -> tuple[list[PressureSeries], dict[str, str]]:
    """The 72 tranche-1 records; consumed sealed records must be tranche-1 sealed studies."""
    reserved = {
        s["study_key"]: s["split"] for s in json.loads(TRANCHE1_RESERVATION.read_text())["studies"]
    }
    series: list[PressureSeries] = []
    hashes = {}
    for path in training_paths():
        record = json.loads(path.read_text())
        if reserved.get(record["study_id"]) != record.get("split"):
            raise SystemExit(f"{path.name} does not match the tranche-1 reservation")
        series.extend(to_pressure_series(record))
        hashes[path.name] = sha256(path)
    return series, hashes


def load_sealed() -> tuple[list[PressureSeries], dict[str, str]]:
    reserved = {
        s["study_key"]
        for s in json.loads(RESERVATION.read_text())["studies"]
        if s["split"] == "sealed_test"
    }
    series: list[PressureSeries] = []
    hashes = {}
    for path in sorted(SEALED.glob("*.json")):
        record = json.loads(path.read_text())
        if record.get("split") != "sealed_test" or record["study_id"] not in reserved:
            raise SystemExit(f"{path.name} is not a reserved tranche-2 sealed record")
        series.extend(to_pressure_series(record))
        hashes[path.name] = sha256(path)
    return series, hashes


def check_protocol(protocol: dict[str, Any]) -> list[str]:
    problems = []
    for relative, expected in protocol["frozen_files_sha256"].items():
        path = ROOT / relative
        if not path.is_file():
            problems.append(f"missing {relative}")
        elif sha256(path) != expected:
            problems.append(f"changed {relative}")
    training = {p.name: sha256(p) for p in training_paths()}
    if training != protocol["training_records_sha256"]:
        problems.append("training records differ from the protocol")
    listed = set(protocol["sealed_records_sha256"])
    present = {p.name for p in SEALED.glob("*.json")}
    if present != listed:
        problems.append(f"sealed record set differs: {sorted(present ^ listed)}")
    for name, expected in protocol["sealed_records_sha256"].items():
        path = SEALED / name
        if path.is_file() and sha256(path) != expected:
            problems.append(f"changed sealed {name}")
    return problems


def sealed_report(
    model: str,
    series: list[PressureSeries],
    sealed: list[PressureSeries],
    exclusions: dict[str, list[str]],
    protocol_sha256: str,
) -> dict[str, Any]:
    """Score every sealed study once with one model version (fresh fit per study)."""
    evaluation = importlib.import_module(HARNESSES[model])
    population = importlib.import_module(MODELS[model])
    studies = []
    for study_id in sorted({s.study_id for s in sealed}):
        held = [s for s in sealed if s.study_id == study_id]
        excluded = set(exclusions.get(study_id, []))
        training = [s for s in series if s.study_id not in excluded]
        result = evaluation.evaluate_study(study_series=held, training=training)
        result["training_exclusions"] = sorted(excluded)
        studies.append(result)
        print(model, study_id, file=sys.stderr, flush=True)
    return {
        "evaluation_id": evaluation.EVALUATION_ID,
        "evaluation_version": evaluation.EVALUATION_VERSION,
        "model_id": population.MODEL_ID,
        "model_version": population.MODEL_VERSION,
        "criteria": {
            "rmse_pp_max": evaluation.RMSE_LIMIT_PP,
            "max_abs_pp_max": evaluation.MAX_LIMIT_PP,
            "forecast_persistence_margin_pp": evaluation.PERSISTENCE_MARGIN_PP,
            "unit": "percentage points of each series' preoperative IOP",
        },
        "protocol_sha256": protocol_sha256,
        "studies": studies,
        "summary": evaluation.summarize(studies),
    }


def _short(summary: dict[str, Any]) -> dict[str, Any]:
    keep: dict[str, Any] = {}
    for task in ("next_observation_group_means", "landmark_group_means"):
        keep[task] = {
            method: {k: values[k] for k in ("series", "series_meeting_criteria", "pooled_rmse_pp")}
            for method, values in summary.get(task, {}).items()
        }
    keep["initial_state_group_mean"] = {
        method: {k: values[k] for k in ("series", "series_meeting_criteria", "pooled_rmse_pp")}
        for method, values in summary["initial_state"]["group_mean"].items()
    }
    return keep


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--model", choices=sorted(HARNESSES), default="1.1.0")
    parser.add_argument("--protocol", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.sealed and (args.protocol is None or args.model != "1.1.0"):
        parser.error("--sealed requires --protocol and model 1.1.0")
    if args.output.exists():
        parser.error("--output must not exist; every run gets a fresh directory")
    evaluation = importlib.import_module(HARNESSES[args.model])
    args.output.mkdir(parents=True)
    started = time.monotonic()
    record: dict[str, Any] = {
        "utc_start": datetime.now(UTC).isoformat(),
        "python": platform.python_version(),
        "mode": "development" if args.development else "sealed",
        "model_version": args.model,
        "harness": HARNESSES[args.model],
        "evaluation_version": evaluation.EVALUATION_VERSION,
    }
    series, record["training_record_hashes"] = load_training()
    record["training_studies"] = sorted({s.study_id for s in series})
    overlap = json.loads(OVERLAP.read_text())
    record["author_overlap"] = {"path": str(OVERLAP.relative_to(ROOT)), "sha256": sha256(OVERLAP)}
    if args.development:
        present = {s.study_id for s in series}
        clusters = [
            [m for m in cluster if m in present] for cluster in overlap["development_clusters"]
        ]
        report = evaluation.leave_one_study_out(
            series,
            clusters=[c for c in clusters if len(c) > 1],
            progress=lambda s: print(s, file=sys.stderr, flush=True),
        )
    else:
        protocol = json.loads(args.protocol.read_text())
        record["protocol"] = str(args.protocol)
        record["protocol_sha256"] = sha256(args.protocol)
        problems = check_protocol(protocol)
        if problems:
            record["refused"] = problems
            (args.output / "execution-record.json").write_text(json.dumps(record, indent=1))
            print("Refusing sealed evaluation:", problems, file=sys.stderr)
            return 1
        sealed, record["sealed_record_hashes"] = load_sealed()
        exclusions = overlap["sealed_training_exclusions"]
        report = sealed_report("1.1.0", series, sealed, exclusions, record["protocol_sha256"])
        reference = sealed_report("1.0.0", series, sealed, exclusions, record["protocol_sha256"])
        (args.output / "reference-1.0.0-report.json").write_text(
            json.dumps(reference, indent=1) + "\n"
        )
        record["reference_report_sha256"] = sha256(args.output / "reference-1.0.0-report.json")
    record["elapsed_seconds"] = round(time.monotonic() - started, 3)
    record["utc_end"] = datetime.now(UTC).isoformat()
    (args.output / "report.json").write_text(json.dumps(report, indent=1) + "\n")
    record["report_sha256"] = sha256(args.output / "report.json")
    (args.output / "execution-record.json").write_text(json.dumps(record, indent=1) + "\n")
    print(json.dumps({"summary": _short(report["summary"]), "record": record}, indent=1)[:6000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
