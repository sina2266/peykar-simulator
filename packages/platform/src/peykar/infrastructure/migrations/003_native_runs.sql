BEGIN IMMEDIATE;
CREATE TABLE IF NOT EXISTS native_runs (
  id TEXT PRIMARY KEY,
  request_digest TEXT NOT NULL,
  idempotency_key TEXT UNIQUE,
  profile_id TEXT NOT NULL,
  profile_version TEXT NOT NULL,
  manifest TEXT NOT NULL,
  manifest_digest TEXT NOT NULL,
  status TEXT NOT NULL CHECK(status IN ('queued','running','succeeded','failed','cancelled')),
  created_at TEXT NOT NULL,
  updated_at TEXT NOT NULL,
  heartbeat REAL,
  claim_token TEXT,
  cancel_requested INTEGER NOT NULL DEFAULT 0 CHECK(cancel_requested IN (0,1)),
  result_digest TEXT,
  error TEXT
);
CREATE INDEX IF NOT EXISTS native_runs_queue ON native_runs(status,created_at,id);
CREATE TABLE IF NOT EXISTS native_events (
  sequence INTEGER PRIMARY KEY AUTOINCREMENT,
  run_id TEXT NOT NULL REFERENCES native_runs(id),
  occurred_at TEXT NOT NULL,
  event TEXT NOT NULL,
  detail TEXT NOT NULL
);
COMMIT;
