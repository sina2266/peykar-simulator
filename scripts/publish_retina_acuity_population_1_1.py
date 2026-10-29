"""Write licence-respecting public artifacts of a retinal visual-acuity model 1.1 report.

Reads private reports of ``evaluate_retina_acuity_population_1_1.py`` (never re-runs a
model). The frozen model 1.0 publisher (``publish_retina_acuity_population.py``,
unchanged because its protocol pins it) is loaded as a library, and through it the IOP
1.0.0 analysis: licence classes, redaction (with the allow-list of initial-state
fields), the human-series subset and the H1-H4 analysis are reused, pointed at the 1.1
record sets (training: the 24 tranche-1 records; sealed: tranche 2) and at the 1.1
primary methods.

With ``--reference`` (the frozen model 1.0.0, candidate ``w0``, scored on the same
studies) H5 compares model 1.1 with 1.0.0 trained on the same records, over common
human group-mean targets: the pooled landmark and initial-state RMSE of 1.1 each lower,
and its pooled next-observation RMSE at most 0.05 pp (letters) higher. One reported pp
is one ETDRS letter (0.02 logMAR). Output goes to a new directory.
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
DATASET = ROOT / "experiments/data/retina-acuity-2026-10-29"
PRIVATE_DEVELOPMENT = ROOT / ".peykar/retina-acuity-development-private/records"
RECORD_DIRS = {
    "development": [
        DATASET / "development/records",
        PRIVATE_DEVELOPMENT,
        ROOT / ".peykar/retina-acuity-sealed-2026-10-29/records",
    ],
    "sealed": [ROOT / ".peykar/retina-acuity-tranche2-sealed-2026-10-29/records"],
}
# Set by the 1.1 development rounds before the freeze (see the 1.1 development log).
PRIMARY = {
    "next_observation": "population_anchored_t4",
    "landmark": "population_anchored_t4",
    "initial_state": "population_initial_state",
}
# The frozen model 1.0.0's primaries.
REFERENCE_PRIMARY = {
    "next_observation": "population_assimilation_t4",
    "landmark": "population_assimilation_t4",
    "initial_state": "population_initial_state",
}
MODEL_VERSION = "1.1.0-w1"
REFERENCE_VERSION = "1.0.0-v3"
TASKS = ("initial_state", "next_observation", "landmark")
TESTED_PREFIX = "human/"
H5_RMSE_TOLERANCE_PP = 0.05
H5_VERDICT = "H5_landmark_and_initial_state_lower_next_observation_within_0_05_pp"


def _previous() -> ModuleType:
    path = ROOT / "scripts/publish_retina_acuity_population.py"
    spec = importlib.util.spec_from_file_location("publish_retina_acuity_population_1_0", path)
    if spec is None or spec.loader is None:
        raise SystemExit("cannot load the retinal acuity 1.0 publisher")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    # The 1.0 licence classes and the IOP 1.0.0 analysis read these module-level names.
    module.__dict__["RECORD_DIRS"] = RECORD_DIRS
    module.__dict__["PRIVATE_DEVELOPMENT"] = PRIVATE_DEVELOPMENT
    module.__dict__["PRIMARY"] = dict(PRIMARY)
    module.base.__dict__["PRIMARY"] = dict(PRIMARY)
    return module


previous = _previous()
base = previous.base


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _rmse(errors: list[float]) -> float | None:
    if not errors:
        return None
    return math.sqrt(sum(e * e for e in errors) / len(errors))


def _rows(report: dict[str, Any], task: str, method: str) -> dict[tuple[Any, ...], float]:
    """Errors in pp (100 (forecast - observed)) of human group-mean rows, by target key."""
    out = {}
    for study in report["studies"]:
        for entry in study[task]:
            if entry["kind"] != "group_mean" or not entry["context"].startswith(TESTED_PREFIX):
                continue
            for row in entry["rows"]:
                if method in row["predictions"]:
                    key = (entry["series_id"], row["origin_month"], row["target_month"])
                    out[key] = 100.0 * (row["predictions"][method] - row["observed"])
    return out


def _initial(report: dict[str, Any], method: str) -> dict[tuple[Any, ...], float]:
    out = {}
    for study in report["studies"]:
        for entry in study["initial_state"]:
            if entry["kind"] != "group_mean" or not entry["context"].startswith(TESTED_PREFIX):
                continue
            predictions = entry.get("predictions", {}).get(method)
            if predictions is None:
                continue
            for month, p, o in zip(entry["months"], predictions, entry["observed"], strict=True):
                out[(entry["series_id"], month)] = 100.0 * (p - o)
    return out


def _comparison(targets: int, new: float | None, old: float | None) -> dict[str, Any]:
    return {
        "common_targets": targets,
        "model_1_1_rmse_pp": new,
        "model_1_0_rmse_pp": old,
        "model_1_1_lower": bool(new is not None and old is not None and new < old),
    }


def versus_reference(report: dict[str, Any], reference: dict[str, Any]) -> dict[str, Any]:
    """Model 1.1 against the frozen 1.0.0 trained on the same records (human group means)."""
    result: dict[str, Any] = {}
    for task in ("next_observation", "landmark"):
        new = _rows(report, task, PRIMARY[task])
        old = _rows(reference, task, REFERENCE_PRIMARY[task])
        common = sorted(set(new) & set(old))
        result[task] = _comparison(
            len(common), _rmse([new[k] for k in common]), _rmse([old[k] for k in common])
        )
    new = _initial(report, PRIMARY["initial_state"])
    old = _initial(reference, REFERENCE_PRIMARY["initial_state"])
    common = sorted(set(new) & set(old))
    result["initial_state"] = _comparison(
        len(common), _rmse([new[k] for k in common]), _rmse([old[k] for k in common])
    )
    return result


def h5_passes(h5: dict[str, Any]) -> bool:
    accuracy = h5["next_observation"]
    return bool(
        all(h5[task]["common_targets"] > 0 for task in TASKS)
        and h5["landmark"]["model_1_1_lower"]
        and h5["initial_state"]["model_1_1_lower"]
        and accuracy["model_1_1_rmse_pp"] is not None
        and accuracy["model_1_0_rmse_pp"] is not None
        and accuracy["model_1_1_rmse_pp"] <= accuracy["model_1_0_rmse_pp"] + H5_RMSE_TOLERANCE_PP
    )


def h5_lines(h5: dict[str, Any]) -> list[str]:
    lines = ["", "## H5: model 1.1 against the frozen model 1.0.0, same training", ""]
    for task in ("next_observation", "landmark", "initial_state"):
        v = h5[task]
        lines.append(
            f"- {task}: 1.1 {base._fmt(v['model_1_1_rmse_pp'])} against 1.0 "
            f"{base._fmt(v['model_1_0_rmse_pp'])} pp on {v['common_targets']} common targets"
        )
    return lines


def reference_checks(reference: dict[str, Any]) -> dict[str, Any]:
    """The frozen 1.0.0's own H1-H4 on the same studies, with its primaries (reported only)."""
    saved = base.__dict__["PRIMARY"]
    base.__dict__["PRIMARY"] = dict(REFERENCE_PRIMARY)
    try:
        result = base.analysis(previous.tested_subset(reference))
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


def reference_lines(checks: dict[str, Any]) -> list[str]:
    u2 = REFERENCE_PRIMARY["next_observation"]
    lines = ["", "## Model 1.0.0 on the same studies, its own H1-H4 (reported, not tested)", ""]
    for name, passed in checks["verdicts"].items():
        lines.append(f"- {name}: {'pass' if passed else 'FAIL'}")
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


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--reference", type=Path, help="frozen 1.0.0 (w0) report (H5)")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    if args.sealed and args.reference is None:
        parser.error("--sealed requires --reference")
    name = "development" if args.development else "sealed"
    report = json.loads(args.report.read_text())
    if report.get("model_version") != MODEL_VERSION:
        raise SystemExit(f"Expected a model {MODEL_VERSION} report")
    reference = None
    if args.reference is not None:
        reference = json.loads(args.reference.read_text())
        if reference.get("model_version") != REFERENCE_VERSION:
            raise SystemExit(f"Expected a model {REFERENCE_VERSION} reference report")
    classes = previous.licence_classes(name)
    unknown = [s["study_id"] for s in report["studies"] if s["study_id"] not in classes]
    if unknown:
        raise SystemExit(f"No record found for {unknown}")
    result = base.analysis(previous.tested_subset(report))
    if reference is not None:
        h5 = versus_reference(report, reference)
        result["H5_versus_model_1_0_same_training"] = h5
        result["verdicts"][H5_VERDICT] = h5_passes(h5)
        result["model_1_0_own_H1_H4_reported_not_tested"] = reference_checks(reference)
    redacted = sorted(k for k, v in classes.items() if not v)
    for block in result["per_series"].values():
        for row in block:
            row["values_public"] = classes[row["study_id"]]

    def public(source: dict[str, Any], path: Path) -> dict[str, Any]:
        return {
            **{k: v for k, v in source.items() if k != "studies"},
            "studies": [
                s if classes[s["study_id"]] else previous.redact(s) for s in source["studies"]
            ],
            "source_report_sha256": sha256(path),
        }

    args.output.mkdir(parents=True)
    (args.output / "report-public.json").write_text(
        json.dumps(public(report, args.report), indent=1) + "\n"
    )
    extra: dict[str, Any] = {}
    if reference is not None and args.reference is not None:
        (args.output / "reference-1.0.0-public.json").write_text(
            json.dumps(public(reference, args.reference), indent=1) + "\n"
        )
        extra["reference_report_sha256"] = sha256(args.reference)
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
                "tested_subset": "human series (contexts human/...)",
                "unit": previous.UNIT,
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
        .replace("IOP-after-surgery model 1.0.0", f"Retinal visual-acuity model {MODEL_VERSION}")
        .replace("## Declared checks (group means)", "## Declared checks (human group means)")
    )
    if "H5_versus_model_1_0_same_training" in result:
        text += "\n".join(h5_lines(result["H5_versus_model_1_0_same_training"])) + "\n"
        checks = result["model_1_0_own_H1_H4_reported_not_tested"]
        text += "\n".join(reference_lines(checks)) + "\n"
    text += f"\nUnits: {previous.UNIT}.\n"
    (args.output / "tables.md").write_text(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
