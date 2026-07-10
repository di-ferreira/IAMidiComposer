package store

import (
	"context"
	"database/sql"
	"errors"
	"path/filepath"
	"testing"

	"log/slog"
)

// newTestStore opens a fresh SQLite database in a per-test temp directory,
// runs migrations, and returns the Store plus a cleanup function.
func newTestStore(t *testing.T) (*Store, func()) {
	t.Helper()
	dir := t.TempDir()
	dsn := "file:" + filepath.Join(dir, "test.db")

	logger := slog.New(slog.NewTextHandler(&discardWriter{}, &slog.HandlerOptions{
		Level: slog.LevelDebug,
	}))

	ctx := context.Background()
	s, err := Open(ctx, dsn, logger)
	if err != nil {
		t.Fatalf("Open(%q): %v", dsn, err)
	}
	return s, func() {
		if cerr := s.Close(); cerr != nil {
			t.Errorf("store close: %v", cerr)
		}
	}
}

// discardWriter is a minimal io.Writer used to silence slog output during
// tests without depending on io.Discard unnecessarily.
type discardWriter struct{}

func (discardWriter) Write(p []byte) (int, error) { return len(p), nil }

func TestMigrator_AppliesMigrationsIdempotently(t *testing.T) {
	s, cleanup := newTestStore(t)
	defer cleanup()

	ctx := context.Background()

	// Re-applying migrations must not error and must not duplicate rows.
	if err := NewMigrator(s.DB(), s.logger).Apply(ctx); err != nil {
		t.Fatalf("second Apply: %v", err)
	}
	if err := NewMigrator(s.DB(), s.logger).Apply(ctx); err != nil {
		t.Fatalf("third Apply: %v", err)
	}

	var count int
	if err := s.DB().QueryRowContext(ctx,
		`SELECT COUNT(*) FROM schema_version`,
	).Scan(&count); err != nil {
		t.Fatalf("count schema_version: %v", err)
	}
	if count != 1 {
		t.Fatalf("schema_version rows = %d, want 1", count)
	}

	var version int64
	if err := s.DB().QueryRowContext(ctx,
		`SELECT version FROM schema_version`,
	).Scan(&version); err != nil {
		t.Fatalf("read version: %v", err)
	}
	if version != 1 {
		t.Fatalf("version = %d, want 1", version)
	}

	// Tables must exist exactly once (IF NOT EXISTS guards re-run).
	for _, tbl := range []string{"shared_context", "context_locks"} {
		var n int64
		if err := s.DB().QueryRowContext(ctx,
			`SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = ?`,
			tbl,
		).Scan(&n); err != nil {
			t.Fatalf("sqlite_master %s: %v", tbl, err)
		}
		if n != 1 {
			t.Fatalf("table %s count = %d, want 1", tbl, n)
		}
	}
}

func TestContextRepository_CRUD(t *testing.T) {
	s, cleanup := newTestStore(t)
	defer cleanup()

	ctx := context.Background()
	repo := NewContextRepository(s)

	const id = "11111111-1111-4111-8111-111111111111"
	const name = "Test Composition"
	payload := []byte{0x0a, 0x03, 'f', 'o', 'o'}

	// Create
	if err := repo.Create(ctx, id, name, payload); err != nil {
		t.Fatalf("Create: %v", err)
	}

	// Get
	gotName, gotPayload, gotType, gotUpdated, err := repo.Get(ctx, id)
	if err != nil {
		t.Fatalf("Get: %v", err)
	}
	if gotName != name {
		t.Errorf("name = %q, want %q", gotName, name)
	}
	if !bytesEqual(gotPayload, payload) {
		t.Errorf("payload = %v, want %v", gotPayload, payload)
	}
	if gotType != payloadTypeV1 {
		t.Errorf("payload_type = %q, want %q", gotType, payloadTypeV1)
	}
	if gotUpdated == "" {
		t.Error("updated_at is empty")
	}

	// Update
	updatedPayload := []byte{0x0a, 0x03, 'b', 'a', 'r'}
	if err := repo.UpdatePayload(ctx, id, updatedPayload); err != nil {
		t.Fatalf("UpdatePayload: %v", err)
	}

	// Get after update
	_, gotPayload2, _, gotUpdated2, err := repo.Get(ctx, id)
	if err != nil {
		t.Fatalf("Get after update: %v", err)
	}
	if !bytesEqual(gotPayload2, updatedPayload) {
		t.Errorf("payload after update = %v, want %v", gotPayload2, updatedPayload)
	}
	if gotUpdated2 < gotUpdated {
		t.Errorf("updated_at did not advance: before=%q after=%q", gotUpdated, gotUpdated2)
	}

	// Add a second context to exercise List ordering.
	const id2 = "22222222-2222-4222-8222-222222222222"
	if err := repo.Create(ctx, id2, "Second", payload); err != nil {
		t.Fatalf("Create second: %v", err)
	}

	// List
	summaries, err := repo.List(ctx, 10)
	if err != nil {
		t.Fatalf("List: %v", err)
	}
	if len(summaries) != 2 {
		t.Fatalf("List len = %d, want 2", len(summaries))
	}
	// Most recent update wins: id was updated, id2 was just created.
	// Both should be present; ordering is by updated_at DESC.
	ids := map[string]bool{}
	for _, sc := range summaries {
		ids[sc.ID] = true
	}
	if !ids[id] || !ids[id2] {
		t.Fatalf("List missing ids: %v", ids)
	}

	// Delete
	if err := repo.Delete(ctx, id); err != nil {
		t.Fatalf("Delete: %v", err)
	}

	// Get after delete
	_, _, _, _, err = repo.Get(ctx, id)
	if !errors.Is(err, sql.ErrNoRows) {
		t.Fatalf("Get after delete: err = %v, want %v", err, sql.ErrNoRows)
	}

	// context_locks cascade: verify no orphans for the deleted context.
	var lockCount int64
	if err := s.DB().QueryRowContext(ctx,
		`SELECT count(*) FROM context_locks WHERE context_id = ?`, id,
	).Scan(&lockCount); err != nil {
		t.Fatalf("count context_locks after delete: %v", err)
	}
	if lockCount != 0 {
		t.Errorf("context_locks for deleted context = %d, want 0", lockCount)
	}
}

func TestContextRepository_GetNotFound(t *testing.T) {
	s, cleanup := newTestStore(t)
	defer cleanup()

	ctx := context.Background()
	repo := NewContextRepository(s)

	_, _, _, _, err := repo.Get(ctx, "deadbeef-0000-0000-0000-000000000000")
	if !errors.Is(err, sql.ErrNoRows) {
		t.Fatalf("Get(missing): err = %v, want %v", err, sql.ErrNoRows)
	}
}

func TestContextRepository_UpdateNotFound(t *testing.T) {
	s, cleanup := newTestStore(t)
	defer cleanup()

	ctx := context.Background()
	repo := NewContextRepository(s)

	err := repo.UpdatePayload(ctx, "nonexistent-id", []byte{0x01})
	if !errors.Is(err, sql.ErrNoRows) {
		t.Fatalf("UpdatePayload(missing): err = %v, want %v", err, sql.ErrNoRows)
	}
}

func TestContextRepository_DeleteNotFound(t *testing.T) {
	s, cleanup := newTestStore(t)
	defer cleanup()

	ctx := context.Background()
	repo := NewContextRepository(s)

	err := repo.Delete(ctx, "nonexistent-id")
	if !errors.Is(err, sql.ErrNoRows) {
		t.Fatalf("Delete(missing): err = %v, want %v", err, sql.ErrNoRows)
	}
}

func TestContextRepository_CreateDuplicate(t *testing.T) {
	s, cleanup := newTestStore(t)
	defer cleanup()

	ctx := context.Background()
	repo := NewContextRepository(s)

	const id = "dup-0000-0000-0000-000000000001"
	payload := []byte{0x0a, 0x01, 0x61}
	if err := repo.Create(ctx, id, "first", payload); err != nil {
		t.Fatalf("Create first: %v", err)
	}
	err := repo.Create(ctx, id, "second", payload)
	if err == nil {
		t.Fatal("Create duplicate: err = nil, want error")
	}
}

// bytesEqual compares two []byte slices without importing bytes, keeping
// the dependency surface minimal.
func bytesEqual(a, b []byte) bool {
	if len(a) != len(b) {
		return false
	}
	for i := range a {
		if a[i] != b[i] {
			return false
		}
	}
	return true
}
