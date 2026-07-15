# 0004. Integração Go ACE → C++ MTE

Data: 2026-07-15
Status: Proposta

## Contexto

O ACE (AI Composition Engine) em Go (`engine/go/`) precisa chamar o MTE (Music Theory Engine) em C++20 (`engine/cpp/`, produz `libaimidi_mte.a`) para gerar MIDI determinístico a partir de um `MusicBlueprint`.

O MTE possui 8 engines especializados: ScaleProvider, HarmonyEngine, ChordEngine, BassEngine, RhythmEngine, DrumEngine, PianoEngine e HumanizationEngine. Todos são determinísticos (seed-based) e produzem saída que o MidiRenderer consolida em SMF Type 1 binário.

Atualmente os handlers de composição em Go são stubs que retornam "not implemented". Go não está instalado no ambiente de desenvolvimento local — apenas no CI.

Forças em conflito:

- **Determinismo**: a seed precisa chegar íntegra ao MTE sem flutuação de serialização.
- **Build offline**: o ambiente local não tem Go toolchain; a interface precisa funcionar no CI sem dependências complexas.
- **Performance**: MTE é hot path, mas não em tempo real (composição é assíncrona, tolera latência na ordem de ms).
- **Modularidade**: ACE e MTE devem evoluir independentemente, sem acoplar toolchains.
- **Debuggability**: desenvolvedores precisam testar o MTE isoladamente, sem rodar o ACE.

## Decisao

Adotar um **CLI tool (`mte_cli`)** como boundary entre Go ACE e C++ MTE para a Sprint 8 / M2.

- O diretório `engine/cpp/tools/mte_cli/` conterá um executável C++ que:
  1. Lê um `MusicBlueprint` serializado (JSON ou protobuf) de `stdin`.
  2. Instancia os 8 engines do MTE via DI, alimentando o `SharedContext`.
  3. Executa o pipeline completo (Harmony → Bass → Rhythm → Drum → Piano → Humanization).
  4. Escreve o SMF Type 1 binário resultante em `stdout`.
  5. Escreve metadados (duração, número de eventos, seed usada) em `stderr` como JSON (para logs e verificação).

- O ACE em Go chamará `mte_cli` via `os/exec` com um `context.Context` para timeout e cancelamento.
- O contrato de entrada/saída será versionado em `docs/mte-cli-protocol.md` e verificado por testes golden no CI.
- Para produção futura (pós-MVP), planeja-se migrar para gRPC (unix domain socket) conforme ADR-0001, sem alterar a API interna do MTE — apenas trocando o adaptador de entrada.

## Alternativas consideradas

- **A. cgo com linkagem estática (`libaimidi_mte.a` no binário Go).**
  - Vantagens: chamada de função direta, menor latência (~µs), binário único.
  - Por que não: exige Go toolchain em toda máquina de desenvolvimento (hoje só CI tem Go); cgo acopla build Go à toolchain C++ (clang, cabeçalhos); viola baixo acoplamento e dificulta testar MTE isoladamente; cross-compilação se torna complexa; lock do scheduler Go durante cgo pode afetar concorrência do ACE.

- **B. Shared library (.so/.dylib) carregada dinamicamente no Go via cgo ou `plugin`.**
  - Vantagens: atualizar MTE sem recompilar Go.
  - Por que não: mesmas desvantagens do cgo, acrescidas de DLL hell (versões incompatíveis da .so), problemas de path em distribuição, e `plugin` package em Go é frágil (Linux-only, sem suporte estável em macOS/Windows).

- **C. gRPC microservice.**
  - Vantagens: consistente com a arquitetura atual (ACE↔Plugin já usa gRPC); schema versionado em proto; suporte nativo a streaming, timeout, cancel, tracing; alinhado com ADR-0001.
  - Por que não: overkill para MVP — adiciona servidor gRPC em C++ (complexidade de build com gRPC C++, mais dependências), latência extra (~50-200µs) sem benefício real enquanto ACE roda MTE em composições completas (não por nota); aumenta o escopo da Sprint 8.

## Consequencias

- **Positivas:**
  - Nenhuma dependência cgo no Go — build do ACE continua puro Go, sem toolchain C++.
  - MTE pode ser testado e depurado independentemente: `echo '{blueprint}' | ./mte_cli > output.mid`.
  - Golden tests funcionam no CI comparando SMF binário saída por seed — sem Go envolvido.
  - Implementação rápida para Sprint 8 / M2 — semanas, não meses.
  - Caminho claro para evolução: encapsular o CLI atrás de uma interface Go (`MteClient`) que pode ser substituída por gRPC no futuro sem alterar `WorkflowManager`.

- **Negativas:**
  - Overhead de processo (~1-5ms por `os/exec`: fork+exec, carregamento de libs, inicialização dos engines). Aceitável pois composição é assíncrona e tipicamente leva segundos.
  - Serialização/desserialização necessária (JSON ou proto) entre Go e C++ — custo de parsing vs chamada direta.
  - Gerenciamento de lifecycle: ACE precisa garantir que o processo filho não vaze; timeout via `context.Context` tratado com `kill` do processo.

- **Neutras:**
  - Pode-se migrar para gRPC (Opção C) na fase de produção mantendo o MTE inalterado — cria-se apenas um novo adaptador de entrada (`mte_grpc_server`) que reusa os mesmos engines.
  - ADR-0001 é respeitado: ADR-0001 define gRPC como boundary para produção; este ADR adota CLI como fase intermediária (Sprint 8 / M2).

## Relacionado

- ADR-0001: Boundary C++ ↔ Go — gRPC socket vs FFI inline (define gRPC como destino; este ADR é uma implementação intermediária).
- ADR-0002: DI container C++ (os engines do MTE serão instanciados via DI no `mte_cli`).
- ADR-0003: Serialização do Shared Context (formato usado para passar contexto ao `mte_cli`).
- Skills: `grpc-cpp`, `protobuf-basics`, `juce-realtime`, `arena-allocator`.
- Roadmap: Sprint 8 / M2 — CLI MVP; Sprint N (pós-MVP) — migração para gRPC.
