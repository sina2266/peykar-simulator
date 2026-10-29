"""Explicit offline operator: retinal-thickness model 1.2 development and its sealed test.

Training data of model 1.2 are the 128 record studies of tranches 1 and 2 of dataset
``retina-cst-2026-10-28``: the 46 tranche-1 development records (public and privately
kept), the 19 tranche-1 sealed records consumed by the test of model 1.0.0 and the 63
tranche-2 sealed records consumed by the test of model 1.1.0; the tranche-3
reservation lists them as development. ``--development --candidate <name>`` runs
leave-one-study-out over them with one declared configuration of
``retina_thickness_population_1_2``; linked studies (tranche-3 author-overlap file,
development clusters) leave training together. Candidate ``e0`` is the frozen model
1.1.0. Series carry model 1.1's declared dose rule (``eye_retina_data_1_1``).
``--shard i/n`` holds out only every n-th study from the i-th; ``--merge <dir> ...``
joins the shares of one run into the report a single run would write. ``--sealed``
refuses to run unless every hash pinned by the frozen 1.2 protocol matches, fits on all
training records (minus those linked to a sealed study) and scores each tranche-3
sealed study exactly once with the protocol's primary candidate and, in the same run,
with each declared reference candidate. Reports go to a new directory. No network
access, model registration or clinical-validity claim is involved. The 1.0.0 and 1.1.0
operators stay unchanged because their protocols pin them.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import platform
import sys
import time
from collections.abc import Callable, Mapping
from datetime import UTC, datetime
from pathlib import Path
from typing import Any

from peykar.model_learning import eye_recovery_population3 as version3
from peykar.model_learning.retina_thickness_population_1_2 import (
    DEVELOPMENT_CANDIDATES,
    DEVICE_CANDIDATES,
    PRIOR_TREATMENT_CANDIDATES,
    REGIMEN_CANDIDATES,
)
from peykar.validation import eye_recovery_evaluation2 as evaluation
from peykar.validation import eye_recovery_evaluation3 as evaluation3
from peykar.validation.eye_retina_data import RetinaSeries
from peykar.validation.eye_retina_data_1_1 import to_retina_series_1_1
from peykar.validation.eye_retina_data_1_2 import (
    to_retina_series_1_2_device,
    to_retina_series_1_2_prior,
    to_retina_series_1_2_regimen,
)

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/retina-cst-2026-10-28"
TRAINING_DIRS = (
    DATASET / "development/records",
    # Records derived from NC/ND/SA-licensed sources stay outside the repository.
    ROOT / ".peykar/retina-development-private/records",
    # Tranche-1 sealed records, consumed by the one-shot test of model 1.0.0.
    ROOT / ".peykar/retina-sealed-2026-10-28/records",
    # Tranche-2 sealed records, consumed by the one-shot test of model 1.1.0.
    ROOT / ".peykar/retina-tranche2-sealed-2026-10-28/records",
)
SEALED = ROOT / ".peykar/retina-tranche3-sealed-2026-10-29/records"
RESERVATION = DATASET / "tranche-3/split-reservation-1.0.0.json"
# Declared author-overlap rule over the 128 training studies and the tranche-3 admits.
OVERLAP = DATASET / "tranche-3/author-overlap-1.0.0.json"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def harness(name: str) -> Any:
    """Version-3 configurations are evaluated by harness 3, all others by harness 2."""
    config = DEVELOPMENT_CANDIDATES[name]
    return evaluation3 if isinstance(config, version3.RecoveryConfig) else evaluation


def adapter(name: str) -> Callable[[Mapping[str, Any]], list[RetinaSeries]]:
    if name in PRIOR_TREATMENT_CANDIDATES:
        return to_retina_series_1_2_prior
    if name in DEVICE_CANDIDATES:
        return to_retina_series_1_2_device
    if name in REGIMEN_CANDIDATES:
        return to_retina_series_1_2_regimen
    return to_retina_series_1_1


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


def load_training(name: str) -> tuple[list[RetinaSeries], dict[str, str]]:
    reserved = _reserved("development")
    to_series = adapter(name)
    series: list[RetinaSeries] = []
    hashes = {}
    for path in training_paths():
        record = json.loads(path.read_text())
        if record["study_id"] not in reserved:
            raise SystemExit(f"{path.name} is not a reserved model-1.2 training record")
        series.extend(to_series(record))
        hashes[path.name] = sha256(path)
    if {s.study_id for s in series} != reserved:
        raise SystemExit("Training records do not cover the reserved development studies")
    return series, hashes


def load_sealed(name: str) -> tuple[list[RetinaSeries], dict[str, str]]:
    reserved = _reserved("sealed_test")
    to_series = adapter(name)
    series: list[RetinaSeries] = []
    hashes = {}
    for path in sorted(SEALED.glob("*.json")):
        record = json.loads(path.read_text())
        if record.get("split") != "sealed_test" or record["study_id"] not in reserved:
            raise SystemExit(f"{path.name} is not a reserved tranche-3 sealed record")
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
    for name in (protocol["primary_candidate"], *protocol.get("reference_candidates", [])):
        if name not in DEVELOPMENT_CANDIDATES:
            problems.append(f"unknown candidate {name}")
        elif DEVELOPMENT_CANDIDATES[name].describe() != protocol["candidates"][name]:
            problems.append(f"candidate {name} differs from the protocol")
    return problems


def sealed_report(
    name: str,
    series: list[RetinaSeries],
    sealed: list[RetinaSeries],
    exclusions: dict[str, list[str]],
    protocol_sha256: str,
) -> dict[str, Any]:
    """Score every sealed study once with one configuration (fresh fit per study)."""
    config, chosen = DEVELOPMENT_CANDIDATES[name], harness(name)
    studies = []
    for study_id in sorted({s.study_id for s in sealed}):
        held = [s for s in sealed if s.study_id == study_id]
        excluded = set(exclusions.get(study_id, []))
        training = [s for s in series if s.study_id not in excluded]
        result = chosen.evaluate_study(config, study_series=held, training=training)
        result["training_exclusions"] = sorted(excluded)
        studies.append(result)
        print(name, study_id, file=sys.stderr, flush=True)
    return {
        "evaluation_id": chosen.EVALUATION_ID,
        "evaluation_version": chosen.EVALUATION_VERSION,
        "model_id": config.model_id,
        "model_version": config.model_version,
        "model_configuration": config.describe(),
        "criteria": chosen.criteria(config),
        "protocol_sha256": protocol_sha256,
        "studies": studies,
        "summary": chosen.summarize(studies),
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


def merge(shards: list[Path], output: Path) -> int:
    """Join the shares of one development run into its single-run report."""
    records = [json.loads((d / "execution-record.json").read_text()) for d in shards]
    reports = [json.loads((d / "report.json").read_text()) for d in shards]
    count = {r["shard"]["count"] for r in records}
    if len(count) != 1 or sorted(r["shard"]["index"] for r in records) != list(range(count.pop())):
        raise SystemExit("The shards do not form one complete run")
    if (
        len({r["candidate"] for r in records}) != 1
        or len({json.dumps(r["training_record_hashes"], sort_keys=True) for r in records}) != 1
    ):
        raise SystemExit("The shards come from different candidates or training data")
    studies = sorted((s for r in reports for s in r["studies"]), key=lambda s: s["study_id"])
    expected = sorted(records[0]["training_studies"])
    if [s["study_id"] for s in studies] != expected:
        raise SystemExit("The shards do not cover every training study exactly once")
    chosen = harness(records[0]["candidate"])
    report = {**reports[0], "studies": studies, "summary": chosen.summarize(studies)}
    output.mkdir(parents=True)
    (output / "report.json").write_text(json.dumps(report, indent=1) + "\n")
    record = {
        **{k: v for k, v in records[0].items() if k not in ("shard", "report_sha256")},
        "merged_from": {
            str(d): {"execution_record": r, "report_sha256": sha256(d / "report.json")}
            for d, r in zip(shards, records, strict=True)
        },
        "report_sha256": sha256(output / "report.json"),
    }
    (output / "execution-record.json").write_text(json.dumps(record, indent=1) + "\n")
    print(json.dumps({"summary": _short(report["summary"])}, indent=1)[:6000])
    return 0


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
        return merge(args.merge, args.output)
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
    }
    overlap = json.loads(OVERLAP.read_text())
    record["author_overlap"] = {"path": str(OVERLAP.relative_to(ROOT)), "sha256": sha256(OVERLAP)}
    if args.development:
        series, record["training_record_hashes"] = load_training(args.candidate)
        record["training_studies"] = sorted({s.study_id for s in series})
        config, chosen = DEVELOPMENT_CANDIDATES[args.candidate], harness(args.candidate)
        record["candidate"] = args.candidate
        record["evaluation_version"] = chosen.EVALUATION_VERSION
        record["model_configuration"] = config.describe()
        present = {s.study_id for s in series}
        clusters = [
            [m for m in cluster if m in present] for cluster in overlap["development_clusters"]
        ]
        studies = sorted(present)
        if shard is not None:
            studies = studies[shard[0] :: shard[1]]
            record["shard"] = {"index": shard[0], "count": shard[1], "studies": studies}
        report = chosen.leave_one_study_out(
            config,
            series,
            studies=studies,
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
        primary = protocol["primary_candidate"]
        series, record["training_record_hashes"] = load_training(primary)
        record["training_studies"] = sorted({s.study_id for s in series})
        sealed, record["sealed_record_hashes"] = load_sealed(primary)
        exclusions = overlap["sealed_training_exclusions"]
        record["evaluation_version"] = harness(primary).EVALUATION_VERSION
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
    print(json.dumps({"summary": _short(report["summary"]), "record": record}, indent=1)[:6000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
