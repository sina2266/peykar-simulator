"""Write licence-respecting public artifacts of an IOP-after-surgery report.

Reads a private report of ``evaluate_iop_population.py`` (never re-runs a model).
Studies whose records are not CC BY or CC0 keep only per-series metrics: observed
values, predictions, intervals, preoperative pressures and signed errors are
removed. In ``--sealed`` mode the analysis declared by the frozen protocol is
computed, including mechanical verdicts for H1-H4 on group means; in
``--development`` mode the same quantities are reported as development reference
only. Output goes to a new directory.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import re
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DATASET = ROOT / "experiments/data/iop-glaucoma-surgery-2026-10-28"
RECORD_DIRS = {
    "development": [
        DATASET / "development/records",
        ROOT / ".peykar/iop-development-private/records",
    ],
    "sealed": [ROOT / ".peykar/iop-sealed-2026-10-28/records"],
}
WINDOWS = {"80": (0.70, 0.90), "90": (0.80, 0.97)}
QUANTILES = {"80": ("0.1", "0.9"), "90": ("0.05", "0.95")}
PRIMARY = {
    "next_observation": "population_assimilation_robust",
    "landmark": "population_assimilation_robust",
    "initial_state": "population_initial_state",
}
# Declared comparators of the tested hypotheses (group means, common targets).
H2_COMPARATORS = ("persistence", "last_two_log_linear", "pooled_curve_ratio", "analog_knn")
H3_COMPARATORS = ("persistence", "pooled_curve_ratio", "analog_knn")
REPORTED = (
    "persistence",
    "last_two_log_linear",
    "reference_to_last_log_linear",
    "pooled_curve_ratio",
    "analog_knn",
    "preoperative_level",
)
KINDS = ("group_mean", "single_eye")
_OPEN_URL = re.compile(r"creativecommons\.org/(licenses/by/|publicdomain/zero/)", re.I)
_OPEN_STATEMENT = re.compile(r"^\s*(CC BY\b(?!-)|CC0\b|Creative Commons Attribution\b(?!-))", re.I)
_RESTRICTED = re.compile(
    r"\bNC\b|\bND\b|\bSA\b|Non[\s-]*Commercial|No[\s-]*Deriv|Share[\s-]*Alike|CC BY-N"
    r"|CC BY-SA|\bnot CC\b|\bunclear\b",
    re.I,
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def publishable(record: dict[str, Any]) -> bool:
    """Values may be republished only from CC BY or CC0 records.

    The statement must name CC BY / CC0 at its start and carry no NC/ND/SA, "not CC"
    or "unclear" wording; a licence URL, when present, must point to a CC BY licence
    or the CC0 dedication.
    """
    licence = record.get("license", {})
    statement = str(licence.get("statement", ""))
    url = str(licence.get("url") or "")
    if _RESTRICTED.search(statement) or not _OPEN_STATEMENT.search(statement):
        return False
    return not url or bool(_OPEN_URL.search(url))


def licence_classes(mode: str) -> dict[str, bool]:
    classes = {}
    for directory in RECORD_DIRS[mode]:
        for path in directory.glob("*.json"):
            record = json.loads(path.read_text())
            private = ".peykar" in path.parts and mode == "development"
            classes[record["study_id"]] = publishable(record) and not private
    return classes


def redact(study: dict[str, Any]) -> dict[str, Any]:
    out = copy.deepcopy(study)
    out["values_redacted"] = "source licence does not permit republishing derived values"
    for entry in out["initial_state"]:
        for key in ("observed", "predictions", "reference_mmhg"):
            entry.pop(key, None)
        for score in entry["scores"].values():
            score.pop("signed_errors_pp", None)
    for task in ("next_observation", "landmark"):
        for entry in out[task]:
            entry["rows"] = [
                {
                    k: row[k]
                    for k in ("origin_month", "target_month", "history_points")
                    + (("population_level_used",) if "population_level_used" in row else ())
                }
                for row in entry["rows"]
            ]
            for score in entry["scores"].values():
                score.pop("signed_errors_pp", None)
    return out


def rmse(values: list[float]) -> float | None:
    return math.sqrt(sum(v * v for v in values) / len(values)) if values else None


def _entries(studies: list[dict[str, Any]], task: str, kind: str | None) -> list[dict[str, Any]]:
    return [e for s in studies for e in s[task] if kind is None or e["kind"] == kind]


def paired(
    studies: list[dict[str, Any]], task: str, first: str, second: str, kind: str | None
) -> dict[str, Any]:
    pairs = []
    for entry in _entries(studies, task, kind):
        for row in entry["rows"]:
            p = row["predictions"]
            if first in p and second in p:
                pairs.append((p[first] - row["observed"], p[second] - row["observed"]))
    a = rmse([100 * x for x, _ in pairs])
    b = rmse([100 * y for _, y in pairs])
    return {
        "common_targets": len(pairs),
        f"{first}_rmse_pp": a,
        f"{second}_rmse_pp": b,
        f"{first}_lower": bool(a is not None and b is not None and a < b),
    }


def coverage(
    studies: list[dict[str, Any]], task: str, method: str, kind: str | None
) -> dict[str, Any]:
    """Pooled interval coverage of ``method`` from the report rows (one subset)."""
    result: dict[str, Any] = {}
    for level, (low_q, high_q) in QUANTILES.items():
        inside = n = 0
        for entry in _entries(studies, task, kind):
            for row in entry["rows"]:
                interval = row.get("intervals", {}).get(method)
                if interval is None:
                    continue
                n += 1
                inside += int(interval[low_q] <= row["observed"] <= interval[high_q])
        low, high = WINDOWS[level]
        value = inside / n if n else None
        result[level] = {
            "inside": inside,
            "n": n,
            "coverage": value,
            "window": [low, high],
            "within_window": value is not None and low <= value <= high,
        }
    return result


def _level_used(entry: dict[str, Any]) -> str | None:
    if "population_level_used" in entry:
        value = entry["population_level_used"]
        return None if value is None else str(value)
    levels = {row.get("population_level_used") for row in entry.get("rows", [])}
    levels.discard(None)
    return str(levels.pop()) if len(levels) == 1 else None


def per_series(
    studies: list[dict[str, Any]], task: str, methods: list[str]
) -> list[dict[str, Any]]:
    rows = []
    for study in studies:
        for entry in study[task]:
            row: dict[str, Any] = {
                "study_id": study["study_id"],
                "series_id": entry["series_id"],
                "context": entry["context"],
                "role": entry.get("role", "control"),
                "kind": entry["kind"],
                "level_used": _level_used(entry),
            }
            for name in methods:
                score = entry["scores"].get(name)
                if score is not None:
                    row[name] = {
                        "rmse_pp": score["rmse_pp"],
                        "max_abs_pp": score["max_abs_pp"],
                        "meets_5_10_pp": score["meets_5_10_pp"],
                        "improves_persistence_by_0_5_pp": score.get(
                            "improves_persistence_by_0_5_pp"
                        ),
                    }
            rows.append(row)
    return rows


def _counts(block: dict[str, Any]) -> dict[str, Any]:
    keys = (
        "series",
        "series_within_5_10_pp",
        "series_meeting_criteria",
        "pooled_rmse_pp",
        "pooled_max_abs_pp",
        "targets",
    )
    return {name: {k: values[k] for k in keys} for name, values in block.items()}


def analysis(report: dict[str, Any]) -> dict[str, Any]:
    studies = report["studies"]
    summary = report["summary"]
    u2, u3, u1 = PRIMARY["next_observation"], PRIMARY["landmark"], PRIMARY["initial_state"]
    h1 = coverage(studies, "next_observation", u2, "group_mean")
    h2 = {
        name: paired(studies, "next_observation", u2, name, "group_mean") for name in H2_COMPARATORS
    }
    h3 = {name: paired(studies, "landmark", u3, name, "group_mean") for name in H3_COMPARATORS}
    initial = summary["initial_state"]["group_mean"]
    h4_model = initial.get(u1, {}).get("pooled_rmse_pp")
    h4_curve = initial.get("pooled_training_curve", {}).get("pooled_rmse_pp")
    verdicts = {
        "H1_calibration_next_observation_group_means": all(v["within_window"] for v in h1.values()),
        "H2_accuracy_next_observation_group_means": all(
            v[f"{u2}_lower"] and v["common_targets"] > 0 for v in h2.values()
        ),
        "H3_landmark_group_means": all(
            v[f"{u3}_lower"] and v["common_targets"] > 0 for v in h3.values()
        ),
        "H4_initial_state_control_group_means": bool(
            h4_model is not None and h4_curve is not None and h4_model < h4_curve
        ),
    }
    return {
        "primary_methods": PRIMARY,
        "verdicts": verdicts,
        "H1_calibration_next_observation_group_means": h1,
        "H2_accuracy_next_observation_group_means": h2,
        "H3_landmark_group_means": {
            "comparisons": h3,
            "calibration_reported": coverage(studies, "landmark", u3, "group_mean"),
        },
        "H4_initial_state_control_group_means": {
            "summary": initial,
            "calibration_reported": {
                level: {
                    **summary["coverage"][level]["initial_state_group_mean"][u1],
                    "window": list(window),
                }
                for level, window in WINDOWS.items()
            },
        },
        "reported_not_tested": {
            "single_eyes": {
                "next_observation": {
                    name: paired(studies, "next_observation", u2, name, "single_eye")
                    for name in REPORTED
                },
                "landmark": {
                    name: paired(studies, "landmark", u3, name, "single_eye") for name in REPORTED
                },
                "coverage_next_observation": coverage(
                    studies, "next_observation", u2, "single_eye"
                ),
                "coverage_landmark": coverage(studies, "landmark", u3, "single_eye"),
                "initial_state": summary["initial_state"]["single_eye"],
            },
            "all_series_coverage": {
                "next_observation": coverage(studies, "next_observation", u2, None),
                "landmark": coverage(studies, "landmark", u3, None),
            },
            "group_mean_other_comparators": {
                "next_observation": {
                    name: paired(studies, "next_observation", u2, name, "group_mean")
                    for name in REPORTED
                    if name not in H2_COMPARATORS
                },
                "landmark": {
                    name: paired(studies, "landmark", u3, name, "group_mean")
                    for name in REPORTED
                    if name not in H3_COMPARATORS
                },
            },
        },
        "pass_counts": {task: _counts(summary[task]) for task in ("next_observation", "landmark")},
        "pass_counts_group_means": {
            task: _counts(summary[f"{task}_group_means"])
            for task in ("next_observation", "landmark")
        },
        "pass_counts_controls": {
            task: _counts(summary[f"{task}_controls"]) for task in ("next_observation", "landmark")
        },
        "by_context": summary["by_context"],
        "per_series": {
            "next_observation": per_series(studies, "next_observation", [u2, *REPORTED]),
            "landmark": per_series(studies, "landmark", [u3, *REPORTED]),
            "initial_state": per_series(studies, "initial_state", [u1, "pooled_training_curve"]),
        },
    }


def _table(block: dict[str, Any]) -> list[str]:
    lines = [
        "| method | series | within 5/10 pp | criteria (incl. persistence gain)"
        " | pooled RMSE pp | max pp | targets |",
        "|---|---|---|---|---|---|---|",
    ]
    for name, v in sorted(block.items(), key=lambda kv: kv[1]["pooled_rmse_pp"]):
        lines.append(
            f"| {name} | {v['series']} | {v['series_within_5_10_pp']} | "
            f"{v['series_meeting_criteria']} | {v['pooled_rmse_pp']:.2f} | "
            f"{v['pooled_max_abs_pp']:.2f} | {v['targets']} |"
        )
    return lines


def _fmt(value: float | None, digits: int = 2) -> str:
    return "n/a" if value is None else f"{value:.{digits}f}"


def tables(result: dict[str, Any], mode: str) -> str:
    u2 = PRIMARY["next_observation"]
    lines = [f"# IOP-after-surgery model 1.0.0 — {mode} results", ""]
    lines += ["## Declared checks (group means)", ""]
    for name, passed in result["verdicts"].items():
        lines.append(f"- {name}: {'pass' if passed else 'FAIL'}")
    lines.append("")
    for level, cov in result["H1_calibration_next_observation_group_means"].items():
        lines.append(
            f"- H1 next observation {level}% interval: {cov['inside']}/{cov['n']} = "
            f"{_fmt(cov['coverage'], 3)} (window {cov['window'][0]:.2f}–{cov['window'][1]:.2f})"
        )
    for label, block in (
        ("H2 next observation", result["H2_accuracy_next_observation_group_means"]),
        ("H3 landmark", result["H3_landmark_group_means"]["comparisons"]),
    ):
        for name, v in block.items():
            lines.append(
                f"- {label} vs {name}: {_fmt(v[f'{u2}_rmse_pp'])} against "
                f"{_fmt(v[f'{name}_rmse_pp'])} pp on {v['common_targets']} common targets"
            )
    for level, cov in result["H3_landmark_group_means"]["calibration_reported"].items():
        lines.append(
            f"- landmark {level}% interval (reported): {cov['inside']}/{cov['n']} = "
            f"{_fmt(cov['coverage'], 3)}"
        )
    initial = result["H4_initial_state_control_group_means"]
    for name in (PRIMARY["initial_state"], "pooled_training_curve"):
        v = initial["summary"].get(name)
        if v is not None:
            lines.append(
                f"- H4 initial state {name}: {_fmt(v['pooled_rmse_pp'])} pp, "
                f"{v['series_within_5_10_pp']}/{v['series']} control group means within 5/10 pp"
            )
    for level, cov in initial["calibration_reported"].items():
        value = cov["inside"] / cov["n"] if cov["n"] else None
        lines.append(
            f"- H4 initial state {level}% interval (reported): {cov['inside']}/{cov['n']} = "
            f"{_fmt(value, 3)}"
        )
    lines.append("")
    for title, key in (("group means", "pass_counts_group_means"), ("all series", "pass_counts")):
        for task in ("next_observation", "landmark"):
            lines += [f"## {task} — {title}", "", *_table(result[key][task]), ""]
    lines += ["## initial state (control group means)", ""]
    lines += _table_initial(initial["summary"])
    lines += ["", "## single eyes (reported, not tested)", ""]
    singles = result["reported_not_tested"]["single_eyes"]
    for task in ("next_observation", "landmark"):
        for name, v in singles[task].items():
            lines.append(
                f"- {task} vs {name}: {_fmt(v[f'{u2}_rmse_pp'])} against "
                f"{_fmt(v[f'{name}_rmse_pp'])} pp on {v['common_targets']} targets"
            )
    for task in ("next_observation", "landmark"):
        for level, cov in singles[f"coverage_{task}"].items():
            value = _fmt(cov["coverage"], 3)
            lines.append(f"- {task} {level}% interval: {cov['inside']}/{cov['n']} = {value}")
    lines += ["", "## by procedure context (primary methods; comparator in parentheses)", ""]
    for context, block in result["by_context"].items():
        parts = []
        for task, method, comparator in (
            ("next_observation", u2, "persistence"),
            ("landmark", PRIMARY["landmark"], "persistence"),
            ("initial_state_group_mean", PRIMARY["initial_state"], "pooled_training_curve"),
        ):
            v = block.get(task, {}).get(method)
            if v is None:
                continue
            p = block.get(task, {}).get(comparator)
            other = f" ({p['pooled_rmse_pp']:.2f})" if p else ""
            parts.append(
                f"{task} {v['series_within_5_10_pp']}/{v['series']} within, "
                f"{v['pooled_rmse_pp']:.2f}{other} pp"
            )
        if parts:
            lines.append(f"- {context}: " + "; ".join(parts))
    return "\n".join(lines) + "\n"


def _table_initial(block: dict[str, Any]) -> list[str]:
    lines = [
        "| method | groups | within 5/10 pp | pooled RMSE pp | max pp | targets |",
        "|---|---|---|---|---|---|",
    ]
    for name, v in sorted(block.items(), key=lambda kv: kv[1]["pooled_rmse_pp"]):
        lines.append(
            f"| {name} | {v['series']} | {v['series_within_5_10_pp']} | "
            f"{v['pooled_rmse_pp']:.2f} | {v['pooled_max_abs_pp']:.2f} | {v['targets']} |"
        )
    return lines


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
    classes = licence_classes(name)
    unknown = [s["study_id"] for s in report["studies"] if s["study_id"] not in classes]
    if unknown:
        raise SystemExit(f"No record found for {unknown}")
    result = analysis(report)
    public = {
        **{k: v for k, v in report.items() if k != "studies"},
        "studies": [s if classes[s["study_id"]] else redact(s) for s in report["studies"]],
        "source_report_sha256": sha256(args.report),
    }
    redacted = sorted(k for k, v in classes.items() if not v)
    for block in result["per_series"].values():
        for row in block:
            row["values_public"] = classes[row["study_id"]]
    args.output.mkdir(parents=True)
    (args.output / "report-public.json").write_text(json.dumps(public, indent=1) + "\n")
    (args.output / f"{name}-analysis.json").write_text(
        json.dumps(
            {
                "mode": name,
                "status": "declared sealed-test analysis"
                if args.sealed
                else "development reference only",
                "source_report_sha256": sha256(args.report),
                "studies_with_redacted_values": [
                    s for s in redacted if any(r["study_id"] == s for r in report["studies"])
                ],
                **result,
            },
            indent=1,
        )
        + "\n"
    )
    (args.output / "tables.md").write_text(tables(result, name))
    print((args.output / "tables.md").read_text())
    return 0


if __name__ == "__main__":
    sys.exit(main())
