package store

import (
	"context"
	"database/sql"
	"errors"
	"fmt"
	"log/slog"
)

// payloadTypeV1 is the MIME-ish identifier for the Shared Musical Context
// proto binary payload, as defined by ADR-0003.
const payloadTypeV1 = "application/vnd.aimidi.context.v1+protobuf"

// ContextSummary is the lightweight view returned by List, suitable for
// UI project pickers without loading the full proto BLOB.
type ContextSummary struct {
	ID        string
	Name      string
	UpdatedAt string
}

// ContextRepository persists Shared Musical Context BLOBs in SQLite.
//
// The payload is treated as opaque []byte: this layer neither decodes nor
// validates proto bytes (ADR-0003). Version migration of the proto payload
// is the responsibility of a future Migrator, not this repository.
type ContextRepository struct {
	db     *sql.DB
	logger *slog.Logger
}

// NewContextRepository builds a ContextRepository backed by s.
func NewContextRepository(s *Store) *ContextRepository {
	return &ContextRepository{db: s.DB(), logger: s.logger}
}

// Create inserts a new Shared Musical Context.
//
// payload_type defaults to the ADR-0003 proto MIME. Returns an error if
// the row is not inserted (e.g. duplicate id).
func (r *ContextRepository) Create(ctx context.Context, id, name string, payload []byte) error {
	res, err := r.db.ExecContext(ctx,
		`INSERT INTO shared_context (id, name, payload, payload_type)
		 VALUES (?, ?, ?, ?)`,
		id, name, payload, payloadTypeV1,
	)
	if err != nil {
		return fmt.Errorf("create context %q: %w", id, err)
	}
	n, err := res.RowsAffected()
	if err != nil {
		return fmt.Errorf("create context %q: rows affected: %w", id, err)
	}
	if n < 1 {
		return fmt.Errorf("create context %q: no rows affected", id)
	}
	return nil
}

// Get retrieves a Shared Musical Context by id.
//
// On miss it returns an error wrapping sql.ErrNoRows so callers can use
// errors.Is to distinguish "not found" from a real failure.
func (r *ContextRepository) Get(ctx context.Context, id string) (name string, payload []byte, payloadType string, updatedAt string, err error) {
	err = r.db.QueryRowContext(ctx,
		`SELECT name, payload, payload_type, updated_at
		 FROM shared_context
		 WHERE id = ?`,
		id,
	).Scan(&name, &payload, &payloadType, &updatedAt)
	if err != nil {
		return "", nil, "", "", fmt.Errorf("context %q: %w", id, err)
	}
	return name, payload, payloadType, updatedAt, nil
}

// UpdatePayload replaces the proto BLOB and bumps updated_at.
//
// Returns an error (wrapping sql.ErrNoRows in the message) if the id
// does not exist, so callers can detect a stale-update scenario.
func (r *ContextRepository) UpdatePayload(ctx context.Context, id string, payload []byte) error {
	res, err := r.db.ExecContext(ctx,
		`UPDATE shared_context
		    SET payload = ?, updated_at = datetime('now')
		  WHERE id = ?`,
		payload, id,
	)
	if err != nil {
		return fmt.Errorf("update context %q: %w", id, err)
	}
	n, err := res.RowsAffected()
	if err != nil {
		return fmt.Errorf("update context %q: rows affected: %w", id, err)
	}
	if n < 1 {
		return fmt.Errorf("update context %q: %w", id, sql.ErrNoRows)
	}
	return nil
}

// Delete removes a Shared Musical Context permanently.
//
// context_locks rows cascade via ON DELETE CASCADE.
func (r *ContextRepository) Delete(ctx context.Context, id string) error {
	res, err := r.db.ExecContext(ctx,
		`DELETE FROM shared_context WHERE id = ?`, id,
	)
	if err != nil {
		return fmt.Errorf("delete context %q: %w", id, err)
	}
	n, err := res.RowsAffected()
	if err != nil {
		return fmt.Errorf("delete context %q: rows affected: %w", id, err)
	}
	if n < 1 {
		return fmt.Errorf("delete context %q: %w", id, sql.ErrNoRows)
	}
	return nil
}

// List returns at most limit Shared Musical Contexts ordered by the most
// recently updated. limit <= 0 falls back to a reasonable default.
func (r *ContextRepository) List(ctx context.Context, limit int) ([]ContextSummary, error) {
	if limit <= 0 {
		limit = 50
	}
	rows, err := r.db.QueryContext(ctx,
		`SELECT id, name, updated_at
		 FROM shared_context
		 ORDER BY updated_at DESC
		 LIMIT ?`,
		limit,
	)
	if err != nil {
		return nil, fmt.Errorf("list contexts: %w", err)
	}
	defer func() {
		if closeErr := rows.Close(); closeErr != nil {
			r.logger.WarnContext(ctx, "context_repo.list.rows_close",
				"error", closeErr.Error(),
			)
		}
	}()

	var out []ContextSummary
	for rows.Next() {
		var s ContextSummary
		if err := rows.Scan(&s.ID, &s.Name, &s.UpdatedAt); err != nil {
			return nil, fmt.Errorf("list contexts scan: %w", err)
		}
		out = append(out, s)
	}
	if err := rows.Err(); err != nil {
		return nil, fmt.Errorf("list contexts rows: %w", err)
	}
	return out, nil
}

// Compile-time guard: ensure the expected error is reachable for callers
// using errors.Is.
var _ = errors.Is
