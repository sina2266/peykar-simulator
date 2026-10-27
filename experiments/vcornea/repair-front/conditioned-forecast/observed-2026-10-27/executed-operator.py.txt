"""Explicit offline future-only test of a24h-conditioned corneal forecast."""

import argparse
import json
import math
import os
import platform
import statistics
import subprocess
import sys
import time
from pathlib import Path

import peykar_engines.corneal_conditioned_front as implementation
from peykar_core.contracts import (
    ExecutionBudget,
    ModelPackage,
    Quantity,
    Reference,
    SimulationPlan,
    SimulationResult,
)
from peykar_core.hashing import bytes_digest, canonical_json
from peykar_core.planning import time_grid, validate_plan
from peykar_engines.corneal_conditioned_front import CornealConditionedFrontEngine

BASE = Path("experiments/vcornea/repair-front/conditioned-forecast")
PROTOCOL = BASE / "protocol-1.0.0.json"
PROTOCOL_SHA256 = "2c8d7f806b01a440c5af4831d7b67d7386301436b5e89e4c73cc37c2124c72cd"
MODEL = Path("model_packages/cornea/conditioned-front/1.0.0/model.json")
ENGINE = Path("packages/engines/src/peykar_engines/corneal_conditioned_front.py")
PARSER = Path("packages/platform/src/peykar/validation/cornea_forecast_evidence.py")


def write(path: Path, value: object) -> None:
    with path.open("x") as stream:
        stream.write(canonical_json(value))


def checked(path: Path, digest: str) -> bytes:
    raw = path.read_bytes()
    if bytes_digest(raw) != digest:
        raise ValueError(f"Frozen artifact changed: {path.name}")
    return raw


def read_protocol() -> dict:
    protocol = json.loads(checked(PROTOCOL, PROTOCOL_SHA256))
    checked(ENGINE, protocol["engine_sha256"])
    checked(Path(implementation.__file__), protocol["engine_sha256"])
    checked(MODEL, protocol["model_sha256"])
    return protocol


def conditioning(protocol: dict) -> dict:
    prefix = json.loads(checked(BASE / "ijms2019-control-prefix.json", protocol["prefix_sha256"]))
    observations = prefix["observations"]
    if (
        prefix["study_doi"] != protocol["study_doi"]
        or prefix["role"] != "conditioning_input_only"
        or prefix["unit"] != "millimeter ** 2"
        or prefix["cutoff_hour"] != 24
        or [r["hour"] for r in observations] != [0, 24]
        or any(r["group"] != "Control" for r in observations)
    ):
        raise ValueError("Require only the declared same-modality0h/24h Control observations")
    return prefix


def child(output: Path) -> None:
    protocol = read_protocol()
    denied = []
    for path in protocol["forbidden_read_probes"]:
        try:
            Path(path).read_bytes()
        except PermissionError:
            denied.append(path)
        else:
            raise RuntimeError("Future-read isolation not enforced")
    prefix = conditioning(protocol)
    model = ModelPackage.model_validate_json(MODEL.read_bytes())
    plan = SimulationPlan(
        model=Reference(id=model.id, version=model.version),
        context=model.context,
        duration=Quantity(value=60, unit="hour"),
        sample_interval=Quantity(value=0.25, unit="hour"),
        seed=0,
        budget=ExecutionBudget(max_samples=1000, wall_seconds=protocol["wall_seconds"]),
        purpose="24h-observation-conditioned forecast to injury84h; future values excluded",
    )
    engine = CornealConditionedFrontEngine()
    values = engine.parameters(model, plan)
    baseline, anchor = prefix["observations"]
    if values != {
        "baseline_area": baseline["mean_area_mm2"],
        "anchor_area": anchor["mean_area_mm2"],
        "anchor_time": 24,
    }:
        raise ValueError("Model conditioning state differs from actual prefix")
    started = time.monotonic()

    def checkpoint() -> None:
        if time.monotonic() - started > protocol["wall_seconds"]:
            raise TimeoutError("Conditioned forecast exceeded wall limit")

    result = engine.execute(model, plan, checkpoint)
    write(output / "candidate.json", result.model_dump(mode="json"))
    write(output / "plan.json", plan.model_dump(mode="json"))
    r0 = math.sqrt(values["baseline_area"] / math.pi)
    ra = math.sqrt(values["anchor_area"] / math.pi)
    write(
        output / "conditioned-state.json",
        {
            "prefix_sha256": protocol["prefix_sha256"],
            "baseline_mean_area_mm2": values["baseline_area"],
            "anchor_mean_area_mm2": values["anchor_area"],
            "anchor_hour_since_injury": 24,
            "equivalent_baseline_radius_mm": r0,
            "equivalent_anchor_radius_mm": ra,
            "effective_interval_average_speed_um_per_h": (r0 - ra) / 24 * 1000,
            "lag_fitted": False,
            "future_observation_used": False,
            "instantaneous_cell_velocity_measured": False,
            "independent_biological_validation": False,
        },
    )
    write(
        output / "child-execution.json",
        {
            "protocol_sha256": PROTOCOL_SHA256,
            "model_sha256": protocol["model_sha256"],
            "engine_sha256": protocol["engine_sha256"],
            "operator_sha256": bytes_digest(Path(__file__).read_bytes()),
            "uv_lock_sha256": bytes_digest(Path("uv.lock").read_bytes()),
            "python_version": platform.python_version(),
            "elapsed_seconds": time.monotonic() - started,
            "verified_denied_read_probes": denied,
            "artifacts": {
                name: bytes_digest((output / name).read_bytes())
                for name in ["candidate.json", "plan.json", "conditioned-state.json"]
            },
            "seed_scope": "New deterministic bookkeeping0, not an experimental animal seed",
        },
    )


def metrics(errors: list[float]) -> dict:
    return {
        "rmse_fraction": math.sqrt(statistics.mean(e * e for e in errors)),
        "maximum_absolute_error_fraction": max(abs(e) for e in errors),
        "mean_error_fraction": statistics.mean(errors),
    }


def analyze(output: Path) -> dict:
    # Future data are read only in the parent, after the actual successful child.
    from peykar.validation.cornea_forecast_evidence import ijms2019_wound_aggregates

    protocol = read_protocol()
    record = json.loads((output / "execution-record.json").read_bytes())
    receipt = json.loads((output / "child-execution.json").read_bytes())
    if (
        record["returncode"] != 0
        or any(r["protocol_sha256"] != PROTOCOL_SHA256 for r in [record, receipt])
        or receipt["verified_denied_read_probes"] != protocol["forbidden_read_probes"]
    ):
        raise ValueError("Require actual successful isolated child with exact protocol")
    checked(output / "frozen-protocol.json", PROTOCOL_SHA256)
    if (receipt["model_sha256"], receipt["engine_sha256"]) != (
        protocol["model_sha256"],
        protocol["engine_sha256"],
    ):
        raise ValueError("Actual candidate model/source identity differs")
    for name, digest in receipt["artifacts"].items():
        checked(output / name, digest)
    model = ModelPackage.model_validate_json(MODEL.read_bytes())
    plan = SimulationPlan.model_validate_json((output / "plan.json").read_bytes())
    validate_plan(model, plan)
    result = SimulationResult.model_validate_json((output / "candidate.json").read_bytes())
    if (
        result.time.unit != "second"
        or result.time.values != time_grid(plan)
        or plan.duration != Quantity(value=60, unit="hour")
        or plan.sample_interval != Quantity(value=0.25, unit="hour")
        or result.seed != 0
        or plan.seed != 0
        or plan.parameters
        or plan.interventions
        or plan.context != model.context
        or {s.name: s.unit for s in result.outputs} != {p.name: p.unit for p in model.ports}
    ):
        raise ValueError("Candidate plan/grid/seed/units/conditioning changed")
    series = {s.name: s.values for s in result.outputs}
    if series["time_since_injury"] != tuple(t / 3600 + 24 for t in result.time.values):
        raise ValueError("Forecast and injury time origins differ")
    prefix = conditioning(protocol)
    baseline, anchor = prefix["observations"]
    baseline_area, anchor_area = baseline["mean_area_mm2"], anchor["mean_area_mm2"]
    if series["open_area_fraction"][0] != anchor_area / baseline_area:
        raise ValueError("Normalization must use original baseline, not the24h anchor")
    checked(PARSER, protocol["parser_sha256"])
    raw = checked(BASE / "ijms2019-original.xml", protocol["source_xml_sha256"])
    admitted = ijms2019_wound_aggregates(raw)
    future = json.loads(
        checked(BASE / "ijms2019-control-future-reserved.json", protocol["future_sha256"])
    )["observations"]
    original = [r for r in admitted["observations"] if r["group"] == "Control"]
    if (
        [r["hour"] for r in future] != protocol["evaluation_hours_since_injury"]
        or [r["hour"] for r in original] != [0, 24, *protocol["evaluation_hours_since_injury"]]
        or any(r["group"] != "Control" for r in future)
    ):
        raise ValueError("Every declared actual future time/group must be preserved")
    for declared in [*prefix["observations"], *future]:
        source = next(r for r in original if r["hour"] == declared["hour"])
        if any(source[k] != declared[k] for k in declared):
            raise ValueError("Separated conditioning/future data differ from original source")
    predicted = dict(zip(series["time_since_injury"], series["open_area_fraction"], strict=True))
    r0_um = math.sqrt(baseline_area / math.pi) * 1000
    rows = []
    for observation in future:
        hour = observation["hour"]
        measured = observation["mean_area_mm2"] / baseline_area
        estimate = predicted[hour]
        initial = (max(0.0, r0_um - 64 * max(0, hour - 5.5)) / r0_um) ** 2
        rows.append(
            {
                **observation,
                "measured_ratio_of_cohort_mean_areas": measured,
                "forecast_fraction": estimate,
                "forecast_error_fraction": estimate - measured,
                "persistence_fraction": anchor_area / baseline_area,
                "persistence_error_fraction": anchor_area / baseline_area - measured,
                "initial_state1986_formula_fraction": initial,
                "initial_state1986_error_fraction": initial - measured,
            }
        )
    forecast = metrics([r["forecast_error_fraction"] for r in rows])
    persistence = metrics([r["persistence_error_fraction"] for r in rows])
    initial = metrics([r["initial_state1986_error_fraction"] for r in rows])
    improvement = persistence["rmse_fraction"] - forecast["rmse_fraction"]
    passed = (
        forecast["rmse_fraction"] <= protocol["maximum_rmse_fraction"]
        and forecast["maximum_absolute_error_fraction"]
        <= protocol["maximum_absolute_error_fraction"]
        and improvement >= protocol["minimum_rmse_improvement_over_persistence"]
    )
    report = {
        "class": protocol["class"],
        "protocol_sha256": PROTOCOL_SHA256,
        "operator_sha256": bytes_digest(Path(__file__).read_bytes()),
        "candidate_sha256": receipt["artifacts"]["candidate.json"],
        "primary_future_only_metrics": forecast,
        "persistence_metrics": persistence,
        "rmse_improvement_over_persistence": improvement,
        "initial_state1986_formula_metrics": initial,
        "baseline_comparator_scope": (
            "Published1986 analytic law with observed initial equivalent radius; "
            "no later data; different information than conditioned forecast, no84h old-engine claim"
        ),
        "rows": rows,
        "all_source_observations": admitted["observations"],
        "state_estimation_inputs": prefix,
        "conditioning_hours_excluded_from_score": [0, 24],
        "other_source_groups_used_for_fitting": False,
        "future_data_used_for_state_estimation": False,
        "individual_animal_records_available": False,
        "status": "conditional_forecast_comparison_passed"
        if passed
        else "conditional_forecast_comparison_failed",
        "passed": passed,
        "independent_biological_validation": False,
        "initial_state_simulator_validated": False,
        "limitations": protocol["limitations"],
    }
    write(output / "report.json", report)
    return report


def run(output: Path) -> int:
    protocol = read_protocol()
    for name, digest in protocol["admitted_files"].items():
        checked(Path(name), digest)
    if sys.platform != "darwin":
        raise RuntimeError("This operator requires its reviewed macOS Seatbelt profile")
    output.mkdir(parents=True, exist_ok=False)
    (output / "frozen-protocol.json").write_bytes(PROTOCOL.read_bytes())
    (output / "executed-operator.py.txt").write_bytes(Path(__file__).read_bytes())
    policy = (
        "(version 1) (allow default) (deny network*) (deny file-write*) "
        f"(allow file-write* (subpath {json.dumps(str(output.resolve()))})) "
        '(allow file-write* (literal "/dev/null")) '
        + " ".join(
            f"(deny file-read* (subpath {json.dumps(str(Path(p).resolve()))}))"
            for p in protocol["forbidden_read_paths"]
        )
    )
    command = [
        "/usr/bin/sandbox-exec",
        "-p",
        policy,
        sys.executable,
        str(Path(__file__).resolve()),
        "--child",
        "--output",
        str(output.resolve()),
    ]
    environment = {k: v for k, v in os.environ.items() if k in {"PATH", "LANG", "LC_ALL"}}
    environment.update(PYTHONHASHSEED="0", PYTHONDONTWRITEBYTECODE="1", PYTHONNOUSERSITE="1")
    started = time.monotonic()
    with (output / "execution.log").open("xb") as log:
        try:
            completed = subprocess.run(
                command,
                env=environment,
                stdout=log,
                stderr=subprocess.STDOUT,
                timeout=protocol["wall_seconds"],
                check=False,
            )
            returncode = completed.returncode
        except subprocess.TimeoutExpired:
            returncode = None
    write(
        output / "execution-record.json",
        {
            "returncode": returncode,
            "elapsed_seconds": time.monotonic() - started,
            "wall_seconds_limit": protocol["wall_seconds"],
            "protocol_sha256": PROTOCOL_SHA256,
            "command": command,
            "isolation": "Seatbelt network/write denied and future/full-source reads denied",
        },
    )
    if returncode != 0:
        print("Actual child failed; log retained:", output)
        return 1
    report = analyze(output)
    print(report["status"], report["primary_future_only_metrics"])
    return 0 if report["passed"] else 2


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--child", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.child:
        child(args.output)
    else:
        raise SystemExit(run(args.output))


if __name__ == "__main__":
    main()
