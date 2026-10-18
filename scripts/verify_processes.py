"""Replay the retained Reactome projection in a new independent workspace, offline."""

import argparse
import json
from pathlib import Path

from peykar.biology.processes import ProcessService
from peykar.biology.reactome_processes import read_bundle
from peykar.simulation.service import SimulationService
from peykar_core.contracts import MolecularStateFrameSpec, ProcessOwnershipSpec, Reference
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, digest


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    network, sources = read_bundle(Path("biological_libraries/processes/reactome-egfr/bundle.json"))
    svc = SimulationService(args.output / "data")
    service = ProcessService(svc.db, svc.artifacts)
    service.register(network, sources)
    reference = Reference(id=network.id, version=network.version)
    source_checks = {s.id: bytes_digest(service.source(reference, s.id)) for s in network.sources}
    assert source_checks == {s.id: s.raw_digest for s in network.sources}
    matrix = service.stoichiometry(reference)

    def net(state: str, reaction: str) -> str:
        return str(
            matrix["net_coefficients"][matrix["state_ids"].index(state)][
                matrix["reaction_ids"].index(reaction)
            ]
        )

    assert net("state.9624419", "reaction.177922") == "-2"
    assert net("state.113592", "reaction.177934") == "-12"
    assert net("state.29370", "reaction.177934") == "12"
    exclusions = tuple(
        {
            "reaction_id": r.id,
            "reason": "No applicable executable model admitted for this exact process context.",
        }
        for r in network.reactions
    )
    ownership = service.own(
        ProcessOwnershipSpec(
            id="ownership.egfr-unimplemented",
            version="1.0.0",
            network=reference,
            context=network.context,
            exclusions=exclusions,
        )
    )
    assert not ownership.execution_authorized and not ownership.spec.assignments
    frame = service.frame(
        MolecularStateFrameSpec(
            id="state-frame.synthetic-egfr",
            version="1.0.0",
            network=reference,
            context=network.context,
            instance_id="synthetic.egfr-1",
            origin="synthetic",
            time={"value": 0.0, "unit": "second"},
            values=(
                {
                    "state_id": "state.179837",
                    "kind": "amount",
                    "quantity": {"value": 1.0, "unit": "nanomole"},
                },
            ),
            evidence=("Explicit synthetic software replay fixture",),
            limitations=("Not measured data, initialized solver state or biological validation.",),
        )
    )
    rejected = []
    overlapping = ownership.spec.model_dump(mode="json")
    overlapping["exclusions"].append(overlapping["exclusions"][0])
    try:
        service.own(ProcessOwnershipSpec.model_validate(overlapping))
    except InvalidRequest:
        rejected.append("duplicate-process-ownership")
    bad_frame = frame.spec.model_dump(mode="json")
    bad_frame["values"][0]["state_id"] = "state.9624194"
    try:
        service.frame(MolecularStateFrameSpec.model_validate(bad_frame))
    except InvalidRequest:
        rejected.append("unresolved-alternative-state-count")
    bad_frame = frame.spec.model_dump(mode="json")
    bad_frame["values"][0]["quantity"]["value"] = 2.0
    try:
        service.frame(MolecularStateFrameSpec.model_validate(bad_frame))
    except Conflict:
        rejected.append("immutable-state-frame-overwrite")
    assert len(rejected) == 3
    reopened = ProcessService(svc.db, svc.artifacts)
    assert reopened.get(reference) == network
    assert reopened.get_frame(Reference(id=frame.spec.id, version=frame.spec.version)) == frame
    assert (
        reopened.ownership(Reference(id=ownership.spec.id, version=ownership.spec.version))
        == ownership
    )
    result = {
        "status": "passed",
        "network": reference.model_dump(mode="json"),
        "network_digest": digest(network),
        "source_hashes": source_checks,
        "states": len(network.states),
        "reactions": len(network.reactions),
        "stoichiometry": matrix,
        "ownership": ownership.model_dump(mode="json"),
        "synthetic_state_frame": frame.model_dump(mode="json"),
        "rejected_cases": rejected,
        "reopened_all_declarations": True,
        "interpretation": (
            "Engineering data-provenance verification, not biological or kinetic validation."
        ),
    }
    (args.output / "verification.json").write_text(json.dumps(result, indent=2) + "\n")
    print(
        f"Passed: {len(source_checks)} sources, {len(network.states)} states, "
        f"{len(network.reactions)} reactions; exact coefficients, exclusions, units and reopen."
    )


if __name__ == "__main__":
    main()
