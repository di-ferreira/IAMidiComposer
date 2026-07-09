---
name: go-context
description: context.Context em Go - cancelamento, timeouts, vec传播. Toda chamada ACE gRPC/tworker segue context; nunca armazenar em struct.
---

# go-context

## Regras

- Primeiro param: `ctx context.Context`.
- Derive via `context.WithCancel`, `context.WithTimeout`, `context.WithValue` (só
  values estruturados como traceID).
- Propagar até go routines filhas; nunca `context.Background()` dentro de handler.
- No-wrapping em struct (`Server{ctx context.Context}`); **proibido**.

## Anti-padroes

- `context.TODO()` em producao (so vale em WIP).
- Manter ctx dentro de field de struct -> data race.

## Com gRPC

```go
func (s *server) Compose(ctx context.Context, req *pb.ComposeRequest) (*pb.ComposeResponse, error) {
    ctx, cancel := context.WithTimeout(ctx, 30*time.Second)
    defer cancel()
    return s.svc.Compose(ctx, req.GetBlueprint())
}
```
