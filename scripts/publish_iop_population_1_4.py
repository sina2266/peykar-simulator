"""Write licence-respecting public artifacts of an IOP model 1.4 report.

Reads private reports of ``evaluate_iop_population_1_4.py`` (never re-runs a model).
The frozen 1.2 publisher (``publish_iop_population_1_2.py``, unchanged because its
protocol pins it) is loaded as a library, and through it the 1.1 and 1.0.0
publishers: their licence classes, redaction, comparisons and the H1-H4 analysis are
reused, pointed at the 1.4 record sets (training: tranches 1-4; sealed: tranche 5)
and at the 1.4 primary methods. As in the 1.3 publisher (unchanged because its protocol
pins it), two changes are declared:

- The tested analysis covers human series only. Non-human series are removed before
  H1-H5 and the summary is recomputed; they are listed per series as reported, not
  tested. (Earlier publishers relied on non-human sealed series having no population
  prior.)
- Studies whose licence does not permit republishing derived values keep only an
  allow-list of initial-state fields. The earlier redaction removed named fields and
  left the preoperative ``reference_value`` that the version-2 harness writes.

With ``--reference`` (the frozen model 1.2.0, candidate ``d0``, scored on the same
studies) H5 compares model 1.4 with 1.2.0 trained on the same records, over common
human group-mean targets, as declared in the 1.4 development log: the pooled
initial-state RMSE of 1.4 lower, and its pooled next-observation and landmark RMSE each
at most 0.05 pp higher. Output goes to a new directory.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import sys
from pathlib import Path
from types import ModuleType
from typing import Any

from peykar.validation import eye_recovery_evaluation2 as evaluation

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/iop-glaucoma-surgery-2026-10-28"
PRIVATE_DEVELOPMENT = ROOT / ".peykar/iop-development-private/records"
RECORD_DIRS = {
    "development": [
        DATASET / "development/records",
        PRIVATE_DEVELOPMENT,
        ROOT / ".peykar/iop-sealed-2026-10-28/records",
        ROOT / ".peykar/iop-tranche2-sealed-2026-10-28/records",
        ROOT / ".peykar/iop-tranche3-sealed-2026-10-28/records",
        ROOT / ".peykar/iop-tranche4-sealed-2026-10-29/records",
    ],
    "sealed": [ROOT / ".peykar/iop-tranche5-sealed-2026-10-30/records"],
}
# Set by the development round before the freeze (see the 1.4 development log).
PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
# The frozen model 1.2.0's primaries.
REFERENCE_PRIMARY = {
    "next_observation": "population_assimilation",
    "landmark": "population_assimilation",
    "initial_state": "population_initial_state",
}
MODEL_VERSION = "1.4.0-d1s"
REFERENCE_VERSION = "1.2.0-c1"
TASKS = ("initial_state", "next_observation", "landmark")
TESTED_PREFIX = "human/"
# H5 (declared in the 1.4 development log): forecast RMSEs at most this much higher.
H5_RMSE_TOLERANCE_PP = 0.05
H5_VERDICT = "H5_initial_state_lower_next_observation_and_landmark_within_0_05_pp"
# Initial-state fields a redacted study keeps (everything else is removed).
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


def _previous() -> ModuleType:
    path = ROOT / "scripts/publish_iop_population_1_2.py"
    spec = importlib.util.spec_from_file_location("publish_iop_population_1_2_0", path)
    if spec is None or spec.loader is None:
        raise SystemExit("cannot load the 1.2.0 publisher")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    # The 1.2 publisher and the 1.1 licence classes read these module-level names.
    for target in (module, module.previous):
        target.__dict__["RECORD_DIRS"] = RECORD_DIRS
        target.__dict__["PRIVATE_DEVELOPMENT"] = PRIVATE_DEVELOPMENT
    module.base.__dict__["PRIMARY"] = dict(PRIMARY)
    return module


previous = _previous()
base = previous.base


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
    return {**report, "studies": studies, "summary": evaluation.summarize(studies)}


def untested_series(report: dict[str, Any]) -> dict[str, Any]:
    """Per-series metrics of the non-human series (reported, not tested)."""
    studies = _split(report, tested=False)
    return {
        "next_observation": base.per_series(
            studies, "next_observation", [PRIMARY["next_observation"], *base.REPORTED]
        ),
        "landmark": base.per_series(studies, "landmark", [PRIMARY["landmark"], *base.REPORTED]),
        "initial_state": base.per_series(
            studies, "initial_state", [PRIMARY["initial_state"], "pooled_training_curve"]
        ),
    }


def redact(study: dict[str, Any]) -> dict[str, Any]:
    """The 1.0.0 redaction, then only the allow-listed initial-state fields."""
    out: dict[str, Any] = base.redact(study)
    for entry in out["initial_state"]:
        for key in set(entry) - INITIAL_STATE_PUBLIC_FIELDS:
            del entry[key]
    return out


def _comparison(targets: int, new: float | None, old: float | None) -> dict[str, Any]:
    return {
        "common_targets": targets,
        "model_1_4_rmse_pp": new,
        "model_1_2_rmse_pp": old,
        "model_1_4_lower": bool(new is not None and old is not None and new < old),
    }


def _first_month_coverage(report: dict[str, Any], reference: dict[str, Any]) -> dict[str, Any]:
    method, ref_method = PRIMARY["next_observation"], REFERENCE_PRIMARY["next_observation"]
    new = previous._rows(report, "next_observation", method)
    old = previous._rows(reference, "next_observation", ref_method)
    early = sorted(k for k in set(new) & set(old) if k[3] <= previous.FIRST_MONTH + 1e-9)
    out: dict[str, Any] = {"targets": len(early)}
    for level, (low, high, nominal) in previous.COVERAGE_LEVELS.items():
        values: dict[str, float | None] = {}
        for label, rows, name in (("model_1_4", new, method), ("model_1_2", old, ref_method)):
            inside = [
                rows[k]["intervals"][name][low]
                <= rows[k]["observed"]
                <= rows[k]["intervals"][name][high]
                for k in early
                if name in rows[k]["intervals"]
            ]
            values[label] = sum(inside) / len(inside) if inside else None
        out[level] = {"nominal": nominal, **values}
    return out


def versus_reference(report: dict[str, Any], reference: dict[str, Any]) -> dict[str, Any]:
    """Model 1.4 against the frozen 1.2.0 trained on the same records (human group means)."""
    result: dict[str, Any] = {}
    for task in ("next_observation", "landmark"):
        new = previous._rows(report, task, PRIMARY[task])
        old = previous._rows(reference, task, REFERENCE_PRIMARY[task])
        common = sorted(set(new) & set(old))
        result[task] = _comparison(
            len(common),
            previous._rmse(
                [new[k]["predictions"][PRIMARY[task]] - new[k]["observed"] for k in common]
            ),
            previous._rmse(
                [
                    old[k]["predictions"][REFERENCE_PRIMARY[task]] - old[k]["observed"]
                    for k in common
                ]
            ),
        )
    first = previous._initial(report, PRIMARY["initial_state"])
    second = previous._initial(reference, REFERENCE_PRIMARY["initial_state"])
    common_initial = sorted(set(first) & set(second))
    result["initial_state"] = _comparison(
        len(common_initial),
        previous._rmse([first[k] for k in common_initial]),
        previous._rmse([second[k] for k in common_initial]),
    )
    result["first_month_next_observation_coverage_reported"] = _first_month_coverage(
        report, reference
    )
    return result


def h5_passes(h5: dict[str, Any]) -> bool:
    return bool(
        all(h5[task]["common_targets"] > 0 for task in TASKS)
        and h5["initial_state"]["model_1_4_lower"]
        and all(
            h5[task]["model_1_4_rmse_pp"] is not None
            and h5[task]["model_1_2_rmse_pp"] is not None
            and h5[task]["model_1_4_rmse_pp"]
            <= h5[task]["model_1_2_rmse_pp"] + H5_RMSE_TOLERANCE_PP
            for task in ("next_observation", "landmark")
        )
    )


def h5_lines(h5: dict[str, Any]) -> list[str]:
    lines = ["", "## H5: model 1.4 against the frozen model 1.2.0, same training", ""]
    for task in ("next_observation", "landmark", "initial_state"):
        v = h5[task]
        lines.append(
            f"- {task}: 1.4 {base._fmt(v['model_1_4_rmse_pp'])} against 1.2 "
            f"{base._fmt(v['model_1_2_rmse_pp'])} pp on {v['common_targets']} common targets"
        )
    cov = h5["first_month_next_observation_coverage_reported"]
    for level in previous.COVERAGE_LEVELS:
        v = cov[level]
        lines.append(
            f"- first-month next-observation {level}% coverage, reported ({cov['targets']} "
            f"targets): 1.4 {base._fmt(v['model_1_4'], 3)} against 1.2 "
            f"{base._fmt(v['model_1_2'], 3)}"
        )
    return lines


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--reference", type=Path, help="frozen 1.2.0 (d0) report (H5)")
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
    classes = previous.previous.licence_classes(name)
    unknown = [s["study_id"] for s in report["studies"] if s["study_id"] not in classes]
    if unknown:
        raise SystemExit(f"No record found for {unknown}")
    result = base.analysis(tested_subset(report))
    result["reported_not_tested"]["non_human_series"] = untested_series(report)
    extra: dict[str, Any] = {"tested_subset": "human series (contexts human/...)"}
    if reference is not None:
        h5 = versus_reference(report, reference)
        result["H5_versus_model_1_2_same_training"] = h5
        result["verdicts"][H5_VERDICT] = h5_passes(h5)
        extra["reference_report_sha256"] = previous.previous.sha256(args.reference)
    redacted = sorted(k for k, v in classes.items() if not v)
    for block in result["per_series"].values():
        for row in block:
            row["values_public"] = classes[row["study_id"]]
    for block in result["reported_not_tested"]["non_human_series"].values():
        for row in block:
            row["values_public"] = classes[row["study_id"]]

    def public(source: dict[str, Any], path: Path) -> dict[str, Any]:
        return {
            **{k: v for k, v in source.items() if k != "studies"},
            "studies": [s if classes[s["study_id"]] else redact(s) for s in source["studies"]],
            "source_report_sha256": previous.previous.sha256(path),
        }

    args.output.mkdir(parents=True)
    (args.output / "report-public.json").write_text(
        json.dumps(public(report, args.report), indent=1) + "\n"
    )
    if reference is not None:
        (args.output / "reference-1.2.0-public.json").write_text(
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
                "source_report_sha256": previous.previous.sha256(args.report),
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
    text = base.tables(result, name).replace("model 1.0.0", f"model {MODEL_VERSION}")
    text = text.replace(
        "## Declared checks (group means)", "## Declared checks (human group means)"
    )
    non_human = result["reported_not_tested"]["non_human_series"]
    text += (
        f"\nNon-human series removed before the tested analysis (reported per series): "
        f"{len(non_human['next_observation'])} forecast series, "
        f"{len(non_human['initial_state'])} control initial-state series.\n"
    )
    if "H5_versus_model_1_2_same_training" in result:
        text += "\n".join(h5_lines(result["H5_versus_model_1_2_same_training"])) + "\n"
    (args.output / "tables.md").write_text(text)
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
