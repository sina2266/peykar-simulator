CREATE TABLE IF NOT EXISTS experiment_runs (
  id TEXT PRIMARY KEY,
  experiment_id TEXT NOT NULL,
  experiment_version TEXT NOT NULL,
  experiment_digest TEXT NOT NULL,
  idempotency_key TEXT UNIQUE,
  status TEXT NOT NULL CHECK(status IN ('queued','running','succeeded','failed','cancelled')),
  cancel_requested INTEGER NOT NULL DEFAULT 0 CHECK(cancel_requested IN (0,1)),
  error TEXT,
  created_at TEXT NOT NULL,
  updated_at TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS experiment_runs_pending ON experiment_runs(status,created_at,id);
CREATE TABLE IF NOT EXISTS experiment_nodes (
  experiment_run_id TEXT NOT NULL REFERENCES experiment_runs(id),
  node_id TEXT NOT NULL,
  run_id TEXT UNIQUE REFERENCES runs(id),
  PRIMARY KEY(experiment_run_id,node_id)
);
CREATE TABLE IF NOT EXISTS experiment_events (
  sequence INTEGER PRIMARY KEY AUTOINCREMENT,
  experiment_run_id TEXT NOT NULL REFERENCES experiment_runs(id),
  occurred_at TEXT NOT NULL,
  event TEXT NOT NULL,
  detail TEXT NOT NULL
);
