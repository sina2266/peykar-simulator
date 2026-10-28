"""Write licence-respecting public artifacts of a population-model 2.0.0 report.

Reads a private report of ``evaluate_corneal_population2.py`` (``version2`` and
``version1_reference`` parts; never re-runs a model). Studies derived from
CC BY-NC-ND sources keep only per-series/per-group metrics: observed values,
predictions, intervals and signed errors are removed. In ``--sealed`` mode the
analysis declared by the frozen 2.0.0 protocol is computed; in
``--development`` mode the same quantities are reported as development
reference only. Output goes to a new directory.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import sys
from pathlib import Path
from typing import Any

NO_DERIVATIVES = {"lsc-dedifferentiation-nat-commun-2026", "fl2-rat-alkali-tvst-2021"}
INTERVALS = {"80": (0.70, 0.90), "90": (0.80, 0.97)}


def rmse(values: list[float]) -> float | None:
    return math.sqrt(sum(v * v for v in values) / len(values)) if values else None


def paired(part: dict[str, Any], first: str, second: str) -> dict[str, Any]:
    pairs = []
    for study in part["studies"]:
        for entry in study["next_observation"]:
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
        f"{first}_lower": (a is not None and b is not None and a < b),
    }


def calibration(part: dict[str, Any], method: str) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for level, (low, high) in INTERVALS.items():
        cov = part["summary"]["coverage"][level]["next_observation"].get(method, {"n": 0})
        n = cov.get("n", 0)
        value = cov["inside"] / n if n else None
        result[level] = {
            "inside": cov.get("inside", 0),
            "n": n,
            "coverage": value,
            f"within_{low:.2f}_{high:.2f}": value is not None and low <= value <= high,
        }
    return result


def initial_state(part: dict[str, Any]) -> list[dict[str, Any]]:
    return [
        {
            "study_id": study["study_id"],
            "context": study["context"],
            "population_context_used": study.get("population_context_used"),
            "group": entry["group"],
            "animals": entry["animals"],
            **{
                name: {
                    k: entry["scores"][name][k] for k in ("rmse_pp", "max_abs_pp", "meets_5_10_pp")
                }
                for name in (
                    "population_initial_state",
                    "crosson1986_fixed",
                    "pooled_training_curve",
                )
            },
        }
        for study in part["studies"]
        for entry in study["initial_state"]
    ]


def analysis(report: dict[str, Any]) -> dict[str, Any]:
    v2, v1 = report["version2"], report["version1_reference"]
    return {
        "H1_calibration_v2_cohort": calibration(v2, "cohort_assimilation"),
        "H2_v2_cohort_vs_last_two_trend": paired(
            v2, "cohort_assimilation", "last_two_radius_trend"
        ),
        "H3_v2_vs_v1_cohort": {
            "v2": v2["summary"]["next_observation"].get("cohort_assimilation"),
            "v1": v1["summary"]["next_observation"].get("cohort_assimilation"),
            "v1_calibration": calibration(v1, "cohort_assimilation"),
        },
        "H4_initial_state_v2": initial_state(v2),
        "initial_state_v1": initial_state(v1),
        "secondary": {
            "v2_cohort_vs_analog_knn": paired(v2, "cohort_assimilation", "analog_knn"),
            "v2_population_t4_vs_last_two_trend": paired(
                v2, "population_assimilation_t4", "last_two_radius_trend"
            ),
            "v2_population_t4_calibration": calibration(v2, "population_assimilation_t4"),
            "series_passing_next_observation_v2": {
                name: f"{v['series_meeting_criteria']}/{v['series']}"
                for name, v in v2["summary"]["next_observation"].items()
            },
            "series_passing_next_observation_v1": {
                name: f"{v['series_meeting_criteria']}/{v['series']}"
                for name, v in v1["summary"]["next_observation"].items()
            },
            "pooled_rmse_pp_v2": {
                name: v["pooled_rmse_pp"] for name, v in v2["summary"]["next_observation"].items()
            },
        },
    }


def redact_part(part: dict[str, Any]) -> dict[str, Any]:
    public = copy.deepcopy(part)
    for study in public["studies"]:
        if study["study_id"] not in NO_DERIVATIVES:
            continue
        study["redaction"] = (
            "CC BY-NC-ND 4.0 source: observed values, predictions, intervals and signed "
            "errors removed; per-series/per-group metrics kept."
        )
        for entry in study["initial_state"]:
            for key in ("observed", "predictions", "population_interval_coverage"):
                entry.pop(key, None)
            for score in entry["scores"].values():
                score.pop("signed_errors_pp", None)
        for entry in study["individual_intervals"]:
            entry["score"].pop("signed_errors_pp", None)
        for entry in study["next_observation"]:
            entry["targets"] = len(entry.pop("rows"))
            for score in entry["scores"].values():
                score.pop("signed_errors_pp", None)
            entry["oracle_fit_floor"] = {
                k: entry["oracle_fit_floor"][k]
                for k in ("in_sample_rmse_pp", "in_sample_max_abs_pp", "within_5_10_pp")
            }
    return public


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--development", action="store_true")
    mode.add_argument("--sealed", action="store_true")
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--protocol", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.sealed and args.protocol is None:
        parser.error("--sealed requires --protocol")
    raw = args.report.read_bytes()
    report = json.loads(raw)
    if args.development:
        # Development LOSO parts are leave_one_study_out results.
        report = {name: part for name, part in report.items()}
    args.output.mkdir(parents=True, exist_ok=False)
    result = analysis(report)
    result["status"] = (
        "sealed one-shot result under the frozen protocol"
        if args.sealed
        else "development reference only (studies already read by the developer)"
    )
    result["private_report_sha256"] = hashlib.sha256(raw).hexdigest()
    if args.protocol is not None:
        protocol = json.loads(args.protocol.read_text())
        result["protocol_criteria"] = protocol.get("criteria")
        result["protocol_sha256"] = hashlib.sha256(args.protocol.read_bytes()).hexdigest()
    name = "primary-analysis.json" if args.sealed else "development-analysis.json"
    (args.output / name).write_text(json.dumps(result, indent=1) + "\n")
    public = {key: redact_part(part) for key, part in report.items()}
    (args.output / "report-public.json").write_text(json.dumps(public, indent=1) + "\n")
    print(
        json.dumps(
            {k: result[k] for k in ("H1_calibration_v2_cohort", "H2_v2_cohort_vs_last_two_trend")},
            indent=1,
        )
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
