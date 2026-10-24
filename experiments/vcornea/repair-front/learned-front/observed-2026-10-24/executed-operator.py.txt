"""Reviewed offline calibration and frozen corneal tests; no API execution."""

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

import peykar.model_learning.corneal_front as learning
import peykar_engines.corneal_learned_front as implementation
from peykar.validation.cornea_evidence import rabbit_brush_aggregates
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
from peykar_engines.corneal_learned_front import CornealLearnedFrontEngine

BASE = Path("experiments/vcornea/repair-front/learned-front")
PROTOCOL = BASE / "learning-protocol-1.0.0.json"
PROTOCOL_SHA256 = "df8a95a48f8a43947720e0c7267f6b49a3ff7fc8d46fc0e58bb60a2d4ac7339b"
LEARNER = Path("packages/platform/src/peykar/model_learning/corneal_front.py")
ENGINE = Path("packages/engines/src/peykar_engines/corneal_learned_front.py")
PARSER = Path("packages/platform/src/peykar/validation/cornea_evidence.py")
TEMPLATE = Path("model_packages/cornea/projected-repair/1.0.0/model.json")


def write(path: Path, value: object) -> None:
    with path.open("x") as stream:
        stream.write(canonical_json(value))


def read_protocol() -> dict:
    if bytes_digest(PROTOCOL.read_bytes()) != PROTOCOL_SHA256:
        raise ValueError("Frozen learning/test protocol changed")
    protocol = json.loads(PROTOCOL.read_bytes())
    for path, key in [(LEARNER, "learner_sha256"), (ENGINE, "engine_sha256")]:
        if bytes_digest(path.read_bytes()) != protocol[key]:
            raise ValueError("Frozen scientific implementation changed")
    if bytes_digest(Path(learning.__file__).read_bytes()) != protocol["learner_sha256"]:
        raise ValueError("Imported learner differs from reviewed source")
    if bytes_digest(Path(implementation.__file__).read_bytes()) != protocol["engine_sha256"]:
        raise ValueError("Imported engine differs from reviewed source")
    if learning.corneal_learning_algorithm() != protocol["algorithm"]:
        raise ValueError("Learning algorithm differs from frozen protocol")
    return protocol


def checked_json(path: Path, digest: str) -> object:
    raw = path.read_bytes()
    if bytes_digest(raw) != digest:
        raise ValueError(f"Input identity differs: {path}")
    return json.loads(raw)


def make_model(protocol: dict, fit: dict, case: str) -> ModelPackage:
    data = checked_json(TEMPLATE, protocol["template_sha256"])
    spec = protocol["cases"][case]
    parameters = fit["parameters"]
    data.update(
        id=spec["model_id"],
        title="Empirical rabbit epithelial repair; learned SDP endpoint kinetics",
        engine=CornealLearnedFrontEngine.id,
        engine_version=CornealLearnedFrontEngine.version,
        source="python:peykar_engines.corneal_learned_front",
        source_digest=protocol["engine_sha256"],
        capabilities=["empirical_dose_interpolation", "deterministic", "offline_operator"],
    )
    data["context"]["conditions"] = ["mechanical epithelial debridement", spec["context"]]
    data["validity"].update(
        status="unvalidated",
        context=data["context"],
        evidence=["https://doi.org/10.1371/journal.pone.0188154"],
        limitations=[
            "Four empirical parameters calibrated on six actual Control/2%SDP rabbits.",
            "Calibration is not validation; no causal drug, substrate or clinical claim.",
            "Linear concentration interpolation; no post-injury measurement as input.",
            "Independent tests are separate immutable evidence, including failures.",
            "Not registered in the default API/Worker catalog.",
        ],
    )
    data["parameters"] = [
        {"name": name, "default": {"value": value, "unit": unit}, "minimum": lo, "maximum": hi}
        for name, value, unit, lo, hi in [
            ("initial_radius", spec["radius_um"], "micrometer", 1, 10000),
            ("concentration", spec["concentration_percent"], "percent", 0, 2),
            ("rate0", parameters["rate0_um_per_h"], "micrometer/hour", 0, 200),
            ("rate2", parameters["rate2_um_per_h"], "micrometer/hour", 0, 200),
            ("lag0", parameters["lag0_h"], "hour", 0, 24),
            ("lag2", parameters["lag2_h"], "hour", 0, 24),
        ]
    ]
    return ModelPackage.model_validate(data)


def child(output: Path) -> None:
    protocol = read_protocol()
    started = time.monotonic()

    def checkpoint() -> None:
        if time.monotonic() - started > protocol["wall_seconds"]:
            raise TimeoutError("Offline learner exceeded its frozen wall limit")

    # The child can read training only; Seatbelt denies original and heldout files.
    denied_reads = []
    for path in protocol["isolation_probe_files"]:
        try:
            Path(path).read_bytes()
        except PermissionError:
            denied_reads.append(path)
        else:
            raise RuntimeError("Heldout-read isolation was not enforced")
    rows = checked_json(BASE / "data/training.json", protocol["training_sha256"])
    fit = learning.fit_corneal_front(
        rows,
        initial_radius_um=3500,
        calibration_source_sha256=protocol["experimental_source_sha256"],
        protocol_sha256=PROTOCOL_SHA256,
        provenance=protocol["training_provenance"],
        checkpoint=checkpoint,
        max_evaluations=protocol["algorithm"]["operator_evaluation_cap"],
    ).as_artifact()
    write(output / "learned-artifact.json", fit)
    engine = CornealLearnedFrontEngine()
    for case, spec in protocol["cases"].items():
        folder = output / case
        folder.mkdir()
        model = make_model(protocol, fit, case)
        plan = SimulationPlan(
            model=Reference(id=model.id, version=model.version),
            context=model.context,
            duration=Quantity(value=spec["duration_h"], unit="hour"),
            sample_interval=Quantity(value=0.25, unit="hour"),
            seed=0,
            budget=ExecutionBudget(max_samples=1000, wall_seconds=protocol["wall_seconds"]),
            purpose="Frozen offline empirical candidate; no post-injury test inputs",
        )
        result = engine.execute(model, plan, checkpoint)
        write(folder / "model.json", model.model_dump(mode="json"))
        write(folder / "plan.json", plan.model_dump(mode="json"))
        write(folder / "candidate.json", result.model_dump(mode="json"))
    checkpoint()
    artifacts = {
        p.relative_to(output).as_posix(): bytes_digest(p.read_bytes())
        for p in sorted(output.rglob("*.json"))
    }
    write(
        output / "child-execution.json",
        {
            "protocol_sha256": PROTOCOL_SHA256,
            "learner_sha256": protocol["learner_sha256"],
            "engine_sha256": protocol["engine_sha256"],
            "operator_sha256": bytes_digest(Path(__file__).read_bytes()),
            "uv_lock_sha256": bytes_digest(Path("uv.lock").read_bytes()),
            "python_version": platform.python_version(),
            "elapsed_seconds": time.monotonic() - started,
            "evaluation_count": fit["evaluation_count"],
            "artifacts": artifacts,
            "test_observations_read_by_child": False,
            "verified_denied_read_probes": denied_reads,
            "deterministic_plan_seed": 0,
            "seed_scope": "New deterministic bookkeeping, not an experimental random seed",
        },
    )


def metrics(errors: list[float]) -> dict:
    return {
        "rmse_fraction": math.sqrt(statistics.mean(e * e for e in errors)),
        "maximum_absolute_error_fraction": max(abs(e) for e in errors),
        "mean_error_fraction": statistics.mean(errors),
    }


def within(values: dict, protocol: dict) -> bool:
    return (
        values["rmse_fraction"] <= protocol["maximum_rmse_fraction"]
        and values["maximum_absolute_error_fraction"] <= protocol["maximum_absolute_error_fraction"]
    )


def analyze(output: Path) -> dict:
    protocol = read_protocol()
    record = json.loads((output / "execution-record.json").read_bytes())
    receipt = json.loads((output / "child-execution.json").read_bytes())
    if record["returncode"] != 0 or any(
        r["protocol_sha256"] != PROTOCOL_SHA256 for r in (record, receipt)
    ):
        raise ValueError("Only actual successful children with frozen inputs can be analyzed")
    for name, digest in receipt["artifacts"].items():
        checked_json(output / name, digest)
    if (
        bytes_digest((output / "frozen-protocol.json").read_bytes()) != PROTOCOL_SHA256
        or receipt["verified_denied_read_probes"] != protocol["isolation_probe_files"]
    ):
        raise ValueError("Frozen protocol/isolation evidence differs")
    fit = json.loads((output / "learned-artifact.json").read_bytes())
    if fit["protocol_sha256"] != PROTOCOL_SHA256:
        raise ValueError("Calibration protocol changed")
    predictions = {}
    for case in protocol["cases"]:
        folder = output / case
        model = ModelPackage.model_validate_json((folder / "model.json").read_bytes())
        if model != make_model(protocol, fit, case):
            raise ValueError("Candidate parameters differ from the frozen training artifact")
        plan = SimulationPlan.model_validate_json((folder / "plan.json").read_bytes())
        validate_plan(model, plan)
        result = SimulationResult.model_validate_json((folder / "candidate.json").read_bytes())
        spec = protocol["cases"][case]
        if (
            result.time.values != time_grid(plan)
            or plan.duration != Quantity(value=spec["duration_h"], unit="hour")
            or plan.sample_interval != Quantity(value=0.25, unit="hour")
            or result.seed != 0
            or result.time.unit != "second"
            or {s.name for s in result.outputs} != {p.name for p in model.ports}
        ):
            raise ValueError("Candidate grid/units/seed/ports differ from declared execution")
        series = next(s for s in result.outputs if s.name == "open_area_fraction")
        if series.unit != "dimensionless" or any(not 0 <= a <= 1 for a in series.values):
            raise ValueError("Invalid projected-area prediction")
        predictions[case] = dict(zip(result.time.values, series.values, strict=True))
    observed = checked_json(BASE / "data/heldout.json", protocol["heldout_sha256"])
    expected = {(r, h) for r in ["1", "2", "3"] for h in range(0, 55, 6)}
    identities = [(r["replicate"], r["hour"]) for r in observed]
    if (
        len(identities) != 30
        or set(identities) != expected
        or any(r["group"] != "0.5% SDP" for r in observed)
    ):
        raise ValueError("All three heldout animals and their complete observations required")
    heldout_rows = []
    individual = []
    baseline_parameters = learning.CornealFrontParameters(64, 64, 5.5, 5.5)
    for hour in range(6, 55, 6):
        mean = statistics.mean(r["wound_area_fraction"] for r in observed if r["hour"] == hour)
        predicted = predictions["heldout2017"][hour * 3600]
        baseline = learning.predict_open_area_fraction(
            baseline_parameters, initial_radius_um=3500, hour=hour, concentration_wv_percent=0.5
        )
        heldout_rows.append(
            {
                "hour": hour,
                "measured_mean_fraction": mean,
                "predicted_fraction": predicted,
                "error_fraction": predicted - mean,
                "unchanged1986_baseline_fraction": baseline,
                "baseline_error_fraction": baseline - mean,
            }
        )
    for replicate in ["1", "2", "3"]:
        errors = [
            predictions["heldout2017"][r["hour"] * 3600] - r["wound_area_fraction"]
            for r in observed
            if r["replicate"] == replicate and r["hour"] != 0
        ]
        individual.append({"replicate": replicate, **metrics(errors)})
    primary = metrics([r["error_fraction"] for r in heldout_rows])
    baseline = metrics([r["baseline_error_fraction"] for r in heldout_rows])
    improvement = baseline["rmse_fraction"] - primary["rmse_fraction"]
    heldout_pass = within(primary, protocol) and improvement >= protocol["minimum_rmse_improvement"]
    raw_path = Path("experiments/vcornea/repair-front/chung2021-manuscript.xml")
    if (
        bytes_digest(raw_path.read_bytes()) != protocol["challenge2021_sha256"]
        or bytes_digest(PARSER.read_bytes()) != protocol["experimental_parser_sha256"]
    ):
        raise ValueError("Independent experimental source/parser changed")
    challenge = []
    categories = []
    for group in ["Blade", "Ocu", "Amo"]:
        rows = []
        for item in rabbit_brush_aggregates(raw_path.read_bytes())["observations"]:
            if item["group"] != group:
                continue
            prediction = predictions["challenge2021"][item["hour"] * 3600]
            if item["kind"] == "categorical":
                categories.append({**item, "predicted_fraction": prediction})
            else:
                rows.append(
                    {
                        **item,
                        "predicted_fraction": prediction,
                        "error_fraction": prediction - item["mean_fraction"],
                    }
                )
        values = metrics([r["error_fraction"] for r in rows])
        challenge.append(
            {"group": group, "rows": rows, **values, "passed": within(values, protocol)}
        )
    constraints = checked_json(
        BASE / "fischak2017/constraints.json", protocol["fischak_constraints_sha256"]
    )
    fischak_rows = []
    for row in constraints["observations"]:
        prediction = predictions["fischak2017"][row["hour"] * 3600]
        fischak_rows.append(
            {
                "hour": row["hour"],
                "measured_ratio_lower_constraint": row["area_ratio_lower_bound"],
                "predicted_fraction": prediction,
                "minimum_possible_absolute_error": max(
                    0, row["area_ratio_lower_bound"] - prediction
                ),
            }
        )
    lower_errors = [r["minimum_possible_absolute_error"] for r in fischak_rows]
    lower_metrics = metrics(lower_errors)
    report = {
        "protocol_sha256": PROTOCOL_SHA256,
        "operator_sha256": bytes_digest(Path(__file__).read_bytes()),
        "class": protocol["class"],
        "fit_parameters": fit["parameters"],
        "heldout2017": {
            "primary_metrics": primary,
            "baseline_metrics": baseline,
            "rmse_improvement_fraction": improvement,
            "passed": heldout_pass,
            "rows": heldout_rows,
            "individual_diagnostics": individual,
            "actual_animals": 3,
            "independent_study": False,
            "blinded_or_previously_unseen_data": False,
        },
        "challenge2021": {
            "groups": challenge,
            "categorical_observations": categories,
            "passed": all(r["passed"] for r in challenge),
        },
        "fischak2017": {
            "rows": fischak_rows,
            "optimistic_error_lower_bounds": lower_metrics,
            "status": "rejected_by_conservative_constraints"
            if not within(lower_metrics, protocol)
            else "indeterminate_not_a_pass",
            "primary_numeric_mean_recovered": False,
            "independent_study": True,
        },
        "independent_biological_validation": False,
        "limitations": protocol["limitations"],
    }
    write(output / "report.json", report)
    return report


def run(output: Path) -> int:
    protocol = read_protocol()
    for key, path in protocol["admitted_inputs"].items():
        if bytes_digest(Path(path["path"]).read_bytes()) != path["sha256"]:
            raise ValueError(f"Input admission failed: {key}")
    if sys.platform != "darwin":
        raise RuntimeError("This operator requires its reviewed macOS Seatbelt isolation")
    output.mkdir(parents=True, exist_ok=False)
    (output / "frozen-protocol.json").write_bytes(PROTOCOL.read_bytes())
    (output / "executed-operator.py").write_bytes(Path(__file__).read_bytes())
    policy = (
        "(version 1) (allow default) (deny network*) (deny file-write*) "
        f"(allow file-write* (subpath {json.dumps(str(output.resolve()))})) "
        '(allow file-write* (literal "/dev/null")) '
        + " ".join(
            f"(deny file-read* (subpath {json.dumps(str(Path(p).resolve()))}))"
            for p in protocol["child_denied_paths"]
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
            "wall_limit_seconds": protocol["wall_seconds"],
            "protocol_sha256": PROTOCOL_SHA256,
            "command": command,
            "isolation": ("Network/write denied; heldout/raw/challenge reads denied by Seatbelt"),
        },
    )
    if returncode != 0:
        print("Actual child failed; original execution/log retained:", output)
        return 1
    report = analyze(output)
    print(
        "heldout2017",
        report["heldout2017"]["primary_metrics"],
        "passed",
        report["heldout2017"]["passed"],
    )
    print("challenge2021 passed", report["challenge2021"]["passed"])
    print("fischak2017", report["fischak2017"]["status"])
    rejected = (
        not report["heldout2017"]["passed"]
        or not report["challenge2021"]["passed"]
        or report["fischak2017"]["status"] == "rejected_by_conservative_constraints"
    )
    # A one-sided constraint can reject, but cannot certify independent accuracy.
    return 2 if rejected else 3


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
