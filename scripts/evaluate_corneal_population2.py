"""Explicit offline operator: corneal population model 2.0.0 development and sealed test.

Training/development studies (dataset release 1.1.0): the consumed 2017/2019/2021
rabbit sources, development records 1.0.0, the two 1.0.0 sealed-test records that
were consumed by the one-shot 1.0.0 test (hash-pinned, private because one is
CC BY-NC-ND), and round-2/3 development records under ``development/records-1.1.0``
(plus privately kept records derived from CC BY-NC-ND sources).

``--development`` runs leave-one-study-out over all of them with model 2.0.0 and,
for reference, the frozen 1.0.0 model on the same data. ``--sealed`` refuses to run
unless every hash of the frozen 2.0.0 protocol matches, fits on all development
studies and scores the 1.1.0 sealed studies exactly once with both versions.
Reports go to a new directory. No network access, model registration or
biological-validity claim is involved.
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

from peykar.validation import cornea_population_evaluation as evaluation1
from peykar.validation import cornea_population_evaluation2 as evaluation2
from peykar.validation.cornea_population_sources import default_consumed, record_series

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/cornea-wound-population-2026-10-27"
RECORDS_1_0 = DATASET / "development/records"
RECORDS_1_1 = DATASET / "development/records-1.1.0"
# Development records derived from CC BY-NC-ND sources stay outside the repository.
RECORDS_1_1_PRIVATE = ROOT / ".peykar/cornea-development-1.1.0-private/records"
CONSUMED_SEALED = ROOT / ".peykar/cornea-sealed-test-2026-10-27/records"
SEALED_1_1 = ROOT / ".peykar/cornea-sealed-test-2026-10-28/records"
PROTOCOL_1 = ROOT / "experiments/vcornea/repair-front/population-model/protocol-1.0.0.json"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def consumed_sealed_paths() -> list[Path]:
    """The 1.0.0 sealed records, refused unless they match the 1.0.0 protocol pins."""
    pins = json.loads(PROTOCOL_1.read_text())["sealed_records_sha256"]
    paths = []
    for name, expected in sorted(pins.items()):
        path = CONSUMED_SEALED / name
        if not path.is_file() or sha256(path) != expected:
            raise SystemExit(f"Consumed 1.0.0 sealed record {name} is missing or changed")
        paths.append(path)
    return paths


def development_1_1_paths() -> list[Path]:
    return sorted([*RECORDS_1_1.glob("*.json"), *RECORDS_1_1_PRIVATE.glob("*.json")])


def development_series() -> tuple[list[Any], dict[str, list[str]]]:
    consumed = default_consumed(ROOT)
    old, old_hashes = record_series(sorted(RECORDS_1_0.glob("*.json")), split="development")
    unsealed, unsealed_hashes = record_series(consumed_sealed_paths(), split="sealed_test")
    new, new_hashes = record_series(development_1_1_paths(), split="development")
    hashes = {
        "development_1_0_0": old_hashes,
        "consumed_sealed_1_0_0": unsealed_hashes,
        "development_1_1_0": new_hashes,
    }
    return consumed + old + unsealed + new, hashes


def check_protocol(protocol: dict[str, Any]) -> list[str]:
    problems = []
    for relative, expected in protocol["frozen_files_sha256"].items():
        path = ROOT / relative
        if not path.is_file():
            problems.append(f"missing {relative}")
        elif sha256(path) != expected:
            problems.append(f"changed {relative}")
    listed = set(protocol["sealed_records_sha256"])
    present = {p.name for p in SEALED_1_1.glob("*.json")}
    if present != listed:
        problems.append(f"sealed record set differs: {sorted(present ^ listed)}")
    for name, expected in protocol["sealed_records_sha256"].items():
        path = SEALED_1_1 / name
        if path.is_file() and sha256(path) != expected:
            problems.append(f"changed sealed {name}")
    development = {p.name: sha256(p) for p in development_1_1_paths()}
    if development != protocol["development_records_1_1_0_sha256"]:
        problems.append("development 1.1.0 records differ from the protocol")
    return problems


def _short(summary: dict[str, Any]) -> dict[str, Any]:
    keep = {}
    for method, values in summary.get("next_observation", {}).items():
        keep[method] = {
            k: values[k] for k in ("series", "series_meeting_criteria", "pooled_rmse_pp")
        }
    return keep


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--protocol", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--draws", type=int, default=2000)
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
        "draws": args.draws,
        "evaluation_version": evaluation2.EVALUATION_VERSION,
    }
    series, record["development_record_hashes"] = development_series()
    record["development_studies"] = sorted({s.study_id for s in series})
    if args.development:
        report = {
            "version2": evaluation2.leave_one_study_out(series, draws=args.draws),
            "version1_reference": evaluation1.leave_one_study_out(series, draws=args.draws),
        }
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
        sealed, record["sealed_record_hashes"] = record_series(
            sorted(SEALED_1_1.glob("*.json")), split="sealed_test"
        )
        report = {}
        for name, module in (("version2", evaluation2), ("version1_reference", evaluation1)):
            studies = []
            for study_id in sorted({s.study_id for s in sealed}):
                held = [s for s in sealed if s.study_id == study_id]
                studies.append(
                    module.evaluate_study(study_series=held, training=series, draws=args.draws)
                )
            report[name] = {"studies": studies, "summary": evaluation1.summarize(studies)}
    record["elapsed_seconds"] = round(time.monotonic() - started, 3)
    record["utc_end"] = datetime.now(UTC).isoformat()
    (args.output / "report.json").write_text(json.dumps(report, indent=1) + "\n")
    record["report_sha256"] = sha256(args.output / "report.json")
    (args.output / "execution-record.json").write_text(json.dumps(record, indent=1) + "\n")
    brief = {name: _short(part["summary"]) for name, part in report.items()}
    print(json.dumps({"summary": brief, "record": record}, indent=1)[:6000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
