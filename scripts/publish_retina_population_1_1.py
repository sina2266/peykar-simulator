"""Write licence-respecting public artifacts of a retinal-thickness model 1.1 report.

Reads private reports of ``evaluate_retina_population_1_1.py`` (never re-runs a model).
The frozen model 1.0.0 publisher (``publish_retina_population.py``, unchanged because
its protocol pins it) is loaded as a library: its licence classes, redaction, H1-H4
analysis (from the IOP 1.0.0 publisher) and H5 comparison are reused, pointed at the
1.1 record sets (training: tranche-1 development and consumed sealed records; sealed:
tranche 2) and at the 1.1 primary and reference methods. Records are classed by
licence: CC BY or CC0 values may be republished, everything else keeps only
per-series metrics; privately kept development records stay private. With
``--reference`` (a report of the frozen model 1.0.0 on the same studies), H5 compares
the primary methods over common group-mean targets of the tested tasks. Output goes
to a new directory.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import sys
from pathlib import Path
from types import ModuleType
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/retina-cst-2026-10-28"
PRIVATE_DEVELOPMENT = ROOT / ".peykar/retina-development-private/records"
RECORD_DIRS = {
    "development": [
        DATASET / "development/records",
        PRIVATE_DEVELOPMENT,
        ROOT / ".peykar/retina-sealed-2026-10-28/records",
    ],
    "sealed": [ROOT / ".peykar/retina-tranche2-sealed-2026-10-28/records"],
}
# Set by the development rounds before the freeze (see the 1.1 development log).
PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
# The reference is the frozen model 1.0.0 (r3), whose primary was the Gaussian variant.
REFERENCE_PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
# H5 tests what the adopted changes target.
H5_TASKS: tuple[str, ...] = ("next_observation", "landmark", "initial_state")


def _publisher_1_0() -> ModuleType:
    path = ROOT / "scripts/publish_retina_population.py"
    spec = importlib.util.spec_from_file_location("publish_retina_population_1_0_0", path)
    if spec is None or spec.loader is None:
        raise SystemExit("cannot load the retinal-thickness 1.0.0 publisher")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    # The 1.0.0 functions read module-level constants; point them at version 1.1.
    module.__dict__["RECORD_DIRS"] = RECORD_DIRS
    module.__dict__["PRIVATE_DEVELOPMENT"] = PRIVATE_DEVELOPMENT
    module.__dict__["PRIMARY"] = dict(PRIMARY)
    module.__dict__["REFERENCE_PRIMARY"] = dict(REFERENCE_PRIMARY)
    module.__dict__["H5_TASKS"] = H5_TASKS
    module.base.__dict__["PRIMARY"] = dict(PRIMARY)
    return module


previous = _publisher_1_0()


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--reference", type=Path, help="frozen model 1.0.0 report (H5)")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    name = "development" if args.development else "sealed"
    report = json.loads(args.report.read_text())
    classes = previous.licence_classes(name)
    unknown = [s["study_id"] for s in report["studies"] if s["study_id"] not in classes]
    if unknown:
        raise SystemExit(f"No record found for {unknown}")
    result = previous.base.analysis(report)
    extra: dict[str, Any] = {}
    reference = None
    if args.reference is not None:
        reference = json.loads(args.reference.read_text())
        h5 = previous.versus_reference(report, reference)
        result["H5_versus_reference_same_training"] = h5
        result["verdicts"]["H5_primary_better_than_model_1_0_0"] = all(
            h5[task]["primary_lower"] and h5[task]["common_targets"] > 0 for task in H5_TASKS
        )
        extra["reference_report_sha256"] = previous.sha256(args.reference)
        extra["reference_model_version"] = reference.get("model_version")
        extra["H5_tested_tasks"] = list(H5_TASKS)
    redacted = sorted(k for k, v in classes.items() if not v)
    for block in result["per_series"].values():
        for row in block:
            row["values_public"] = classes[row["study_id"]]
    args.output.mkdir(parents=True)
    (args.output / "report-public.json").write_text(
        json.dumps(previous._public(report, classes, args.report), indent=1) + "\n"
    )
    if reference is not None:
        (args.output / "reference-public.json").write_text(
            json.dumps(previous._public(reference, classes, args.reference), indent=1) + "\n"
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
    title = f"Retinal-thickness model {report.get('model_version')}"
    text = (
        previous.base.tables(result, name)
        .replace("IOP-after-surgery model 1.0.0", title)
        .replace("by procedure context", "by context")
    )
    if "H5_versus_reference_same_training" in result:
        lines = ["", "## H5: primary against the frozen model 1.0.0, same training", ""]
        for task, v in result["H5_versus_reference_same_training"].items():
            tested = "tested" if task in H5_TASKS else "reported"
            lines.append(
                f"- {task} ({tested}): primary {previous.base._fmt(v['primary_rmse_pp'])} "
                f"against 1.0.0 {previous.base._fmt(v['reference_rmse_pp'])} pp on "
                f"{v['common_targets']} common targets"
            )
        text += "\n".join(lines) + "\n"
    (args.output / "tables.md").write_text(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
