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

| ADR | Título | Status | Fase |
|---|---|---|---|---|
| **[ADR-0001](ADR-0001-boundary-cpp-go.md)** | Boundary C++ ↔ Go: gRPC socket vs FFI inline | **Aceita** | Fase 0 |
| **[ADR-0002](ADR-0002-di-container-cpp.md)** | DI container (ServiceLocator) no MTE C++20 | Proposta | Fase 1.1 |
| **[ADR-0003](ADR-0003-shared-context-serialization.md)** | Serialização do Shared Musical Context (proto vs JSON) | Proposta | Fase 1.3 |
| **[ADR-0004](ADR-0004-mte-integration-go.md)** | MTE como CLI vs servidor gRPC persistente | **Aceita** | Fase 2.4 |
| **[ADR-0005](ADR-0005-llm-integration.md)** | Integração LLM Local (llama.cpp) | **Aceita** | Fase 3.1 |
| **[ADR-0006](ADR-0006-ai-model-distribution.md)** | AI model distribution (download first-run) | **Aceita** | Fase 7.3 |

### Previstos (a abrir conforme roadmap)

| ADR | Título | Fase |
|---|---|---|
| ADR-0007 | CLAP parity com VST3 | Fase 6.1 |

## Histórico (placeholders removidos)

Os três placeholders anteriores (`bind-cpp-go-mte`, `stream-grpc-vs-async-saga`, `modelos-llama-vs-onnx`) foram absorvidos, respectivamente, por ADR-0001, ADR-0005 (a abrir) e ADR-0005 (a abrir).
