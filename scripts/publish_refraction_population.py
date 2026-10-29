"""Write licence-respecting public artifacts of a refraction model 1.0 report.

Reads private reports of ``evaluate_refraction_population.py`` (never re-runs a model).
The frozen IOP 1.0.0 publisher (``publish_iop_population.py``, unchanged because its
protocol pins it) is loaded as a library: its licence test, row redaction, paired
comparisons, coverage, per-series tables and the H1-H3 analysis are reused with the
refraction primaries. The baseline comparator is ``baseline_level`` (no change from
the preoperative SE) and the plano target ``target_level`` (full correction) is an
H2/H3 comparator. Every reported pp is 0.1 D of spherical equivalent
(``eye_additive_evaluation``). Declared changes against the IOP analysis:

- H4 compares the population initial state with the plano target over common
  control group-mean targets; the pooled training curve, which ignores the attempted
  correction, is reported only.
- Only human series are tested (the summary is recomputed without the others), and
  studies whose licence does not permit republishing derived values keep only
  per-series metrics and an allow-list of initial-state fields.

With ``--reference`` (candidate ``f0`` scored on the same studies) H5 compares model
1.0 (``f1``) with f0 trained on the same records, over common human group-mean
targets: the initial-state RMSE of f1 lower, and its next-observation and landmark
RMSE each at most 0.05 pp (0.005 D) higher. Output goes to a new directory.
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

from peykar.validation import eye_additive_evaluation as additive

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/cornea-refraction-2026-10-29"
PRIVATE_DEVELOPMENT = ROOT / ".peykar/refraction-development-private/records"
RECORD_DIRS = {
    "development": [DATASET / "development/records", PRIVATE_DEVELOPMENT],
    "sealed": [ROOT / ".peykar/refraction-sealed-2026-10-29/records"],
}
# Set by the development round before the freeze (see the 1.0 development log).
PRIMARY = {
    "next_observation": "population_assimilation_robust",
    "landmark": "population_assimilation_robust",
    "initial_state": "population_initial_state",
}
REFERENCE_PRIMARY = dict(PRIMARY)
MODEL_VERSION = "1.0.0-f1"
REFERENCE_VERSION = "1.0.0-f0"
TASKS = ("initial_state", "next_observation", "landmark")
TESTED_PREFIX = "human/"
TARGET = "target_level"
H5_RMSE_TOLERANCE_PP = 0.05
H4_VERDICT = "H4_initial_state_below_plano_target_control_group_means"
H5_VERDICT = "H5_initial_state_lower_than_f0_forecasts_within_0_05_pp"
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
    module.__dict__["REPORTED"] = (
        *("baseline_level" if name == "preoperative_level" else name for name in module.REPORTED),
        TARGET,
    )
    module.__dict__["H2_COMPARATORS"] = (*module.H2_COMPARATORS, TARGET)
    module.__dict__["H3_COMPARATORS"] = (*module.H3_COMPARATORS, TARGET)
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


def _split(report: dict[str, Any], *, tested: bool) -> list[dict[str, Any]]:
    return [
        {
            **study,
            **{
                task: [e for e in study[task] if e["context"].startswith(TESTED_PREFIX) == tested]
                for task in TASKS
            },
        }
        for study in report["studies"]
    ]


def tested_subset(report: dict[str, Any]) -> dict[str, Any]:
    """The report restricted to human series, with its summary recomputed."""
    studies = _split(report, tested=True)
    return {**report, "studies": studies, "summary": additive.summarize(studies)}


def redact(study: dict[str, Any]) -> dict[str, Any]:
    """The 1.0.0 redaction, then only the allow-listed initial-state fields."""
    out: dict[str, Any] = base.redact(study)
    for entry in out["initial_state"]:
        for key in set(entry) - INITIAL_STATE_PUBLIC_FIELDS:
            del entry[key]
    return out


def _rmse(errors: list[float]) -> float | None:
    return math.sqrt(sum((100 * e) ** 2 for e in errors) / len(errors)) if errors else None


def _initial_errors(report: dict[str, Any], method: str) -> dict[tuple[Any, ...], float]:
    out = {}
    for study in report["studies"]:
        for entry in study["initial_state"]:
            if entry["kind"] != "group_mean" or not entry["context"].startswith(TESTED_PREFIX):
                continue
            values = entry.get("predictions", {}).get(method)
            if values is None:
                continue
            for i, (p, o) in enumerate(zip(values, entry["observed"], strict=True)):
                out[(study["study_id"], entry["series_id"], i)] = p - o
    return out


def _row_errors(report: dict[str, Any], task: str, method: str) -> dict[tuple[Any, ...], float]:
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
                    out[key] = row["predictions"][method] - row["observed"]
    return out


def _paired(first: dict[Any, float], second: dict[Any, float]) -> dict[str, Any]:
    common = sorted(set(first) & set(second))
    a = _rmse([first[k] for k in common])
    b = _rmse([second[k] for k in common])
    return {"common_targets": len(common), "first_rmse_pp": a, "second_rmse_pp": b}


def initial_versus_target(report: dict[str, Any]) -> dict[str, Any]:
    """H4: population initial state against the plano target (control group means)."""
    v = _paired(_initial_errors(report, PRIMARY["initial_state"]), _initial_errors(report, TARGET))
    return {
        "common_targets": v["common_targets"],
        "population_initial_state_rmse_pp": v["first_rmse_pp"],
        "target_level_rmse_pp": v["second_rmse_pp"],
        "population_lower": bool(
            v["common_targets"] > 0 and v["first_rmse_pp"] < v["second_rmse_pp"]
        ),
    }


def versus_reference(report: dict[str, Any], reference: dict[str, Any]) -> dict[str, Any]:
    """H5: model 1.0 (f1) against f0 trained on the same records (human group means)."""
    result: dict[str, Any] = {}
    for task in ("next_observation", "landmark"):
        v = _paired(
            _row_errors(report, task, PRIMARY[task]),
            _row_errors(reference, task, REFERENCE_PRIMARY[task]),
        )
        result[task] = {
            "common_targets": v["common_targets"],
            "f1_rmse_pp": v["first_rmse_pp"],
            "f0_rmse_pp": v["second_rmse_pp"],
        }
    v = _paired(
        _initial_errors(report, PRIMARY["initial_state"]),
        _initial_errors(reference, REFERENCE_PRIMARY["initial_state"]),
    )
    result["initial_state"] = {
        "common_targets": v["common_targets"],
        "f1_rmse_pp": v["first_rmse_pp"],
        "f0_rmse_pp": v["second_rmse_pp"],
    }
    return result


def h5_passes(h5: dict[str, Any]) -> bool:
    if not all(h5[task]["common_targets"] > 0 for task in TASKS):
        return False
    initial = h5["initial_state"]
    return bool(
        initial["f1_rmse_pp"] < initial["f0_rmse_pp"]
        and all(
            h5[task]["f1_rmse_pp"] <= h5[task]["f0_rmse_pp"] + H5_RMSE_TOLERANCE_PP
            for task in ("next_observation", "landmark")
        )
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--reference", type=Path, help="candidate f0 report (H5)")
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
            raise SystemExit(f"Expected a {REFERENCE_VERSION} reference report")
    classes = licence_classes(name)
    unknown = [s["study_id"] for s in report["studies"] if s["study_id"] not in classes]
    if unknown:
        raise SystemExit(f"No record found for {unknown}")
    tested = tested_subset(report)
    result = base.analysis(tested)
    h4 = initial_versus_target(tested)
    result["H4_versus_plano_target"] = h4
    result["verdicts"].pop("H4_initial_state_control_group_means")
    result["verdicts"][H4_VERDICT] = h4["population_lower"]
    extra: dict[str, Any] = {
        "tested_subset": "human series (contexts human/...)",
        "unit": "1 pp = 0.1 D of spherical equivalent change from the preoperative SE",
        "H4_pooled_training_curve_reported_only": True,
    }
    if reference is not None:
        h5 = versus_reference(report, reference)
        result["H5_versus_f0_same_training"] = h5
        result["verdicts"][H5_VERDICT] = h5_passes(h5)
        extra["reference_report_sha256"] = sha256(args.reference)
    redacted = sorted(k for k, v in classes.items() if not v)
    for block in result["per_series"].values():
        for row in block:
            row["values_public"] = classes[row["study_id"]]

    def public(source: dict[str, Any], path: Path) -> dict[str, Any]:
        return {
            **{k: v for k, v in source.items() if k != "studies"},
            "studies": [s if classes[s["study_id"]] else redact(s) for s in source["studies"]],
            "source_report_sha256": sha256(path),
        }

    args.output.mkdir(parents=True)
    (args.output / "report-public.json").write_text(
        json.dumps(public(report, args.report), indent=1) + "\n"
    )
    if reference is not None:
        (args.output / "reference-f0-public.json").write_text(
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
    text = (
        base.tables(result, name)
        .replace("IOP-after-surgery model 1.0.0", f"Refraction model {MODEL_VERSION}")
        .replace("## Declared checks (group means)", "## Declared checks (human group means)")
        .replace("- H4 initial state", "- initial state (reported)")
    )
    lines = [
        "",
        "Units: 1 pp = 0.1 D of spherical equivalent change from the preoperative SE.",
        "",
        "## H4: population initial state against the plano target (control group means)",
        "",
        f"- {base._fmt(h4['population_initial_state_rmse_pp'])} against "
        f"{base._fmt(h4['target_level_rmse_pp'])} pp on {h4['common_targets']} common targets",
    ]
    if "H5_versus_f0_same_training" in result:
        lines += ["", "## H5: model 1.0 (f1) against f0, same training", ""]
        for task in ("initial_state", "next_observation", "landmark"):
            v = result["H5_versus_f0_same_training"][task]
            lines.append(
                f"- {task}: f1 {base._fmt(v['f1_rmse_pp'])} against f0 "
                f"{base._fmt(v['f0_rmse_pp'])} pp on {v['common_targets']} common targets"
            )
    text += "\n".join(lines) + "\n"
    (args.output / "tables.md").write_text(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
