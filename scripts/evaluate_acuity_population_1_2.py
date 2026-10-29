"""Explicit offline operator: visual-acuity model 1.2 development and its sealed test.

Training data are the 42 tranche-1 record studies of dataset ``cornea-acuity-2026-10-29``
listed as development in ``tranche-2/split-reservation-1.0.0.json`` (the training data of
model 1.1): the 27 development records and the 15 sealed records consumed by the
one-shot test of model 1.0.0. The sealed test set is tranche 2 (records held by
custodians in ``.peykar/acuity-tranche2-sealed-2026-10-29``), unread until the 1.2
protocol is frozen.

``--development --candidate <name>`` runs leave-one-study-out over the training studies
with one population candidate of ``acuity_population_1_2`` through
``eye_additive_evaluation4`` (harness 6: harness 2 plus initial-state intervals);
studies of one development cluster of ``tranche-2/author-overlap-1.0.0.json`` leave
training together. ``--shard`` and ``--merge`` work as in the model 1.0 operator.
``--stack --candidate <stacked name> --report <LOSO report.json>`` adds the stacked
forecasts of a stacked candidate to the leave-one-study-out report of its base, each
held-out study with weights fitted without it (``eye_stacking_evaluation``), and
writes the development comparison of the declared adoption rule. ``--sealed`` refuses
to run unless every hash pinned by the frozen protocol matches; it fits the base
candidate on the training records (minus those linked to a sealed study), scores each
sealed study exactly once and writes that report as the reference (the base model,
unstacked) and the same report with the protocol's frozen stacking weights as the
report of model 1.2. Scores are on the additive scale of model 1.0 (one reported pp =
0.02 logMAR, one ETDRS letter). The scale, the shard merge and the report summary are
imported from the frozen model 1.0 operator, which is not changed. Reports go to a new
directory. No network access, model registration or clinical-validity claim is
involved.
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

from peykar.model_learning.acuity_population_1_2 import (
    DEVELOPMENT_CANDIDATES,
    STACKED_CANDIDATES,
    adapter,
)
from peykar.model_learning.eye_stacking import StackingWeights
from peykar.validation import eye_additive_evaluation4 as additive
from peykar.validation import eye_stacking_evaluation as stacking
from peykar.validation.cornea_acuity_data import AcuitySeries

ROOT = Path(__file__).resolve().parents[1]
_SPEC = importlib.util.spec_from_file_location(
    "evaluate_acuity_population_1_0", ROOT / "scripts/evaluate_acuity_population.py"
)
assert _SPEC is not None and _SPEC.loader is not None
version_1_0 = importlib.util.module_from_spec(_SPEC)
_SPEC.loader.exec_module(version_1_0)

DATASET = ROOT / "experiments/data/cornea-acuity-2026-10-29"
TRANCHE = DATASET / "tranche-2"
TRAINING_DIRS = (
    DATASET / "development/records",
    ROOT / ".peykar/acuity-development-private/records",
    # Tranche-1 sealed records, consumed by the model 1.0.0 test (kept private).
    ROOT / ".peykar/acuity-sealed-2026-10-29/records",
)
SEALED = ROOT / ".peykar/acuity-tranche2-sealed-2026-10-29/records"
RESERVATION = TRANCHE / "split-reservation-1.0.0.json"
TRANCHE_1_RESERVATION = DATASET / "split-reservation-1.0.0.json"
OVERLAP = TRANCHE / "author-overlap-1.0.0.json"
SCALE = version_1_0.SCALE
sha256 = version_1_0.sha256
OPERATOR = "scripts/evaluate_acuity_population_1_2.py"
MODEL_VERSION = "1.2.0"


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
    name = protocol["primary_candidate"]
    stacked = STACKED_CANDIDATES.get(name)
    if stacked is None:
        problems.append(f"unknown stacked candidate {name}")
    else:
        if stacked.describe() != protocol["candidates"].get(name):
            problems.append(f"candidate {name} differs from the protocol")
        base = DEVELOPMENT_CANDIDATES[stacked.base]
        if base.describe() != protocol["candidates"].get(stacked.base):
            problems.append(f"candidate {stacked.base} differs from the protocol")
        try:
            StackingWeights.from_description(protocol["stacking_weights"])
        except Exception as error:  # noqa: BLE001 - reported, the run refuses
            problems.append(f"invalid stacking weights: {error}")
    return problems


def sealed_report(
    name: str,
    series: list[AcuitySeries],
    sealed: list[AcuitySeries],
    exclusions: dict[str, list[str]],
    protocol_sha256: str,
) -> dict[str, Any]:
    """Score every sealed study once with one population candidate (fresh fit per study)."""
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
        "harness_version": additive.harness(config).EVALUATION_VERSION,
        "model_id": config.model_id,
        "model_version": config.model_version,
        "model_configuration": config.describe(),
        "scale": SCALE.describe(),
        "criteria": additive.criteria(SCALE),
        "protocol_sha256": protocol_sha256,
        "studies": studies,
        "summary": additive.summarize(studies),
    }


def check_development_report(name: str, report: dict[str, Any]) -> list[str]:
    """The LOSO report a stacked candidate is developed on must be its base's, run by
    this operator's harness on the 42 training studies."""
    stacked = STACKED_CANDIDATES[name]
    base = DEVELOPMENT_CANDIDATES[stacked.base]
    problems = []
    if report.get("model_configuration") != base.describe():
        problems.append(f"the report is not a leave-one-study-out report of {stacked.base}")
    if (
        report.get("evaluation_version") != additive.EVALUATION_VERSION
        or report.get("harness_version") != additive.harness(base).EVALUATION_VERSION
    ):
        problems.append("the report was not made by additive evaluation 4 and harness 6")
    reserved = _reserved(RESERVATION, "development")
    if {s["study_id"] for s in report["studies"]} != reserved:
        problems.append("the report does not cover the 42 training studies")
    return problems


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--stack", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    mode.add_argument("--merge", type=Path, nargs="+", metavar="SHARD_DIR")
    parser.add_argument(
        "--candidate", choices=sorted({*DEVELOPMENT_CANDIDATES, *STACKED_CANDIDATES})
    )
    parser.add_argument("--report", type=Path, help="--stack: the base candidate's LOSO report")
    parser.add_argument("--shard", help="i/n: hold out the i-th of every n studies")
    parser.add_argument("--protocol", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.development and args.candidate not in DEVELOPMENT_CANDIDATES:
        parser.error("--development requires a population --candidate")
    if args.stack and (args.candidate not in STACKED_CANDIDATES or args.report is None):
        parser.error("--stack requires a stacked --candidate and --report")
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
        "mode": next(m for m in ("development", "stack", "sealed") if getattr(args, m)),
        "operator": OPERATOR,
        "evaluation_version": additive.EVALUATION_VERSION,
        "stacking_evaluation_version": stacking.EVALUATION_VERSION,
    }
    overlap = json.loads(OVERLAP.read_text())
    record["author_overlap"] = {"path": str(OVERLAP.relative_to(ROOT)), "sha256": sha256(OVERLAP)}
    extra: dict[str, Any] = {}
    if args.development:
        series, record["training_record_hashes"] = load_training(args.candidate)
        record["training_studies"] = sorted({s.study_id for s in series})
        present = {s.study_id for s in series}
        clusters = [
            [m for m in cluster if m in present] for cluster in overlap["development_clusters"]
        ]
        clusters = [c for c in clusters if len(c) > 1]
        studies = sorted(present)
        if shard is not None:
            studies = studies[shard[0] :: shard[1]]
            record["shard"] = {"index": shard[0], "count": shard[1], "studies": studies}
        config = DEVELOPMENT_CANDIDATES[args.candidate]
        record["candidate"] = args.candidate
        record["model_configuration"] = config.describe()
        report = additive.leave_one_study_out(
            config,
            series,
            scale=SCALE,
            studies=studies,
            clusters=clusters,
            progress=lambda s: print(s, file=sys.stderr, flush=True),
        )
    elif args.stack:
        source = json.loads(args.report.read_text())
        problems = check_development_report(args.candidate, source)
        if problems:
            record["refused"] = problems
            (args.output / "execution-record.json").write_text(json.dumps(record, indent=1))
            print("Refusing to stack:", problems, file=sys.stderr)
            return 1
        stacked = STACKED_CANDIDATES[args.candidate]
        record["candidate"] = args.candidate
        record["stacked_candidate"] = stacked.describe()
        record["source_report"] = {"path": str(args.report), "sha256": sha256(args.report)}
        report = {
            **stacking.nested_stack_report(source, stacked.spec),
            "model_version": f"{MODEL_VERSION}-{args.candidate}",
            "stacked_candidate": stacked.describe(),
        }
        extra["comparison.json"] = stacking.development_comparison(report, stacked.spec)
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
        stacked = STACKED_CANDIDATES[protocol["primary_candidate"]]
        weights = StackingWeights.from_description(protocol["stacking_weights"])
        series, record["training_record_hashes"] = load_training(stacked.base)
        record["training_studies"] = sorted({s.study_id for s in series})
        sealed, record["sealed_record_hashes"] = load_sealed(stacked.base)
        exclusions = overlap["sealed_training_exclusions"]
        reference = sealed_report(
            stacked.base, series, sealed, exclusions, record["protocol_sha256"]
        )
        path = args.output / f"reference-{stacked.base}-report.json"
        path.write_text(json.dumps(reference, indent=1) + "\n")
        record["reference_reports"] = {stacked.base: sha256(path)}
        report = {
            **stacking.stack_report(reference, weights, stacked.spec),
            "model_version": f"{protocol['model']['version']}-{protocol['primary_candidate']}",
            "stacked_candidate": stacked.describe(),
        }
    for name, content in extra.items():
        (args.output / name).write_text(json.dumps(content, indent=1) + "\n")
        record[f"{name.removesuffix('.json')}_sha256"] = sha256(args.output / name)
    record["elapsed_seconds"] = round(time.monotonic() - started, 3)
    record["utc_end"] = datetime.now(UTC).isoformat()
    (args.output / "report.json").write_text(json.dumps(report, indent=1) + "\n")
    record["report_sha256"] = sha256(args.output / "report.json")
    (args.output / "execution-record.json").write_text(json.dumps(record, indent=1) + "\n")
    summary = version_1_0._short(report["summary"])
    printed = {"summary": summary, "record": record, **extra}
    print(json.dumps(printed, indent=1)[:8000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
