"""Single-host durable queue and registries.

BEGIN IMMEDIATE serializes claims. Tokens fence stale workers. No connection is shared
across threads. Stale jobs fail explicitly; reruns always receive a new manifest.
"""

import json
import sqlite3
import time
import uuid
from collections.abc import Iterator
from contextlib import contextmanager
from datetime import UTC, datetime
from pathlib import Path
from typing import Any

from peykar_core.contracts import RunManifest, RunRecord, RunStatus
from peykar_core.errors import Conflict, NotFound, OwnershipLost
from peykar_core.hashing import canonical_json, digest


def now() -> str:
    return datetime.now(UTC).isoformat()


class Database:
    def __init__(self, path: Path) -> None:
        self.path = path.resolve()
        self.path.parent.mkdir(parents=True, exist_ok=True)
        with self.connection() as db:
            version = db.execute("PRAGMA user_version").fetchone()[0]
            if version not in (0, 1):
                raise Conflict("Unsupported database schema version")
            db.execute("PRAGMA journal_mode=WAL")
            if version == 0:
                schema = Path(__file__).with_name("migrations") / "001_initial.sql"
                db.executescript(schema.read_text())

    @contextmanager
    def connection(
        self, *, immediate: bool = False, timeout: float = 30.0
    ) -> Iterator[sqlite3.Connection]:
        db = sqlite3.connect(self.path, timeout=timeout)
        db.row_factory = sqlite3.Row
        db.execute("PRAGMA foreign_keys=ON")
        try:
            if immediate:
                db.execute("BEGIN IMMEDIATE")
            yield db
            db.commit()
        except BaseException:
            db.rollback()
            raise
        finally:
            db.close()

    def register(self, kind: str, id: str, version: str, payload: str) -> str:
        return self.register_many(((kind, id, version, payload),))[0]

    def register_many(self, entries: tuple[tuple[str, str, str, str], ...]) -> tuple[str, ...]:
        """Commit related immutable objects together or leave all of them unchanged."""
        with self.connection(immediate=True) as db:
            return tuple(self.register_in_transaction(db, *entry) for entry in entries)

    @staticmethod
    def register_in_transaction(
        db: sqlite3.Connection, kind: str, id: str, version: str, payload: str
    ) -> str:
        """Preserve immutable objects in a caller-owned atomic dispatch transaction."""
        checksum = digest(json.loads(payload))
        row = db.execute(
            "SELECT digest,payload FROM objects WHERE kind=? AND id=? AND version=?",
            (kind, id, version),
        ).fetchone()
        if row:
            if row["digest"] != checksum or digest(json.loads(row["payload"])) != checksum:
                raise Conflict("This immutable version already exists with different content")
        else:
            db.execute(
                "INSERT INTO objects VALUES (?,?,?,?,?)", (kind, id, version, checksum, payload)
            )
        return checksum

    def object(self, kind: str, id: str, version: str) -> str:
        with self.connection() as db:
            row = db.execute(
                "SELECT payload,digest FROM objects WHERE kind=? AND id=? AND version=?",
                (kind, id, version),
            ).fetchone()
        if not row:
            raise NotFound(f"{kind} {id}@{version} not found")
        if digest(json.loads(row["payload"])) != row["digest"]:
            raise Conflict("Registered object integrity verification failed")
        return str(row["payload"])

    def objects(
        self,
        kind: str,
        limit: int = 100,
        offset: int = 0,
        capability: str | None = None,
        species: str | None = None,
    ) -> list[dict[str, Any]]:
        clauses = ["kind=?"]
        parameters: list[str | int] = [kind]
        if species is not None:
            clauses.append("json_extract(payload,'$.context.species')=?")
            parameters.append(species)
        if capability is not None:
            clauses.append(
                "EXISTS(SELECT 1 FROM json_each(payload,'$.capabilities') WHERE value=?)"
            )
            parameters.append(capability)
        query = (
            "SELECT id,version,digest,payload FROM objects WHERE "
            + " AND ".join(clauses)
            + " ORDER BY id,version LIMIT ? OFFSET ?"
        )
        parameters.extend((limit, offset))
        with self.connection() as db:
            rows = db.execute(query, parameters).fetchall()
        if any(digest(json.loads(row["payload"])) != row["digest"] for row in rows):
            raise Conflict("Registered object integrity verification failed")
        return [dict(row) for row in rows]

    @staticmethod
    def _event(db: sqlite3.Connection, run_id: str, event: str, detail: str = "{}") -> None:
        db.execute(
            "INSERT INTO events(run_id,occurred_at,event,detail) VALUES(?,?,?,?)",
            (run_id, now(), event, detail),
        )

    def submit(self, manifest: RunManifest, idempotency_key: str | None) -> RunRecord:
        with self.connection(immediate=True) as db:
            run_id = self.enqueue(db, manifest, idempotency_key)
        return self.run(run_id)

    @staticmethod
    def enqueue(db: sqlite3.Connection, manifest: RunManifest, idempotency_key: str | None) -> str:
        """Enqueue within a caller-owned transaction, including dependent workflow dispatch."""
        request_hash = digest(manifest.plan)
        if idempotency_key:
            existing = db.execute(
                "SELECT id,request_digest FROM runs WHERE idempotency_key=?", (idempotency_key,)
            ).fetchone()
            if existing:
                if existing["request_digest"] != request_hash:
                    raise Conflict("Idempotency key already used for another plan")
                return str(existing["id"])
        db.execute(
            "INSERT INTO runs(id,request_digest,idempotency_key,manifest,manifest_digest,"
            "status,created_at,updated_at) VALUES(?,?,?,?,?,?,?,?)",
            (
                manifest.run_id,
                request_hash,
                idempotency_key,
                canonical_json(manifest),
                digest(manifest),
                "queued",
                manifest.created_at,
                manifest.created_at,
            ),
        )
        Database._event(db, manifest.run_id, "submitted")
        return manifest.run_id

    @staticmethod
    def _record(row: sqlite3.Row) -> RunRecord:
        fields = (
            "id",
            "manifest_digest",
            "status",
            "created_at",
            "updated_at",
            "cancel_requested",
            "result_digest",
            "error",
        )
        return RunRecord.model_validate({field: row[field] for field in fields})

    def run(self, id: str) -> RunRecord:
        with self.connection() as db:
            row = db.execute("SELECT * FROM runs WHERE id=?", (id,)).fetchone()
        if not row:
            raise NotFound("Run not found")
        return self._record(row)

    def runs(self, limit: int = 100, offset: int = 0) -> tuple[RunRecord, ...]:
        with self.connection() as db:
            rows = db.execute(
                "SELECT * FROM runs ORDER BY created_at DESC,id LIMIT ? OFFSET ?", (limit, offset)
            ).fetchall()
        return tuple(self._record(row) for row in rows)

    def manifest(self, id: str) -> RunManifest:
        with self.connection() as db:
            row = db.execute(
                "SELECT manifest,manifest_digest FROM runs WHERE id=?", (id,)
            ).fetchone()
        if not row:
            raise NotFound("Run not found")
        manifest = RunManifest.model_validate_json(row["manifest"])
        if digest(manifest) != row["manifest_digest"]:
            raise Conflict("Run manifest integrity verification failed")
        return manifest

    def claim(self) -> tuple[str, str] | None:
        with self.connection(immediate=True) as db:
            row = db.execute(
                "SELECT id FROM runs WHERE status='queued' ORDER BY created_at,id LIMIT 1"
            ).fetchone()
            if not row:
                return None
            token = uuid.uuid4().hex
            db.execute(
                "UPDATE runs SET status='running',claim_token=?,heartbeat=?,updated_at=? "
                "WHERE id=?",
                (token, time.time(), now(), row["id"]),
            )
            self._event(db, row["id"], "started")
            return str(row["id"]), token

    def heartbeat(self, id: str, token: str) -> bool:
        # A busy database must not block the parent's resource supervisor for 30 seconds.
        with self.connection(immediate=True, timeout=0.05) as db:
            row = db.execute(
                "SELECT status,claim_token,cancel_requested FROM runs WHERE id=?", (id,)
            ).fetchone()
            if not row or row["status"] != "running" or row["claim_token"] != token:
                raise OwnershipLost("Worker no longer owns this run")
            db.execute("UPDATE runs SET heartbeat=? WHERE id=?", (time.time(), id))
        return bool(row["cancel_requested"])

    def finish(
        self,
        id: str,
        token: str,
        status: RunStatus,
        result_digest: str | None = None,
        error: str | None = None,
    ) -> RunRecord:
        if status not in (RunStatus.SUCCEEDED, RunStatus.FAILED, RunStatus.CANCELLED):
            raise Conflict("Invalid terminal transition")
        with self.connection(immediate=True) as db:
            row = db.execute("SELECT * FROM runs WHERE id=?", (id,)).fetchone()
            if not row or row["status"] != "running" or row["claim_token"] != token:
                raise OwnershipLost("Stale worker cannot commit results")
            if row["cancel_requested"]:
                status, result_digest, error = RunStatus.CANCELLED, None, None
            db.execute(
                "UPDATE runs SET status=?,result_digest=?,error=?,updated_at=?,"
                "claim_token=NULL WHERE id=?",
                (status, result_digest, error, now(), id),
            )
            self._event(db, id, str(status))
        return self.run(id)

    def cancel(self, id: str) -> RunRecord:
        with self.connection(immediate=True) as db:
            row = db.execute("SELECT * FROM runs WHERE id=?", (id,)).fetchone()
            if not row:
                raise NotFound("Run not found")
            if row["status"] in ("succeeded", "failed"):
                raise Conflict("Completed runs cannot be cancelled")
            if row["status"] == "cancelled" or row["cancel_requested"]:
                return self._record(row)
            status = "cancelled" if row["status"] == "queued" else "running"
            db.execute(
                "UPDATE runs SET status=?,cancel_requested=1,updated_at=? WHERE id=?",
                (status, now(), id),
            )
            self._event(db, id, "cancel_requested")
        return self.run(id)

    def recover_stale(self, age_seconds: float) -> int:
        with self.connection(immediate=True) as db:
            rows = db.execute(
                "SELECT id FROM runs WHERE status='running' AND heartbeat < ?",
                (time.time() - age_seconds,),
            ).fetchall()
            for row in rows:
                db.execute(
                    "UPDATE runs SET status='failed',error='worker_lost',claim_token=NULL,"
                    "updated_at=? WHERE id=?",
                    (now(), row["id"]),
                )
                self._event(db, row["id"], "worker_lost")
        return len(rows)

    def events(self, id: str) -> list[dict[str, Any]]:
        self.run(id)
        with self.connection() as db:
            rows = db.execute(
                "SELECT sequence,occurred_at,event,detail FROM events "
                "WHERE run_id=? ORDER BY sequence",
                (id,),
            ).fetchall()
        return [dict(row) for row in rows]
