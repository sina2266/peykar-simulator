"""Durable native-agent protocol; no Docker access or scientific runtime imports."""

import sqlite3
import time
import uuid
from pathlib import Path, PurePosixPath
from typing import Any

from peykar.infrastructure.database import now
from peykar.model_import.native_models import NativeModelService
from peykar.model_import.native_runtimes import NativeRuntimeService
from peykar.simulation.native_outputs import validate_native_outputs
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    NativeAgentClaim,
    NativeRunManifest,
    NativeRunRecord,
    NativeRunResult,
    NativeRunSpec,
    Reference,
)
from peykar_core.errors import Conflict, InvalidRequest, NotFound, OwnershipLost
from peykar_core.hashing import bytes_digest, canonical_json, digest
from peykar_core.units import convert

LEASE_SECONDS = 30
MAX_RESULT_BYTES = 1_000_000


class NativeJobService:
    def __init__(self, service: SimulationService) -> None:
        self.service, self.db, self.artifacts = service, service.db, service.artifacts
        with self.db.connection() as db:
            count = db.execute(
                "SELECT count(*) FROM sqlite_master WHERE type='table' "
                "AND name IN ('native_runs','native_events')"
            ).fetchone()[0]
            if count != 2:
                db.executescript(
                    (
                        Path(__file__).parents[1] / "infrastructure/migrations/003_native_runs.sql"
                    ).read_text()
                )

    @staticmethod
    def _event(db: sqlite3.Connection, id: str, event: str) -> None:
        db.execute(
            "INSERT INTO native_events(run_id,occurred_at,event,detail) VALUES(?,?,?,?)",
            (id, now(), event, "{}"),
        )

    @staticmethod
    def _record(row: sqlite3.Row) -> NativeRunRecord:
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
        return NativeRunRecord.model_validate({field: row[field] for field in fields})

    def submit(self, plan: NativeRunSpec, key: str | None = None) -> NativeRunRecord:
        if key is not None and not 1 <= len(key) <= 128:
            raise InvalidRequest("Idempotency key requires 1..128 characters")
        models = NativeModelService(self.db, self.artifacts)
        model, profile = models.get(plan.model), models.profile(plan.profile)
        expected = {"copasi_sbml": "sbml", "osp_pkml": "pkml"}[profile.kind]
        if model.spec.format != expected:
            raise InvalidRequest("Model format and execution profile differ")
        if profile.kind == "copasi_sbml":
            if plan.duration is None or plan.intervals is None:
                raise InvalidRequest("COPASI requires an explicit physical duration and intervals")
            if convert(plan.duration, "second") <= 0:
                raise InvalidRequest("Duration must be positive")
        elif plan.duration is not None or plan.intervals is not None:
            raise InvalidRequest(
                "PKML uses its original output schedule; overrides are unsupported"
            )
        runtime = NativeRuntimeService(self.db, self.artifacts).get(profile.runtime)
        manifest = NativeRunManifest(
            run_id=str(uuid.uuid4()),
            created_at=now(),
            plan=plan,
            model_snapshot=model,
            profile_snapshot=profile,
            runtime_snapshot=runtime,
        )
        with self.db.connection(immediate=True) as db:
            existing = (
                db.execute(
                    "SELECT id,request_digest FROM native_runs WHERE idempotency_key=?", (key,)
                ).fetchone()
                if key
                else None
            )
            if existing:
                if existing["request_digest"] != digest(plan):
                    raise Conflict("Idempotency key already identifies another native plan")
                id = str(existing["id"])
            else:
                id = manifest.run_id
                db.execute(
                    "INSERT INTO native_runs(id,request_digest,idempotency_key,profile_id,"
                    "profile_version,manifest,manifest_digest,status,created_at,updated_at) "
                    "VALUES(?,?,?,?,?,?,?,'queued',?,?)",
                    (
                        id,
                        digest(plan),
                        key,
                        profile.id,
                        profile.version,
                        canonical_json(manifest),
                        digest(manifest),
                        manifest.created_at,
                        manifest.created_at,
                    ),
                )
                self._event(db, id, "submitted")
        return self.run(id)

    def run(self, id: str) -> NativeRunRecord:
        with self.db.connection() as db:
            row = db.execute("SELECT * FROM native_runs WHERE id=?", (id,)).fetchone()
        if not row:
            raise NotFound("Native run not found")
        return self._record(row)

    def runs(self, limit: int = 100, offset: int = 0) -> tuple[NativeRunRecord, ...]:
        with self.db.connection() as db:
            rows = db.execute(
                "SELECT * FROM native_runs ORDER BY created_at DESC,id LIMIT ? OFFSET ?",
                (limit, offset),
            ).fetchall()
        return tuple(self._record(row) for row in rows)

    def manifest(self, id: str) -> NativeRunManifest:
        with self.db.connection() as db:
            row = db.execute(
                "SELECT manifest,manifest_digest FROM native_runs WHERE id=?", (id,)
            ).fetchone()
        if not row:
            raise NotFound("Native run not found")
        manifest = NativeRunManifest.model_validate_json(row["manifest"])
        if digest(manifest) != row["manifest_digest"]:
            raise Conflict("Native manifest integrity failure")
        return manifest

    def claim(self, profiles: tuple[Reference, ...]) -> NativeAgentClaim | None:
        if not profiles or len(profiles) > 50:
            raise InvalidRequest("Agent must explicitly name 1..50 locally admitted profiles")
        self.recover_stale()
        where = " OR ".join("(profile_id=? AND profile_version=?)" for _ in profiles)
        args = tuple(value for p in profiles for value in (p.id, p.version))
        with self.db.connection(immediate=True) as db:
            row = db.execute(
                "SELECT * FROM native_runs WHERE status='queued' AND ("
                + where
                + ") ORDER BY created_at,id LIMIT 1",
                args,
            ).fetchone()
            if not row:
                return None
            manifest = NativeRunManifest.model_validate_json(row["manifest"])
            if digest(manifest) != row["manifest_digest"]:
                raise Conflict("Native queue manifest integrity failure")
            token = uuid.uuid4().hex
            db.execute(
                "UPDATE native_runs SET status='running',heartbeat=?,claim_token=?,"
                "updated_at=? WHERE id=?",
                (time.time(), token, now(), row["id"]),
            )
            self._event(db, row["id"], "started")
        return NativeAgentClaim(manifest=manifest, token=token)

    def heartbeat(self, id: str, token: str) -> bool:
        with self.db.connection(immediate=True) as db:
            row = db.execute(
                "SELECT status,claim_token,cancel_requested,heartbeat FROM native_runs WHERE id=?",
                (id,),
            ).fetchone()
            if (
                not row
                or row["status"] != "running"
                or row["claim_token"] != token
                or row["heartbeat"] is None
                or row["heartbeat"] < time.time() - LEASE_SECONDS
            ):
                raise OwnershipLost("Agent no longer owns this native run")
            db.execute("UPDATE native_runs SET heartbeat=? WHERE id=?", (time.time(), id))
        return bool(row["cancel_requested"])

    def finish(
        self,
        id: str,
        token: str,
        status: str,
        result: NativeRunResult | None = None,
        blobs: dict[str, bytes] | None = None,
    ) -> NativeRunRecord:
        if status not in ("succeeded", "failed", "cancelled"):
            raise InvalidRequest("Native finish requires a terminal status")
        manifest = self.manifest(id)
        with self.db.connection() as db:
            owner = db.execute(
                "SELECT status,claim_token,heartbeat FROM native_runs WHERE id=?", (id,)
            ).fetchone()
            if (
                not owner
                or owner["status"] != "running"
                or owner["claim_token"] != token
                or owner["heartbeat"] is None
                or owner["heartbeat"] < time.time() - LEASE_SECONDS
            ):
                raise OwnershipLost("Agent completion lease is absent or expired")
        checksum = None
        if status == "succeeded":
            if (
                result is None
                or blobs is None
                or result.run_id != id
                or result.manifest_digest != digest(manifest)
            ):
                raise InvalidRequest(
                    "Successful native completion requires matching manifest/output evidence"
                )
            if (
                result.image_digest != manifest.runtime_snapshot.image_digest
                or result.driver_digest != manifest.profile_snapshot.driver_digest
            ):
                raise Conflict("Native result image/driver identity differs from frozen manifest")
            if (
                set(blobs) != set(result.artifacts)
                or sum(len(raw) for raw in blobs.values()) > MAX_RESULT_BYTES
            ):
                raise InvalidRequest("Native output bundle differs or exceeds 1 MB")
            for name, raw in blobs.items():
                path = PurePosixPath(name)
                if (
                    path.is_absolute()
                    or not path.parts
                    or ".." in path.parts
                    or bytes_digest(raw) != result.artifacts[name]
                ):
                    raise Conflict("Native result artifact path/checksum differs")
            validate_native_outputs(manifest, blobs)
            for raw in blobs.values():
                self.artifacts.put(raw)
            checksum = self.artifacts.put(canonical_json(result).encode())
        elif result is not None or blobs:
            raise InvalidRequest("Unsuccessful native completion cannot publish outputs")
        with self.db.connection(immediate=True) as db:
            row = db.execute("SELECT * FROM native_runs WHERE id=?", (id,)).fetchone()
            if (
                not row
                or row["status"] != "running"
                or row["claim_token"] != token
                or row["heartbeat"] is None
                or row["heartbeat"] < time.time() - LEASE_SECONDS
            ):
                raise OwnershipLost("Stale agent cannot publish native outputs")
            if row["cancel_requested"]:
                status, checksum = "cancelled", None
            error = "native_execution_failed" if status == "failed" else None
            db.execute(
                "UPDATE native_runs SET status=?,result_digest=?,error=?,claim_token=NULL,"
                "updated_at=? WHERE id=?",
                (status, checksum, error, now(), id),
            )
            self._event(db, id, status)
        return self.run(id)

    def result(self, id: str) -> NativeRunResult:
        row = self.run(id)
        if row.status != "succeeded" or row.result_digest is None:
            raise Conflict("Successful native results are not available")
        result = NativeRunResult.model_validate_json(self.artifacts.get(row.result_digest))
        for checksum in result.artifacts.values():
            self.artifacts.get(checksum)
        return result

    def cancel(self, id: str) -> NativeRunRecord:
        with self.db.connection(immediate=True) as db:
            row = db.execute("SELECT * FROM native_runs WHERE id=?", (id,)).fetchone()
            if not row:
                raise NotFound("Native run not found")
            if row["status"] in ("succeeded", "failed"):
                raise Conflict("Completed native runs cannot be cancelled")
            if not row["cancel_requested"]:
                status = "cancelled" if row["status"] == "queued" else row["status"]
                db.execute(
                    "UPDATE native_runs SET status=?,cancel_requested=1,updated_at=? WHERE id=?",
                    (status, now(), id),
                )
                self._event(db, id, "cancel_requested")
        return self.run(id)

    def recover_stale(self) -> int:
        with self.db.connection(immediate=True) as db:
            rows = db.execute(
                "SELECT id FROM native_runs WHERE status='running' AND heartbeat<?",
                (time.time() - LEASE_SECONDS,),
            ).fetchall()
            for row in rows:
                db.execute(
                    "UPDATE native_runs SET status='failed',claim_token=NULL,"
                    "error='agent_lost',updated_at=? WHERE id=?",
                    (now(), row["id"]),
                )
                self._event(db, row["id"], "agent_lost")
        return len(rows)

    def events(self, id: str) -> list[dict[str, Any]]:
        self.run(id)
        with self.db.connection() as db:
            rows = db.execute(
                "SELECT sequence,occurred_at,event FROM native_events "
                "WHERE run_id=? ORDER BY sequence",
                (id,),
            ).fetchall()
        return [dict(row) for row in rows]
