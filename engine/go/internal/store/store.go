package store

import (
	"context"
	"database/sql"
	"fmt"
	"log/slog"

	_ "modernc.org/sqlite"
)

// Store owns the SQLite connection and provides access to repositories.
//
// The connection is configured for single-writer (MaxOpenConns=1) usage
// with WAL journaling and foreign keys enabled, mirroring the constraints
// described in CONTEXT.md (100% offline, deterministic, low latency).
type Store struct {
	db     *sql.DB
	logger *slog.Logger
}

// Open connects to the SQLite database at dsn, applies pragmas and any
// pending migrations, and returns a ready Store.
//
// The dsn should be a filesystem path prefixed with "file:" (e.g.
// "file:/tmp/ace.db"); for in-memory databases the engine builds a DSN
// like "file::memory:?cache=shared".
func Open(ctx context.Context, dsn string, logger *slog.Logger) (*Store, error) {
	db, err := sql.Open("sqlite", dsn)
	if err != nil {
		return nil, fmt.Errorf("open sqlite %q: %w", dsn, err)
	}

	pragmas := []string{
		"PRAGMA journal_mode=WAL;",
		"PRAGMA foreign_keys=ON;",
		"PRAGMA busy_timeout=5000;",
	}
	for _, p := range pragmas {
		if _, err := db.ExecContext(ctx, p); err != nil {
			db.Close()
			return nil, fmt.Errorf("exec pragma %q: %w", p, err)
		}
	}

	// SQLite serializes writes on a single file; capping at one open
	// connection avoids lock contention and keeps the semantics simple.
	db.SetMaxOpenConns(1)

	if err := db.PingContext(ctx); err != nil {
		db.Close()
		return nil, fmt.Errorf("ping sqlite %q: %w", dsn, err)
	}

	if err := NewMigrator(db, logger).Apply(ctx); err != nil {
		db.Close()
		return nil, fmt.Errorf("apply migrations: %w", err)
	}

	return &Store{db: db, logger: logger}, nil
}

// Close releases the database connection.
func (s *Store) Close() error { return s.db.Close() }

// DB returns the underlying *sql.DB.
//
// This is an escape hatch intended for tests and repository constructors
// within the store package; external callers should prefer the typed
// repository methods instead of issuing raw SQL.
func (s *Store) DB() *sql.DB { return s.db }
