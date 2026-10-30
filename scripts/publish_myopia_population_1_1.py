"""Write licence-respecting public artifacts of a childhood-myopia axial-length model 1.1
report.

Reads private reports of ``evaluate_myopia_population_1_1.py`` (never re-runs a model).
The frozen 1.0 publisher (``publish_myopia_population.py``, unchanged because its
protocol pins it) is loaded as a library, and through it the frozen IOP 1.0.0
publisher: the licence test, the redaction with its initial-state allow-list, the tested
subset (human series), the H1-H4 analysis and the tables are reused, pointed at the 1.1
record sets (training: the 33 tranche-1 records; sealed: tranche 2) and at the 1.1
primaries. Every reported pp is 0.01 mm of axial-length change from the baseline axial
length.

With ``--reference`` (the frozen model 1.0.0, candidate ``m4``, scored on the same
studies in the same sealed run; for a development analysis also ``n0``, which is m4
with a new version string) H5 compares the primaries over common human group-mean
targets, as declared in the 1.1 development log: the pooled next-observation and
landmark RMSE of model 1.1 each at most 0.05 pp above model 1.0.0's. Output goes to a
new directory.
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
DATASET = ROOT / "experiments/data/myopia-axial-2026-10-30"
PRIVATE_DEVELOPMENT = ROOT / ".peykar/myopia-development-private/records"
RECORD_DIRS = {
    "development": [
        DATASET / "development/records",
        PRIVATE_DEVELOPMENT,
        # Tranche-1 sealed records, consumed by the one-shot test of model 1.0.0.
        ROOT / ".peykar/myopia-sealed-2026-10-30/records",
    ],
    "sealed": [ROOT / ".peykar/myopia-tranche2-sealed-2026-10-30/records"],
}
# Set by the development rounds before the freeze (see the 1.1 development log): n3 was
# adopted in round 1 and its Gaussian variant is the primary by the declared rule.
PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
# The frozen model 1.0.0's primaries.
REFERENCE_PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
MODEL_VERSION = "1.1.0-n3"
REFERENCE_VERSIONS = {"sealed": {"1.0.0-m4"}, "development": {"1.0.0-m4", "1.1.0-n0"}}
FORECASTS = ("next_observation", "landmark")
# H5 (declared in the 1.1 development log): forecast RMSEs at most this much higher.
H5_RMSE_TOLERANCE_PP = 0.05
H5_VERDICT = "H5_next_observation_and_landmark_within_0_05_pp_of_model_1_0"


def _previous() -> ModuleType:
    path = ROOT / "scripts/publish_myopia_population.py"
    spec = importlib.util.spec_from_file_location("publish_myopia_population_1_0_0", path)
    if spec is None or spec.loader is None:
        raise SystemExit("cannot load the myopia 1.0 publisher")
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


def _rows(report: dict[str, Any], task: str, method: str) -> dict[tuple[Any, ...], Any]:
    """Human group-mean forecast rows with a prediction of ``method``, by target."""
    out = {}
    for study in report["studies"]:
        for entry in study[task]:
            if entry["kind"] != "group_mean" or not entry["context"].startswith("human/"):
                continue
            for row in entry["rows"]:
                if method in row["predictions"]:
                    key = (study["study_id"], entry["series_id"], row["origin_month"])
                    out[(*key, row["target_month"])] = row
    return out


def _rmse(errors: list[float]) -> float | None:
    """Pooled RMSE in pp (reported values are 1 + change / 1 mm, so 100 x error)."""
    return math.sqrt(sum((100 * e) ** 2 for e in errors) / len(errors)) if errors else None


def versus_reference(report: dict[str, Any], reference: dict[str, Any]) -> dict[str, Any]:
    """Model 1.1 against the frozen 1.0.0 trained on the same records (human group means)."""
    result: dict[str, Any] = {}
    for task in FORECASTS:
        new = _rows(report, task, PRIMARY[task])
        old = _rows(reference, task, REFERENCE_PRIMARY[task])
        common = sorted(set(new) & set(old))
        a = _rmse([new[k]["predictions"][PRIMARY[task]] - new[k]["observed"] for k in common])
        b = _rmse(
            [old[k]["predictions"][REFERENCE_PRIMARY[task]] - old[k]["observed"] for k in common]
        )
        result[task] = {
            "common_targets": len(common),
            "model_1_1_rmse_pp": a,
            "model_1_0_rmse_pp": b,
            "within_tolerance": bool(
                common and a is not None and b is not None and a <= b + H5_RMSE_TOLERANCE_PP
            ),
        }
    return result


def h5_lines(h5: dict[str, Any]) -> list[str]:
    lines = ["", "## H5: model 1.1 against the frozen model 1.0.0, same training", ""]
    for task in FORECASTS:
        v = h5[task]
        lines.append(
            f"- {task}: 1.1 {base._fmt(v['model_1_1_rmse_pp'])} against 1.0 "
            f"{base._fmt(v['model_1_0_rmse_pp'])} pp on {v['common_targets']} common targets "
            f"({'within' if v['within_tolerance'] else 'NOT within'} "
            f"{H5_RMSE_TOLERANCE_PP} pp)"
        )
    return lines


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--reference", type=Path, help="frozen 1.0.0 (m4) report (H5)")
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
        if reference.get("model_version") not in REFERENCE_VERSIONS[name]:
            raise SystemExit(f"Expected a reference report of {sorted(REFERENCE_VERSIONS[name])}")
        if [s["study_id"] for s in reference["studies"]] != [
            s["study_id"] for s in report["studies"]
        ]:
            raise SystemExit("The reference report scores different studies")
    classes = previous.licence_classes(name)
    unknown = [s["study_id"] for s in report["studies"] if s["study_id"] not in classes]
    if unknown:
        raise SystemExit(f"No record found for {unknown}")
    result = base.analysis(previous.tested_subset(report))
    extra: dict[str, Any] = {"tested_subset": "human series (contexts human/...)"}
    if reference is not None:
        h5 = versus_reference(report, reference)
        result["H5_versus_model_1_0_same_training"] = h5
        result["verdicts"][H5_VERDICT] = all(h5[t]["within_tolerance"] for t in FORECASTS)
        extra["reference_model_version"] = reference.get("model_version")
        extra["reference_report_sha256"] = previous.sha256(args.reference)
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
            "source_report_sha256": previous.sha256(path),
        }

    args.output.mkdir(parents=True)
    (args.output / "report-public.json").write_text(
        json.dumps(public(report, args.report), indent=1) + "\n"
    )
    if reference is not None and args.sealed:
        # A development reference (n0) is a private development report; only the
        # sealed run's 1.0.0 reference is published, with the same redaction.
        (args.output / "reference-1.0.0-public.json").write_text(
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
        .replace("IOP-after-surgery model 1.0.0", f"Myopia axial-length model {MODEL_VERSION}")
        .replace("## Declared checks (group means)", "## Declared checks (human group means)")
    )
    if "H5_versus_model_1_0_same_training" in result:
        text += "\n".join(h5_lines(result["H5_versus_model_1_0_same_training"])) + "\n"
    text += f"\nUnits: {previous.UNIT}.\n"
    (args.output / "tables.md").write_text(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
