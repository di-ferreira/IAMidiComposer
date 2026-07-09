---
name: idiomatic-go
description: Go idiomático usado na ACE - interfaces pequenas, erros explícitos (erros são valores), context para cancelamento/timeouts, nao loops feios. Doc comentario sobre exportados.
---

# idiomatic-go

## Regras no projeto (ACE Go)

- **Interfaces definidas pelo consumidor** (`type FooClient interface { ... }`), nao
  pela implementacao.
- **errors is values**: retorne `error`, verifique, ao menos que wrapped com `fmt.Errorf("...
  %w", err)`.
- **context.Context** como 1o param em todo API de longa duracao; nao armazenar em
  struct.
- **Doc comment dar** em toda exported identifier.
- **Nao** padrao: panics, global state, init() com efeitos colaterais; CGO minimo.

## Padrões extras

- `golang.org/x/sync/errgroup` para task groups paralelos.
- `cmp` em vez de `if ==` em mqat comparisons.
- Struct tags:jax `json:"field"` consistentes.

## Anti-padroes

- `interface{}` (prefira `any` em 1.18+).
- `reflect` em hot paths.
- `gofmt`-fallthrough: sempre `goimports` em CI.
