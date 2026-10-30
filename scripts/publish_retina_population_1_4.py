"""Write licence-respecting public artifacts of a retinal-thickness model 1.4 report.

Reads private reports of ``evaluate_retina_population_1_4.py`` (never re-runs a model).
The frozen model 1.0.0 publisher (``publish_retina_population.py``, unchanged because
its protocol pins it) is loaded as a library, and through it the IOP 1.0.0 analysis:
licence classes, redaction and the H1-H4 analysis are reused, pointed at the 1.4 record
sets (training: the 195 record studies of tranches 1-3; sealed: tranche 4) and at the
primaries of model 1.4: the stacked forecasts from follow-up (``stacked``, with the
scaled interval) for the next observation and the landmark, and the model's own
``population_initial_state`` for the initial state. Only human series are tested, with
the summary recomputed by ``eye_stacking_evaluation2`` so the coverage of the stacked
intervals is kept. Redacted studies also lose their initial-state intervals.

With ``--reference`` (the frozen model 1.1.0, candidate ``e0``, the unstacked report of
the same sealed run) H5 compares model 1.4 with 1.1.0 over common human group-mean
targets: the pooled landmark RMSE of 1.4 lower and its pooled next-observation RMSE at
most 0.05 pp higher. The initial states are 1.1.0's by construction (checked, and their
RMSE reported). The reference's own H1-H4 on the same studies are reported, not tested.
Output goes to a new directory.
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

from peykar.validation import eye_stacking_evaluation2 as stacking
from peykar.validation.eye_iop_evaluation import summarize
from peykar.validation.eye_recovery_evaluation6 import INTERVAL_FIELD

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/retina-cst-2026-10-28"
PRIVATE_DEVELOPMENT = ROOT / ".peykar/retina-development-private/records"
CONSUMED = (
    ROOT / ".peykar/retina-sealed-2026-10-28/records",
    ROOT / ".peykar/retina-tranche2-sealed-2026-10-28/records",
    ROOT / ".peykar/retina-tranche3-sealed-2026-10-29/records",
)
RECORD_DIRS = {
    "development": [DATASET / "development/records", PRIVATE_DEVELOPMENT, *CONSUMED],
    "sealed": [ROOT / ".peykar/retina-tranche4-sealed-2026-10-30/records"],
}
PRIMARY = {
    "next_observation": stacking.STACKED,
    "landmark": stacking.STACKED,
    "initial_state": "population_initial_state",
}
REFERENCE_PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
MODEL_VERSION = "1.4.0-k3"
TASKS = ("initial_state", "next_observation", "landmark")
TESTED_PREFIX = "human/"
H5_RMSE_TOLERANCE_PP = 0.05
H5_VERDICT = "H5_landmark_lower_next_observation_within_0_05_pp"


def _publisher_1_0() -> ModuleType:
    path = ROOT / "scripts/publish_retina_population.py"
    spec = importlib.util.spec_from_file_location("publish_retina_population_1_0_0", path)
    if spec is None or spec.loader is None:
        raise SystemExit("cannot load the retinal-thickness 1.0.0 publisher")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    module.__dict__["RECORD_DIRS"] = RECORD_DIRS
    module.__dict__["PRIVATE_DEVELOPMENT"] = PRIVATE_DEVELOPMENT
    module.base.__dict__["PRIMARY"] = dict(PRIMARY)
    return module


previous = _publisher_1_0()
base = previous.base


def licence_classes(mode: str) -> dict[str, bool]:
    """Publishable studies; consumed sealed records of earlier tests count as private in
    development (their values were published, if at all, by those tests)."""
    classes: dict[str, bool] = previous.licence_classes(mode)
    if mode == "development":
        for directory in CONSUMED:
            for path in directory.glob("*.json"):
                classes[json.loads(path.read_text())["study_id"]] = False
    return classes


def tested_subset(report: dict[str, Any]) -> dict[str, Any]:
    """Human series only, summary recomputed with the stacked coverage kept."""
    studies = [
        {
            **study,
            **{
                task: [e for e in study[task] if e["context"].startswith(TESTED_PREFIX)]
                for task in TASKS
            },
        }
        for study in report["studies"]
    ]
    return {**report, "studies": studies, "summary": stacking.summarize_stacked2(studies)}


def reference_subset(report: dict[str, Any]) -> dict[str, Any]:
    studies = tested_subset(report)["studies"]
    return {**report, "studies": studies, "summary": summarize(studies)}


def redact(study: dict[str, Any]) -> dict[str, Any]:
    out: dict[str, Any] = previous.redact(study)
    for entry in out["initial_state"]:
        entry.pop(INTERVAL_FIELD, None)
    return out


def _public(report: dict[str, Any], classes: dict[str, bool], source: Path) -> dict[str, Any]:
    return {
        **{k: v for k, v in report.items() if k != "studies"},
        "studies": [s if classes[s["study_id"]] else redact(s) for s in report["studies"]],
        "source_report_sha256": previous.sha256(source),
    }


def _rmse(errors: list[float]) -> float | None:
    return math.sqrt(sum(e * e for e in errors) / len(errors)) if errors else None


def _rows(report: dict[str, Any], task: str, method: str) -> dict[tuple[Any, ...], float]:
    out = {}
    for study in report["studies"]:
        for entry in study[task]:
            if entry["kind"] != "group_mean" or not entry["context"].startswith(TESTED_PREFIX):
                continue
            for row in entry["rows"]:
                if method in row["predictions"]:
                    key = (
                        study["study_id"],
                        entry["series_id"],
                        row["origin_month"],
                        row["target_month"],
                    )
                    out[key] = 100 * (row["predictions"][method] - row["observed"])
    return out


def _initial(report: dict[str, Any], method: str) -> dict[tuple[Any, ...], float]:
    out = {}
    for study in report["studies"]:
        for entry in study["initial_state"]:
            values = entry["predictions"].get(method)
            if entry["kind"] != "group_mean" or values is None:
                continue
            if not entry["context"].startswith(TESTED_PREFIX):
                continue
            for month, p, o in zip(entry["months"], values, entry["observed"], strict=True):
                out[(study["study_id"], entry["series_id"], month)] = 100 * (p - o)
    return out


def versus_reference(report: dict[str, Any], reference: dict[str, Any]) -> dict[str, Any]:
    """Model 1.4 against the frozen 1.1.0 with the same fit (human group means)."""
    result: dict[str, Any] = {}
    for task in TASKS:
        if task == "initial_state":
            new = _initial(report, PRIMARY[task])
            old = _initial(reference, REFERENCE_PRIMARY[task])
        else:
            new = _rows(report, task, PRIMARY[task])
            old = _rows(reference, task, REFERENCE_PRIMARY[task])
        common = sorted(set(new) & set(old))
        a = _rmse([new[k] for k in common])
        b = _rmse([old[k] for k in common])
        result[task] = {
            "common_targets": len(common),
            "model_1_4_rmse_pp": a,
            "model_1_1_rmse_pp": b,
            "model_1_4_lower": bool(a is not None and b is not None and a < b),
        }
    result["initial_state"]["tested"] = False
    result["initial_state"]["note"] = "model 1.1.0's initial states by construction"
    return result


def h5_passes(h5: dict[str, Any]) -> bool:
    accuracy = h5["next_observation"]
    return bool(
        h5["landmark"]["common_targets"] > 0
        and accuracy["common_targets"] > 0
        and h5["landmark"]["model_1_4_lower"]
        and accuracy["model_1_4_rmse_pp"] is not None
        and accuracy["model_1_1_rmse_pp"] is not None
        and accuracy["model_1_4_rmse_pp"] <= accuracy["model_1_1_rmse_pp"] + H5_RMSE_TOLERANCE_PP
    )


def reference_checks(reference: dict[str, Any]) -> dict[str, Any]:
    """The frozen 1.1.0's own H1-H4 on the same studies, with its primaries (reported only)."""
    saved = base.__dict__["PRIMARY"]
    base.__dict__["PRIMARY"] = dict(REFERENCE_PRIMARY)
    try:
        result = base.analysis(reference_subset(reference))
    finally:
        base.__dict__["PRIMARY"] = saved
    keep = (
        "primary_methods",
        "verdicts",
        "H1_calibration_next_observation_group_means",
        "H2_accuracy_next_observation_group_means",
        "H3_landmark_group_means",
    )
    return {
        **{key: result[key] for key in keep},
        "H4_initial_state_control_group_means": {
            "summary": result["H4_initial_state_control_group_means"]["summary"]
        },
    }


def _extra_lines(result: dict[str, Any]) -> list[str]:
    lines = ["", "## H5: model 1.4 against the frozen model 1.1.0, same fit", ""]
    for task in ("next_observation", "landmark", "initial_state"):
        v = result["H5_versus_model_1_1_same_fit"][task]
        note = " (not tested: 1.1.0's own initial states)" if task == "initial_state" else ""
        lines.append(
            f"- {task}: 1.4 {base._fmt(v['model_1_4_rmse_pp'])} against 1.1 "
            f"{base._fmt(v['model_1_1_rmse_pp'])} pp on {v['common_targets']} common "
            f"targets{note}"
        )
    checks = result["model_1_1_own_H1_H4_reported_not_tested"]
    lines += ["", "## Model 1.1.0 on the same studies, its own H1-H4 (reported, not tested)", ""]
    for name, passed in checks["verdicts"].items():
        lines.append(f"- {name}: {'pass' if passed else 'FAIL'}")
    u2 = REFERENCE_PRIMARY["next_observation"]
    for level, cov in checks["H1_calibration_next_observation_group_means"].items():
        lines.append(
            f"- H1 next observation {level}% interval: {cov['inside']}/{cov['n']} = "
            f"{base._fmt(cov['coverage'], 3)}"
        )
    for label, block in (
        ("H2 next observation", checks["H2_accuracy_next_observation_group_means"]),
        ("H3 landmark", checks["H3_landmark_group_means"]["comparisons"]),
    ):
        for name, v in block.items():
            lines.append(
                f"- {label} vs {name}: {base._fmt(v[f'{u2}_rmse_pp'])} against "
                f"{base._fmt(v[f'{name}_rmse_pp'])} pp on {v['common_targets']} common targets"
            )
    initial = checks["H4_initial_state_control_group_means"]["summary"]
    for name in (REFERENCE_PRIMARY["initial_state"], "pooled_training_curve"):
        v = initial.get(name)
        if v is not None:
            lines.append(f"- H4 initial state {name}: {base._fmt(v['pooled_rmse_pp'])} pp")
    return lines


def _check_reference(report: dict[str, Any], reference: dict[str, Any]) -> None:
    """The stacked report must carry the reference's own forecasts unchanged."""
    for task in ("next_observation", "landmark"):
        method = REFERENCE_PRIMARY[task]
        if _rows(report, task, method) != _rows(reference, task, method):
            raise SystemExit(f"The report's {task} forecasts are not the reference's")
    method = REFERENCE_PRIMARY["initial_state"]
    if _initial(report, method) != _initial(reference, method):
        raise SystemExit("The report's initial states are not the reference's")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--reference", type=Path, help="frozen 1.1.0 (e0) report (H5)")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    name = "development" if args.development else "sealed"
    report = json.loads(args.report.read_text())
    if report.get("model_version") != MODEL_VERSION:
        raise SystemExit(f"Expected a model {MODEL_VERSION} report")
    reference = None
    if args.reference is not None:
        reference = json.loads(args.reference.read_text())
        if {s["study_id"] for s in reference["studies"]} != {
            s["study_id"] for s in report["studies"]
        }:
            raise SystemExit("The reference report scores other studies")
        _check_reference(report, reference)
    classes = licence_classes(name)
    unknown = [s["study_id"] for s in report["studies"] if s["study_id"] not in classes]
    if unknown:
        raise SystemExit(f"No record found for {unknown}")
    result = base.analysis(tested_subset(report))
    extra: dict[str, Any] = {}
    if reference is not None and args.reference is not None:
        h5 = versus_reference(report, reference)
        result["H5_versus_model_1_1_same_fit"] = h5
        result["verdicts"][H5_VERDICT] = h5_passes(h5)
        result["model_1_1_own_H1_H4_reported_not_tested"] = reference_checks(reference)
        extra["reference_report_sha256"] = previous.sha256(args.reference)
        extra["reference_model_version"] = reference.get("model_version")
    redacted = sorted(k for k, v in classes.items() if not v)
    for block in result["per_series"].values():
        for row in block:
            row["values_public"] = classes[row["study_id"]]
    args.output.mkdir(parents=True)
    (args.output / "report-public.json").write_text(
        json.dumps(_public(report, classes, args.report), indent=1) + "\n"
    )
    if reference is not None and args.reference is not None:
        (args.output / "reference-e0-report-public.json").write_text(
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
                "stacking": report.get("stacking"),
                "source_report_sha256": previous.sha256(args.report),
                **extra,
                "tested_subset": "human series (contexts human/...)",
                "studies_with_redacted_values": [
                    s for s in redacted if any(r["study_id"] == s for r in report["studies"])
                ],
                **result,
            },
            indent=1,
        )
        + "\n"
    )
    text = (
        base.tables(result, name)
        .replace("IOP-after-surgery model 1.0.0", f"Retinal-thickness model {MODEL_VERSION}")
        .replace("## Declared checks (group means)", "## Declared checks (human group means)")
        .replace("by procedure context", "by context")
    )
    if "H5_versus_model_1_1_same_fit" in result:
        text += "\n".join(_extra_lines(result)) + "\n"
    text += "\nUnits: pp of each series' baseline central retinal thickness.\n"
    (args.output / "tables.md").write_text(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
