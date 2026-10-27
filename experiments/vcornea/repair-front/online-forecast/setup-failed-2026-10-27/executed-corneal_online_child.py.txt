"""Reviewed offline prefix-only corneal forecast child; never an API entry point."""

import argparse
import hashlib
import json
import math
import platform
import socket
import time
from pathlib import Path

import peykar.model_learning.corneal_online as implementation
from peykar.model_learning.corneal_online import forecast_corneal_radius_trend


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write(path: Path, value: object) -> None:
    with path.open("x") as stream:
        json.dump(value, stream, sort_keys=True, indent=2, allow_nan=False)
        stream.write("\n")


def predict(request: dict) -> dict:
    """All estimators receive the same prefix; target values are absent."""
    if set(request) != {"series_id", "history", "origin_hour", "target_hour", "initial_radius_um"}:
        raise ValueError("Require only prefix observations, schedule and initial geometry")
    history = request["history"]
    if any(set(row) != {"hour", "area_fraction", "eligible_for_conditioning"} for row in history):
        raise ValueError("Unexpected history fields, including possible future outcomes")
    observed = [(r["hour"], r["area_fraction"]) for r in history if r["eligible_for_conditioning"]]
    origin, target = request["origin_hour"], request["target_hour"]
    local = forecast_corneal_radius_trend(
        history=observed, origin_hour=origin, future_hours=[target]
    )
    if (
        any(r["hour"] > origin for r in history)
        or any(b["hour"] <= a["hour"] for a, b in zip(history, history[1:], strict=False))
        or any(r["eligible_for_conditioning"] is not True for r in history)
    ):
        raise ValueError("Require causal ordered actual-observation prefix")
    r0 = request["initial_radius_um"]
    if isinstance(r0, bool) or not isinstance(r0, (int, float)) or not math.isfinite(r0) or r0 <= 0:
        raise ValueError("Require declared positive initial radius for the fixed prior comparator")
    last = observed[-1][1]
    # The first ACTUAL numeric point is4h in2021; derived0h normalization is not used.
    first_hour, first_q = observed[0]
    baseline_slope = (math.sqrt(last) - math.sqrt(first_q)) / (origin - first_hour)
    return {
        "series_id": request["series_id"],
        "local_radius_trend": local,
        "forecasts": {
            "local_radius_trend": local["predictions"][0]["area_fraction"],
            "persistence": last,
            "first_to_current_radius": max(
                0.0, math.sqrt(last) + baseline_slope * (target - origin)
            )
            ** 2,
            "crosson1986_initial_only": (max(0.0, r0 - 64 * max(0.0, target - 5.5)) / r0) ** 2,
        },
    }


def run(job: Path) -> None:
    started = time.monotonic()
    protocol_path = job / "frozen-protocol.json"
    protocol = json.loads(protocol_path.read_bytes())
    if digest(Path(implementation.__file__)) != protocol["method_sha256"]:
        raise ValueError("Frozen prefix method source changed")
    if digest(Path(__file__)) != protocol["child_sha256"]:
        raise ValueError("Frozen child changed")
    controls = json.loads((job / "isolation-controls.json").read_bytes())
    denied = []
    for name in controls["forbidden_read_probes"]:
        try:
            Path(name).read_bytes()
        except PermissionError:
            denied.append(name)
        else:
            raise RuntimeError("Actual source/future read denial is not enforced")
    try:
        with Path(controls["forbidden_write_probe"]).open("x") as stream:
            stream.write("isolation failure\n")
    except PermissionError:
        pass
    else:
        raise RuntimeError("Actual write denial is not enforced")
    with socket.socket() as connection:
        try:
            connection.connect(("127.0.0.1", 9))
        except PermissionError:
            pass
        else:
            raise RuntimeError("Actual network denial is not enforced")
    prediction = predict(json.loads((job / "prefix.json").read_bytes()))
    write(job / "prediction.json", prediction)
    write(
        job / "child-receipt.json",
        {
            "protocol_sha256": digest(protocol_path),
            "prefix_sha256": digest(job / "prefix.json"),
            "prediction_sha256": digest(job / "prediction.json"),
            "method_sha256": protocol["method_sha256"],
            "child_sha256": protocol["child_sha256"],
            "verified_denied_read_probes": denied,
            "write_denial_verified": True,
            "network_denial_verified": True,
            "python_version": platform.python_version(),
            "elapsed_seconds": time.monotonic() - started,
            "future_values_in_input": False,
            "training_or_parameter_search": False,
        },
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--job", type=Path, required=True)
    run(parser.parse_args().job)
