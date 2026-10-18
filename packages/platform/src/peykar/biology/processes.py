"""Contextual multiparty declarations; no implicit kinetics or solver authorization."""

import json
from fractions import Fraction
from graphlib import CycleError, TopologicalSorter
from typing import Any

from peykar.infrastructure.artifacts import ArtifactStore
from peykar.infrastructure.database import Database
from peykar_core.contracts import (
    BiologicalProcessNetwork,
    MolecularStateFrame,
    MolecularStateFrameSpec,
    ProcessOwnershipPlan,
    ProcessOwnershipSpec,
    Reference,
    StoichiometricTerm,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound
from peykar_core.hashing import bytes_digest, digest
from peykar_core.units import convert


def coefficient(term: StoichiometricTerm) -> Fraction:
    return Fraction(term.numerator, term.denominator)


def fraction_text(value: Fraction) -> str:
    return str(value)


def unique(values: list[str], label: str) -> set[str]:
    if len(values) != len(set(values)):
        raise InvalidRequest(f"Duplicate {label}")
    return set(values)


class ProcessService:
    def __init__(self, db: Database, artifacts: ArtifactStore) -> None:
        self.db, self.artifacts = db, artifacts

    def register(
        self, network: BiologicalProcessNetwork, raw_sources: dict[str, bytes] | None = None
    ) -> BiologicalProcessNetwork:
        sources = unique([s.id for s in network.sources], "source identities")
        if raw_sources is not None and set(raw_sources) != sources:
            raise InvalidRequest("Exact declared source bytes are required")
        states = unique([s.id for s in network.states], "state identities")
        unique([s.external_id for s in network.states], "external state identities")
        unique([r.id for r in network.reactions], "reaction identities")
        unique([r.external_id for r in network.reactions], "external reaction identities")
        for source in network.sources:
            raw = (
                raw_sources[source.id]
                if raw_sources is not None
                else self.artifacts.get(source.raw_digest)
            )
            if not 0 < len(raw) <= 10 * 1024**2 or bytes_digest(raw) != source.raw_digest:
                raise InvalidRequest("Process source size or digest differs")
        for state in network.states:
            if state.source_id not in sources or state.species not in (
                None,
                network.context.species,
            ):
                raise InvalidRequest("State source/species differs from its network context")
            if state.components and state.kind != "complex":
                raise InvalidRequest("Only complex states have stoichiometric components")
            if state.alternatives and state.kind != "set":
                raise InvalidRequest("Only set states have alternative members")
            if state.kind == "complex" and not state.components:
                raise InvalidRequest("A complex requires explicit components")
            if state.kind == "set" and not state.alternatives:
                raise InvalidRequest("A set requires explicit alternatives")
            unique([v.id for v in state.locations], "state locations")
            self._terms(state.components, states)
            if not unique(list(state.alternatives), "alternative members") <= states:
                raise InvalidRequest("Unknown alternative state")
        try:
            tuple(
                TopologicalSorter(
                    {
                        s.id: {c.state_id for c in s.components} | set(s.alternatives)
                        for s in network.states
                    }
                ).static_order()
            )
        except CycleError as error:
            raise InvalidRequest("Cyclic molecular composition/alternatives") from error
        for reaction in network.reactions:
            if reaction.source_id not in sources:
                raise InvalidRequest("Unknown reaction source")
            self._terms(reaction.inputs, states)
            self._terms(reaction.outputs, states)
            unique([m.state_id + ":" + m.role for m in reaction.modifiers], "reaction modifiers")
            if any(m.state_id not in states for m in reaction.modifiers):
                raise InvalidRequest("Unknown modifier state")
        if raw_sources is not None:
            for raw in raw_sources.values():
                self.artifacts.put(raw)
        self.db.register("process_network", network.id, network.version, network.model_dump_json())
        return network

    @staticmethod
    def _terms(terms: tuple[StoichiometricTerm, ...], states: set[str]) -> None:
        if not unique([t.state_id for t in terms], "stoichiometric participants") <= states:
            raise InvalidRequest("Unknown stoichiometric state")
        if any(
            t.numerator != coefficient(t).numerator or t.denominator != coefficient(t).denominator
            for t in terms
        ):
            raise InvalidRequest("Stoichiometric fractions must be reduced")

    def get(self, reference: Reference) -> BiologicalProcessNetwork:
        return BiologicalProcessNetwork.model_validate_json(
            self.db.object("process_network", reference.id, reference.version)
        )

    def discover(self, limit: int = 20, offset: int = 0) -> tuple[BiologicalProcessNetwork, ...]:
        return tuple(
            self.get(Reference(id=r["id"], version=r["version"]))
            for r in self.db.objects("process_network", limit=limit, offset=offset)
        )

    def source(self, reference: Reference, source_id: str) -> bytes:
        network = self.get(reference)
        source = next((s for s in network.sources if s.id == source_id), None)
        if source is None:
            raise NotFound("Source is absent from the process network")
        return self.artifacts.get(source.raw_digest)

    def stoichiometry(self, reference: Reference) -> dict[str, Any]:
        network = self.get(reference)
        states = sorted(network.states, key=lambda s: s.id)
        reactions = sorted(network.reactions, key=lambda r: r.id)
        rows = []
        for state in states:
            values = []
            for reaction in reactions:
                value = sum(
                    (coefficient(t) for t in reaction.outputs if t.state_id == state.id), Fraction()
                ) - sum(
                    (coefficient(t) for t in reaction.inputs if t.state_id == state.id), Fraction()
                )
                values.append(fraction_text(value))
            rows.append(values)
        return {
            "network": reference.model_dump(mode="json"),
            "network_digest": digest(network),
            "state_ids": [s.id for s in states],
            "reaction_ids": [r.id for r in reactions],
            "net_coefficients": rows,
            "interpretation": (
                "Exact rational output minus input coefficients. Modifiers are separate. "
                "No rate law, elemental balance or biological approval is inferred."
            ),
        }

    def own(self, spec: ProcessOwnershipSpec) -> ProcessOwnershipPlan:
        network = self.get(spec.network)
        if spec.context != network.context:
            raise InvalidRequest("Ownership context differs from the network")
        unique([a.owner_id for a in spec.assignments], "process owners")
        covered: list[str] = []
        model_digests = {}
        for assignment in spec.assignments:
            covered.extend(assignment.reaction_ids)
            kind = {"reference": "model", "native": "native_model", "cellml": "cellml_model"}[
                assignment.model_kind
            ]
            payload = json.loads(
                self.db.object(kind, assignment.model.id, assignment.model.version)
            )
            context = payload["context"] if kind == "model" else payload["spec"]["context"]
            if context != spec.context.model_dump(mode="json"):
                raise InvalidRequest("Owner model context differs from the network")
            model_digests[assignment.owner_id] = digest(payload)
        covered.extend(e.reaction_id for e in spec.exclusions)
        if unique(covered, "process ownership (including exclusions)") != {
            r.id for r in network.reactions
        }:
            raise InvalidRequest("Every network reaction needs one owner or explicit exclusion")
        plan = ProcessOwnershipPlan(
            spec=spec, network_digest=digest(network), model_digests=model_digests
        )
        self.db.register("process_ownership", spec.id, spec.version, plan.model_dump_json())
        return plan

    def ownership(self, reference: Reference) -> ProcessOwnershipPlan:
        plan = ProcessOwnershipPlan.model_validate_json(
            self.db.object("process_ownership", reference.id, reference.version)
        )
        if digest(self.get(plan.spec.network)) != plan.network_digest:
            raise Conflict("Process ownership network lineage differs")
        for assignment in plan.spec.assignments:
            kind = {"reference": "model", "native": "native_model", "cellml": "cellml_model"}[
                assignment.model_kind
            ]
            if (
                digest(
                    json.loads(self.db.object(kind, assignment.model.id, assignment.model.version))
                )
                != plan.model_digests[assignment.owner_id]
            ):
                raise Conflict("Process owner model lineage differs")
        return plan

    def frame(self, spec: MolecularStateFrameSpec) -> MolecularStateFrame:
        network = self.get(spec.network)
        if spec.context != network.context:
            raise InvalidRequest("State frame context differs from the network")
        states = {s.id: s for s in network.states}
        if not unique([v.state_id for v in spec.values], "state frame values") <= set(states):
            raise InvalidRequest("Unknown state frame value")
        if spec.time.value < 0:
            raise InvalidRequest("State frame time must be nonnegative")
        convert(spec.time, "second")
        for value in spec.values:
            if value.quantity.value < 0:
                raise InvalidRequest("State values must be nonnegative")
            if states[value.state_id].kind == "set":
                raise InvalidRequest(
                    "Alternative sets are not resolved measurable molecular states"
                )
            if value.kind == "count":
                if (
                    value.quantity.unit != "count"
                    or not value.quantity.value.is_integer()
                    or value.quantity.value > 9007199254740991
                ):
                    raise InvalidRequest("Molecular counts require integral values in count units")
            else:
                convert(value.quantity, "mole" if value.kind == "amount" else "mole/liter")
        record = MolecularStateFrame(spec=spec, network_digest=digest(network))
        self.db.register("molecular_frame", spec.id, spec.version, record.model_dump_json())
        return record

    def get_frame(self, reference: Reference) -> MolecularStateFrame:
        frame = MolecularStateFrame.model_validate_json(
            self.db.object("molecular_frame", reference.id, reference.version)
        )
        if digest(self.get(frame.spec.network)) != frame.network_digest:
            raise Conflict("State frame network lineage differs")
        return frame

    def records(
        self, kind: str, limit: int = 20, offset: int = 0
    ) -> tuple[ProcessOwnershipPlan | MolecularStateFrame, ...]:
        return tuple(
            (self.ownership if kind == "process_ownership" else self.get_frame)(
                Reference(id=r["id"], version=r["version"])
            )
            for r in self.db.objects(kind, limit=limit, offset=offset)
        )
