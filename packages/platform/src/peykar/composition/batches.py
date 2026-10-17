"""Frozen parameter grids and bounded, transactionally dispatched job cohorts."""

import itertools
import math
import sqlite3
import uuid
from pathlib import Path
from typing import Any, Literal

from peykar.infrastructure.database import Database, now
from peykar.model_import.native_models import NativeModelService
from peykar.model_import.native_runtimes import NativeRuntimeService
from peykar.simulation.native_jobs import NativeJobService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    BatchMemberRecord,
    BatchRunRecord,
    BatchSpec,
    FrozenBatch,
    FrozenBatchMember,
    NativeRunManifest,
    ParameterSweep,
    ParameterValue,
    Reference,
    RunManifest,
    SimulationPlan,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound, PeykarError
from peykar_core.hashing import canonical_json, digest
from peykar_core.planning import check_parameter, time_grid

TERMINAL = {"succeeded", "failed", "cancelled"}
MAX_FROZEN_BYTES = 4 * 1024**2


class BatchService:
    def __init__(self, service: SimulationService) -> None:
        self.service, self.db = service, service.db
        self.native = NativeJobService(service)
        with self.db.connection() as db:
            count = db.execute(
                "SELECT count(*) FROM sqlite_master WHERE type='table' "
                "AND name IN ('batch_runs','batch_members','batch_events')"
            ).fetchone()[0]
            if count != 3:
                db.executescript(
                    (
                        Path(__file__).parents[1] / "infrastructure/migrations/004_batches.sql"
                    ).read_text()
                )

    def freeze(self, spec: BatchSpec) -> FrozenBatch:
        reference = Reference(id=spec.id, version=spec.version)
        try:
            existing = self.get(reference)
        except NotFound:
            pass
        else:
            if existing.spec != spec:
                raise Conflict("Batch version already identifies another specification")
            return existing
        cases = {case.id: case for case in spec.cases}
        if len(cases) != len(spec.cases):
            raise InvalidRequest("Batch case identities must be unique")
        axes: dict[str, list[ParameterSweep]] = {id: [] for id in cases}
        seen = set()
        for sweep in spec.sweeps:
            if sweep.case_id not in cases or (sweep.case_id, sweep.parameter) in seen:
                raise InvalidRequest("Sweep case is unknown or its parameter is duplicated")
            seen.add((sweep.case_id, sweep.parameter))
            case = cases[sweep.case_id]
            if not isinstance(case.plan, SimulationPlan):
                raise InvalidRequest("Native drivers do not support parameter overrides")
            model = self.service.validate(case.plan)
            physical_values = [check_parameter(model, sweep.parameter, q) for q in sweep.values]
            if any(
                math.isclose(a, b, rel_tol=1e-12, abs_tol=0)
                for i, a in enumerate(physical_values)
                for b in physical_values[i + 1 :]
            ):
                raise InvalidRequest("Sweep values must differ in the parameter's physical units")
            axes[sweep.case_id].append(sweep)
        total = sum(
            math.prod(len(a.values) for a in axes[c.id]) * spec.replicates for c in spec.cases
        )
        if total > spec.budget.max_jobs:
            raise InvalidRequest("Expanded batch exceeds its job budget")
        if spec.base_seed + total - 1 > 2**32 - 1:
            raise InvalidRequest("Deterministic batch seeds exceed the supported range")
        if spec.replicates > 1 and any(not isinstance(c.plan, SimulationPlan) for c in spec.cases):
            raise InvalidRequest(
                "Native drivers have no seed control; stochastic replicas unsupported"
            )
        members: list[FrozenBatchMember] = []
        policy: Literal["unique_reference_seed", "unsupported_by_native_driver"]
        samples, wall = 0, 0.0
        expanded_bytes = len(canonical_json(spec).encode())
        for case in spec.cases:
            names = [axis.parameter for axis in axes[case.id]]
            grid = itertools.product(*(a.values for a in axes[case.id]))
            for grid_index, values in enumerate(grid):
                selected = tuple(
                    ParameterValue(name=n, quantity=q) for n, q in zip(names, values, strict=True)
                )
                for replicate in range(spec.replicates):
                    ordinal = len(members)
                    if isinstance(case.plan, SimulationPlan):
                        overrides = tuple(p for p in case.plan.parameters if p.name not in names)
                        plan = SimulationPlan.model_validate(
                            case.plan.model_copy(
                                update={
                                    "seed": spec.base_seed + ordinal,
                                    "parameters": overrides + selected,
                                }
                            ).model_dump(mode="json")
                        )
                        manifest: RunManifest | NativeRunManifest = self.service.prepare(plan)
                        samples += len(time_grid(plan))
                        wall += plan.budget.wall_seconds
                        policy = "unique_reference_seed"
                    else:
                        manifest = self.native.prepare(case.plan)
                        # PKML retains its original schedule. Use the server's hard result
                        # envelope limit as a conservative sample bound, never a guessed count.
                        samples += (
                            case.plan.intervals + 1 if case.plan.intervals is not None else 100001
                        )
                        wall += case.plan.wall_seconds
                        policy = "unsupported_by_native_driver"
                    if (
                        samples > spec.budget.max_total_samples
                        or wall > spec.budget.max_total_wall_seconds
                    ):
                        raise InvalidRequest("Expanded batch exceeds aggregate sample/wall budgets")
                    expanded_bytes += len(canonical_json(manifest).encode())
                    if expanded_bytes > MAX_FROZEN_BYTES:
                        raise InvalidRequest("Expanded batch snapshots exceed 4 MiB")
                    members.append(
                        FrozenBatchMember(
                            id=f"m{ordinal:04d}",
                            case_id=case.id,
                            grid_index=grid_index,
                            replicate=replicate,
                            parameters=selected,
                            seed_policy=policy,
                            manifest=manifest,
                        )
                    )
        frozen = FrozenBatch(
            spec=spec,
            members=tuple(members),
            sample_upper_bound=samples,
            total_wall_seconds_limit=wall,
        )
        payload = frozen.model_dump_json()
        if len(payload.encode()) > MAX_FROZEN_BYTES:
            raise InvalidRequest("Frozen batch exceeds 4 MiB")
        self.db.register("batch", spec.id, spec.version, payload)
        return frozen

    def get(self, ref: Reference) -> FrozenBatch:
        frozen = FrozenBatch.model_validate_json(self.db.object("batch", ref.id, ref.version))
        models = NativeModelService(self.db, self.service.artifacts)
        checked: set[str] = set()
        for member in frozen.members:
            template = member.manifest
            fingerprint = digest(
                (
                    digest(template.model_snapshot),
                    tuple(digest(d) for d in template.dependency_snapshots),
                )
                if isinstance(template, RunManifest)
                else (
                    digest(template.model_snapshot),
                    digest(template.profile_snapshot),
                    digest(template.runtime_snapshot),
                )
            )
            if fingerprint in checked:
                continue
            checked.add(fingerprint)
            if isinstance(template, RunManifest):
                model = self.service.models.get(template.plan.model)
                if (
                    model != template.model_snapshot
                    or self.service.models.dependencies(model) != template.dependency_snapshots
                ):
                    raise Conflict("Frozen batch model/dependency identity differs")
            else:
                if (
                    models.get(template.plan.model) != template.model_snapshot
                    or models.profile(template.plan.profile) != template.profile_snapshot
                    or NativeRuntimeService(self.db, self.service.artifacts).get(
                        template.profile_snapshot.runtime
                    )
                    != template.runtime_snapshot
                ):
                    raise Conflict("Frozen native batch model/profile/runtime identity differs")
        return frozen

    def discover(self, limit: int = 100, offset: int = 0) -> tuple[FrozenBatch, ...]:
        return tuple(
            self.get(Reference(id=r["id"], version=r["version"]))
            for r in self.db.objects("batch", limit, offset)
        )

    @staticmethod
    def _event(db: sqlite3.Connection, id: str, event: str, detail: Any = None) -> None:
        db.execute(
            "INSERT INTO batch_events(batch_run_id,occurred_at,event,detail) VALUES(?,?,?,?)",
            (id, now(), event, canonical_json(detail or {})),
        )

    def submit(self, ref: Reference, key: str | None = None) -> BatchRunRecord:
        if key is not None and not 1 <= len(key) <= 128:
            raise InvalidRequest("Idempotency key requires 1..128 characters")
        frozen = self.get(ref)
        id, stamp = str(uuid.uuid4()), now()
        with self.db.connection(immediate=True) as db:
            old = (
                db.execute(
                    "SELECT id,batch_digest FROM batch_runs WHERE idempotency_key=?", (key,)
                ).fetchone()
                if key
                else None
            )
            if old:
                if old["batch_digest"] != digest(frozen):
                    raise Conflict("Batch idempotency key identifies another frozen batch")
                id = old["id"]
            else:
                db.execute(
                    "INSERT INTO batch_runs(id,batch_id,batch_version,batch_digest,"
                    "idempotency_key,status,created_at,updated_at) VALUES(?,?,?,?,?,'queued',?,?)",
                    (id, ref.id, ref.version, digest(frozen), key, stamp, stamp),
                )
                db.executemany(
                    "INSERT INTO batch_members(batch_run_id,member_id,ordinal,queue) "
                    "VALUES(?,?,?,?)",
                    (
                        (
                            id,
                            m.id,
                            i,
                            "reference" if isinstance(m.manifest, RunManifest) else "native",
                        )
                        for i, m in enumerate(frozen.members)
                    ),
                )
                self._event(db, id, "submitted")
        return self.record(id)

    @staticmethod
    def _members(db: sqlite3.Connection, id: str) -> list[sqlite3.Row]:
        return db.execute(
            "SELECT b.*,COALESCE(r.status,n.status) AS status,"
            "COALESCE(b.reference_run_id,b.native_run_id) AS run_id "
            "FROM batch_members b LEFT JOIN runs r ON r.id=b.reference_run_id "
            "LEFT JOIN native_runs n ON n.id=b.native_run_id "
            "WHERE b.batch_run_id=? ORDER BY b.ordinal",
            (id,),
        ).fetchall()

    def record(self, id: str) -> BatchRunRecord:
        with self.db.connection() as db:
            row = db.execute("SELECT * FROM batch_runs WHERE id=?", (id,)).fetchone()
            if not row:
                raise NotFound("Batch run not found")
            members = self._members(db, id)
        return BatchRunRecord(
            id=id,
            batch=Reference(id=row["batch_id"], version=row["batch_version"]),
            batch_digest=row["batch_digest"],
            status=row["status"],
            cancel_requested=bool(row["cancel_requested"]),
            error=row["error"],
            created_at=row["created_at"],
            updated_at=row["updated_at"],
            members=tuple(
                BatchMemberRecord(
                    member_id=m["member_id"],
                    queue=m["queue"],
                    run_id=m["run_id"],
                    status=m["status"] or ("skipped" if row["status"] in TERMINAL else "pending"),
                )
                for m in members
            ),
        )

    def runs(self, limit: int = 100, offset: int = 0) -> tuple[BatchRunRecord, ...]:
        with self.db.connection() as db:
            rows = db.execute(
                "SELECT id FROM batch_runs ORDER BY created_at DESC,id LIMIT ? OFFSET ?",
                (limit, offset),
            ).fetchall()
        return tuple(self.record(row["id"]) for row in rows)

    @staticmethod
    def _cancel_children(db: sqlite3.Connection, id: str) -> None:
        for member in BatchService._members(db, id):
            if member["status"] not in ("queued", "running"):
                continue
            table = "runs" if member["queue"] == "reference" else "native_runs"
            row = db.execute(
                f"SELECT cancel_requested FROM {table} WHERE id=?", (member["run_id"],)
            ).fetchone()
            if row[0]:
                continue
            db.execute(
                f"UPDATE {table} SET cancel_requested=1,status=?,updated_at=? WHERE id=?",
                (
                    "cancelled" if member["status"] == "queued" else "running",
                    now(),
                    member["run_id"],
                ),
            )
            if member["queue"] == "reference":
                Database._event(db, member["run_id"], "cancel_requested")
            else:
                NativeJobService._event(db, member["run_id"], "cancel_requested")

    def cancel(self, id: str) -> BatchRunRecord:
        with self.db.connection(immediate=True) as db:
            row = db.execute(
                "SELECT status,cancel_requested FROM batch_runs WHERE id=?", (id,)
            ).fetchone()
            if not row:
                raise NotFound("Batch run not found")
            if row["status"] in ("succeeded", "failed"):
                raise Conflict("Completed batches cannot be cancelled")
            if not row["cancel_requested"]:
                db.execute(
                    "UPDATE batch_runs SET cancel_requested=1,updated_at=? WHERE id=?", (now(), id)
                )
                self._event(db, id, "cancel_requested")
                self._cancel_children(db, id)
        self.advance(id)
        return self.record(id)

    def advance(self, id: str) -> None:
        record = self.record(id)
        if record.status in TERMINAL:
            return
        frozen = self.get(record.batch)
        if digest(frozen) != record.batch_digest:
            raise Conflict("Batch frozen snapshot checksum differs")
        with self.db.connection(immediate=True) as db:
            row = db.execute("SELECT * FROM batch_runs WHERE id=?", (id,)).fetchone()
            if row["status"] in TERMINAL:
                return
            members = self._members(db, id)
            failed = any(m["status"] in ("failed", "cancelled") for m in members)
            if len(members) != len(frozen.members) or any(
                m["member_id"] != expected.id
                or m["ordinal"] != i
                or (m["queue"] == "reference") != isinstance(expected.manifest, RunManifest)
                for i, (m, expected) in enumerate(zip(members, frozen.members, strict=False))
            ):
                raise Conflict("Batch member association differs from its frozen specification")
            error = row["error"]
            if (
                row["cancel_requested"]
                or error
                or (failed and frozen.spec.failure_policy == "fail_fast")
            ):
                self._cancel_children(db, id)
            else:
                slots = frozen.spec.budget.max_in_flight - sum(
                    m["status"] in ("queued", "running") for m in members
                )
                for member in members:
                    if slots <= 0:
                        break
                    if member["run_id"]:
                        continue
                    template = frozen.members[member["ordinal"]].manifest
                    manifest = template.model_copy(
                        update={"run_id": str(uuid.uuid4()), "created_at": now()}
                    )
                    # No public idempotency key: association and child insert commit
                    # together. Users cannot preempt an internal key to hijack a child.
                    if isinstance(manifest, RunManifest):
                        run_id = Database.enqueue(db, manifest, None)
                        column = "reference_run_id"
                    else:
                        run_id = NativeJobService.enqueue(db, manifest, None)
                        column = "native_run_id"
                    db.execute(
                        f"UPDATE batch_members SET {column}=? WHERE batch_run_id=? AND member_id=?",
                        (run_id, id, member["member_id"]),
                    )
                    self._event(
                        db,
                        id,
                        "member_submitted",
                        {"member_id": member["member_id"], "run_id": run_id},
                    )
                    slots -= 1
            states = self._members(db, id)
            active = any(m["status"] in ("queued", "running") for m in states)
            done = all(m["status"] in TERMINAL for m in states)
            if row["cancel_requested"]:
                status = "running" if active else "cancelled"
            elif error or (failed and (done or frozen.spec.failure_policy == "fail_fast")):
                error = error or "member_failed"
                status = "running" if active else "failed"
            else:
                status = "succeeded" if done else "running"
            db.execute(
                "UPDATE batch_runs SET status=?,error=?,updated_at=? WHERE id=?",
                (status, error, now(), id),
            )
            if status != row["status"]:
                self._event(db, id, status)

    def advance_pending(self, limit: int = 20) -> None:
        with self.db.connection() as db:
            rows = db.execute(
                "SELECT id FROM batch_runs WHERE status IN ('queued','running') "
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
                        m["status"] in ("queued", "running") for m in self._members(db, row["id"])
                    )
                    db.execute(
                        "UPDATE batch_runs SET status=?,error='integrity_failure',updated_at=? "
                        "WHERE id=? AND status IN ('queued','running')",
                        ("running" if active else "failed", now(), row["id"]),
                    )
                    self._event(db, row["id"], "integrity_failure")

    def events(self, id: str) -> list[dict[str, Any]]:
        self.record(id)
        with self.db.connection() as db:
            return [
                dict(r)
                for r in db.execute(
                    "SELECT sequence,occurred_at,event,detail "
                    "FROM batch_events WHERE batch_run_id=? ORDER BY sequence",
                    (id,),
                )
            ]
