"""Durable model workflows: explicit terminal-output to initial-parameter transfer.

This is sequential experiment orchestration, not a simultaneous feedback solver.
Dispatch and child association share one SQLite transaction, so retries, concurrent
workers and cancellation cannot produce unassociated child jobs.
"""

import sqlite3
import uuid
from pathlib import Path

from peykar.composition.compatibility import Connection, composition_order
from peykar.infrastructure.database import now
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    ExperimentNodeRecord,
    ExperimentRunRecord,
    ExperimentSpec,
    FrozenExperiment,
    ParameterValue,
    Quantity,
    Reference,
    SimulationPlan,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound, PeykarError
from peykar_core.hashing import canonical_json, digest
from peykar_core.units import convert

TERMINAL = {"succeeded", "failed", "cancelled"}


class ExperimentService:
    def __init__(self, service: SimulationService) -> None:
        self.service, self.db = service, service.db
        with self.db.connection() as db:
            exists = db.execute(
                "SELECT 1 FROM sqlite_master WHERE type='table' AND name='experiment_runs'"
            ).fetchone()
            if not exists:
                migration = (
                    Path(__file__).parents[1] / "infrastructure/migrations/002_experiments.sql"
                )
                db.executescript(migration.read_text())

    def freeze(self, spec: ExperimentSpec) -> FrozenExperiment:
        nodes = {node.id: node for node in spec.nodes}
        if len(nodes) != len(spec.nodes):
            raise InvalidRequest("Duplicate experiment node IDs")
        models = {node.id: self.service.validate(node.plan) for node in spec.nodes}
        if len({canonical_json(node.plan.context) for node in spec.nodes}) != 1:
            raise InvalidRequest("Cross-context workflows require a separately implemented mapping")
        connections = tuple(
            Connection(
                source_model=b.source_node,
                source_port=b.source_port,
                target_model=b.target_node,
                target_port=b.target_port,
            )
            for b in spec.bindings
        )
        order = composition_order(models, connections)
        if any(port.shape != "scalar" for model in models.values() for port in model.ports):
            raise InvalidRequest("Endpoint workflow supports scalar ports only")
        parameters = set()
        for binding in spec.bindings:
            if any(not item.strip() or len(item) > 2000 for item in binding.evidence):
                raise InvalidRequest("Bindings require nonblank bounded evidence references")
            key = (binding.target_node, binding.target_parameter)
            if key in parameters:
                raise InvalidRequest("Multiple bindings write the same parameter")
            parameters.add(key)
            plan = nodes[binding.target_node].plan
            if binding.target_parameter in {p.name for p in plan.parameters}:
                raise InvalidRequest("Bound parameters cannot also have explicit overrides")
            model = models[binding.target_node]
            parameter = next(
                (p for p in model.parameters if p.name == binding.target_parameter), None
            )
            if parameter is None:
                raise InvalidRequest("Binding targets an undeclared parameter")
            port = next(p for p in model.ports if p.name == binding.target_port)
            convert(Quantity(value=1.0, unit=port.unit), parameter.default.unit)
        frozen = FrozenExperiment(spec=spec, models=tuple(models[n] for n in order), order=order)
        self.db.register("experiment", spec.id, spec.version, frozen.model_dump_json())
        return frozen

    def get(self, reference: Reference) -> FrozenExperiment:
        frozen = FrozenExperiment.model_validate_json(
            self.db.object("experiment", reference.id, reference.version)
        )
        for model in frozen.models:
            if self.service.models.get(Reference(id=model.id, version=model.version)) != model:
                raise Conflict("Frozen model differs from its registered immutable version")
        return frozen

    def discover(self) -> tuple[FrozenExperiment, ...]:
        return tuple(
            self.get(Reference(id=r["id"], version=r["version"]))
            for r in self.db.objects("experiment", limit=200)
        )

    @staticmethod
    def _event(db: sqlite3.Connection, id: str, event: str, detail: str = "{}") -> None:
        db.execute(
            "INSERT INTO experiment_events(experiment_run_id,occurred_at,event,detail) "
            "VALUES(?,?,?,?)",
            (id, now(), event, detail),
        )

    def submit(self, reference: Reference, key: str | None = None) -> ExperimentRunRecord:
        if key is not None and not 1 <= len(key) <= 128:
            raise InvalidRequest("Idempotency key must contain 1..128 characters")
        frozen = self.get(reference)
        checksum = digest(frozen)
        id = str(uuid.uuid4())
        with self.db.connection(immediate=True) as db:
            row = (
                db.execute(
                    "SELECT id,experiment_digest FROM experiment_runs WHERE idempotency_key=?",
                    (key,),
                ).fetchone()
                if key
                else None
            )
            if row:
                if row["experiment_digest"] != checksum:
                    raise Conflict("Experiment idempotency key was used for another specification")
                id = row["id"]
            else:
                stamp = now()
                db.execute(
                    "INSERT INTO experiment_runs(id,experiment_id,experiment_version,"
                    "experiment_digest,idempotency_key,status,created_at,updated_at) "
                    "VALUES(?,?,?,?,?,'queued',?,?)",
                    (id, reference.id, reference.version, checksum, key, stamp, stamp),
                )
                db.executemany(
                    "INSERT INTO experiment_nodes VALUES(?,?,NULL)", ((id, n) for n in frozen.order)
                )
                self._event(db, id, "submitted")
        return self.record(id)

    def record(self, id: str) -> ExperimentRunRecord:
        with self.db.connection() as db:
            row = db.execute("SELECT * FROM experiment_runs WHERE id=?", (id,)).fetchone()
            if not row:
                raise NotFound("Experiment run not found")
            nodes = db.execute(
                "SELECT n.node_id,n.run_id,r.status FROM experiment_nodes n "
                "LEFT JOIN runs r ON r.id=n.run_id WHERE n.experiment_run_id=? "
                "ORDER BY n.node_id",
                (id,),
            ).fetchall()
        return ExperimentRunRecord(
            id=id,
            experiment=Reference(id=row["experiment_id"], version=row["experiment_version"]),
            experiment_digest=row["experiment_digest"],
            status=row["status"],
            cancel_requested=bool(row["cancel_requested"]),
            error=row["error"],
            created_at=row["created_at"],
            updated_at=row["updated_at"],
            nodes=tuple(
                ExperimentNodeRecord(
                    node_id=n["node_id"],
                    run_id=n["run_id"],
                    status=n["status"] or ("skipped" if row["status"] in TERMINAL else "pending"),
                )
                for n in nodes
            ),
        )

    @staticmethod
    def _cancel_children(db: sqlite3.Connection, id: str) -> None:
        rows = db.execute(
            "SELECT r.id,r.status FROM runs r JOIN experiment_nodes n ON r.id=n.run_id "
            "WHERE n.experiment_run_id=? AND r.status IN ('queued','running') "
            "AND r.cancel_requested=0",
            (id,),
        ).fetchall()
        for row in rows:
            db.execute(
                "UPDATE runs SET cancel_requested=1,status=?,updated_at=? WHERE id=?",
                ("cancelled" if row["status"] == "queued" else "running", now(), row["id"]),
            )
            from peykar.infrastructure.database import Database

            Database._event(db, row["id"], "cancel_requested")

    def cancel(self, id: str) -> ExperimentRunRecord:
        with self.db.connection(immediate=True) as db:
            row = db.execute(
                "SELECT status,cancel_requested FROM experiment_runs WHERE id=?", (id,)
            ).fetchone()
            if not row:
                raise NotFound("Experiment run not found")
            if row["status"] in ("succeeded", "failed"):
                raise Conflict("Completed experiments cannot be cancelled")
            if not row["cancel_requested"] and row["status"] != "cancelled":
                db.execute(
                    "UPDATE experiment_runs SET cancel_requested=1,updated_at=? WHERE id=?",
                    (now(), id),
                )
                self._event(db, id, "cancel_requested")
                self._cancel_children(db, id)
        self.advance(id)
        return self.record(id)

    def _bound_plan(
        self, frozen: FrozenExperiment, node_id: str, runs: dict[str, sqlite3.Row]
    ) -> SimulationPlan:
        node = next(n for n in frozen.spec.nodes if n.id == node_id)
        overrides = list(node.plan.parameters)
        for binding in frozen.spec.bindings:
            if binding.target_node != node_id:
                continue
            upstream = self.service.result(runs[binding.source_node]["run_id"])
            series = next((s for s in upstream.outputs if s.name == binding.source_port), None)
            if series is None:
                raise Conflict("Upstream result lacks the pinned binding endpoint")
            target = self.service.models.get(node.plan.model)
            parameter = next(p for p in target.parameters if p.name == binding.target_parameter)
            value = convert(
                Quantity(value=series.values[-1], unit=series.unit), parameter.default.unit
            )
            overrides.append(
                ParameterValue(
                    name=parameter.name, quantity=Quantity(value=value, unit=parameter.default.unit)
                )
            )
        return SimulationPlan.model_validate(
            node.plan.model_copy(update={"parameters": tuple(overrides)}).model_dump(mode="json")
        )

    def advance(self, id: str) -> None:
        record = self.record(id)
        if record.status in TERMINAL:
            return
        frozen = self.get(record.experiment)
        if digest(frozen) != record.experiment_digest:
            raise Conflict("Experiment run specification checksum differs")
        with self.db.connection(immediate=True) as db:
            row = db.execute("SELECT * FROM experiment_runs WHERE id=?", (id,)).fetchone()
            if row["status"] in TERMINAL:
                return
            runs = {
                n["node_id"]: n
                for n in db.execute(
                    "SELECT n.node_id,n.run_id,r.status FROM experiment_nodes n LEFT JOIN runs r "
                    "ON r.id=n.run_id WHERE n.experiment_run_id=?",
                    (id,),
                ).fetchall()
            }
            error = row["error"]
            if (
                any(n["status"] in ("failed", "cancelled") for n in runs.values())
                and not row["cancel_requested"]
            ):
                error = "node_failed"
            if row["cancel_requested"] or error:
                self._cancel_children(db, id)
            else:
                for node_id in frozen.order:
                    if runs[node_id]["run_id"]:
                        continue
                    parents = [
                        b.source_node for b in frozen.spec.bindings if b.target_node == node_id
                    ]
                    if not all(runs[p]["status"] == "succeeded" for p in parents):
                        continue
                    try:
                        plan = self._bound_plan(frozen, node_id, runs)
                        manifest = self.service.prepare(plan)
                    except PeykarError:
                        error = "binding_or_execution_constraint"
                        self._cancel_children(db, id)
                        break
                    run_id = self.db.enqueue(db, manifest, f"experiment:{id}:{node_id}")
                    db.execute(
                        "UPDATE experiment_nodes SET run_id=? WHERE experiment_run_id=? "
                        "AND node_id=?",
                        (run_id, id, node_id),
                    )
                    self._event(
                        db,
                        id,
                        "node_submitted",
                        canonical_json({"node": node_id, "run_id": run_id}),
                    )
            states = db.execute(
                "SELECT r.status FROM experiment_nodes n LEFT JOIN runs r ON r.id=n.run_id "
                "WHERE n.experiment_run_id=?",
                (id,),
            ).fetchall()
            active = any(n["status"] in ("queued", "running") for n in states)
            if error:
                status = "running" if active else "failed"
            elif row["cancel_requested"]:
                status = "running" if active else "cancelled"
            else:
                status = (
                    "succeeded" if all(n["status"] == "succeeded" for n in states) else "running"
                )
            db.execute(
                "UPDATE experiment_runs SET status=?,error=?,updated_at=? WHERE id=?",
                (status, error, now(), id),
            )
            if status != row["status"]:
                self._event(db, id, status)

    def advance_pending(self, limit: int = 20) -> None:
        with self.db.connection() as db:
            ids = db.execute(
                "SELECT id FROM experiment_runs WHERE status IN ('queued','running') "
                "ORDER BY updated_at,id LIMIT ?",
                (limit,),
            ).fetchall()
        for row in ids:
            try:
                self.advance(row["id"])
            except PeykarError:
                # One invalid workflow must not stop the ordinary simulation queue.
                with self.db.connection(immediate=True) as db:
                    self._cancel_children(db, row["id"])
                    active = db.execute(
                        "SELECT 1 FROM experiment_nodes n JOIN runs r ON r.id=n.run_id "
                        "WHERE n.experiment_run_id=? AND r.status IN ('queued','running')",
                        (row["id"],),
                    ).fetchone()
                    db.execute(
                        "UPDATE experiment_runs SET error='integrity_failure',"
                        "status=?,updated_at=? "
                        "WHERE id=? AND status IN ('queued','running')",
                        ("running" if active else "failed", now(), row["id"]),
                    )

    def results(self, id: str) -> dict[str, object]:
        record = self.record(id)
        if record.status != "succeeded":
            raise Conflict("Experiment results require every node to succeed")
        return {
            "run": record.model_dump(mode="json"),
            "execution_mode": "sequential_endpoint_transfer",
            "nodes": [
                {
                    "node_id": n.node_id,
                    "run": self.db.run(str(n.run_id)).model_dump(mode="json"),
                    "result": self.service.result(str(n.run_id)).model_dump(mode="json"),
                }
                for n in record.nodes
            ],
            "scientific_validity": "not_established_by_orchestration",
        }

    def events(self, id: str) -> list[dict[str, object]]:
        self.record(id)
        with self.db.connection() as db:
            return [
                dict(row)
                for row in db.execute(
                    "SELECT sequence,occurred_at,event,detail "
                    "FROM experiment_events WHERE experiment_run_id=? ORDER BY sequence",
                    (id,),
                )
            ]
