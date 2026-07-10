// Package store manages SQLite persistence for the AI Composition Engine.
//
// The store layer owns:
//   - the database connection (modernc.org/sqlite, pure-Go, no cgo);
//   - the versioned migration runner (see migrations/*.sql);
//   - repositories exposing small interfaces to the rest of the engine.
//
// Design rules (see CONTEXT.md and the database-engineer agent charter):
//   - every IO function takes a context.Context as its first argument;
//   - the Shared Musical Context payload is an opaque []byte (proto binary
//     per ADR-0003); this package never decodes/validates proto bytes here;
//   - no package-level mutable state; dependencies are injected.
package store

import (
	"context"
	"database/sql"
	_ "embed"
	"errors"
	"fmt"
	"log/slog"
)

//go:embed migrations/0001_init.sql
var migration_0001_init string

// migrations is the ordered list of embedded SQL migrations.
// Index i corresponds to schema_version.version = i+1.
var migrations = []string{migration_0001_init}

// Migrator applies versioned SQL migrations to the SQLite database.
//
// Each migration runs in its own transaction and is recorded in
// schema_version. Re-running Apply is idempotent: already-applied
// migrations are skipped.
type Migrator struct {
	db     *sql.DB
	logger *slog.Logger
}

// NewMigrator builds a Migrator bound to db.
func NewMigrator(db *sql.DB, logger *slog.Logger) *Migrator {
	return &Migrator{db: db, logger: logger}
}

// Apply runs every pending migration in order.
//
// It first ensures schema_version exists (created with IF NOT EXISTS
// in its own transaction), then iterates migrations. For migration i
// (0-indexed) it checks whether version i+1 is already recorded; if
// not, it executes the SQL and inserts the version row, all in one
// transaction.
func (m *Migrator) Apply(ctx context.Context) error {
	// Ensure the bookkeeping table exists before anything else.
	if _, err := m.db.ExecContext(ctx, `CREATE TABLE IF NOT EXISTS schema_version (
		version     INTEGER PRIMARY KEY,
		applied_at  TEXT NOT NULL DEFAULT (datetime('now'))
	);`); err != nil {
		return fmt.Errorf("ensure schema_version: %w", err)
	}

	for i, stmt := range migrations {
		version := int64(i + 1)

		var existing int64
		err := m.db.QueryRowContext(ctx,
			`SELECT version FROM schema_version WHERE version = ?`, version,
		).Scan(&existing)
		switch {
		case err == nil:
			m.logger.InfoContext(ctx, "migration.skip",
				"version", version,
				"reason", "already applied",
			)
			continue
		case errors.Is(err, sql.ErrNoRows):
			// proceed to apply
		default:
			return fmt.Errorf("query schema_version v%d: %w", version, err)
		}

		tx, err := m.db.BeginTx(ctx, nil)
		if err != nil {
			return fmt.Errorf("begin tx for migration v%d: %w", version, err)
		}
		commit := func() error { return tx.Commit() }
		rollback := func() {
			if txErr := tx.Rollback(); txErr != nil && !errors.Is(txErr, sql.ErrTxDone) {
				m.logger.WarnContext(ctx, "migration.rollback",
					"version", version,
					"error", txErr.Error(),
				)
			}
		}

		if _, err := tx.ExecContext(ctx, stmt); err != nil {
			rollback()
			return fmt.Errorf("exec migration v%d: %w", version, err)
		}
		if _, err := tx.ExecContext(ctx,
			`INSERT INTO schema_version (version, applied_at) VALUES (?, datetime('now'))`,
			version,
		); err != nil {
			rollback()
			return fmt.Errorf("record migration v%d: %w", version, err)
		}
		if err := commit(); err != nil {
			return fmt.Errorf("commit migration v%d: %w", version, err)
		}

		m.logger.InfoContext(ctx, "migration.applied",
			"version", version,
			"index", i,
		)
	}
	return nil
}
