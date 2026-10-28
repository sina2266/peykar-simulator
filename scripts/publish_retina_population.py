"""Write licence-respecting public artifacts of a retinal-thickness model report.

Reads private reports of ``evaluate_retina_population.py`` (never re-runs a model).
The frozen IOP 1.0.0 publisher (``publish_iop_population.py``, unchanged because its
protocol pins it) is loaded as a library: its redaction, paired comparisons,
coverage, per-series tables and the H1-H4 analysis are reused with the retinal
primary methods and the baseline comparator named ``baseline_level``. Records are
classed by licence: CC BY or CC0 values may be republished, everything else keeps
only per-series metrics; privately kept development records stay private. With
``--reference`` (a report of a reference configuration on the same studies), H5
compares the primary methods over common group-mean targets. Output goes to a new
directory.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import math
import sys
from pathlib import Path
from types import ModuleType
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/retina-cst-2026-10-28"
PRIVATE_DEVELOPMENT = ROOT / ".peykar/retina-development-private/records"
RECORD_DIRS = {
    "development": [DATASET / "development/records", PRIVATE_DEVELOPMENT],
    "sealed": [ROOT / ".peykar/retina-sealed-2026-10-28/records"],
}
# Set by the development rounds before the freeze (see the development log): the
# adopted candidate r3 (Gaussian variant) and the reference candidate r0 (robust).
PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
REFERENCE_PRIMARY = {
    "next_observation": "population_assimilation_robust",
    "landmark": "population_assimilation_robust",
    "initial_state": "population_initial_state",
}
# H5 tests what the adopted change targets: next-observation forecasts.
H5_TASKS = ("next_observation",)


def _base() -> ModuleType:
    path = ROOT / "scripts/publish_iop_population.py"
    spec = importlib.util.spec_from_file_location("publish_iop_population_1_0_0", path)
    if spec is None or spec.loader is None:
        raise SystemExit("cannot load the IOP 1.0.0 publisher")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    # The 1.0.0 analysis reads module-level constants; point them at this component.
    module.__dict__["PRIMARY"] = dict(PRIMARY)
    module.__dict__["REPORTED"] = tuple(
        "baseline_level" if name == "preoperative_level" else name for name in module.REPORTED
    )
    return module


base = _base()


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def licence_classes(mode: str) -> dict[str, bool]:
    classes = {}
    for directory in RECORD_DIRS[mode]:
        for path in directory.glob("*.json"):
            record = json.loads(path.read_text())
            private = directory == PRIVATE_DEVELOPMENT
            classes[record["study_id"]] = base.publishable(record) and not private
    return classes


def redact(study: dict[str, Any]) -> dict[str, Any]:
    out: dict[str, Any] = base.redact(study)
    for entry in out["initial_state"]:
        entry.pop("reference_value", None)
    return out


def _rows(
    report: dict[str, Any], task: str, method: str
) -> dict[tuple[str, str, float, float], float]:
    out = {}
    for study in report["studies"]:
        for entry in study[task]:
            if entry["kind"] != "group_mean":
                continue
            for row in entry["rows"]:
                if method in row["predictions"]:
                    key = (
                        study["study_id"],
                        entry["series_id"],
                        row["origin_month"],
                        row["target_month"],
                    )
                    out[key] = row["predictions"][method] - row["observed"]
    return out


def _initial_rows(report: dict[str, Any], method: str) -> dict[tuple[str, str, int], float]:
    out = {}
    for study in report["studies"]:
        for entry in study["initial_state"]:
            if entry["kind"] != "group_mean" or method not in entry.get("predictions", {}):
                continue
            for i, (p, o) in enumerate(
                zip(entry["predictions"][method], entry["observed"], strict=True)
            ):
                out[(study["study_id"], entry["series_id"], i)] = p - o
    return out


def versus_reference(report: dict[str, Any], reference: dict[str, Any]) -> dict[str, Any]:
    """H5: pooled group-mean RMSE of the primary against the reference configuration's
    primary trained on the same records, over common targets of every task."""
    result: dict[str, Any] = {}
    for task in ("next_observation", "landmark"):
        new = _rows(report, task, PRIMARY[task])
        old = _rows(reference, task, REFERENCE_PRIMARY[task])
        result[task] = _compare([(new[k], old[k]) for k in sorted(set(new) & set(old))])
    first = _initial_rows(report, PRIMARY["initial_state"])
    second = _initial_rows(reference, REFERENCE_PRIMARY["initial_state"])
    result["initial_state"] = _compare(
        [(first[k], second[k]) for k in sorted(set(first) & set(second))]
    )
    return result


def _compare(pairs: list[tuple[float, float]]) -> dict[str, Any]:
    n = len(pairs)
    a = math.sqrt(sum((100 * x) ** 2 for x, _ in pairs) / n) if n else None
    b = math.sqrt(sum((100 * y) ** 2 for _, y in pairs) / n) if n else None
    return {
        "common_targets": n,
        "primary_rmse_pp": a,
        "reference_rmse_pp": b,
        "primary_lower": bool(a is not None and b is not None and a < b),
    }


def _public(report: dict[str, Any], classes: dict[str, bool], source: Path) -> dict[str, Any]:
    return {
        **{k: v for k, v in report.items() if k != "studies"},
        "studies": [s if classes[s["study_id"]] else redact(s) for s in report["studies"]],
        "source_report_sha256": sha256(source),
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--reference", type=Path, help="reference configuration report (H5)")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    name = "development" if args.development else "sealed"
    report = json.loads(args.report.read_text())
    classes = licence_classes(name)
    unknown = [s["study_id"] for s in report["studies"] if s["study_id"] not in classes]
    if unknown:
        raise SystemExit(f"No record found for {unknown}")
    result = base.analysis(report)
    extra: dict[str, Any] = {}
    reference = None
    if args.reference is not None:
        reference = json.loads(args.reference.read_text())
        h5 = versus_reference(report, reference)
        result["H5_versus_reference_same_training"] = h5
        result["verdicts"]["H5_primary_better_than_reference_next_observation"] = all(
            h5[task]["primary_lower"] and h5[task]["common_targets"] > 0 for task in H5_TASKS
        )
        extra["reference_report_sha256"] = sha256(args.reference)
        extra["reference_model_version"] = reference.get("model_version")
    redacted = sorted(k for k, v in classes.items() if not v)
    for block in result["per_series"].values():
        for row in block:
            row["values_public"] = classes[row["study_id"]]
    args.output.mkdir(parents=True)
    (args.output / "report-public.json").write_text(
        json.dumps(_public(report, classes, args.report), indent=1) + "\n"
    )
    if reference is not None:
        (args.output / "reference-public.json").write_text(
            json.dumps(_public(reference, classes, args.reference), indent=1) + "\n"
        )
    (args.output / f"{name}-analysis.json").write_text(
        json.dumps(
            {
                "mode": name,
                "status": "declared sealed-test analysis"
                if args.sealed
                else "development reference only",
                "model_version": report.get("model_version"),
                "source_report_sha256": sha256(args.report),
                **extra,
                "studies_with_redacted_values": [
                    s for s in redacted if any(r["study_id"] == s for r in report["studies"])
                ],
                **result,
            },
            indent=1,
        )
        + "\n"
    )
    title = f"Retinal-thickness model {report.get('model_version')}"
    text = (
        base.tables(result, name)
        .replace("IOP-after-surgery model 1.0.0", title)
        .replace("by procedure context", "by context")
    )
    if "H5_versus_reference_same_training" in result:
        lines = ["", "## H5: primary against the reference configuration, same training", ""]
        for task, v in result["H5_versus_reference_same_training"].items():
            lines.append(
                f"- {task}: primary {base._fmt(v['primary_rmse_pp'])} against reference "
                f"{base._fmt(v['reference_rmse_pp'])} pp on {v['common_targets']} common targets"
            )
        text += "\n".join(lines) + "\n"
    (args.output / "tables.md").write_text(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
