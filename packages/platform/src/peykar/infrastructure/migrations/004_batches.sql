BEGIN IMMEDIATE;
CREATE TABLE IF NOT EXISTS batch_runs (
  id TEXT PRIMARY KEY,
  batch_id TEXT NOT NULL,
  batch_version TEXT NOT NULL,
  batch_digest TEXT NOT NULL,
  idempotency_key TEXT UNIQUE,
  status TEXT NOT NULL CHECK(status IN ('queued','running','succeeded','failed','cancelled')),
  cancel_requested INTEGER NOT NULL DEFAULT 0 CHECK(cancel_requested IN (0,1)),
  error TEXT,
  created_at TEXT NOT NULL,
  updated_at TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS batch_runs_pending ON batch_runs(status,updated_at,id);
CREATE TABLE IF NOT EXISTS batch_members (
  batch_run_id TEXT NOT NULL REFERENCES batch_runs(id),
  member_id TEXT NOT NULL,
  ordinal INTEGER NOT NULL,
  queue TEXT NOT NULL CHECK(queue IN ('reference','native')),
  reference_run_id TEXT UNIQUE REFERENCES runs(id),
  native_run_id TEXT UNIQUE REFERENCES native_runs(id),
  CHECK(reference_run_id IS NULL OR native_run_id IS NULL),
  CHECK((queue='reference' AND native_run_id IS NULL) OR
        (queue='native' AND reference_run_id IS NULL)),
  PRIMARY KEY(batch_run_id,member_id),
  UNIQUE(batch_run_id,ordinal)
);
CREATE TABLE IF NOT EXISTS batch_events (
  sequence INTEGER PRIMARY KEY AUTOINCREMENT,
  batch_run_id TEXT NOT NULL REFERENCES batch_runs(id),
  occurred_at TEXT NOT NULL,
  event TEXT NOT NULL,
  detail TEXT NOT NULL
);
COMMIT;
