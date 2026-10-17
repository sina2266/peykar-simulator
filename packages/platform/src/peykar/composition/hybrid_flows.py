"""Durable scalar feed-forward flows across reference and admitted native models."""

import json
import sqlite3
import uuid
from graphlib import CycleError, TopologicalSorter
from pathlib import Path
from typing import Any

from peykar.composition.compatibility import check_connection
from peykar.infrastructure.database import Database, now
from peykar.model_import.native_interfaces import NativeInterfaceService
from peykar.model_import.native_models import NativeModelService
from peykar.model_import.native_runtimes import NativeRuntimeService
from peykar.simulation.native_jobs import NativeJobService
from peykar.simulation.service import SimulationService
from peykar.simulation.terminal_values import terminal_values
from peykar_core.contracts import (
    FlowNodeRecord,
    FlowTransfer,
    FrozenFlowNode,
    FrozenHybridFlow,
    HybridFlowRunRecord,
    HybridFlowSpec,
    NativeRunManifest,
    ParameterValue,
    Quantity,
    Reference,
    RunManifest,
    ScientificPort,
    SimulationPlan,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound, PeykarError, Unsupported
from peykar_core.hashing import canonical_json, digest
from peykar_core.planning import time_grid
from peykar_core.units import convert

TERMINAL = {"succeeded", "failed", "cancelled"}


class HybridFlowService:
    def __init__(self, service: SimulationService) -> None:
        self.service, self.db, self.artifacts = service, service.db, service.artifacts
        self.native = NativeJobService(service)
        self.interfaces = NativeInterfaceService(service)
        with self.db.connection() as db:
            count = db.execute(
                "SELECT count(*) FROM sqlite_master WHERE type='table' "
                "AND name IN ('hybrid_runs','hybrid_nodes','hybrid_events')"
            ).fetchone()[0]
            if count != 3:
                db.executescript(
                    (
                        Path(__file__).parents[1] / "infrastructure/migrations/005_hybrid_flows.sql"
                    ).read_text()
                )

    @staticmethod
    def ports(node: FrozenFlowNode) -> dict[str, ScientificPort]:
        return (
            {p.port.name: p.port for p in node.native_interface.ports}
            if node.native_interface
            else {p.name: p for p in node.manifest.model_snapshot.ports}
            if isinstance(node.manifest, RunManifest)
            else {}
        )

    def freeze(self, spec: HybridFlowSpec) -> FrozenHybridFlow:
        try:
            existing = self.get(Reference(id=spec.id, version=spec.version))
        except NotFound:
            pass
        else:
            if existing.spec != spec:
                raise Conflict("Immutable flow version differs")
            return existing
        if len({n.id for n in spec.nodes}) != len(spec.nodes):
            raise InvalidRequest("Duplicate flow node IDs")
        if len(spec.nodes) > spec.budget.max_jobs:
            raise InvalidRequest("Flow exceeds job budget")
        nodes: dict[str, FrozenFlowNode] = {}
        contexts = set()
        samples = 0
        wall = 0.0
        size = len(canonical_json(spec).encode())
        for n in spec.nodes:
            if n.native_interface:
                interface = self.interfaces.get(n.native_interface)
                if interface.model != n.plan.model:
                    raise InvalidRequest("Interface must name the exact native model")
            else:
                interface = None
            if isinstance(n.plan, SimulationPlan):
                if interface:
                    raise InvalidRequest("Reference models use their declared model ports")
                manifest: RunManifest | NativeRunManifest = self.service.prepare(n.plan)
                contexts.add(canonical_json(self.service.models.get(n.plan.model).context))
                samples += len(time_grid(n.plan))
                wall += n.plan.budget.wall_seconds
            else:
                if interface is None:
                    raise InvalidRequest("Native flow nodes require an admitted interface")
                manifest = self.native.prepare(n.plan)
                contexts.add(canonical_json(manifest.model_snapshot.spec.context))
                samples += n.plan.intervals + 1 if n.plan.intervals is not None else 100001
                wall += n.plan.wall_seconds
            node = FrozenFlowNode(id=n.id, manifest=manifest, native_interface=interface)
            if any(p.shape != "scalar" for p in self.ports(node).values()):
                raise InvalidRequest("Flow ports must be scalar")
            nodes[n.id] = node
            size += len(canonical_json(node).encode())
            if (
                samples > spec.budget.max_total_samples
                or wall > spec.budget.max_total_wall_seconds
                or size > 4 * 1024**2
            ):
                raise InvalidRequest("Flow exceeds aggregate sample/wall/snapshot budgets")
        if len(contexts) != 1:
            raise InvalidRequest("Cross-context flows require a separate biological mapping")
        graph: dict[str, set[str]] = {n: set() for n in nodes}
        writes = set()
        for b in spec.bindings:
            if any(not e.strip() or len(e) > 2000 for e in b.evidence):
                raise InvalidRequest("Binding evidence must be bounded and nonblank")
            if b.source_node not in nodes or b.target_node not in nodes:
                raise InvalidRequest("Unknown binding node")
            source, target = nodes[b.source_node], nodes[b.target_node]
            source_port = self.ports(source).get(b.source_port)
            target_port = self.ports(target).get(b.target_port)
            if source_port is None or target_port is None:
                raise InvalidRequest("Unknown binding port")
            check_connection(source_port, target_port)
            if target_port.ownership != "write":
                raise InvalidRequest("Flow targets must explicitly declare write ownership")
            selector = b.target_port
            if isinstance(target.manifest, RunManifest):
                parameter = next(
                    (
                        p
                        for p in target.manifest.model_snapshot.parameters
                        if p.name == b.target_parameter
                    ),
                    None,
                )
                if parameter is None:
                    raise InvalidRequest("Reference binding requires an explicit target parameter")
                if any(p.name == parameter.name for p in target.manifest.plan.parameters):
                    raise InvalidRequest("Bound parameter already has an override")
                convert(Quantity(value=1, unit=target_port.unit), parameter.default.unit)
                selector = parameter.name
            elif b.target_parameter:
                raise InvalidRequest("Native input is selected by its admitted interface")
            key = (b.target_node, selector)
            if key in writes:
                raise InvalidRequest("Multiple writers target the same input")
            writes.add(key)
            graph[b.target_node].add(b.source_node)
        try:
            order = tuple(TopologicalSorter(graph).static_order())
        except CycleError as error:
            raise Unsupported(
                "Feedback needs a numerical solver; hybrid flow requires a DAG"
            ) from error
        frozen = FrozenHybridFlow(
            spec=spec,
            nodes=tuple(nodes[n] for n in order),
            order=order,
            sample_upper_bound=samples,
            total_wall_seconds_limit=wall,
        )
        payload = frozen.model_dump_json()
        if len(payload.encode()) > 4 * 1024**2:
            raise InvalidRequest("Frozen flow exceeds 4 MiB")
        self.db.register("hybrid_flow", spec.id, spec.version, payload)
        return frozen

    def get(self, ref: Reference) -> FrozenHybridFlow:
        frozen = FrozenHybridFlow.model_validate_json(
            self.db.object("hybrid_flow", ref.id, ref.version)
        )
        models = NativeModelService(self.db, self.artifacts)
        for n in frozen.nodes:
            m = n.manifest
            if isinstance(m, RunManifest):
                if (
                    self.service.models.get(m.plan.model) != m.model_snapshot
                    or self.service.models.dependencies(m.model_snapshot) != m.dependency_snapshots
                ):
                    raise Conflict("Frozen reference model/dependencies differ")
            elif (
                models.get(m.plan.model) != m.model_snapshot
                or models.profile(m.plan.profile) != m.profile_snapshot
                or NativeRuntimeService(self.db, self.artifacts).get(m.profile_snapshot.runtime)
                != m.runtime_snapshot
                or n.native_interface is None
                or self.interfaces.get(
                    Reference(id=n.native_interface.id, version=n.native_interface.version)
                )
                != n.native_interface
            ):
                raise Conflict("Frozen native model/interface/runtime differs")
        return frozen

    @staticmethod
    def _event(db: sqlite3.Connection, id: str, event: str, detail: Any = None) -> None:
        db.execute(
            "INSERT INTO hybrid_events(hybrid_run_id,occurred_at,event,detail) VALUES(?,?,?,?)",
            (id, now(), event, canonical_json(detail or {})),
        )

    def submit(self, ref: Reference, key: str | None = None) -> HybridFlowRunRecord:
        if key is not None and not 1 <= len(key) <= 128:
            raise InvalidRequest("Idempotency key requires 1..128 characters")
        frozen = self.get(ref)
        id = str(uuid.uuid4())
        stamp = now()
        with self.db.connection(immediate=True) as db:
            existing = (
                db.execute(
                    "SELECT id,flow_digest FROM hybrid_runs WHERE idempotency_key=?", (key,)
                ).fetchone()
                if key
                else None
            )
            if existing:
                if existing["flow_digest"] != digest(frozen):
                    raise Conflict("Flow idempotency key identifies another specification")
                id = existing["id"]
            else:
                db.execute(
                    "INSERT INTO hybrid_runs(id,flow_id,flow_version,flow_digest,idempotency_key,"
                    "status,created_at,updated_at) VALUES(?,?,?,?,?,'queued',?,?)",
                    (id, ref.id, ref.version, digest(frozen), key, stamp, stamp),
                )
                db.executemany(
                    "INSERT INTO hybrid_nodes(hybrid_run_id,node_id,ordinal,queue) VALUES(?,?,?,?)",
                    (
                        (
                            id,
                            n.id,
                            i,
                            "reference" if isinstance(n.manifest, RunManifest) else "native",
                        )
                        for i, n in enumerate(frozen.nodes)
                    ),
                )
                self._event(db, id, "submitted")
        return self.record(id)

    @staticmethod
    def _nodes(db: sqlite3.Connection, id: str) -> list[sqlite3.Row]:
        return db.execute(
            "SELECT h.*,COALESCE(h.reference_run_id,h.native_run_id) AS run_id,"
            "COALESCE(r.status,n.status) AS status "
            "FROM hybrid_nodes h LEFT JOIN runs r ON r.id=h.reference_run_id "
            "LEFT JOIN native_runs n ON n.id=h.native_run_id "
            "WHERE h.hybrid_run_id=? ORDER BY h.ordinal",
            (id,),
        ).fetchall()

    def record(self, id: str) -> HybridFlowRunRecord:
        with self.db.connection() as db:
            row = db.execute("SELECT * FROM hybrid_runs WHERE id=?", (id,)).fetchone()
            if not row:
                raise NotFound("Hybrid flow run not found")
            nodes = self._nodes(db, id)
        return HybridFlowRunRecord(
            id=id,
            flow=Reference(id=row["flow_id"], version=row["flow_version"]),
            flow_digest=row["flow_digest"],
            status=row["status"],
            cancel_requested=bool(row["cancel_requested"]),
            error=row["error"],
            created_at=row["created_at"],
            updated_at=row["updated_at"],
            nodes=tuple(
                FlowNodeRecord(
                    node_id=n["node_id"],
                    queue=n["queue"],
                    run_id=n["run_id"],
                    manifest_digest=n["manifest_digest"],
                    transfer_digest=n["transfer_digest"],
                    derivation=Reference(id=n["derivation_id"], version="1.0.0")
                    if n["derivation_id"]
                    else None,
                    status=n["status"] or ("skipped" if row["status"] in TERMINAL else "pending"),
                )
                for n in nodes
            ),
        )

    @staticmethod
    def _cancel_children(db: sqlite3.Connection, id: str) -> None:
        for n in HybridFlowService._nodes(db, id):
            if n["status"] not in ("queued", "running"):
                continue
            table = "runs" if n["queue"] == "reference" else "native_runs"
            row = db.execute(
                f"SELECT cancel_requested FROM {table} WHERE id=?", (n["run_id"],)
            ).fetchone()
            if row[0]:
                continue
            db.execute(
                f"UPDATE {table} SET cancel_requested=1,status=?,updated_at=? WHERE id=?",
                ("cancelled" if n["status"] == "queued" else "running", now(), n["run_id"]),
            )
            (Database._event if table == "runs" else NativeJobService._event)(
                db, n["run_id"], "cancel_requested"
            )

    def cancel(self, id: str) -> HybridFlowRunRecord:
        with self.db.connection(immediate=True) as db:
            row = db.execute(
                "SELECT status,cancel_requested FROM hybrid_runs WHERE id=?", (id,)
            ).fetchone()
            if not row:
                raise NotFound("Hybrid flow run not found")
            if row["status"] in ("succeeded", "failed"):
                raise Conflict("Completed flows cannot be cancelled")
            if not row["cancel_requested"]:
                db.execute(
                    "UPDATE hybrid_runs SET cancel_requested=1,updated_at=? WHERE id=?", (now(), id)
                )
                self._cancel_children(db, id)
                self._event(db, id, "cancel_requested")
        self.advance(id)
        return self.record(id)

    def _child(self, node: sqlite3.Row) -> RunManifest | NativeRunManifest:
        manifest = (
            self.service.db.manifest(node["run_id"])
            if node["queue"] == "reference"
            else self.native.manifest(node["run_id"])
        )
        if digest(manifest) != node["manifest_digest"]:
            raise Conflict("Flow child manifest integrity differs")
        return manifest

    def _transfers(
        self, frozen: FrozenHybridFlow, target: FrozenFlowNode, states: dict[str, sqlite3.Row]
    ) -> tuple[FlowTransfer, ...]:
        nodes = {n.id: n for n in frozen.nodes}
        transfers = []
        for b in frozen.spec.bindings:
            if b.target_node != target.id:
                continue
            source = nodes[b.source_node]
            state = states[b.source_node]
            manifest = self._child(state)
            source_port = self.ports(source)[b.source_port]
            target_port = self.ports(target)[b.target_port]
            if source.native_interface:
                p = next(p for p in source.native_interface.ports if p.port.name == b.source_port)
                value = self.interfaces.read(p, state["run_id"])
                source_digest = self.native.run(state["run_id"]).result_digest
            else:
                values = terminal_values(self.service, manifest, state["run_id"])
                if b.source_port not in values:
                    raise Conflict("Missing reference endpoint")
                raw = values[b.source_port]
                value = Quantity(value=convert(raw, source_port.unit), unit=source_port.unit)
                source_digest = self.service.db.run(state["run_id"]).result_digest
            if source_digest is None:
                raise Conflict("Source result identity is absent")
            selector = (
                b.target_parameter
                or next(
                    p.species_id
                    for p in target.native_interface.ports
                    if p.port.name == b.target_port
                )
                if target.native_interface
                else b.target_parameter
            )
            transfers.append(
                FlowTransfer(
                    source_node=b.source_node,
                    source_run_id=state["run_id"],
                    source_manifest_digest=digest(manifest),
                    source_result_digest=source_digest,
                    source_port=b.source_port,
                    target_port=b.target_port,
                    target_selector=str(selector),
                    source_quantity=value,
                    target_quantity=Quantity(
                        value=convert(value, target_port.unit), unit=target_port.unit
                    ),
                    evidence=b.evidence,
                )
            )
        return tuple(transfers)

    def advance(self, id: str) -> None:
        record = self.record(id)
        if record.status in TERMINAL:
            return
        frozen = self.get(record.flow)
        if digest(frozen) != record.flow_digest:
            raise Conflict("Frozen flow checksum differs")
        with self.db.connection(immediate=True) as db:
            row = db.execute("SELECT * FROM hybrid_runs WHERE id=?", (id,)).fetchone()
            if row["status"] in TERMINAL:
                return
            rows = self._nodes(db, id)
            states = {r["node_id"]: r for r in rows}
            if len(rows) != len(frozen.nodes) or any(
                r["node_id"] != n.id
                or r["ordinal"] != i
                or (r["queue"] == "reference") != isinstance(n.manifest, RunManifest)
                for i, (r, n) in enumerate(zip(rows, frozen.nodes, strict=False))
            ):
                raise Conflict("Flow membership differs")
            error = row["error"]
            if (
                any(r["status"] in ("failed", "cancelled") for r in rows)
                and not row["cancel_requested"]
            ):
                error = error or "node_failed"
            if row["cancel_requested"] or error:
                self._cancel_children(db, id)
            else:
                slots = frozen.spec.budget.max_in_flight - sum(
                    r["status"] in ("queued", "running") for r in rows
                )
                for node in frozen.nodes:
                    if slots <= 0:
                        break
                    if states[node.id]["run_id"]:
                        continue
                    parents = [
                        b.source_node for b in frozen.spec.bindings if b.target_node == node.id
                    ]
                    if not all(states[p]["status"] == "succeeded" for p in parents):
                        continue
                    transfers = self._transfers(frozen, node, states)
                    manifest = node.manifest.model_copy(
                        update={"run_id": str(uuid.uuid4()), "created_at": now()}
                    )
                    derivation_id = None
                    if isinstance(manifest, RunManifest):
                        overrides = manifest.plan.parameters + tuple(
                            ParameterValue(name=t.target_selector, quantity=t.target_quantity)
                            for t in transfers
                        )
                        plan = manifest.plan.model_copy(update={"parameters": overrides})
                        self.service.validate(plan)
                        manifest = manifest.model_copy(update={"plan": plan})
                        child_id = Database.enqueue(db, manifest, None)
                        column = "reference_run_id"
                    else:
                        if transfers:
                            if node.native_interface is None:
                                raise InvalidRequest("Native input interface missing")
                            package, receipt, _ = self.interfaces.derive(
                                manifest.model_snapshot, node.native_interface, transfers
                            )
                            derivation_id = package.spec.id
                            Database.register_in_transaction(
                                db,
                                "native_model",
                                package.spec.id,
                                package.spec.version,
                                package.model_dump_json(),
                            )
                            Database.register_in_transaction(
                                db,
                                "native_derivation",
                                package.spec.id,
                                "1.0.0",
                                receipt.model_dump_json(),
                            )
                            plan_native = manifest.plan.model_copy(
                                update={
                                    "model": Reference(
                                        id=package.spec.id, version=package.spec.version
                                    )
                                }
                            )
                            manifest = manifest.model_copy(
                                update={"plan": plan_native, "model_snapshot": package}
                            )
                        child_id = NativeJobService.enqueue(db, manifest, None)
                        column = "native_run_id"
                    transfer_digest = self.artifacts.put(
                        canonical_json([t.model_dump(mode="json") for t in transfers]).encode()
                    )
                    db.execute(
                        f"UPDATE hybrid_nodes SET {column}=?,manifest_digest=?,transfer_digest=?,"
                        "derivation_id=? WHERE hybrid_run_id=? AND node_id=?",
                        (child_id, digest(manifest), transfer_digest, derivation_id, id, node.id),
                    )
                    self._event(
                        db,
                        id,
                        "node_submitted",
                        {
                            "node_id": node.id,
                            "run_id": child_id,
                            "transfer_digest": transfer_digest,
                        },
                    )
                    slots -= 1
            states_after = self._nodes(db, id)
            active = any(r["status"] in ("queued", "running") for r in states_after)
            status = (
                ("running" if active else "cancelled")
                if row["cancel_requested"]
                else ("running" if active else "failed")
                if error
                else "succeeded"
                if all(r["status"] == "succeeded" for r in states_after)
                else "running"
            )
            db.execute(
                "UPDATE hybrid_runs SET status=?,error=?,updated_at=? WHERE id=?",
                (status, error, now(), id),
            )
            if status != row["status"]:
                self._event(db, id, status)

    def advance_pending(self, limit: int = 20) -> None:
        with self.db.connection() as db:
            rows = db.execute(
                "SELECT id FROM hybrid_runs WHERE status IN ('queued','running') "
                "ORDER BY updated_at,id LIMIT ?",
                (limit,),
            ).fetchall()
        for row in rows:
            try:
                self.advance(row["id"])
            except PeykarError:
                with self.db.connection(immediate=True) as db:
                    self._cancel_children(db, row["id"])
                    active = any(
                        n["status"] in ("queued", "running") for n in self._nodes(db, row["id"])
                    )
                    db.execute(
                        "UPDATE hybrid_runs SET error='binding_or_integrity_failure',status=?,"
                        "updated_at=? WHERE id=? AND status IN ('queued','running')",
                        ("running" if active else "failed", now(), row["id"]),
                    )
                    self._event(db, row["id"], "binding_or_integrity_failure")

    def transfers(self, id: str, node_id: str) -> tuple[FlowTransfer, ...]:
        node = next((n for n in self.record(id).nodes if n.node_id == node_id), None)
        if node is None or node.transfer_digest is None:
            raise NotFound("No dispatched node transfers")
        return tuple(
            FlowTransfer.model_validate(t)
            for t in json.loads(self.artifacts.get(node.transfer_digest))
        )

    def results(self, id: str) -> dict[str, Any]:
        record = self.record(id)
        if record.status != "succeeded":
            raise Conflict("Flow results require all nodes to succeed")
        frozen = self.get(record.flow)
        if digest(frozen) != record.flow_digest:
            raise Conflict("Flow frozen snapshot differs")
        if {n.node_id for n in record.nodes} != {n.id for n in frozen.nodes}:
            raise Conflict("Flow node membership differs")
        outputs = []
        with self.db.connection() as db:
            rows = self._nodes(db, id)
        for node in rows:
            manifest = self._child(node)
            values = terminal_values(self.service, manifest, node["run_id"])
            result = (
                self.service.db.run(node["run_id"])
                if node["queue"] == "reference"
                else self.native.run(node["run_id"])
            )
            outputs.append(
                {
                    "node_id": node["node_id"],
                    "queue": node["queue"],
                    "run_id": node["run_id"],
                    "manifest_digest": digest(manifest),
                    "result_digest": result.result_digest,
                    "terminal_values": {k: v.model_dump(mode="json") for k, v in values.items()},
                    "transfers": [
                        t.model_dump(mode="json") for t in self.transfers(id, node["node_id"])
                    ],
                }
            )
        return {
            "run": record.model_dump(mode="json"),
            "nodes": outputs,
            "execution_mode": "sequential_scalar_initial_conditions",
            "scientific_validity": "not_established",
            "limitations": frozen.spec.limitations,
        }

    def events(self, id: str) -> list[dict[str, Any]]:
        self.record(id)
        with self.db.connection() as db:
            return [
                dict(r)
                for r in db.execute(
                    "SELECT sequence,occurred_at,event,detail FROM hybrid_events "
                    "WHERE hybrid_run_id=? ORDER BY sequence",
                    (id,),
                )
            ]
