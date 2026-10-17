BEGIN IMMEDIATE;
CREATE TABLE IF NOT EXISTS hybrid_runs (
  id TEXT PRIMARY KEY,
  flow_id TEXT NOT NULL,
  flow_version TEXT NOT NULL,
  flow_digest TEXT NOT NULL,
  idempotency_key TEXT UNIQUE,
  status TEXT NOT NULL CHECK(status IN ('queued','running','succeeded','failed','cancelled')),
  cancel_requested INTEGER NOT NULL DEFAULT 0,
  error TEXT,
  created_at TEXT NOT NULL,
  updated_at TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS hybrid_pending ON hybrid_runs(status,updated_at,id);
CREATE TABLE IF NOT EXISTS hybrid_nodes (
  hybrid_run_id TEXT NOT NULL REFERENCES hybrid_runs(id),
  node_id TEXT NOT NULL,
  ordinal INTEGER NOT NULL,
  queue TEXT NOT NULL CHECK(queue IN ('reference','native')),
  reference_run_id TEXT UNIQUE REFERENCES runs(id),
  native_run_id TEXT UNIQUE REFERENCES native_runs(id),
  manifest_digest TEXT,
  transfer_digest TEXT,
  derivation_id TEXT,
  CHECK(reference_run_id IS NULL OR native_run_id IS NULL),
  CHECK((queue='reference' AND native_run_id IS NULL) OR (queue='native' AND reference_run_id IS NULL)),
  PRIMARY KEY(hybrid_run_id,node_id),
  UNIQUE(hybrid_run_id,ordinal)
);
CREATE TABLE IF NOT EXISTS hybrid_events (
  sequence INTEGER PRIMARY KEY AUTOINCREMENT,
  hybrid_run_id TEXT NOT NULL REFERENCES hybrid_runs(id),
  occurred_at TEXT NOT NULL,
  event TEXT NOT NULL,
  detail TEXT NOT NULL
);
COMMIT;
