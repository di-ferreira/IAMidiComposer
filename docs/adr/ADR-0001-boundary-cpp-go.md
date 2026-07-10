# 0001. Boundary C++ ↔ Go — gRPC socket vs FFI inline

Data: 2026-07-09
Status: Aceita

Aprovado em 2026-07-10 por:
- Software Architect (design modular e contratos gRPC/protobuf)
- CTO (trade-off aceito: latência de IPC bem menor que custo de cgo em macOS; contrato versionado compensa acoplamento)

## Contexto

O AI MIDI Composer tem duas camadas de engine com linguagens diferentes:

- **Music Theory Engine (MTE)** em C++20 — determinística, hot path, alta performance, arena allocator, SIMD-latência previsível.
- **AI Composition Engine server (ACE)** em Go — gRPC server, workflow manager, persistência SQLite, integração com llama.cpp/ONNX.

O `WorkflowManager` em Go precisa orquestrar chamadas ao MTE para gerar MIDI a partir de um `MusicBlueprint`. A pergunta é: **como o código Go chama o C++ MTE?** Esta decisão impacta performance, deployment, build, debugging, segurança de threading e a.facilidade de adicionar novos módulos musicais.

Forças em confloito:

- Performance: MTE é o hot path crítico (bench targets em `standards/performance.md`).
- Modularidade: master prompt exige baixo acoplamento e Clean Architecture (§7) — não acoplar Go a símbolos C++internos.
- Offline e cross-platform: build precisa funcionar em Win/macOS/Linux.
- Determinismo: a seed precisa chegar íntegra ao MTE sem flutuação de serialização.
- Ergonomia de desenvolvimento: contributors Open Source precisam rodar/testar MTE isoladamente.

## Decisao

**Adotar boundary por gRPC local (unix domain socket em POSIX, named pipe em Windows)** entre ACE (Go) e MTE (C++) servido como processo separado.

- MTE roda como **servidor gRPC em C++** ouvindo em socket local (ex.: `/tmp/aimidi-mte.sock` em Linux/macOS, `\\.\pipe\aimidi-mte` em Windows).
- ACE em Go atua como cliente gRPC do MTE para todos os fluxos musicais.
- Definição de serviço no `proto/aimidi/v1/aimidi.proto` (a ser estendido com `MusicTheoryService` em ADR-0004 com detalhes).
- Plugin VST3 fala apenas com ACE (composição.go), nunca diretamente com MTE — mantém o §3.1 do master prompt ("Plugin nunca executa composição").

Esta proposta será revisada quando o ADR-0004 (lib estática vs processo separado) ser aberto na Fase 2.4. **ADR-0001 declara o boundary como gRPC; ADR-0004 decide se há também uma forma in-process para hot path.**

## Alternativas consideradas

- **A. cgo com a libaimidi_mte.a linkada no binário Go (FFI in-process).**
  - Vantagens: zero rede/IPC, menor latência (~µs), menos processos para empacotar.
  - Por que não: acopla binário Go a toolchain clang e cabeçalhos C++; cgo tem custo de runtime (lock de scheduler, GC boundary); dificulta rodar MTE isolado para testes; viola o princípio de.baixo acoplamento entre modules (um contributor novo teria que recompilar Go ao mexer em C++); problemático em macOS (entitlements, codesigning). Aceitável somente se provado em ADR-0004 que a latência do gRPC é inaceitável e foi refutada.

- **B. FFI direto sem gRPC — expor API C pura (header `aimidi_mte.h`) e cgo wrapper manual.**
  - Vantagens: menos overhead que cgo para tipos C, controla ABI.
  - Por que não: expressividade perdida; quem escreve o wrapper é responsável por serialização/desserialização manual, repetindo o trabalho do protobuf; fica a cargo de engineers manter dois schemas sincronizados — alto risco de drift manual entre o schema proto e a ABI C;西山 viola Clean Architecture ao expor internals C++ em cabeçalho C consumido por outra camada.

- **C. JSON sobre stdin/stdout (subprocess).**
  - Vantagens: trivial de debugar, qualquer linguagem consegue falar.
  - Por que não: alto overhead de parsing, sem schema estrito, sem streaming bidirecional, sem timeout/cancel estruturado (Go `context` não fala com subprocess sem convenção custom).

- **D. named pipe/socket local com formato binário proprietário (sem gRPC).**
  - Vantagens: minimiza deps.
  - Por que não: re-implementa framing, versionamento, streaming, interceptors (logs/tracing/retry/cancel) que gRPC já resolve. Incompatível com a ecosystem de profilers, tracing (OpenTelemetry gRPC), e contributors já familiarizados com gRPC.

## Consequencias

- **Positivas:**
  - Contrato MTE-Go versionado em `proto/aimidi/v1/` — só.schema evolui via PR revisado.
  - MTE pode ser rebuildado e reiniciado sem rebuildar Go; encaixa-se em CI incremental.
  - WorkflowManager chama MTE com `context.Context` Go, ganhando timeout, cancel e tracing nativos.
  - Plugin|x não tem dependência de toolchain C++; só consome gRPC para ACE.
  - Testes de MTE isolados (já existem com GoogleTest). Testes do ACE usam gRPC mock.
  - Adequado ao padrão Clean Arch. da `standards/architecture.md`.

- **Negativas:**
  - Latência de IPC por socket (~50-200µs cada chamada em Linux local). Mitigada por batching: `GenerateComposition(blueprint) -> CompositionResponse` em uma RPC, não chamada por compasso.
  - Mais um binário/processo para empacotar releases. DevOps cria launcher que sobe ACE + MTE.
  - Strongly-typed proto entre MTE e Go: requer attention ao schema em cada PR musical (revisão do Software Architect).

- **Neutras:**
  - MTE vira servidor gRPC com handlers implementados em C++ (`grpc-cpp` skill); DI também via interfaces C++ (ADR-0002).

## Relacionado

- ADR-0002: DI framework C++ (a abrir).
- ADR-0003: Serialização do Shared Context (a abrir).
- ADR-0004: MTE como lib estática linkada no ACE Go, ou processo separado? (a abrir na Fase 2.4 — pode confirmar ou reverter este ADR-0001).
- Skills: `grpc-cpp`, `protobuf-basics`, `juce-realtime`.
- Standards: `standards/architecture.md`, `standards/realtime_audio.md`, `standards/performance.md`.
- Roadmap: ver `docs/roadmap/ROADMAP.md` Fase 0 (1.2 motor gRPC) e Fase 2.4.
