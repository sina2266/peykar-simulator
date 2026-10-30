"""Write licence-respecting public artifacts of a childhood-myopia axial-length model 1.0
report.

Reads private reports of ``evaluate_myopia_population.py`` (never re-runs a model). The
frozen IOP 1.0.0 publisher (``publish_iop_population.py``, unchanged because its
protocol pins it) is loaded as a library, as the acuity 1.0 publisher does: its licence
test, row redaction, paired comparisons, coverage, per-series tables and the H1-H4
analysis are reused with the myopia primaries. H2 and H3 keep its comparators
(persistence, last-two trend, pooled-curve increment and analog kNN; H3 without the
last-two trend), and H4 its comparison of the population initial state with the pooled
training curve over common control group-mean targets. The no-change comparator is
``baseline_level`` (the baseline axial length), reported only. Every reported pp is
0.01 mm of axial-length change (``eye_additive_evaluation5``). Declared changes
against the IOP analysis:

- Only human series are tested (here every series), with the summary recomputed.
- Studies whose licence does not permit republishing derived values keep only
  per-series metrics and an allow-list of initial-state fields (no baseline axial
  length, no initial-state intervals).

Output goes to a new directory.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import sys
from pathlib import Path
from types import ModuleType
from typing import Any

from peykar.validation import eye_additive_evaluation5 as additive

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/myopia-axial-2026-10-30"
PRIVATE_DEVELOPMENT = ROOT / ".peykar/myopia-development-private/records"
RECORD_DIRS = {
    "development": [DATASET / "development/records", PRIVATE_DEVELOPMENT],
    "sealed": [ROOT / ".peykar/myopia-sealed-2026-10-30/records"],
}
# Set by the development rounds before the freeze (see the 1.0 development log): m4
# was adopted in round 2 and its Gaussian variant is the primary by the declared rule.
PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
MODEL_VERSION = "1.0.0-m4"
TASKS = ("initial_state", "next_observation", "landmark")
TESTED_PREFIX = "human/"
UNIT = "1 pp = 0.01 mm of axial-length change from the baseline axial length"
INITIAL_STATE_PUBLIC_FIELDS = frozenset(
    {
        "series_id",
        "context",
        "kind",
        "months",
        "pooled_level_used",
        "population_level_used",
        "population_interval_coverage",
        "population_unavailable",
        "scores",
    }
)


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


def tested_subset(report: dict[str, Any]) -> dict[str, Any]:
    """The report restricted to human series, with its summary recomputed."""
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
    return {**report, "studies": studies, "summary": additive.summarize(studies)}


def redact(study: dict[str, Any]) -> dict[str, Any]:
    """The 1.0.0 redaction, then only the allow-listed initial-state fields."""
    out: dict[str, Any] = base.redact(study)
    for entry in out["initial_state"]:
        for key in set(entry) - INITIAL_STATE_PUBLIC_FIELDS:
            del entry[key]
    return out


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    name = "development" if args.development else "sealed"
    report = json.loads(args.report.read_text())
    if report.get("model_version") != MODEL_VERSION:
        raise SystemExit(f"Expected a model {MODEL_VERSION} report")
    classes = licence_classes(name)
    unknown = [s["study_id"] for s in report["studies"] if s["study_id"] not in classes]
    if unknown:
        raise SystemExit(f"No record found for {unknown}")
    tested = tested_subset(report)
    result = base.analysis(tested)
    redacted = sorted(k for k, v in classes.items() if not v)
    for block in result["per_series"].values():
        for row in block:
            row["values_public"] = classes[row["study_id"]]
    args.output.mkdir(parents=True)
    public = {
        **{k: v for k, v in report.items() if k != "studies"},
        "studies": [s if classes[s["study_id"]] else redact(s) for s in report["studies"]],
        "source_report_sha256": sha256(args.report),
    }
    (args.output / "report-public.json").write_text(json.dumps(public, indent=1) + "\n")
    (args.output / f"{name}-analysis.json").write_text(
        json.dumps(
            {
                "mode": name,
                "status": "declared sealed-test analysis"
                if args.sealed
                else "development reference only",
                "model_version": report.get("model_version"),
                "source_report_sha256": sha256(args.report),
                "tested_subset": "human series (contexts human/...)",
                "unit": UNIT,
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
    text += f"\nUnits: {UNIT}.\n"
    (args.output / "tables.md").write_text(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
