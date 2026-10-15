CREATE TABLE IF NOT EXISTS objects (
  kind TEXT NOT NULL,
  id TEXT NOT NULL,
  version TEXT NOT NULL,
  digest TEXT NOT NULL,
  payload TEXT NOT NULL,
  PRIMARY KEY(kind, id, version)
);
CREATE TABLE IF NOT EXISTS runs (
  id TEXT PRIMARY KEY,
  request_digest TEXT NOT NULL,
  idempotency_key TEXT UNIQUE,
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
CREATE INDEX IF NOT EXISTS runs_queue ON runs(status, created_at, id);
CREATE TABLE IF NOT EXISTS events (
  sequence INTEGER PRIMARY KEY AUTOINCREMENT,
  run_id TEXT NOT NULL REFERENCES runs(id),
  occurred_at TEXT NOT NULL,
  event TEXT NOT NULL,
  detail TEXT NOT NULL
);
PRAGMA user_version = 1;
