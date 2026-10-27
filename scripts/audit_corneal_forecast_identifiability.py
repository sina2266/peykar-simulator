"""Prefix-only retrospective precision audit; never reads future measured areas."""

import argparse
import json
from pathlib import Path

from peykar.validation.cornea_front_identifiability import corneal_front_identifiability
from peykar_core.hashing import bytes_digest, canonical_json

BASE = Path("experiments/vcornea/repair-front/conditioned-forecast")
PROTOCOL = BASE / "identifiability-protocol-1.0.0.json"
PROTOCOL_SHA256 = "ba25bcf8064c2d1ca147ff3c79919635b09de09bbb4e817ce0464333ebac9a72"
SOURCE = Path("packages/platform/src/peykar/validation/cornea_front_identifiability.py")


def audit() -> dict:
    if bytes_digest(PROTOCOL.read_bytes()) != PROTOCOL_SHA256:
        raise ValueError("Frozen diagnostic protocol changed")
    protocol = json.loads(PROTOCOL.read_bytes())
    raw = (BASE / "ijms2019-control-prefix.json").read_bytes()
    if (
        bytes_digest(raw) != protocol["prefix_sha256"]
        or bytes_digest(SOURCE.read_bytes()) != protocol["solver_sha256"]
    ):
        raise ValueError("Frozen prefix/solver changed")
    prefix = json.loads(raw)
    if (
        prefix["role"] != "conditioning_input_only"
        or prefix["unit"] != "millimeter ** 2"
        or [r["hour"] for r in prefix["observations"]] != [0, 24]
    ):
        raise ValueError("Only the declared two actual prefix observations are admitted")
    baseline, anchor = prefix["observations"]
    report = corneal_front_identifiability(
        baseline_area_mm2=baseline["mean_area_mm2"],
        anchor_area_mm2=anchor["mean_area_mm2"],
        anchor_hour=24,
        maximum_lag_hours=protocol["maximum_lag_hours"],
        maximum_speed_um_per_hour=protocol["maximum_speed_um_per_hour"],
        future_hours=protocol["future_hours_since_injury"],
    )
    return {
        **report,
        "protocol_sha256": PROTOCOL_SHA256,
        "prefix_sha256": protocol["prefix_sha256"],
        "solver_sha256": protocol["solver_sha256"],
        "operator_sha256": bytes_digest(Path(__file__).read_bytes()),
        "bound_basis": protocol["bound_basis"],
        "next_evidence": (
            "Additional post-injury motion measurement or independently evidenced lag/rate; "
            "still requires separate future testing and model-family applicability review."
        ),
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    result = audit()
    args.output.mkdir(parents=True, exist_ok=False)
    (args.output / "report.json").write_text(canonical_json(result))
    (args.output / "frozen-protocol.json").write_bytes(PROTOCOL.read_bytes())
    (args.output / "executed-operator.py.txt").write_bytes(Path(__file__).read_bytes())
    (args.output / "executed-solver.py.txt").write_bytes(SOURCE.read_bytes())
    print(
        "abstain_from_claiming_target_precision",
        result["abstain_from_claiming_target_precision"],
        "optimistic_minimax_rmse_bound",
        result["optimistic_minimax_point_rmse_lower_bound_fraction"],
    )
