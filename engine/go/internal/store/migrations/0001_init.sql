-- ADR-0003: Shared Musical Context persistence.
-- Schema MVP: apenas a tabela shared_context + tabelas auxiliares.
CREATE TABLE IF NOT EXISTS schema_version (
    version     INTEGER PRIMARY KEY,
    applied_at  TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS shared_context (
    id           TEXT PRIMARY KEY,               -- UUID v4 string
    name         TEXT NOT NULL DEFAULT 'Untitled',
    payload      BLOB NOT NULL,                  -- proto binario (ver ADR-0003)
    payload_type TEXT NOT NULL DEFAULT 'application/vnd.aimidi.context.v1+protobuf',
    created_at   TEXT NOT NULL DEFAULT (datetime('now')),
    updated_at   TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE INDEX IF NOT EXISTS idx_shared_context_updated_at
    ON shared_context(updated_at DESC);

-- locks por regiao (compassos) sobre um contexto.
CREATE TABLE IF NOT EXISTS context_locks (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    context_id   TEXT NOT NULL REFERENCES shared_context(id) ON DELETE CASCADE,
    track_name   TEXT NOT NULL,                  -- "harmony", "drums", "piano", ...
    start_bar    INTEGER NOT NULL,
    end_bar      INTEGER NOT NULL,
    reason       TEXT NOT NULL DEFAULT 'manual',
    created_at   TEXT NOT NULL DEFAULT (datetime('now')),
    UNIQUE(context_id, track_name, start_bar, end_bar)
);

CREATE INDEX IF NOT EXISTS idx_context_locks_context
    ON context_locks(context_id, track_name);
