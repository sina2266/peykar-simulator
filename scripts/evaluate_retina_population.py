"""Explicit offline operator: retinal-thickness model development and its sealed test.

Training data are the development records of dataset ``retina-cst-2026-10-28``
(public ones in the repository and privately kept ones). ``--development
--candidate <name>`` runs leave-one-study-out over them with one declared
configuration of ``retina_thickness_population``; studies of one author-overlap
cluster leave training together. ``--sealed`` refuses to run unless every hash
pinned by the frozen protocol matches, fits on all development records (minus those
linked to a sealed study) and scores each sealed study exactly once with the
protocol's primary configuration and, in the same run, with each declared reference
configuration. Reports go to a new directory. No network access, model registration
or clinical-validity claim is involved.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import platform
import sys
import time
from datetime import UTC, datetime
from pathlib import Path
from typing import Any

from peykar.model_learning.eye_recovery_population import RecoveryConfig
from peykar.model_learning.retina_thickness_population import DEVELOPMENT_CANDIDATES
from peykar.validation import eye_recovery_evaluation as evaluation
from peykar.validation.eye_retina_data import RetinaSeries, to_retina_series

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/retina-cst-2026-10-28"
TRAINING_DIRS = (
    DATASET / "development/records",
    # Records derived from NC/ND/SA-licensed sources stay outside the repository.
    ROOT / ".peykar/retina-development-private/records",
)
SEALED = ROOT / ".peykar/retina-sealed-2026-10-28/records"
RESERVATION = DATASET / "split-reservation-1.0.0.json"
OVERLAP = DATASET / "author-overlap-1.0.0.json"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def training_paths() -> list[Path]:
    return sorted(
        (p for directory in TRAINING_DIRS for p in directory.glob("*.json")), key=lambda p: p.name
    )


def _reserved(split: str) -> set[str]:
    return {
        s["study_key"]
        for s in json.loads(RESERVATION.read_text())["studies"]
        if s["split"] == split
    }


def load_training() -> tuple[list[RetinaSeries], dict[str, str]]:
    reserved = _reserved("development")
    series: list[RetinaSeries] = []
    hashes = {}
    for path in training_paths():
        record = json.loads(path.read_text())
        if record.get("split") != "development" or record["study_id"] not in reserved:
            raise SystemExit(f"{path.name} is not a reserved development record")
        series.extend(to_retina_series(record))
        hashes[path.name] = sha256(path)
    return series, hashes


def load_sealed() -> tuple[list[RetinaSeries], dict[str, str]]:
    reserved = _reserved("sealed_test")
    series: list[RetinaSeries] = []
    hashes = {}
    for path in sorted(SEALED.glob("*.json")):
        record = json.loads(path.read_text())
        if record.get("split") != "sealed_test" or record["study_id"] not in reserved:
            raise SystemExit(f"{path.name} is not a reserved sealed record")
        series.extend(to_retina_series(record))
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
    for name in (protocol["primary_candidate"], *protocol.get("reference_candidates", [])):
        if name not in DEVELOPMENT_CANDIDATES:
            problems.append(f"unknown candidate {name}")
        elif DEVELOPMENT_CANDIDATES[name].describe() != protocol["candidates"][name]:
            problems.append(f"candidate {name} differs from the protocol")
    return problems


def sealed_report(
    config: RecoveryConfig,
    series: list[RetinaSeries],
    sealed: list[RetinaSeries],
    exclusions: dict[str, list[str]],
    protocol_sha256: str,
) -> dict[str, Any]:
    """Score every sealed study once with one configuration (fresh fit per study)."""
    studies = []
    for study_id in sorted({s.study_id for s in sealed}):
        held = [s for s in sealed if s.study_id == study_id]
        excluded = set(exclusions.get(study_id, []))
        training = [s for s in series if s.study_id not in excluded]
        result = evaluation.evaluate_study(config, study_series=held, training=training)
        result["training_exclusions"] = sorted(excluded)
        studies.append(result)
        print(config.model_version, study_id, file=sys.stderr, flush=True)
    return {
        "evaluation_id": evaluation.EVALUATION_ID,
        "evaluation_version": evaluation.EVALUATION_VERSION,
        "model_id": config.model_id,
        "model_version": config.model_version,
        "model_configuration": config.describe(),
        "criteria": evaluation.criteria(config),
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
    parser.add_argument("--candidate", choices=sorted(DEVELOPMENT_CANDIDATES))
    parser.add_argument("--protocol", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.development and args.candidate is None:
        parser.error("--development requires --candidate")
    if args.sealed and (args.protocol is None or args.candidate is not None):
        parser.error("--sealed requires --protocol (the protocol names the candidates)")
    if args.output.exists():
        parser.error("--output must not exist; every run gets a fresh directory")
    args.output.mkdir(parents=True)
    started = time.monotonic()
    record: dict[str, Any] = {
        "utc_start": datetime.now(UTC).isoformat(),
        "python": platform.python_version(),
        "mode": "development" if args.development else "sealed",
        "evaluation_version": evaluation.EVALUATION_VERSION,
    }
    series, record["training_record_hashes"] = load_training()
    record["training_studies"] = sorted({s.study_id for s in series})
    overlap = json.loads(OVERLAP.read_text())
    record["author_overlap"] = {"path": str(OVERLAP.relative_to(ROOT)), "sha256": sha256(OVERLAP)}
    if args.development:
        config = DEVELOPMENT_CANDIDATES[args.candidate]
        record["candidate"] = args.candidate
        record["model_configuration"] = config.describe()
        present = {s.study_id for s in series}
        clusters = [
            [m for m in cluster if m in present] for cluster in overlap["development_clusters"]
        ]
        report = evaluation.leave_one_study_out(
            config,
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
        primary = DEVELOPMENT_CANDIDATES[protocol["primary_candidate"]]
        report = sealed_report(primary, series, sealed, exclusions, record["protocol_sha256"])
        record["reference_reports"] = {}
        for name in protocol.get("reference_candidates", []):
            reference = sealed_report(
                DEVELOPMENT_CANDIDATES[name], series, sealed, exclusions, record["protocol_sha256"]
            )
            path = args.output / f"reference-{name}-report.json"
            path.write_text(json.dumps(reference, indent=1) + "\n")
            record["reference_reports"][name] = sha256(path)
    record["elapsed_seconds"] = round(time.monotonic() - started, 3)
    record["utc_end"] = datetime.now(UTC).isoformat()
    (args.output / "report.json").write_text(json.dumps(report, indent=1) + "\n")
    record["report_sha256"] = sha256(args.output / "report.json")
    (args.output / "execution-record.json").write_text(json.dumps(record, indent=1) + "\n")
    print(json.dumps({"summary": _short(report["summary"]), "record": record}, indent=1)[:6000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
