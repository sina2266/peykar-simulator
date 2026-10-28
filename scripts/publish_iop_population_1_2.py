"""Write licence-respecting public artifacts of an IOP model 1.2 report.

Reads private reports of ``evaluate_iop_population_1_2.py`` (never re-runs a model).
The frozen 1.1.0 publisher (``publish_iop_population_1_1.py``, unchanged because its
protocol pins it) is loaded as a library, and through it the 1.0.0 publisher: their
licence classes, redaction and the H1-H4 analysis are reused, pointed at the 1.2
record sets (training: tranches 1 and 2; sealed: tranche 3) and at the 1.2 primary
methods. Records are classed by licence: CC BY or CC0 values may be republished,
everything else keeps only per-series metrics; privately kept tranche-1 development
records stay private. With ``--reference`` (the frozen model 1.1.0 on the same
studies), H5 compares model 1.2 with 1.1.0 trained on the same records: pooled
group-mean RMSE over common targets, and next-observation interval coverage for
targets within the first month. Output goes to a new directory.
"""

from __future__ import annotations

import argparse
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
        ROOT / ".peykar/iop-tranche2-sealed-2026-10-28/records",
    ],
    "sealed": [ROOT / ".peykar/iop-tranche3-sealed-2026-10-28/records"],
}
# Set by the development rounds before the freeze (see the 1.2 development log).
PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
# The frozen model 1.1.0's primaries.
REFERENCE_PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
FIRST_MONTH = 1.0
COVERAGE_LEVELS = {"80": ("0.1", "0.9", 0.80), "90": ("0.05", "0.95", 0.90)}


def _previous() -> ModuleType:
    path = ROOT / "scripts/publish_iop_population_1_1.py"
    spec = importlib.util.spec_from_file_location("publish_iop_population_1_1_0", path)
    if spec is None or spec.loader is None:
        raise SystemExit("cannot load the 1.1.0 publisher")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    module.__dict__["RECORD_DIRS"] = RECORD_DIRS
    module.__dict__["PRIVATE_DEVELOPMENT"] = PRIVATE_DEVELOPMENT
    module.__dict__["PRIMARY"] = dict(PRIMARY)
    module.base.__dict__["PRIMARY"] = dict(PRIMARY)
    return module


previous = _previous()
base = previous.base


def _rows(report: dict[str, Any], task: str, method: str) -> dict[tuple[Any, ...], Any]:
    out = {}
    for study in report["studies"]:
        for entry in study[task]:
            if entry["kind"] != "group_mean" or not entry["context"].startswith("human/"):
                continue
            for row in entry["rows"]:
                if method in row["predictions"]:
                    key = (
                        study["study_id"],
                        entry["series_id"],
                        row["origin_month"],
                        row["target_month"],
                    )
                    out[key] = row
    return out


def _initial(report: dict[str, Any], method: str) -> dict[tuple[Any, ...], float]:
    out = {}
    for study in report["studies"]:
        for entry in study["initial_state"]:
            if entry["kind"] != "group_mean" or not entry["context"].startswith("human/"):
                continue
            values = entry.get("predictions", {}).get(method)
            if values is None:
                continue
            for i, (p, o) in enumerate(zip(values, entry["observed"], strict=True)):
                out[(study["study_id"], entry["series_id"], i)] = p - o
    return out


def _rmse(errors: list[float]) -> float | None:
    return math.sqrt(sum((100 * e) ** 2 for e in errors) / len(errors)) if errors else None


def versus_reference(report: dict[str, Any], reference: dict[str, Any]) -> dict[str, Any]:
    """Model 1.2 against the frozen 1.1.0 trained on the same records (human group means)."""
    result: dict[str, Any] = {}
    for task in ("next_observation", "landmark"):
        new = _rows(report, task, PRIMARY[task])
        old = _rows(reference, task, REFERENCE_PRIMARY[task])
        common = sorted(set(new) & set(old))
        a = _rmse([new[k]["predictions"][PRIMARY[task]] - new[k]["observed"] for k in common])
        b = _rmse(
            [old[k]["predictions"][REFERENCE_PRIMARY[task]] - old[k]["observed"] for k in common]
        )
        result[task] = {
            "common_targets": len(common),
            "model_1_2_rmse_pp": a,
            "model_1_1_rmse_pp": b,
            "model_1_2_lower": bool(a is not None and b is not None and a < b),
        }
    first = _initial(report, PRIMARY["initial_state"])
    second = _initial(reference, REFERENCE_PRIMARY["initial_state"])
    common_initial = sorted(set(first) & set(second))
    a = _rmse([first[k] for k in common_initial])
    b = _rmse([second[k] for k in common_initial])
    result["initial_state"] = {
        "common_targets": len(common_initial),
        "model_1_2_rmse_pp": a,
        "model_1_1_rmse_pp": b,
        "model_1_2_lower": bool(a is not None and b is not None and a < b),
    }
    new = _rows(report, "next_observation", PRIMARY["next_observation"])
    old = _rows(reference, "next_observation", REFERENCE_PRIMARY["next_observation"])
    early = sorted(k for k in set(new) & set(old) if k[3] <= FIRST_MONTH + 1e-9)
    coverage: dict[str, Any] = {"targets": len(early)}
    for level, (low, high, nominal) in COVERAGE_LEVELS.items():
        values = []
        for rows, method in (
            (new, PRIMARY["next_observation"]),
            (old, REFERENCE_PRIMARY["next_observation"]),
        ):
            inside = [
                rows[k]["intervals"][method][low]
                <= rows[k]["observed"]
                <= rows[k]["intervals"][method][high]
                for k in early
                if method in rows[k]["intervals"]
            ]
            values.append(sum(inside) / len(inside) if inside else None)
        coverage[level] = {
            "nominal": nominal,
            "model_1_2": values[0],
            "model_1_1": values[1],
            "model_1_2_closer": bool(
                values[0] is not None
                and values[1] is not None
                and abs(values[0] - nominal) < abs(values[1] - nominal)
            ),
        }
    result["first_month_next_observation_coverage"] = coverage
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--reference", type=Path, help="frozen 1.1.0 report (H5)")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    if args.sealed and args.reference is None:
        parser.error("--sealed requires --reference")
    name = "development" if args.development else "sealed"
    report = json.loads(args.report.read_text())
    classes = previous.licence_classes(name)
    unknown = [s["study_id"] for s in report["studies"] if s["study_id"] not in classes]
    if unknown:
        raise SystemExit(f"No record found for {unknown}")
    result = base.analysis(report)
    extra: dict[str, Any] = {}
    reference = None
    if args.reference is not None:
        reference = json.loads(args.reference.read_text())
        h5 = versus_reference(report, reference)
        result["H5_versus_model_1_1_same_training"] = h5
        extra["reference_report_sha256"] = previous.sha256(args.reference)
    redacted = sorted(k for k, v in classes.items() if not v)
    for block in result["per_series"].values():
        for row in block:
            row["values_public"] = classes[row["study_id"]]

    def public(source: dict[str, Any], path: Path) -> dict[str, Any]:
        return {
            **{k: v for k, v in source.items() if k != "studies"},
            "studies": [s if classes[s["study_id"]] else base.redact(s) for s in source["studies"]],
            "source_report_sha256": previous.sha256(path),
        }

    args.output.mkdir(parents=True)
    (args.output / "report-public.json").write_text(
        json.dumps(public(report, args.report), indent=1) + "\n"
    )
    if reference is not None:
        (args.output / "reference-1.1.0-public.json").write_text(
            json.dumps(public(reference, args.reference), indent=1) + "\n"
        )
    (args.output / f"{name}-analysis.json").write_text(
        json.dumps(
            {
                "mode": name,
                "status": "declared sealed-test analysis"
                if args.sealed
                else "development reference only",
                "model_version": report.get("model_version"),
                "source_report_sha256": previous.sha256(args.report),
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
    text = base.tables(result, name).replace("model 1.0.0", f"model {report.get('model_version')}")
    if "H5_versus_model_1_1_same_training" in result:
        h5 = result["H5_versus_model_1_1_same_training"]
        lines = ["", "## H5: model 1.2 against the frozen model 1.1.0, same training", ""]
        for task in ("next_observation", "landmark", "initial_state"):
            v = h5[task]
            lines.append(
                f"- {task}: 1.2 {base._fmt(v['model_1_2_rmse_pp'])} against 1.1 "
                f"{base._fmt(v['model_1_1_rmse_pp'])} pp on {v['common_targets']} common targets"
            )
        cov = h5["first_month_next_observation_coverage"]
        for level in COVERAGE_LEVELS:
            v = cov[level]
            lines.append(
                f"- first-month next-observation {level}% coverage ({cov['targets']} targets): "
                f"1.2 {base._fmt(v['model_1_2'])} against 1.1 {base._fmt(v['model_1_1'])}"
            )
        text += "\n".join(lines) + "\n"
    (args.output / "tables.md").write_text(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
