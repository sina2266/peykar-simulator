"""Explicit bounded offline next-observation replay on already-consumed studies."""

import argparse
import hashlib
import json
import math
import os
import statistics
import subprocess
import sys
import time
from importlib.util import module_from_spec, spec_from_file_location
from pathlib import Path

from peykar.validation.cornea_online_evidence import corneal_online_series

BASE = Path("experiments/vcornea/repair-front/online-forecast")
PROTOCOL = BASE / "protocol-1.0.1.json"
# Filled once after implementation/criteria review, before any real forecasts.
PROTOCOL_SHA256 = "92d7bb2f717b1122cdd23bd33a4bdc8db3e37fab5d037c793113ee1c57c2a9d5"
METHODS = (
    "local_radius_trend",
    "persistence",
    "first_to_current_radius",
    "crosson1986_initial_only",
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def checked(path: Path, expected: str) -> bytes:
    raw = path.read_bytes()
    if hashlib.sha256(raw).hexdigest() != expected:
        raise ValueError(f"Frozen artifact changed: {path}")
    return raw


def write(path: Path, value: object) -> None:
    with path.open("x") as stream:
        json.dump(value, stream, sort_keys=True, indent=2, allow_nan=False)
        stream.write("\n")


def read_protocol() -> dict:
    protocol = json.loads(checked(PROTOCOL, PROTOCOL_SHA256))
    template = (
        Path(__file__)
        .read_text()
        .replace(
            f'PROTOCOL_SHA256 = "{PROTOCOL_SHA256}"',
            'PROTOCOL_SHA256 = "TO_BE_FROZEN_BEFORE_EXECUTION"',
            1,
        )
    )
    if hashlib.sha256(template.encode()).hexdigest() != protocol["operator_template_sha256"]:
        raise ValueError("Frozen operator arithmetic/eligibility changed")
    for name, expected in protocol["source_pins"].items():
        checked(Path(name), expected)
    return protocol


def admitted(protocol: dict) -> dict:
    return corneal_online_series(
        **{
            key: checked(Path(value["path"]), value["sha256"])
            for key, value in protocol["datasets"].items()
        }
    )


def origins(series: dict) -> tuple[list[dict], list[dict]]:
    """Use every adjacent pair of actual numeric observations, no interpolation."""
    actual = [r for r in series["numeric_observations"] if r["eligible_for_conditioning"]]
    tasks, excluded = [], []
    targets = sorted([*actual, *series["categorical_observations"]], key=lambda r: r["hour"])
    for target in targets:
        available = [r for r in actual if r["hour"] < target["hour"]]
        if len(available) < 2:
            excluded.append(
                {
                    "hour": target["hour"],
                    "reason": "fewer_than_two_preceding_actual_numeric_observations",
                }
            )
            continue
        origin = available[-1]
        tasks.append(
            {
                "series_id": series["series_id"],
                "origin_hour": origin["hour"],
                "target_hour": target["hour"],
                "history": [
                    {
                        "hour": r["hour"],
                        "area_fraction": r["area_fraction"],
                        "eligible_for_conditioning": r["eligible_for_conditioning"],
                    }
                    for r in available
                ],
                "initial_radius_um": series["baseline_radius_um"],
            }
        )
    return tasks, excluded


def policy(job: Path) -> str:
    # Deny all repository data/docs/other jobs. The child sees only reviewed code,
    # installed environment and its own prefix+outputs, never the next outcome.
    root = Path.cwd().resolve()
    exceptions = [root / ".venv", job.resolve()]
    conditions = " ".join(f"(require-not (subpath {json.dumps(str(p))}))" for p in exceptions)
    child = (root / "scripts/corneal_online_child.py").resolve()
    code_paths = [
        child,
        root / "packages/platform/src/peykar/__init__.py",
        root / "packages/platform/src/peykar/model_learning/__init__.py",
        root / "packages/platform/src/peykar/model_learning/corneal_online.py",
        root / "packages/core/src/peykar_core/__init__.py",
        root / "packages/core/src/peykar_core/errors.py",
    ]
    # CPython enumerates parent directories to locate even explicitly allowed modules.
    # Directory listing is allowed; neighboring file contents remain denied.
    code_paths += [
        root / "packages/platform/src",
        root / "packages/platform/src/peykar",
        root / "packages/platform/src/peykar/model_learning",
        root / "packages/core/src",
        root / "packages/core/src/peykar_core",
    ]
    conditions += " " + " ".join(
        f"(require-not (literal {json.dumps(str(p))}))" for p in code_paths
    )
    return (
        "(version 1) (allow default) (deny network*) (deny file-write*) "
        f"(allow file-write* (subpath {json.dumps(str(job.resolve()))})) "
        '(allow file-write* (literal "/dev/null")) '
        f"(deny file-read-data (require-all (subpath {json.dumps(str(root))}) "
        f"{conditions}))"
    )


def metrics(errors: list[float]) -> dict:
    return {
        "count": len(errors),
        "rmse_fraction": math.sqrt(statistics.mean(e * e for e in errors)),
        "maximum_absolute_error_fraction": max(abs(e) for e in errors),
        "mean_error_fraction": statistics.mean(errors),
    }


def analyze(output: Path) -> dict:
    """Replay checked actual children; this function starts no scientific process."""
    protocol = read_protocol()
    checked(output / "frozen-protocol.json", PROTOCOL_SHA256)
    record = json.loads((output / "execution-record.json").read_bytes())
    if record["status"] != "completed" or record["protocol_sha256"] != PROTOCOL_SHA256:
        raise ValueError("Require actual completed children under the frozen protocol")
    evidence = admitted(protocol)
    if json.loads((output / "evidence.json").read_bytes()) != evidence:
        raise ValueError("Retained evidence differs from pinned original sources")
    all_rows, reports, exclusions = [], [], []
    spec = spec_from_file_location("reviewed_online_child", "scripts/corneal_online_child.py")
    child_module = module_from_spec(spec)
    spec.loader.exec_module(child_module)
    ordinal = 0
    for series in evidence["series"]:
        tasks, omitted = origins(series)
        exclusions.append({"series_id": series["series_id"], "observations": omitted})
        rows = []
        for task in tasks:
            job = output / "jobs" / f"{ordinal:03d}"
            ordinal += 1
            receipt = json.loads((job / "child-receipt.json").read_bytes())
            execution = json.loads((job / "execution-record.json").read_bytes())
            controls = json.loads((job / "isolation-controls.json").read_bytes())
            if (
                execution["returncode"] != 0
                or execution["protocol_sha256"] != PROTOCOL_SHA256
                or receipt["protocol_sha256"] != PROTOCOL_SHA256
                or receipt["method_sha256"] != protocol["method_sha256"]
                or receipt["child_sha256"] != protocol["child_sha256"]
                or receipt["verified_denied_read_probes"] != controls["forbidden_read_probes"]
                or not receipt["write_denial_verified"]
                or not receipt["network_denial_verified"]
            ):
                raise ValueError("Require actual successful isolated child and verified denials")
            checked(job / "frozen-protocol.json", PROTOCOL_SHA256)
            prefix = json.loads(checked(job / "prefix.json", receipt["prefix_sha256"]))
            prediction = json.loads(checked(job / "prediction.json", receipt["prediction_sha256"]))
            if prefix != task or set(prediction["forecasts"]) != set(METHODS):
                raise ValueError("Candidate prefix or comparator coverage changed")
            # Deterministic read-only replay of the already-executed estimator,
            # with the identical prefix, detects changed arithmetic/comparators.
            if prediction != child_module.predict(prefix):
                raise ValueError("Retained prediction differs from frozen prefix arithmetic")
            observed = next(
                r
                for r in [*series["numeric_observations"], *series["categorical_observations"]]
                if r["hour"] == task["target_hour"]
            )
            if (
                checked(job / "revealed-outcome.json", execution["outcome_sha256"])
                != (json.dumps(observed, sort_keys=True, indent=2, allow_nan=False) + "\n").encode()
            ):
                raise ValueError("Post-child revealed outcome differs from actual source")
            forecasts = prediction["forecasts"]
            if any(
                not isinstance(v, (float, int)) or not math.isfinite(v) or v < 0
                for v in forecasts.values()
            ):
                raise ValueError("Forecasts must be finite nonnegative fractions")
            local = prediction["local_radius_trend"]
            actual_history = [r for r in task["history"] if r["eligible_for_conditioning"]]
            if (
                local["origin_hour"] != task["origin_hour"]
                or local["observed_history"]
                != [
                    {"hour": r["hour"], "area_fraction": r["area_fraction"]} for r in actual_history
                ]
                or local["predictions"][0]["hour"] != task["target_hour"]
                or local["predictions"][0]["area_fraction"] != forecasts["local_radius_trend"]
                or local["future_observations_used"]
            ):
                raise ValueError("Actual forecast origin/history/target differs")
            row = {
                "series_id": series["series_id"],
                "origin_hour": task["origin_hour"],
                "target_hour": task["target_hour"],
                "horizon_hours": task["target_hour"] - task["origin_hour"],
                "actual_area_fraction": observed.get("area_fraction"),
                "original": observed,
                "scored": "area_fraction" in observed,
                "forecasts": forecasts,
                "errors": (
                    {k: v - observed["area_fraction"] for k, v in forecasts.items()}
                    if "area_fraction" in observed
                    else None
                ),
                "job": f"jobs/{ordinal - 1:03d}",
            }
            rows.append(row)
        per_method = {
            method: metrics([row["errors"][method] for row in rows if row["scored"]])
            for method in METHODS
        }
        candidate = per_method["local_radius_trend"]
        passes_accuracy = (
            candidate["rmse_fraction"] <= protocol["maximum_rmse_fraction"]
            and candidate["maximum_absolute_error_fraction"]
            <= protocol["maximum_absolute_error_fraction"]
        )
        improvement = per_method["persistence"]["rmse_fraction"] - candidate["rmse_fraction"]
        passes_improvement = improvement >= protocol["minimum_rmse_improvement_over_persistence"]
        reports.append(
            {
                "series_id": series["series_id"],
                "study_doi": series["study_doi"],
                "group": series["group"],
                "series_nature": series["series_nature"],
                "metrics": per_method,
                "passes_accuracy": passes_accuracy,
                "rmse_improvement_over_persistence_fraction": improvement,
                "passes_improvement": passes_improvement,
                "passed": passes_accuracy and passes_improvement,
            }
        )
        all_rows.extend(rows)
    if ordinal != len(record["jobs"]) or ordinal != protocol["expected_forecast_count"]:
        raise ValueError("Missing or unexpected actual forecast jobs")
    if record["jobs"] != [
        {"job": f"jobs/{index:03d}", "returncode": 0} for index in range(ordinal)
    ]:
        raise ValueError("Parent summary does not retain every successful actual child")
    if [r["series_id"] for r in reports] != protocol["expected_series_ids"]:
        raise ValueError("Study/group/animal coverage differs from frozen selection")
    if sum(r["scored"] for r in all_rows) != protocol["expected_numeric_forecast_count"]:
        raise ValueError("Numeric versus categorical target coverage changed")
    passed = all(r["passed"] for r in reports)
    return {
        "schema_version": "1.0.0",
        "protocol_sha256": PROTOCOL_SHA256,
        "status": "passed_retrospective_task_only" if passed else "rejected",
        "passed": passed,
        "series_count": len(reports),
        "forecast_count": ordinal,
        "numeric_forecast_count": sum(r["scored"] for r in all_rows),
        "categorical_unscored_count": sum(not r["scored"] for r in all_rows),
        "series_reports": reports,
        "rows": all_rows,
        "excluded_observations": exclusions,
        "pooled_accuracy_used_for_acceptance": False,
        "future_values_in_child_prefix": False,
        "initial_state_simulator_validated": False,
        "independent_biological_validation": False,
        "previously_consumed_studies": True,
        "task": "Retrospective next observation, conditioned on available same-series prefix",
    }


def run(output: Path) -> int:
    protocol = read_protocol()
    if sys.platform != "darwin":
        raise RuntimeError("Reviewed operator requires macOS Seatbelt; use analyze elsewhere")
    output = output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    (output / "jobs").mkdir()
    (output / "reserved").mkdir()
    (output / "frozen-protocol.json").write_bytes(PROTOCOL.read_bytes())
    for source in [Path(__file__), Path("scripts/corneal_online_child.py")]:
        (output / f"executed-{source.name}.txt").write_bytes(source.read_bytes())
    evidence = admitted(protocol)
    write(output / "evidence.json", evidence)
    environment = {k: v for k, v in os.environ.items() if k in {"PATH", "LANG", "LC_ALL"}}
    environment.update(PYTHONHASHSEED="0", PYTHONDONTWRITEBYTECODE="1", PYTHONNOUSERSITE="1")
    started = time.monotonic()
    records = []
    for series in evidence["series"]:
        tasks, _ = origins(series)
        for task in tasks:
            ordinal = len(records)
            if time.monotonic() - started >= protocol["total_wall_seconds"]:
                raise TimeoutError(
                    "Full offline replay wall budget exhausted; partial jobs retained"
                )
            job = output / "jobs" / f"{ordinal:03d}"
            job.mkdir()
            write(job / "prefix.json", task)
            (job / "frozen-protocol.json").write_bytes(PROTOCOL.read_bytes())
            outcome = next(
                r
                for r in [*series["numeric_observations"], *series["categorical_observations"]]
                if r["hour"] == task["target_hour"]
            )
            reserved = output / "reserved" / f"{ordinal:03d}.json"
            write(reserved, outcome)
            controls = {
                "forbidden_read_probes": [
                    str(Path(v["path"]).resolve()) for v in protocol["datasets"].values()
                ]
                + [str(output / "evidence.json"), str(reserved)],
                "forbidden_write_probe": str(output / f"write-denial-probe-{ordinal:03d}"),
            }
            write(job / "isolation-controls.json", controls)
            command = [
                "/usr/bin/sandbox-exec",
                "-p",
                policy(job),
                sys.executable,
                str(Path("scripts/corneal_online_child.py").resolve()),
                "--job",
                str(job),
            ]
            # CPython must not read an older source-matching cached bytecode file.
            environment["PYTHONPYCACHEPREFIX"] = str(job / "unused-bytecode-cache")
            child_started = time.monotonic()
            with (job / "execution.log").open("xb") as log:
                try:
                    completed = subprocess.run(
                        command,
                        env=environment,
                        stdout=log,
                        stderr=subprocess.STDOUT,
                        timeout=min(
                            protocol["child_wall_seconds"],
                            protocol["total_wall_seconds"] - (time.monotonic() - started),
                        ),
                        check=False,
                    )
                    code = completed.returncode
                except subprocess.TimeoutExpired:
                    code = None
            execution = {
                "protocol_sha256": PROTOCOL_SHA256,
                "returncode": code,
                "elapsed_seconds": time.monotonic() - child_started,
                "command": command,
                "child_wall_seconds": protocol["child_wall_seconds"],
            }
            if code == 0:
                # Outcome becomes available in the child's own directory ONLY AFTER it exits.
                (job / "revealed-outcome.json").write_bytes(reserved.read_bytes())
                execution["outcome_sha256"] = digest(job / "revealed-outcome.json")
            write(job / "execution-record.json", execution)
            records.append({"job": f"jobs/{ordinal:03d}", "returncode": code})
            if code != 0:
                write(
                    output / "execution-record.json",
                    {
                        "status": "failed",
                        "protocol_sha256": PROTOCOL_SHA256,
                        "jobs": records,
                        "elapsed_seconds": time.monotonic() - started,
                    },
                )
                print("Actual child failed; all outputs retained:", job)
                return 1
    write(
        output / "execution-record.json",
        {
            "status": "completed",
            "protocol_sha256": PROTOCOL_SHA256,
            "jobs": records,
            "elapsed_seconds": time.monotonic() - started,
            "total_wall_seconds": protocol["total_wall_seconds"],
            "operator_sha256": digest(Path(__file__)),
            "uv_lock_sha256": digest(Path("uv.lock")),
        },
    )
    report = analyze(output)
    write(output / "report.json", report)
    print(
        report["status"],
        "forecasts:",
        report["forecast_count"],
        "series passing:",
        sum(r["passed"] for r in report["series_reports"]),
    )
    return 0 if report["passed"] else 2


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--analyze", action="store_true")
    args = parser.parse_args()
    if args.analyze:
        result = analyze(args.output)
        print(json.dumps(result, indent=2, allow_nan=False))
    else:
        try:
            status = run(args.output)
        except Exception as error:
            if args.output.is_dir():
                write(
                    args.output / "operator-failure.json",
                    {
                        "exception_type": type(error).__name__,
                        "message": str(error),
                        "protocol_sha256": PROTOCOL_SHA256,
                    },
                )
            raise
        raise SystemExit(status)
