"""Explicit offline operator: corneal population model development and sealed test.

``--development`` runs leave-one-study-out on all development studies (consumed
2017/2019/2021 rabbit sources plus newly admitted development records).
``--sealed`` refuses to run unless the frozen protocol's code, data and sealed
record SHA-256 values all match, then fits on every development study and scores
the sealed studies exactly once. Reports are written to a new output directory.
No network access, model registration or biological-validity claim is involved.
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

from peykar.validation.cornea_population_evaluation import (
    evaluate_study,
    leave_one_study_out,
    summarize,
)
from peykar.validation.cornea_population_sources import default_consumed, record_series

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/cornea-wound-population-2026-10-27"
DEVELOPMENT_RECORDS = DATASET / "development/records"
SEALED_RECORDS = ROOT / ".peykar/cornea-sealed-test-2026-10-27/records"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def development_series() -> tuple[list[Any], list[str]]:
    consumed = default_consumed(ROOT)
    new, hashes = record_series(sorted(DEVELOPMENT_RECORDS.glob("*.json")), split="development")
    return consumed + new, hashes


def check_protocol(protocol: dict[str, Any]) -> list[str]:
    problems = []
    for relative, expected in protocol["frozen_files_sha256"].items():
        path = ROOT / relative
        if not path.is_file():
            problems.append(f"missing {relative}")
        elif sha256(path) != expected:
            problems.append(f"changed {relative}")
    for name, expected in protocol["sealed_records_sha256"].items():
        path = SEALED_RECORDS / name
        if not path.is_file():
            problems.append(f"missing sealed {name}")
        elif sha256(path) != expected:
            problems.append(f"changed sealed {name}")
    return problems


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--protocol", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--draws", type=int, default=2000)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist; every run gets a fresh directory")
    args.output.mkdir(parents=True)
    started = time.monotonic()
    record: dict[str, Any] = {
        "utc_start": datetime.now(UTC).isoformat(),
        "python": platform.python_version(),
        "mode": "development" if args.development else "sealed",
        "draws": args.draws,
    }
    series, development_hashes = development_series()
    record["development_record_hashes"] = development_hashes
    if args.development:
        report = leave_one_study_out(series, draws=args.draws)
        exit_code = 0
    else:
        if args.protocol is None:
            parser.error("--sealed requires --protocol")
        protocol = json.loads(args.protocol.read_text())
        record["protocol"] = str(args.protocol)
        record["protocol_sha256"] = sha256(args.protocol)
        problems = check_protocol(protocol)
        if problems:
            record["refused"] = problems
            (args.output / "execution-record.json").write_text(json.dumps(record, indent=1))
            print("Refusing sealed evaluation:", problems, file=sys.stderr)
            return 1
        sealed, sealed_hashes = record_series(
            sorted(SEALED_RECORDS.glob("*.json")), split="sealed_test"
        )
        record["sealed_record_hashes"] = sealed_hashes
        studies = []
        for study_id in sorted({s.study_id for s in sealed}):
            held = [s for s in sealed if s.study_id == study_id]
            studies.append(evaluate_study(study_series=held, training=series, draws=args.draws))
        report = {"studies": studies, "summary": summarize(studies)}
        exit_code = 0
    record["elapsed_seconds"] = round(time.monotonic() - started, 3)
    record["utc_end"] = datetime.now(UTC).isoformat()
    (args.output / "report.json").write_text(json.dumps(report, indent=1) + "\n")
    record["report_sha256"] = sha256(args.output / "report.json")
    (args.output / "execution-record.json").write_text(json.dumps(record, indent=1) + "\n")
    print(json.dumps({"summary": report["summary"], "record": record}, indent=1)[:4000])
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
