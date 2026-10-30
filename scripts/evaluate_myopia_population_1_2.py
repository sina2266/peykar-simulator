"""Explicit offline operator: childhood-myopia axial-length model 1.2 development and its
screen-2 sealed test.

Training data of model 1.2 are the 59 consumed records of dataset
``myopia-axial-2026-10-30``: the 33 tranche-1 records (22 development records and the
11 sealed records consumed by the test of model 1.0.0) and the 26 tranche-2 records
consumed by the test of model 1.1.0. Each training record must carry the split that its
tranche's reservation gave its study. ``--development --candidate <name> --overlap
<file>`` runs leave-one-study-out over them with one declared configuration of
``myopia_population_1_2``; linked studies leave training together. The clusters are the
connected components, among the training studies, of the overlap file's pairs and
development clusters. A scaled candidate (``SCALED_CANDIDATES``) runs its base
candidate's leave-one-study-out and then scales each held-out study's intervals with a
factor chosen without it and its linked studies (``eye_interval_scaling``). Every study
result is scored on the additive scale of ``eye_additive_evaluation5`` (one reported pp
= 0.01 mm). ``--sealed`` refuses to run unless every hash pinned by the frozen 1.2
protocol matches and the screen-2 reservation lists every training study as
development; it fits on all 59 training records (minus those linked to a sealed study),
scores each screen-2 sealed study exactly once with the protocol's primary candidate
(scaled with the protocol's frozen factor if it is a scaled candidate) and, in the same
run, with each declared reference (a frozen earlier version from
``myopia_population_1_2.REFERENCES``). Reports go to a new directory. No network access,
model registration or clinical-validity claim is involved. The 1.0 and 1.1 operators
stay unchanged because their protocols pin them.
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

from peykar.model_learning.eye_recovery_population2 import RecoveryConfig
from peykar.model_learning.eye_stacking2 import HistoryStacking
from peykar.model_learning.myopia_population_1_2 import (
    DEVELOPMENT_CANDIDATES,
    REFERENCES,
    SCALED_CANDIDATES,
    adapter,
)
from peykar.validation import eye_additive_evaluation5 as additive
from peykar.validation.eye_interval_scaling import nested_scale_report, scale_report
from peykar.validation.myopia_axial_data import (
    COVARIATE_UNIT_MM,
    MODEL_UNIT_MM,
    REPORT_UNIT_MM,
    AxialSeries,
)

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/myopia-axial-2026-10-30"
TRAINING_DIRS = (
    DATASET / "development/records",
    # Records derived from NC/ND/SA-licensed sources stay outside the repository.
    ROOT / ".peykar/myopia-development-private/records",
    # Tranche-1 sealed records, consumed by the one-shot test of model 1.0.0.
    ROOT / ".peykar/myopia-sealed-2026-10-30/records",
    # Tranche-2 sealed records, consumed by the one-shot test of model 1.1.0.
    ROOT / ".peykar/myopia-tranche2-sealed-2026-10-30/records",
)
TRANCHE1_RESERVATION = DATASET / "split-reservation-1.0.0.json"
TRANCHE2_RESERVATION = DATASET / "tranche-2/split-reservation-1.0.0.json"
SEALED = ROOT / ".peykar/myopia-screen2-sealed-2026-10-30/records"
RESERVATION = DATASET / "screen-2/split-reservation-1.0.0.json"
# Declared author-overlap rule over the 59 training studies and the screen-2 admits.
OVERLAP = DATASET / "screen-2/author-overlap-1.0.0.json"
SCALE = additive.AdditiveScale(
    quantity="axial length",
    unit="mm",
    model_unit=MODEL_UNIT_MM,
    covariate_unit=COVARIATE_UNIT_MM,
    report_unit=REPORT_UNIT_MM,
)
CANDIDATE_NAMES = sorted({*DEVELOPMENT_CANDIDATES, *SCALED_CANDIDATES})


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def configuration(name: str) -> RecoveryConfig:
    """A population candidate (the base of a scaled one) or a frozen reference."""
    if name in SCALED_CANDIDATES:
        name = SCALED_CANDIDATES[name].base
    if name in DEVELOPMENT_CANDIDATES:
        return DEVELOPMENT_CANDIDATES[name]
    return REFERENCES[name]


def training_paths() -> list[Path]:
    return sorted(
        (p for directory in TRAINING_DIRS for p in directory.glob("*.json")), key=lambda p: p.name
    )


def _splits(path: Path) -> dict[str, str]:
    return {s["study_key"]: s["split"] for s in json.loads(path.read_text())["studies"]}


def load_training(name: str) -> tuple[list[AxialSeries], dict[str, str]]:
    """The 59 consumed records, each with the split its tranche's reservation gave it."""
    tranche1 = _splits(TRANCHE1_RESERVATION)
    tranche2 = {k for k, v in _splits(TRANCHE2_RESERVATION).items() if v == "sealed_test"}
    to_series = adapter(name)
    series: list[AxialSeries] = []
    hashes = {}
    for path in training_paths():
        record = json.loads(path.read_text())
        study, split = record["study_id"], record.get("split")
        first = tranche1.get(study) == split
        second = study in tranche2 and split == "sealed_test"
        if first == second:
            raise SystemExit(f"{path.name} does not match exactly one tranche reservation")
        series.extend(to_series(record))
        hashes[path.name] = sha256(path)
    return series, hashes


def load_sealed(name: str) -> tuple[list[AxialSeries], dict[str, str]]:
    reserved = {k for k, v in _splits(RESERVATION).items() if v == "sealed_test"}
    to_series = adapter(name)
    series: list[AxialSeries] = []
    hashes = {}
    for path in sorted(SEALED.glob("*.json")):
        record = json.loads(path.read_text())
        if record.get("split") != "sealed_test" or record["study_id"] not in reserved:
            raise SystemExit(f"{path.name} is not a reserved screen-2 sealed record")
        series.extend(to_series(record))
        hashes[path.name] = sha256(path)
    return series, hashes


def clusters(overlap: dict[str, Any], present: set[str]) -> list[list[str]]:
    """Connected components, among ``present``, of the file's pairs and clusters."""
    parent: dict[str, str] = {}

    def find(x: str) -> str:
        parent.setdefault(x, x)
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    links = [list(p["studies"]) for p in overlap.get("pairs", [])]
    links += [list(c) for c in overlap.get("development_clusters", [])]
    for members in links:
        inside = [m for m in members if m in present]
        for a, b in zip(inside, inside[1:], strict=False):
            parent[find(a)] = find(b)
    groups: dict[str, list[str]] = {}
    for member in parent:
        groups.setdefault(find(member), []).append(member)
    return sorted(sorted(g) for g in groups.values() if len(g) > 1)


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
    primary = protocol["primary_candidate"]
    if primary not in DEVELOPMENT_CANDIDATES and primary not in SCALED_CANDIDATES:
        problems.append(f"unknown candidate {primary}")
    else:
        if primary in SCALED_CANDIDATES:
            scaled = SCALED_CANDIDATES[primary]
            if scaled.describe() != protocol["candidates"][primary]:
                problems.append(f"scaled candidate {primary} differs from the protocol")
            base = scaled.base
            if DEVELOPMENT_CANDIDATES[base].describe() != protocol["candidates"].get(base):
                problems.append(f"candidate {base} differs from the protocol")
            if "interval_scaling" not in protocol:
                problems.append("the protocol gives no frozen interval scale")
        elif DEVELOPMENT_CANDIDATES[primary].describe() != protocol["candidates"][primary]:
            problems.append(f"candidate {primary} differs from the protocol")
    for name in protocol.get("reference_candidates", []):
        if name not in REFERENCES:
            problems.append(f"unknown reference {name}")
        elif REFERENCES[name].describe() != protocol["candidates"][name]:
            problems.append(f"reference {name} differs from the protocol")
    if RESERVATION.is_file():
        development = {k for k, v in _splits(RESERVATION).items() if v == "development"}
        studies = {json.loads(p.read_text())["study_id"] for p in training_paths()}
        if studies - development:
            problems.append("the screen-2 reservation does not list every training study")
    else:
        problems.append("no screen-2 reservation")
    return problems


def sealed_report(
    name: str,
    series: list[AxialSeries],
    sealed: list[AxialSeries],
    exclusions: dict[str, list[str]],
    protocol_sha256: str,
) -> dict[str, Any]:
    """Score every sealed study once with one configuration (fresh fit per study)."""
    config = configuration(name)
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
    parser.add_argument("--candidate", choices=CANDIDATE_NAMES)
    parser.add_argument("--overlap", type=Path, help="development: the declared overlap file")
    parser.add_argument("--protocol", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.development and (args.candidate is None or args.overlap is None):
        parser.error("--development requires --candidate and --overlap")
    if args.sealed and (args.protocol is None or args.candidate or args.overlap):
        parser.error("--sealed requires --protocol only (the protocol names the candidates)")
    if args.output.exists():
        parser.error("--output must not exist; every run gets a fresh directory")
    args.output.mkdir(parents=True)
    started = time.monotonic()
    record: dict[str, Any] = {
        "utc_start": datetime.now(UTC).isoformat(),
        "python": platform.python_version(),
        "mode": "development" if args.development else "sealed",
        "evaluation_version": additive.EVALUATION_VERSION,
    }
    if args.development:
        overlap_path = args.overlap.resolve()
        overlap = json.loads(overlap_path.read_text())
        record["author_overlap"] = {
            "path": str(overlap_path.relative_to(ROOT)),
            "sha256": sha256(overlap_path),
        }
        series, record["training_record_hashes"] = load_training(args.candidate)
        record["training_studies"] = sorted({s.study_id for s in series})
        linked = clusters(overlap, set(record["training_studies"]))
        record["clusters"] = linked
        record["candidate"] = args.candidate
        config = configuration(args.candidate)
        record["model_configuration"] = config.describe()
        report = additive.leave_one_study_out(
            config,
            series,
            scale=SCALE,
            clusters=linked,
            progress=lambda m: print(m, file=sys.stderr, flush=True),
        )
        if args.candidate in SCALED_CANDIDATES:
            scaled = SCALED_CANDIDATES[args.candidate]
            record["scaled_candidate"] = scaled.describe()
            report = nested_scale_report(report, scaled.spec)
            report["model_version"] = f"{report['model_version']}+{args.candidate}"
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
        overlap = json.loads(OVERLAP.read_text())
        record["author_overlap"] = {
            "path": str(OVERLAP.relative_to(ROOT)),
            "sha256": sha256(OVERLAP),
        }
        primary = protocol["primary_candidate"]
        series, record["training_record_hashes"] = load_training(primary)
        record["training_studies"] = sorted({s.study_id for s in series})
        sealed, record["sealed_record_hashes"] = load_sealed(primary)
        exclusions = overlap["sealed_training_exclusions"]
        report = sealed_report(primary, series, sealed, exclusions, record["protocol_sha256"])
        if primary in SCALED_CANDIDATES:
            frozen = HistoryStacking.from_description(protocol["interval_scaling"])
            report = scale_report(report, frozen, SCALED_CANDIDATES[primary].spec)
            report["model_version"] = f"{report['model_version']}+{primary}"
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
