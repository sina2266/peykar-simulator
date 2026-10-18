BEGIN IMMEDIATE;
CREATE TABLE IF NOT EXISTS vocabulary_terms (
  vocabulary_id TEXT NOT NULL,
  vocabulary_version TEXT NOT NULL,
  term_id TEXT NOT NULL,
  label TEXT NOT NULL,
  namespace TEXT NOT NULL,
  obsolete INTEGER NOT NULL CHECK(obsolete IN (0,1)),
  digest TEXT NOT NULL,
  payload TEXT NOT NULL,
  PRIMARY KEY(vocabulary_id,vocabulary_version,term_id)
);
CREATE INDEX IF NOT EXISTS vocabulary_labels ON vocabulary_terms(vocabulary_id,vocabulary_version,label);
CREATE TABLE IF NOT EXISTS vocabulary_aliases (
  vocabulary_id TEXT NOT NULL,
  vocabulary_version TEXT NOT NULL,
  alias_id TEXT NOT NULL,
  term_id TEXT NOT NULL,
  PRIMARY KEY(vocabulary_id,vocabulary_version,alias_id),
  FOREIGN KEY(vocabulary_id,vocabulary_version,term_id)
    REFERENCES vocabulary_terms(vocabulary_id,vocabulary_version,term_id)
);
COMMIT;
