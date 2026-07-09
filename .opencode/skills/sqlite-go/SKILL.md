---
name: sqlite-go
description: Acesso SQLite em Go no projeto - modernc.org/sqlite (puro Go, sem cgo) por padrao, migracoes versionadas, repository pattern com interfaces.
---

# sqlite-go

## Driver

- `modernc.org/sqlite` - recommendado (puro Go, sem cgo, cross-build trivial).
- Fallback: `github.com/mattn/go-sqlite3` (CGO) - se houver pressão por performance
  extrema (raro).
- DSN: `file:aimidi.db?_pragma=foreign_keys(1)&_pragma=journal_mode(WAL)&_pragma=busy_timeout(5000)`

## Migracoes

- Em `engine/go/internal/store/migrations/0001_init.sql`, `0002_*.sql`, ...
- Aplicar em ordem; rastrear em `schema_migrations` (version).
- Testes in-memory (`file::memory:?cache=shared`).

## Repository pattern

```go
type ProjectRepository interface {
    Get(ctx, id) (*Project, error)
    Save(ctx, *Project) error
    List(ctx, filter) ([]Project, error)
}
```

- Implementacao SQLite injetada via DI em handlers gRPC.

## Anti-padroes

- `SELECT *` (fragil a novo campo).
- SQL inline em handlers gRPC; passar por interface.
- GedContributList blog outside repository - adia DI; mantem coeso.
