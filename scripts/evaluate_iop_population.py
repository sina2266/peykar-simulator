"""Explicit offline operator: IOP-after-surgery model development and sealed test.

Development studies (dataset 2026-10-28, split reservation 1.0.0) are the records
under ``development/records`` plus privately kept records derived from NC/ND/SA
licensed sources. ``--development`` runs leave-one-study-out over them.
``--sealed`` refuses to run unless every hash pinned by the frozen protocol
matches, fits on all development studies (minus those linked to a sealed study
by the author-overlap rule) and scores each sealed study exactly once. Reports go
to a new directory. No network access, model registration or biological-validity
claim is involved.
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

from peykar.model_learning import iop_population as population
from peykar.validation import eye_iop_evaluation as evaluation
from peykar.validation.eye_iop_data import PressureSeries, to_pressure_series

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/iop-glaucoma-surgery-2026-10-28"
RECORDS = DATASET / "development/records"
# Development records derived from NC/ND/SA-licensed sources stay outside the repository.
RECORDS_PRIVATE = ROOT / ".peykar/iop-development-private/records"
SEALED = ROOT / ".peykar/iop-sealed-2026-10-28/records"
# Declared author-overlap rule: linked development studies leave training together
# (development) or are excluded from a linked sealed study's training (sealed test).
OVERLAP = DATASET / "author-overlap-1.0.0.json"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def development_paths() -> list[Path]:
    return sorted([*RECORDS.glob("*.json"), *RECORDS_PRIVATE.glob("*.json")], key=lambda p: p.name)


def load_series(paths: list[Path], *, split: str) -> tuple[list[PressureSeries], dict[str, str]]:
    series: list[PressureSeries] = []
    hashes = {}
    for path in paths:
        record = json.loads(path.read_text())
        if record.get("split") != split:
            raise SystemExit(f"{path.name} is not a {split} record")
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
    development = {p.name: sha256(p) for p in development_paths()}
    if development != protocol["development_records_sha256"]:
        problems.append("development records differ from the protocol")
    listed = set(protocol["sealed_records_sha256"])
    present = {p.name for p in SEALED.glob("*.json")}
    if present != listed:
        problems.append(f"sealed record set differs: {sorted(present ^ listed)}")
    for name, expected in protocol["sealed_records_sha256"].items():
        path = SEALED / name
        if path.is_file() and sha256(path) != expected:
            problems.append(f"changed sealed {name}")
    return problems


def _short(summary: dict[str, Any]) -> dict[str, Any]:
    keep: dict[str, Any] = {}
    for task in ("next_observation", "landmark"):
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
    parser.add_argument("--protocol", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.sealed and args.protocol is None:
        parser.error("--sealed requires --protocol")
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
    series, record["development_record_hashes"] = load_series(
        development_paths(), split="development"
    )
    record["development_studies"] = sorted({s.study_id for s in series})
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
        sealed, record["sealed_record_hashes"] = load_series(
            sorted(SEALED.glob("*.json")), split="sealed_test"
        )
        studies = []
        exclusions = overlap["sealed_training_exclusions"]
        for study_id in sorted({s.study_id for s in sealed}):
            held = [s for s in sealed if s.study_id == study_id]
            excluded = set(exclusions.get(study_id, []))
            training = [s for s in series if s.study_id not in excluded]
            result = evaluation.evaluate_study(study_series=held, training=training)
            result["training_exclusions"] = sorted(excluded)
            studies.append(result)
        report = {
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
            "protocol_sha256": record["protocol_sha256"],
            "studies": studies,
            "summary": evaluation.summarize(studies),
        }
    record["elapsed_seconds"] = round(time.monotonic() - started, 3)
    record["utc_end"] = datetime.now(UTC).isoformat()
    (args.output / "report.json").write_text(json.dumps(report, indent=1) + "\n")
    record["report_sha256"] = sha256(args.output / "report.json")
    (args.output / "execution-record.json").write_text(json.dumps(record, indent=1) + "\n")
    print(json.dumps({"summary": _short(report["summary"]), "record": record}, indent=1)[:8000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
