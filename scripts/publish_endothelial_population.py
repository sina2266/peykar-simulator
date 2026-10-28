"""Write licence-respecting public artifacts of an endothelial density report.

Reads a private report of ``evaluate_endothelial_population.py`` (never re-runs a
model). Studies whose records are not CC BY or CC0 keep only per-series metrics:
observed values, predictions, intervals and signed errors are removed. In
``--sealed`` mode the analysis declared by the frozen protocol is computed; in
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
DATASET = ROOT / "experiments/data/cornea-endothelium-2026-10-28"
RECORD_DIRS = {
    "development": [
        DATASET / "development/records",
        ROOT / ".peykar/endothelium-development-private/records",
    ],
    "sealed": [ROOT / ".peykar/endothelium-sealed-2026-10-28/records"],
}
WINDOWS = {"80": (0.70, 0.90), "90": (0.80, 0.97)}
PRIMARY = {
    "next_observation": "population_anchored_t4",
    "landmark": "population_anchored_t4",
    "initial_state": "population_initial_state",
}
COMPARATORS = ("persistence", "last_two_log_linear", "pooled_curve_ratio", "analog_knn")
_OPEN = re.compile(r"\bCC0\b|\bCC BY\b|Creative Commons Attribution", re.I)
_RESTRICTED = re.compile(
    r"\bNC\b|\bND\b|\bSA\b|Non[\s-]*Commercial|No[\s-]*Deriv|Share[\s-]*Alike|CC BY-N|CC BY-SA",
    re.I,
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def publishable(record: dict[str, Any]) -> bool:
    """Values may be republished only from CC BY or CC0 records (no NC/ND/SA terms)."""
    statement = str(record.get("license", {}).get("statement", ""))
    return bool(_OPEN.search(statement)) and not _RESTRICTED.search(statement)


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
        for key in ("observed", "predictions"):
            entry.pop(key, None)
        for score in entry["scores"].values():
            score.pop("signed_errors_pp", None)
    for task in ("next_observation", "landmark"):
        for entry in out[task]:
            entry["rows"] = [
                {k: row[k] for k in ("origin_month", "target_month", "history_points")}
                for row in entry["rows"]
            ]
            for score in entry["scores"].values():
                score.pop("signed_errors_pp", None)
    return out


def rmse(values: list[float]) -> float | None:
    return math.sqrt(sum(v * v for v in values) / len(values)) if values else None


def paired(studies: list[dict[str, Any]], task: str, first: str, second: str) -> dict[str, Any]:
    pairs = []
    for study in studies:
        for entry in study[task]:
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


def calibration(summary: dict[str, Any], task: str, method: str) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for level, (low, high) in WINDOWS.items():
        block = summary["coverage"][level][task]
        cov = block.get(method, {"inside": 0, "n": 0})
        value = cov["inside"] / cov["n"] if cov["n"] else None
        result[level] = {
            **cov,
            "coverage": value,
            f"within_{low:.2f}_{high:.2f}": value is not None and low <= value <= high,
        }
    return result


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
                "level_used": entry.get("population_level_used"),
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


def analysis(report: dict[str, Any]) -> dict[str, Any]:
    studies = report["studies"]
    summary = report["summary"]
    u2 = PRIMARY["next_observation"]
    u3 = PRIMARY["landmark"]
    u1 = PRIMARY["initial_state"]
    return {
        "primary_methods": PRIMARY,
        "H1_calibration_next_observation": calibration(summary, "next_observation", u2),
        "H2_accuracy_next_observation": {
            name: paired(studies, "next_observation", u2, name) for name in COMPARATORS
        },
        "H3_landmark": {
            "calibration": calibration(summary, "landmark", u3),
            **{name: paired(studies, "landmark", u3, name) for name in COMPARATORS},
        },
        "H4_initial_state_group_means": {
            "summary": summary["initial_state"]["group_mean"],
            "calibration": {
                level: {
                    **summary["coverage"][level]["initial_state_group_mean"][u1],
                    "window": list(window),
                }
                for level, window in WINDOWS.items()
            },
        },
        "pass_counts": {
            task: {
                name: {
                    k: values[k]
                    for k in (
                        "series",
                        "series_within_5_10_pp",
                        "series_meeting_criteria",
                        "pooled_rmse_pp",
                        "pooled_max_abs_pp",
                        "targets",
                    )
                }
                for name, values in summary[task].items()
            }
            for task in ("next_observation", "landmark")
        },
        "by_family": summary["by_family"],
        "per_series": {
            "next_observation": per_series(studies, "next_observation", [u2, *COMPARATORS]),
            "landmark": per_series(studies, "landmark", [u3, *COMPARATORS]),
            "initial_state": per_series(studies, "initial_state", [u1, "pooled_training_curve"]),
        },
    }


def tables(result: dict[str, Any], mode: str) -> str:
    lines = [f"# Endothelial density model 1.0.0 — {mode} results", ""]
    for task in ("next_observation", "landmark"):
        lines += [
            f"## {task}",
            "",
            "| method | series | within 5/10 pp | criteria (incl. persistence gain)"
            " | pooled RMSE pp | max pp |",
            "|---|---|---|---|---|---|",
        ]
        rows = sorted(result["pass_counts"][task].items(), key=lambda kv: kv[1]["pooled_rmse_pp"])
        for name, v in rows:
            lines.append(
                f"| {name} | {v['series']} | {v['series_within_5_10_pp']} | "
                f"{v['series_meeting_criteria']} | {v['pooled_rmse_pp']:.2f} | "
                f"{v['pooled_max_abs_pp']:.2f} |"
            )
        lines.append("")
    lines += [
        "## initial state (control group means)",
        "",
        "| method | groups | within 5/10 pp | pooled RMSE pp | max pp |",
        "|---|---|---|---|---|",
    ]
    for name, v in sorted(
        result["H4_initial_state_group_means"]["summary"].items(),
        key=lambda kv: kv[1]["pooled_rmse_pp"],
    ):
        lines.append(
            f"| {name} | {v['series']} | {v['series_within_5_10_pp']} | "
            f"{v['pooled_rmse_pp']:.2f} | {v['pooled_max_abs_pp']:.2f} |"
        )
    lines += ["", "## calibration of primary methods", ""]
    for label, block in (
        ("next observation", result["H1_calibration_next_observation"]),
        ("landmark", result["H3_landmark"]["calibration"]),
    ):
        for level, cov in block.items():
            low, high = WINDOWS[level]
            lines.append(
                f"- {label}, {level}% interval: {cov['inside']}/{cov['n']} = "
                f"{(cov['coverage'] or 0):.3f} (window {low:.2f}–{high:.2f})"
            )
    for level, cov in result["H4_initial_state_group_means"]["calibration"].items():
        value = cov["inside"] / cov["n"] if cov["n"] else 0.0
        lines.append(
            f"- initial state group means, {level}% interval: "
            f"{cov['inside']}/{cov['n']} = {value:.3f}"
        )
    return "\n".join(lines) + "\n"


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
