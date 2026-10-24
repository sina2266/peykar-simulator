"""Reviewed read-only inverse audit; no model execution or validation promotion."""

import argparse
import json
from pathlib import Path

import peykar.validation.cornea_compatibility as solver
import peykar.validation.cornea_evidence as parser
from peykar.validation.cornea_compatibility import (
    CircularAreaObservation,
    shared_front_feasibility,
)
from peykar.validation.cornea_evidence import (
    rabbit_brush_aggregates,
    rabbit_wound_observations,
)
from peykar_core.hashing import bytes_digest, canonical_json

PROTOCOL = Path("experiments/vcornea/repair-front/shared-front-audit/protocol-1.0.0.json")
PROTOCOL_SHA256 = "5a66ae5734e436acaa9e3c919024ba3d4c20fb737db6c168f85834b064570949"
PBS_SOURCE = Path("experiments/data/cornea-evidence-20261024/raw/rabbit-figure2.xlsx")
BRUSH_SOURCE = Path("experiments/vcornea/repair-front/chung2021-manuscript.xml")


def audit() -> dict:
    if bytes_digest(PROTOCOL.read_bytes()) != PROTOCOL_SHA256:
        raise ValueError("The frozen shared-front hypothesis/protocol changed")
    protocol = json.loads(PROTOCOL.read_bytes())
    for path, expected in [
        (PBS_SOURCE, protocol["experimental_sources"]["pbs2017_raw_xlsx_sha256"]),
        (BRUSH_SOURCE, protocol["experimental_sources"]["challenge2021_raw_xml_sha256"]),
        (Path(solver.__file__), protocol["solver_sha256"]),
        (Path(parser.__file__), protocol["parser_sha256"]),
    ]:
        if bytes_digest(path.read_bytes()) != expected:
            raise ValueError(f"Frozen original observation/implementation changed: {path.name}")
    pbs = [
        row
        for row in rabbit_wound_observations(PBS_SOURCE.read_bytes())
        if row["group"] == protocol["pbs2017_group"]
    ]
    brush = rabbit_brush_aggregates(BRUSH_SOURCE.read_bytes())["observations"]
    common = sorted({r["hour"] for r in pbs} & {r["hour"] for r in brush})
    if common != protocol["shared_observation_hours"]:
        raise ValueError("Use every exact shared time, without interpolation or selection")
    results = []
    for hour in common:
        animals = [r for r in pbs if r["hour"] == hour]
        if sorted(r["replicate"] for r in animals) != protocol["pbs2017_actual_rabbits"]:
            raise ValueError("Preserve all actual PBS animals without duplicate IDs")
        control = CircularAreaObservation(
            "2017 Control mean",
            protocol["pbs2017_nominal_radius_um"],
            sum(r["wound_area_fraction"] for r in animals) / len(animals),
            protocol["pbs2017_rounding_fraction"],
        )
        groups = [r for r in brush if r["hour"] == hour]
        if sorted(r["group"] for r in groups) != sorted(protocol["challenge2021_groups"]):
            raise ValueError("Keep every actual removal method at the shared time")
        if any(r["kind"] != "numeric_aggregate" for r in groups):
            raise ValueError("Do not convert categorical healing to a numeric observation")
        observations = [control] + [
            CircularAreaObservation(
                f"2021 {r['group']}",
                protocol["challenge2021_nominal_radius_um"],
                r["mean_fraction"],
                protocol["published_table_rounding_fraction"],
            )
            for r in groups
        ]
        joint = shared_front_feasibility(
            tuple(observations), maximum_error_fraction=protocol["maximum_error_fraction"]
        )
        pairwise = [
            shared_front_feasibility(
                (control, observation),
                maximum_error_fraction=protocol["maximum_error_fraction"],
            )
            for observation in observations[1:]
        ]
        results.append({"hour": hour, "joint": joint, "pairwise": pairwise})
    return {
        "schema_version": "1.0.0",
        "class": protocol["class"],
        "protocol_sha256": PROTOCOL_SHA256,
        "operator_sha256": bytes_digest(Path(__file__).read_bytes()),
        "solver_sha256": protocol["solver_sha256"],
        "parser_sha256": protocol["parser_sha256"],
        "experimental_sources": protocol["experimental_sources"],
        "maximum_error_fraction": protocol["maximum_error_fraction"],
        "shared_time_results": results,
        "shared_hypothesis_feasible_at_all_common_times": all(
            r["joint"]["feasible"] for r in results
        ),
        "lower_bound_on_any_shared_front_maximum_error_fraction": max(
            r["joint"]["optimistic_minimum_maximum_error_fraction"] for r in results
        ),
        "model_execution_performed": False,
        "diagnostic_inverse_fit": True,
        "kinetic_parameters_fitted": False,
        "forward_prediction_or_validation": False,
        "independent_biological_validation": False,
        "limitations": protocol["limitations"],
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    report = audit()
    args.output.mkdir(parents=True, exist_ok=False)
    (args.output / "report.json").write_text(canonical_json(report))
    (args.output / "frozen-protocol.json").write_bytes(PROTOCOL.read_bytes())
    (args.output / "executed-operator.py.txt").write_bytes(Path(__file__).read_bytes())
    (args.output / "executed-solver.py.txt").write_bytes(Path(solver.__file__).read_bytes())
    print(
        "Common-front hypothesis feasible:",
        report["shared_hypothesis_feasible_at_all_common_times"],
        "minimum required maximum error at least:",
        report["lower_bound_on_any_shared_front_maximum_error_fraction"],
    )


if __name__ == "__main__":
    main()
