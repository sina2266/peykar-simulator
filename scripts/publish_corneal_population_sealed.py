"""Write licence-respecting public artifacts of the sealed corneal population test.

Reads the private sealed report (never re-runs a model). Studies whose source
licence forbids sharing adapted material (CC BY-NC-ND) keep only per-series and
per-group metrics: observed values, predictions, intervals and signed errors are
removed. CC BY studies are published in full with attribution. The primary
analysis declared in the frozen protocol is computed from the private report.
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

NO_DERIVATIVES = {"lsc-dedifferentiation-nat-commun-2026"}


def rmse(values: list[float]) -> float:
    return math.sqrt(sum(v * v for v in values) / len(values))


def primary_analysis(report: dict[str, Any], protocol: dict[str, Any]) -> dict[str, Any]:
    pairs = []
    for study in report["studies"]:
        for entry in study["next_observation"]:
            for row in entry["rows"]:
                p = row["predictions"]
                if "cohort_assimilation" in p and "last_two_radius_trend" in p:
                    pairs.append(
                        (
                            100 * (p["cohort_assimilation"] - row["observed"]),
                            100 * (p["last_two_radius_trend"] - row["observed"]),
                        )
                    )
    coverage = report["summary"]["coverage"]
    c80 = coverage["80"]["next_observation"]["cohort_assimilation"]
    c90 = coverage["90"]["next_observation"]["cohort_assimilation"]
    initial = [
        {
            "study_id": study["study_id"],
            "group": entry["group"],
            "animals": entry["animals"],
            "population_initial_state": {
                k: entry["scores"]["population_initial_state"][k]
                for k in ("rmse_pp", "max_abs_pp", "meets_5_10_pp")
            },
        }
        for study in report["studies"]
        for entry in study["initial_state"]
    ]
    cohort_rmse = rmse([a for a, _ in pairs])
    trend_rmse = rmse([b for _, b in pairs])
    return {
        "protocol_criteria": protocol["criteria"],
        "primary_comparison": {
            "common_targets": len(pairs),
            "cohort_assimilation_rmse_pp": cohort_rmse,
            "last_two_radius_trend_rmse_pp": trend_rmse,
            "cohort_lower": cohort_rmse < trend_rmse,
        },
        "calibration": {
            "cohort_80_coverage": c80["inside"] / c80["n"],
            "cohort_80_within_0.70_0.90": 0.70 <= c80["inside"] / c80["n"] <= 0.90,
            "cohort_90_coverage": c90["inside"] / c90["n"],
            "cohort_90_within_0.80_0.97": 0.80 <= c90["inside"] / c90["n"] <= 0.97,
        },
        "series_passing_next_observation": {
            name: f"{v['series_meeting_criteria']}/{v['series']}"
            for name, v in report["summary"]["next_observation"].items()
        },
        "initial_state_groups": initial,
    }


def redact(report: dict[str, Any]) -> dict[str, Any]:
    public = copy.deepcopy(report)
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
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--protocol", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    raw = args.report.read_bytes()
    report = json.loads(raw)
    protocol = json.loads(args.protocol.read_text())
    args.output.mkdir(parents=True, exist_ok=False)
    analysis = primary_analysis(report, protocol)
    analysis["private_report_sha256"] = hashlib.sha256(raw).hexdigest()
    (args.output / "primary-analysis.json").write_text(json.dumps(analysis, indent=1) + "\n")
    (args.output / "report-public.json").write_text(json.dumps(redact(report), indent=1) + "\n")
    print(json.dumps(analysis, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
