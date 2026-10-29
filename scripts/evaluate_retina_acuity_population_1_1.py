"""Explicit offline operator: retinal visual-acuity model 1.1 development and its sealed test.

Training data are the 24 tranche-1 record studies of dataset ``retina-acuity-2026-10-29``
listed as development in ``tranche-2/split-reservation-1.0.0.json``: the 15 development
records (public and privately kept) and the 9 sealed records consumed by the one-shot
test of model 1.0.0. The sealed test set is tranche 2 (records held by custodians in
``.peykar/retina-acuity-tranche2-sealed-2026-10-29``).

``--development --candidate <name>`` runs leave-one-study-out over the training studies
with one declared configuration of ``retina_acuity_population_1_1``; studies of one
development cluster of ``tranche-2/author-overlap-1.0.0.json`` leave training together.
``--shard`` and ``--merge`` work as in the model 1.0 operator. ``--sealed`` refuses to
run unless every hash pinned by the frozen protocol matches, fits on the training
records (minus those linked to a sealed study) and scores each sealed study exactly once
with the protocol's primary candidate and, in the same run, with each declared reference
candidate. Scores are on the additive scale of model 1.0 (one reported pp = 0.02 logMAR,
one ETDRS letter) through ``eye_additive_evaluation2``. The scale, the shard merge and the
report summary are imported from the frozen model 1.0 operator, which is not changed.
Reports go to a new directory. No network access, model registration or
clinical-validity claim is involved.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import platform
import sys
import time
from datetime import UTC, datetime
from pathlib import Path
from typing import Any

from peykar.model_learning.retina_acuity_population_1_1 import DEVELOPMENT_CANDIDATES, adapter
from peykar.validation import eye_additive_evaluation2 as additive
from peykar.validation.eye_retina_acuity_data import RetinaAcuitySeries as AcuitySeries

ROOT = Path(__file__).resolve().parents[1]
_SPEC = importlib.util.spec_from_file_location(
    "evaluate_retina_acuity_population_1_0",
    ROOT / "scripts/evaluate_retina_acuity_population.py",
)
assert _SPEC is not None and _SPEC.loader is not None
version_1_0 = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(version_1_0)

DATASET = ROOT / "experiments/data/retina-acuity-2026-10-29"
TRANCHE = DATASET / "tranche-2"
TRAINING_DIRS = (
    DATASET / "development/records",
    ROOT / ".peykar/retina-acuity-development-private/records",
    # Tranche-1 sealed records, consumed by the model 1.0.0 test (kept private).
    ROOT / ".peykar/retina-acuity-sealed-2026-10-29/records",
)
SEALED = ROOT / ".peykar/retina-acuity-tranche2-sealed-2026-10-29/records"
RESERVATION = TRANCHE / "split-reservation-1.0.0.json"
TRANCHE_1_RESERVATION = DATASET / "split-reservation-1.0.0.json"
OVERLAP = TRANCHE / "author-overlap-1.0.0.json"
SCALE = version_1_0.SCALE
sha256 = version_1_0.sha256


def training_paths() -> list[Path]:
    return sorted(
        (p for directory in TRAINING_DIRS for p in directory.glob("*.json")), key=lambda p: p.name
    )


def _reserved(path: Path, split: str) -> set[str]:
    return {s["study_key"] for s in json.loads(path.read_text())["studies"] if s["split"] == split}


def load_training(name: str) -> tuple[list[AcuitySeries], dict[str, str]]:
    reserved = _reserved(RESERVATION, "development")
    consumed = _reserved(TRANCHE_1_RESERVATION, "sealed_test")
    to_series = adapter(name)
    series: list[AcuitySeries] = []
    hashes = {}
    for path in training_paths():
        record = json.loads(path.read_text())
        study = record["study_id"]
        split = record.get("split")
        if study not in reserved or split not in {"development", "sealed_test"}:
            raise SystemExit(f"{path.name} is not a reserved training record")
        if split == "sealed_test" and study not in consumed:
            raise SystemExit(f"{path.name} is a sealed record that was not consumed")
        series.extend(to_series(record))
        hashes[path.name] = sha256(path)
    if len({s.study_id for s in series}) != len(reserved):
        raise SystemExit("training records do not cover the reserved training studies")
    return series, hashes


def load_sealed(name: str) -> tuple[list[AcuitySeries], dict[str, str]]:
    reserved = _reserved(RESERVATION, "sealed_test")
    to_series = adapter(name)
    series: list[AcuitySeries] = []
    hashes = {}
    for path in sorted(SEALED.glob("*.json")):
        record = json.loads(path.read_text())
        if record.get("split") != "sealed_test" or record["study_id"] not in reserved:
            raise SystemExit(f"{path.name} is not a reserved tranche-2 sealed record")
        series.extend(to_series(record))
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
    if protocol.get("scale") != SCALE.describe():
        problems.append("the additive scale differs from the protocol")
    for name in (protocol["primary_candidate"], *protocol.get("reference_candidates", [])):
        if name not in DEVELOPMENT_CANDIDATES:
            problems.append(f"unknown candidate {name}")
        elif DEVELOPMENT_CANDIDATES[name].describe() != protocol["candidates"][name]:
            problems.append(f"candidate {name} differs from the protocol")
    return problems


def sealed_report(
    name: str,
    series: list[AcuitySeries],
    sealed: list[AcuitySeries],
    exclusions: dict[str, list[str]],
    protocol_sha256: str,
) -> dict[str, Any]:
    """Score every sealed study once with one configuration (fresh fit per study)."""
    config = DEVELOPMENT_CANDIDATES[name]
    studies = []
    for study_id in sorted({s.study_id for s in sealed}):
        held = [s for s in sealed if s.study_id == study_id]
        excluded = set(exclusions.get(study_id, []))
        training = [s for s in series if s.study_id not in excluded]
        result = additive.evaluate_study(config, study_series=held, training=training, scale=SCALE)
        result["training_exclusions"] = sorted(excluded)
        studies.append(result)
        print(name, study_id, file=sys.stderr, flush=True)
    return {
        "evaluation_id": additive.EVALUATION_ID,
        "evaluation_version": additive.EVALUATION_VERSION,
        "model_id": config.model_id,
        "model_version": config.model_version,
        "model_configuration": config.describe(),
        "scale": SCALE.describe(),
        "criteria": additive.criteria(SCALE),
        "protocol_sha256": protocol_sha256,
        "studies": studies,
        "summary": additive.summarize(studies),
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    mode.add_argument("--merge", type=Path, nargs="+", metavar="SHARD_DIR")
    parser.add_argument("--candidate", choices=sorted(DEVELOPMENT_CANDIDATES))
    parser.add_argument("--shard", help="i/n: hold out the i-th of every n studies")
    parser.add_argument("--protocol", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.development and args.candidate is None:
        parser.error("--development requires --candidate")
    if args.sealed and (args.protocol is None or args.candidate):
        parser.error("--sealed requires --protocol (the protocol names the candidates)")
    if args.output.exists():
        parser.error("--output must not exist; every run gets a fresh directory")
    if args.merge:
        return int(version_1_0.merge(args.merge, args.output))
    shard = None
    if args.shard is not None:
        if not args.development:
            parser.error("--shard applies to --development only")
        index, _, count = args.shard.partition("/")
        shard = (int(index), int(count))
        if not 0 <= shard[0] < shard[1]:
            parser.error("--shard must be i/n with 0 <= i < n")
    args.output.mkdir(parents=True)
    started = time.monotonic()
    record: dict[str, Any] = {
        "utc_start": datetime.now(UTC).isoformat(),
        "python": platform.python_version(),
        "mode": "development" if args.development else "sealed",
        "operator": "scripts/evaluate_retina_acuity_population_1_1.py",
        "evaluation_version": additive.EVALUATION_VERSION,
    }
    overlap = json.loads(OVERLAP.read_text())
    record["author_overlap"] = {"path": str(OVERLAP.relative_to(ROOT)), "sha256": sha256(OVERLAP)}
    if args.development:
        series, record["training_record_hashes"] = load_training(args.candidate)
        record["training_studies"] = sorted({s.study_id for s in series})
        present = {s.study_id for s in series}
        clusters = [
            [m for m in cluster if m in present] for cluster in overlap["development_clusters"]
        ]
        clusters = [c for c in clusters if len(c) > 1]

        def progress(s: str) -> None:
            print(s, file=sys.stderr, flush=True)

        studies = sorted(present)
        if shard is not None:
            studies = studies[shard[0] :: shard[1]]
            record["shard"] = {"index": shard[0], "count": shard[1], "studies": studies}
        config = DEVELOPMENT_CANDIDATES[args.candidate]
        record["candidate"] = args.candidate
        record["model_configuration"] = config.describe()
        report = additive.leave_one_study_out(
            config, series, scale=SCALE, studies=studies, clusters=clusters, progress=progress
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
        primary = protocol["primary_candidate"]
        series, record["training_record_hashes"] = load_training(primary)
        record["training_studies"] = sorted({s.study_id for s in series})
        sealed, record["sealed_record_hashes"] = load_sealed(primary)
        exclusions = overlap["sealed_training_exclusions"]
        report = sealed_report(primary, series, sealed, exclusions, record["protocol_sha256"])
        record["reference_reports"] = {}
        for name in protocol.get("reference_candidates", []):
            ref_series, _ = load_training(name)
            ref_sealed, _ = load_sealed(name)
            reference = sealed_report(
                name, ref_series, ref_sealed, exclusions, record["protocol_sha256"]
            )
            path = args.output / f"reference-{name}-report.json"
            path.write_text(json.dumps(reference, indent=1) + "\n")
            record["reference_reports"][name] = sha256(path)
    record["elapsed_seconds"] = round(time.monotonic() - started, 3)
    record["utc_end"] = datetime.now(UTC).isoformat()
    (args.output / "report.json").write_text(json.dumps(report, indent=1) + "\n")
    record["report_sha256"] = sha256(args.output / "report.json")
    (args.output / "execution-record.json").write_text(json.dumps(record, indent=1) + "\n")
    summary = version_1_0._short(report["summary"])
    print(json.dumps({"summary": summary, "record": record}, indent=1)[:6000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
