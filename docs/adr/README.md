# ADR — Architecture Decision Records

Cada decisao arquitetural relevante vira um ADR. Ver skill `adr-format` para template.
ADR válido em PR: revisado por **Software Architect** + **CTO**.

## Numeracao

NNNN (4 dígitos, zero-padded); primeiro ADR = `0001`. Número nunca é reusado;
mudança de decisão exige um novo ADR que substitui o anterior (status `Substituida por NNNN`).

## Status

- `Proposta` — aberto, aguardando revisão.
- `Aceita` — aprovado por Software Architect + CTO e mergeado.
- `Depreciada` — substituído ou obsoleto sem substituto direto.
- `Substituida por NNNN` — substituído por nova decisão.

## Index

### Vigentes

- **[ADR-0001 — Boundary C++ ↔ Go: gRPC socket vs FFI inline](ADR-0001-boundary-cpp-go.md)** — Status: Proposta. Fase 0 do roadmap.

### Previstos (a abrir conforme roadmap)

| ADR | Título | Fase |
|---|---|---|
| ADR-0002 | DI framework C++ (boost::di vs manual ServiceLocator) | Fase 1.1 |
| ADR-0003 | Serialização do Shared Musical Context (proto vs JSON) | Fase 1.3 |
| ADR-0004 | MTE como lib estática linkada no ACE Go, ou processo separado? (pode confirmar ou reverter ADR-0001) | Fase 2.4 |
| ADR-0005 | Wrapper llama.cpp via cgo vs Python sidecar vs ONNX-only | Fase 3.1 |
| ADR-0006 | AI model distribution (download first-run vs bundled <2GB) | Fase 7.3 |
| ADR-0007 | CLAP parity com VST3 | Fase 6.1 |

## Histórico (placeholders removidos)

Os três placeholders anteriores (`bind-cpp-go-mte`, `stream-grpc-vs-async-saga`, `modelos-llama-vs-onnx`) foram absorvidos, respectivamente, por ADR-0001, ADR-0005 (a abrir) e ADR-0005 (a abrir).
