# Go Guidelines — AI MIDI Composer

## Padrao

- **Go 1.22+**.
- `gofmt` + `goimports` enforcing no CI (`-l` flag falha).
- `golangci-lint` com `.golangci.yml` em repo (habilitar errcheck, staticcheck,
  revive, gosec, misspell, dupl).

## Estrutura

```
engine/go/
    cmd/ace/main.go           # entrypoint
    internal/server/          # gRPC servers
    internal/service/         # domain services (comportamento)
    internal/store/           # persistence SQLite (interfaces + impls)
    internal/proto/           # generated protobuf (CI regenera, nao commit)
    internal/mi/              # Musical Intelligence bindings (CGO ou gRPC)
    internal/mte/             # Music Theory Engine bindings
    internal/audio/           # Audio Analysis bindings
    internal/sharedcontext/   # Shared Musical Context
    internal/workflow/        # Workflow Manager
    internal/seed/            # RNG deterministico
    internal/log/             # slog setup
```

## Estilo

- Pacotes pequenos; nomes curtos `store`, `server`, `seedera`.
- Errors returned explicitos; nunca panic em library code.
- `context.Context` como 1o arg em todo API que bloqueia.
- `slog` para logging; sem `fmt.Printf` em produção.
- `errors.Is`/`errors.As` em vez de `== `.

## Tests

- `table_test`s + `t.Parallel` + cmp Diff (ver `go-testing`).
- Cobertura mínima: 70% em domain; 85% em service.
- Fuzz para parser (ver `fuzz-testing`).
- Benchmarks com `b.ReportMetric` em criticos.

## Performance

- Profiling com pprof; sem alloc em hot loops (ver `profiling`).
- `sync.Pool` para mensagens internas reutilizadas.

## Build

- `go.mod` curado; `vendor/` opcional apenas para CI offline (não commit default).
- `goreleaser` para releases multi-platform (cli ACE).

## CGO

- Evitar; se necessário (binding C++ da MTE), manter isolado em `internal/mte/c`.
- Documentar em ADR antes de usar CGO.
