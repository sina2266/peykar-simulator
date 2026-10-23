"""Compare completed native trajectories against the previously frozen archive protocol.

Run with the separate scientific environment. Importing this file needs only stdlib;
dataframe and plotting dependencies are loaded only by the explicit operator.
"""

import argparse
import hashlib
import json
import math
from pathlib import Path


def checksum(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def identity(value):
    raw = json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False, allow_nan=False
    ).encode()
    return hashlib.sha256(raw).hexdigest()


def bounded_file(root, relative, expected):
    path = (root / relative).resolve(strict=True)
    if not path.is_relative_to(root.resolve(strict=True)) or checksum(path) != expected:
        raise ValueError(f"Artifact identity/confinement differs: {relative}")
    return path


def mean_check(observed, reference, sd_multiplier, allowance):
    if not all(math.isfinite(v) for v in [observed, reference["mean"], reference["sample_sd"]]):
        raise ValueError("Nonfinite comparison value")
    if reference["sample_sd"] < 0 or sd_multiplier < 0 or allowance < 0:
        raise ValueError("Negative spread or tolerance")
    tolerance = sd_multiplier * reference["sample_sd"] + allowance
    difference = observed - reference["mean"]
    return {
        "observed": observed,
        "reference_mean": reference["mean"],
        "reference_sample_sd": reference["sample_sd"],
        "difference": difference,
        "absolute_tolerance": tolerance,
        "passed": abs(difference) <= tolerance,
    }


def trajectory_check(observed, means, deviations, sd_multiplier, allowance, minimum):
    if not observed or len(observed) != len(means) or len(means) != len(deviations):
        raise ValueError("Full matching trajectory grids are required")
    checks = [
        mean_check(v, {"mean": m, "sample_sd": s}, sd_multiplier, allowance)
        for v, m, s in zip(observed, means, deviations, strict=True)
    ]
    coverage = sum(c["passed"] for c in checks) / len(checks)
    return {
        "samples": len(checks),
        "coverage": coverage,
        "minimum_coverage": minimum,
        "rmse": math.sqrt(sum(c["difference"] ** 2 for c in checks) / len(checks)),
        "maximum_absolute_difference": max(abs(c["difference"]) for c in checks),
        "passed": coverage >= minimum,
    }


def recovery_summary(counts, injury_hour=720, preinjury_hours=24, persistence_hours=24):
    """Descriptive count return after the post-injury nadir; never wound closure.

    This endpoint was added after execution began and has no frozen pass threshold.
    Requiring a complete persistence window avoids counting a one-hour crossing.
    """
    if len(counts) <= injury_hour + persistence_hours or injury_hour < preinjury_hours:
        raise ValueError("Incomplete injury recovery trajectory")
    target = sum(counts[injury_hour - preinjury_hours : injury_hour]) / preinjury_hours
    early = counts[injury_hour : injury_hour + 5 * 24 + 1]
    nadir_index = min(range(len(early)), key=early.__getitem__) + injury_hour
    recovered = next(
        (
            i
            for i in range(nadir_index + 1, len(counts) - persistence_hours + 1)
            if all(v >= target for v in counts[i : i + persistence_hours])
        ),
        None,
    )
    return {
        "preinjury_24h_mean_total": target,
        "early_5day_nadir_total": counts[nadir_index],
        "nadir_hours_after_injury": nadir_index - injury_hour,
        "first_24h_sustained_count_return_hours_after_injury": None
        if recovered is None
        else recovered - injury_hour,
        "classification": "descriptive_population_count_return_not_wound_closure",
        "pre_registered_acceptance_endpoint": False,
    }


def load_run(plan, job, protocol_digest, protocol):
    import numpy as np
    import pandas as pd

    root = Path(job["directory"]).resolve(strict=True)
    manifest = json.loads((root / "manifest.json").read_bytes())
    record = json.loads((root / "record.json").read_bytes())
    runtime = json.loads((root / "runtime-profile.json").read_bytes())
    if (
        record["status"] != "succeeded"
        or record.get("returncode") != 0
        or identity(manifest) != job["manifest_digest"]
        or record["manifest_digest"] != job["manifest_digest"]
        or manifest["comparison_protocol_digest"] != protocol_digest
        or identity(runtime) != plan["profile_digest"]
        or record["runtime_digest"] != plan["profile_digest"]
        or manifest["publication_recipe"] != job["case"]
        or manifest["controls"]["seed"] != job["seed"]
    ):
        raise ValueError("Completed run/plan/runtime/protocol identities disagree")
    for relative, expected in manifest["execution_files"].items():
        bounded_file(root / "project", relative, expected)
    bounded_file(root, "driver.py", manifest["driver_digest"])
    if manifest.get("execution_guard_digest"):
        bounded_file(root, "completion_guard.py", manifest["execution_guard_digest"])
        proof_path = bounded_file(root, "completion.json", record["completion_digest"])
        proof = json.loads(proof_path.read_bytes())
        if (
            proof.get("scope") != "observed_native_child_exit"
            or proof["returncode"] != 0
            or proof["manifest_digest"] != job["manifest_digest"]
            or proof["runtime_digest"] != plan["profile_digest"]
            or proof["outputs"] != record["outputs"]
        ):
            raise ValueError("Durable native completion disagrees with the execution record")
    outputs = {name: bounded_file(root, name, sha) for name, sha in record["outputs"].items()}
    counts = [p for p in outputs.values() if p.name.startswith("cell_count_")]
    heights = [p for p in outputs.values() if p.name.startswith("thickness_rep")]
    if len(counts) != 1 or len(heights) != 1:
        raise ValueError("Expected exactly one complete native count and thickness artifact")
    group = protocol["groups"][job["case"]]
    horizon = group["end_hour"]
    frame = pd.read_csv(counts[0])
    columns = protocol["count_columns"]
    values = frame[columns].to_numpy(dtype=float)
    if (
        list(frame.columns) != columns + ["Time"]
        or len(frame) != horizon + 1
        or not np.array_equal(frame.Time, np.arange(horizon + 1))
        or not np.isfinite(values).all()
        or (values < 0).any()
        or not np.array_equal(values, np.floor(values))
        or frame.iloc[0][columns].to_dict() != group["initial_counts"]
        or manifest["controls"]["steps_mcs"] != horizon * 10
    ):
        raise ValueError("Native counts, original hour grid, initial state or full horizon differ")
    height = pd.read_parquet(heights[0])
    if (
        list(height.columns) != ["Time", "Bin", "Height"]
        or len(height) != (horizon + 1) * 10
        or height.duplicated(["Time", "Bin"]).any()
        or set(height.Bin) != set(range(10))
        or any(not np.array_equal(f.Time, np.arange(horizon + 1)) for _, f in height.groupby("Bin"))
        or not np.isfinite(height.Height).all()
        or (height.Height < 0).any()
    ):
        raise ValueError("Native height must retain every hourly sample and all ten lattice bins")
    injury = manifest.get("parameter_overrides", {}).get("InjuryTime")
    pulse = None
    if injury is not None:
        paths = [p for p in outputs.values() if p.name.startswith("surfactant_")]
        if len(paths) != 1:
            raise ValueError("Chemical injury requires actual surfactant tracking output")
        surfactant = pd.read_csv(paths[0])
        # The original tracker logs one value per MCS, with Time already in hours.
        if list(surfactant.columns) != ["Total Amount", "Time"]:
            raise ValueError("Original surfactant schema differs")
        positive = surfactant[surfactant["Total Amount"] > 0]
        if positive.empty or float(positive.Time.iloc[0]) != injury / 10:
            raise ValueError("Actual chemical pulse time differs from the frozen recipe")
        expected = manifest["parameter_overrides"]["SLS_Concentration"]
        if not math.isclose(
            float(positive["Total Amount"].iloc[0]), expected, rel_tol=1e-9, abs_tol=1e-9
        ):
            raise ValueError("Actual chemical pulse amount differs from the frozen recipe")
        pulse = {
            "first_positive_hour": float(positive.Time.iloc[0]),
            "first_positive_total": float(positive["Total Amount"].iloc[0]),
        }
    return root, manifest, record, frame, height, pulse


def validate_series_jobs(plan, protocol, diagnostic_cases=None):
    jobs = plan["jobs"]
    required = set(protocol["groups"])
    if diagnostic_cases is not None:
        required = set(diagnostic_cases)
        if not required or not required < set(protocol["groups"]):
            raise ValueError("Diagnostic selection must be a nonempty proper subset")
        jobs = [job for job in jobs if job["case"] in required]
    cases = {j["case"] for j in jobs}
    seeds = {j["seed"] for j in jobs}
    pairs = [(j["case"], j["seed"]) for j in jobs]
    if (
        cases != required
        or len(seeds) < 2
        or len(pairs) != len(set(pairs))
        or set(pairs) != {(c, s) for c in cases for s in seeds}
    ):
        raise ValueError("Every required cohort and at least two distinct seeds are required")
    return jobs


def run(directory, references, destination, *, diagnostic_cases=None):
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import numpy as np

    plan = json.loads((directory / "plan.json").read_bytes())
    protocol_path = directory / "comparison.json"
    protocol_digest = checksum(protocol_path)
    protocol = json.loads(protocol_path.read_bytes())
    if (
        plan["status"] != "execution_completed"
        or protocol_digest != plan["protocol_digest"]
        or not protocol["frozen_before_full_candidate_execution"]
    ):
        raise ValueError("Finish every frozen job before comparison; preserve failed/active runs")
    jobs = validate_series_jobs(plan, protocol, diagnostic_cases)
    for group in protocol["groups"].values():
        for relative, expected in group["reference_files"].items():
            bounded_file(references, relative, expected)
    # Admission is complete before any output is created.
    loaded = [(job, load_run(plan, job, protocol_digest, protocol)) for job in jobs]
    destination.mkdir(parents=True, exist_ok=False)
    report = {
        "schema_version": "1.0.0",
        "class": "simulation_reference_comparison",
        "protocol_digest": protocol_digest,
        "plan_digest": checksum(directory / "plan.json"),
        "reference_replicates": sum(g["reference_replicates"] for g in protocol["groups"].values()),
        "candidate_runs": len(loaded),
        "jobs": [],
        "limitations": protocol["limitations"],
        "independent_biological_validation": False,
        "diagnostic_subset": diagnostic_cases,
        "all_publication_groups_assessed": diagnostic_cases is None,
    }
    for job, (root, _manifest, record, counts, height, pulse) in loaded:
        group = protocol["groups"][job["case"]]
        stable = counts[counts.Time >= protocol["stable_from_hour"]]
        columns = protocol["count_columns"]
        means = np.asarray(group["pointwise_count_mean"])
        sds = np.asarray(group["pointwise_count_sample_sd"])
        result = {
            "case": job["case"],
            "seed": job["seed"],
            "manifest_digest": job["manifest_digest"],
            "runtime_digest": plan["profile_digest"],
            "record_digest": checksum(root / "record.json"),
            "outputs": record["outputs"],
            "elapsed_seconds": record["elapsed_seconds"],
            "cell_counts": {},
            "thickness_um": {},
            "chemical_pulse": pulse,
        }
        for i, name in enumerate(columns):
            result["cell_counts"][name] = {
                "stable_mean": mean_check(
                    float(stable[name].mean()),
                    group["stable_count_means"][name],
                    protocol["mean_envelope_sd_multiplier"],
                    protocol["count_quantization_allowance"],
                ),
                "full_trajectory": trajectory_check(
                    counts[name].tolist(),
                    means[:, i].tolist(),
                    sds[:, i].tolist(),
                    protocol["pointwise_sd_multiplier"],
                    protocol["count_quantization_allowance"],
                    protocol["minimum_pointwise_coverage"],
                ),
            }
        stable_heights = (
            height[height.Time >= protocol["stable_from_hour"]].groupby("Bin").Height.mean()
        )
        for bin_id in range(10):
            observed = (
                float(stable_heights[bin_id]) - protocol["membrane_bins_lattice"][str(bin_id)]
            ) * protocol["thickness_lattice_to_um"]
            result["thickness_um"][str(bin_id)] = mean_check(
                observed,
                group["stable_thickness_um"][str(bin_id)],
                protocol["mean_envelope_sd_multiplier"],
                protocol["thickness_quantization_allowance_um"],
            )
        if pulse is not None:
            result["descriptive_recovery"] = recovery_summary(counts[columns].sum(axis=1).tolist())
        result["passed"] = all(
            v["stable_mean"]["passed"] and v["full_trajectory"]["passed"]
            for v in result["cell_counts"].values()
        ) and all(v["passed"] for v in result["thickness_um"].values())
        report["jobs"].append(result)
        figure, axes = plt.subplots(4, 1, figsize=(12, 11), sharex=True)
        days = counts.Time / 24
        for i, (name, ax) in enumerate(zip(columns, axes, strict=True)):
            ax.fill_between(
                days,
                means[:, i] - 3 * sds[:, i],
                means[:, i] + 3 * sds[:, i],
                color="#c1ced7",
                alpha=0.6,
                label="Archive mean ±3 sample SD",
            )
            ax.plot(days, means[:, i], color="#4c6475", linewidth=0.8, label="Archive mean")
            ax.plot(days, counts[name], color="#ac3d32", linewidth=0.65, label="New native run")
            if pulse is not None:
                ax.axvline(30, color="#222222", linestyle="--", linewidth=0.8)
            ax.set_ylabel(f"{name}\ncells")
            ax.grid(alpha=0.15)
        axes[0].legend(loc="upper right", ncols=3, fontsize=8)
        axes[0].set_title(f"{job['case']} · seed {job['seed']} · full horizon")
        axes[-1].set_xlabel("Simulated time (days); original Time is hours")
        figure.tight_layout()
        figure.savefig(destination / f"{job['case']}-seed-{job['seed']}-counts.png", dpi=160)
        plt.close(figure)
    report["status"] = "passed" if all(j["passed"] for j in report["jobs"]) else "not_reproduced"
    if diagnostic_cases is not None:
        report["status"] = "diagnostic_" + report["status"]
    report["files"] = {p.name: checksum(p) for p in destination.iterdir() if p.is_file()}
    (destination / "report.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    rows = [
        "# Corneal archive comparison",
        "",
        "These are model-to-model variability checks. "
        "Independent biological validation remains open.",
        "",
        "| Scenario | Seed | Count means | Hourly trajectories | Thickness bins | Result |",
        "|---|---|---|---|---|---|",
    ]
    for job in report["jobs"]:
        means = sum(v["stable_mean"]["passed"] for v in job["cell_counts"].values())
        traces = sum(v["full_trajectory"]["passed"] for v in job["cell_counts"].values())
        thickness = sum(v["passed"] for v in job["thickness_um"].values())
        rows.append(
            f"| {job['case']} | {job['seed']} | {means}/4 | {traces}/4 | "
            f"{thickness}/10 | {job['passed']} |"
        )
    rows.extend(
        [
            "",
            f"Frozen result: **{report['status']}**.",
            "",
            "Count return is descriptive and is not structural wound closure. "
            "Two new seeds do not establish population-level stochastic equivalence.",
        ]
    )
    (destination / "report.md").write_text("\n".join(rows) + "\n")
    print(
        report["status"],
        len(report["jobs"]),
        "full native runs;",
        report["reference_replicates"],
        "archived replicates",
    )
    return report


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runs", required=True, type=Path)
    parser.add_argument("--references", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--diagnostic-case", action="append")
    args = parser.parse_args()
    result = run(args.runs, args.references, args.output, diagnostic_cases=args.diagnostic_case)
    raise SystemExit(0 if result["status"] in ("passed", "diagnostic_passed") else 2)
