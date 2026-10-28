"""Write licence-respecting public artifacts of an IOP model 1.1 report.

Reads private reports of ``evaluate_iop_population_1_1.py`` (never re-runs a model).
The frozen 1.0.0 publisher (``publish_iop_population.py``, unchanged because its
protocol pins it) is loaded as a library: its redaction, paired comparisons,
coverage, per-series tables and the H1-H4 analysis are reused with the 1.1 primary
methods. Records are classed by licence: CC BY or CC0 values may be republished,
everything else keeps only per-series metrics; privately kept tranche-1 development
records stay private. In ``--sealed`` mode the analysis declared by the frozen 1.1
protocol is computed, including H5: the 1.1 primary against the 1.0.0 primary
trained on the same 72 records, over common group-mean targets. Output goes to a
new directory.
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
DATASET = ROOT / "experiments/data/iop-glaucoma-surgery-2026-10-28"
PRIVATE_DEVELOPMENT = ROOT / ".peykar/iop-development-private/records"
RECORD_DIRS = {
    "development": [
        DATASET / "development/records",
        PRIVATE_DEVELOPMENT,
        ROOT / ".peykar/iop-sealed-2026-10-28/records",
    ],
    "sealed": [ROOT / ".peykar/iop-tranche2-sealed-2026-10-28/records"],
}
# Set by the development rounds before the freeze (see the 1.1 development log).
PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
REFERENCE_PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}


def _base() -> ModuleType:
    path = ROOT / "scripts/publish_iop_population.py"
    spec = importlib.util.spec_from_file_location("publish_iop_population_1_0_0", path)
    if spec is None or spec.loader is None:
        raise SystemExit("cannot load the 1.0.0 publisher")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    # The 1.0.0 analysis reads the module-level PRIMARY; point it at the 1.1 primaries.
    module.__dict__["PRIMARY"] = dict(PRIMARY)
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


def versus_reference(report: dict[str, Any], reference: dict[str, Any]) -> dict[str, Any]:
    """H5: pooled group-mean RMSE of the 1.1 primary against the 1.0.0 primary trained on
    the same records, over common targets (next observation and landmark)."""
    result: dict[str, Any] = {}
    for task in ("next_observation", "landmark"):
        new = _rows(report, task, PRIMARY[task])
        old = _rows(reference, task, REFERENCE_PRIMARY[task])
        common = sorted(set(new) & set(old))
        a = math.sqrt(sum((100 * new[k]) ** 2 for k in common) / len(common)) if common else None
        b = math.sqrt(sum((100 * old[k]) ** 2 for k in common) / len(common)) if common else None
        result[task] = {
            "common_targets": len(common),
            "model_1_1_rmse_pp": a,
            "model_1_0_rmse_pp": b,
            "model_1_1_lower": bool(a is not None and b is not None and a < b),
        }
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--reference", type=Path, help="1.0.0 report on the same studies (H5)")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    if args.sealed and args.reference is None:
        parser.error("--sealed requires --reference")
    name = "development" if args.development else "sealed"
    report = json.loads(args.report.read_text())
    classes = licence_classes(name)
    unknown = [s["study_id"] for s in report["studies"] if s["study_id"] not in classes]
    if unknown:
        raise SystemExit(f"No record found for {unknown}")
    result = base.analysis(report)
    extra: dict[str, Any] = {}
    if args.reference is not None:
        reference = json.loads(args.reference.read_text())
        h5 = versus_reference(report, reference)
        result["H5_versus_model_1_0_same_training"] = h5
        result["verdicts"]["H5_model_1_1_better_than_1_0_on_group_means"] = all(
            v["model_1_1_lower"] for v in h5.values()
        )
        extra["reference_report_sha256"] = sha256(args.reference)
    public = {
        **{k: v for k, v in report.items() if k != "studies"},
        "studies": [s if classes[s["study_id"]] else base.redact(s) for s in report["studies"]],
        "source_report_sha256": sha256(args.report),
    }
    redacted = sorted(k for k, v in classes.items() if not v)
    for block in result["per_series"].values():
        for row in block:
            row["values_public"] = classes[row["study_id"]]
    args.output.mkdir(parents=True)
    (args.output / "report-public.json").write_text(json.dumps(public, indent=1) + "\n")
    if args.reference is not None:
        reference = json.loads(args.reference.read_text())
        reference_public = {
            **{k: v for k, v in reference.items() if k != "studies"},
            "studies": [
                s if classes[s["study_id"]] else base.redact(s) for s in reference["studies"]
            ],
            "source_report_sha256": sha256(args.reference),
        }
        (args.output / "reference-1.0.0-public.json").write_text(
            json.dumps(reference_public, indent=1) + "\n"
        )
    (args.output / f"{name}-analysis.json").write_text(
        json.dumps(
            {
                "mode": name,
                "status": "declared sealed-test analysis"
                if args.sealed
                else "development reference only",
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
    text = base.tables(result, name).replace("model 1.0.0", "model 1.1.0")
    if "H5_versus_model_1_0_same_training" in result:
        lines = ["", "## H5: model 1.1 against model 1.0.0 trained on the same records", ""]
        for task, v in result["H5_versus_model_1_0_same_training"].items():
            lines.append(
                f"- {task}: 1.1 {base._fmt(v['model_1_1_rmse_pp'])} against 1.0 "
                f"{base._fmt(v['model_1_0_rmse_pp'])} pp on {v['common_targets']} common targets"
            )
        text += "\n".join(lines) + "\n"
    (args.output / "tables.md").write_text(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
