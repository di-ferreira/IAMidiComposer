# AI MIDI Composer (Open Source MVP)

Plataforma Open Source de composição musical assistida por IA, 100% offline.
A IA interpreta intenção; uma Music Theory Engine determinística gera MIDI reproduzível por seed.
O usuário mantém controle total na sua DAW favorita.

---

## Estado atual (baseline M0 — 09/07/2026)

| Componente | Estado |
|---|---|
| Workspace de agentes (38 = 14 primários + 24 subagentes) | ✅ |
| Skills (56) e registries YAML (5) | ✅ |
| Build C++ verde: `libaimidi_mte.a` + GoogleTest | ✅ |
| `proto/aimidi/v1/aimidi.proto` (3/10 RPCs) | ⚠️ |
| `engine/cpp/MTE` — apenas `ScaleProvider` real; `HarmonyEngine` = stub | ⚠️ |
| `engine/go/ACE` — `main.go` só `select{}` | ❌ |
| `plugin/` JUCE VST3 — `processBlock` só `buffer.clear()` | ⚠️ |
| CI — 4 jobs (cpp/go bloqueantes, plugin/lint não-bloqueantes), Ubuntu only | ⚠️ |
| SQLite, IA (llama.cpp/ONNX), Audio Analysis | ❌ |

**Completude aproximada: ~16%** — ver análise do CTO em [`docs/roadmap/ROADMAP.md`](docs/roadmap/ROADMAP.md).

---

## Objetivos do MVP

- 100% Offline
- Open Source (MIT ou Apache 2.0)
- Multiplataforma
- Alto desempenho
- Modular
- Compatível com qualquer DAW VST3
- Sem dependência de APIs
- IA apenas para decisões criativas
- Engine determinística para geração musical

---

## Stack Tecnológica

### Linguagens

| Camada         | Tecnologia      |
| -------------- | --------------- |
| Plugin         | C++20           |
| Engine Musical | C++20           |
| MIDI Engine    | C++20           |
| Engine de IA   | Go              |
| Comunicação    | gRPC + Protobuf |

### Frameworks

| Tecnologia     | Uso                |
| -------------- | ------------------ |
| JUCE           | Plugin VST3        |
| llama.cpp      | Inferência local   |
| ONNX Runtime   | Modelos auxiliares |
| SQLite         | Banco local        |
| spdlog         | Logs               |
| GoogleTest     | Testes             |
| CMake          | Build              |
| GitHub Actions | CI/CD              |

---

## Arquitetura Geral

```text
┌───────────────────────────────────────────────────────────────────────┐
│                           DAW (Host)                                   │
│      Reaper | Cubase | Studio One | FL | Ableton | Bitwig              │
└───────────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌───────────────────────────────────────────────────────────────────────┐
│                    AI MIDI Plugin (JUCE / C++)                        │
│ • Prompt UI • Piano Roll Preview • Workflow Manager • Settings        │
└───────────────────────────────────────────────────────────────────────┘
                               │  gRPC local (unix socket / named pipe)
                               ▼
╔══════════════════════════════════════════════════════════════════════╗
║                  AI COMPOSITION ENGINE (ACE) — Go                     ║
╠═══════════════════════════════════════════════════════════════════[...║
║ 1. Shared Musical Context (SMC) — SQLite                              ║
║    Tempo · Tom · Escala · Estrutura · Progressão · Instrumentos ·     ║
║    Histórico · Preferências · Locks                                   ║
╠═══════════════════════════════════════════════════════════════════════╣
║ 2. Musical Intelligence (MI) — llama.cpp/ONNX local                   ║
║    Prompt Interpreter · Style/Gerne/Mood Detection · Blueprint Gen    ║
║    → Music Blueprint                                                  ║
╠═══════════════════════════════════════════════════════════════════════╣
║ 3. Musical Planning Layer (MPL)                                       ║
║    Timeline · Section · Track · Instrument · Chord/Rhythm/Melody BP   ║
╠═══════════════════════════════════════════════════════════════════════╣
║ 4. Music Theory Engine (MTE) — C++20 (gRPC server, ADR-0001)          ║
║    Harmony · Chords · Voice Leading · Counterpoint · Melody ·         ║
║    Rhythm · Bass · Drum · Guitar · Piano · Strings · Humanization     ║
╠═══════════════════════════════════════════════════════════════════════╣
║ 5. MIDI Rendering Engine (MRE) — SMF1 export + live event stream      ║
╠═══════════════════════════════════════════════════════════════════════╣
║ 6. Audio Intelligence Layer (AIL) — BPM/Key/Chord/Beat detection      ║
╠═══════════════════════════════════════════════════════════════════════╣
║ 7. Instrument Integration Layer (IIL) — Scanner · DB · Mapper ·       ║
║    Preset Manager · Templates por estilo                              ║
╚═══════════════════════════════════════════════════════════════════════╝
```

> Boundary C++ (MTE) ↔ Go (ACE) via gRPC local — ver [`ADR-0001`](docs/adr/ADR-0001-boundary-cpp-go.md).

---

## Fluxo

```text
Prompt

↓

Musical Intelligence

↓

Music Blueprint

↓

Planning Layer

↓

Music Theory Engine

↓

MIDI Rendering Engine

↓

Shared Musical Context

↓

Plugin

↓

Usuário
```

---

## Workflows (10)

1. New Composition
2. Instrument Composer
3. Audio Assisted Composer
4. Continue Composition
5. Smart Regeneration
6. Generate Variations
7. Replace Instrument
8. Reharmonize
9. Orchestrate
10. Arrange

Detalhes em [`docs/workflows/`](docs/workflows/) e registry em [`.opencode/registry/workflows.yaml`](.opencode/registry/workflows.yaml).

**Status atual: 0/10 workflows implementados** (apenas 3/10 têm RPC declarado no `.proto`).

---

## Roadmap

O roadmap canônico completo está em [`docs/roadmap/ROADMAP.md`](docs/roadmap/ROADMAP.md).
A tabela abaixo resume os milestones M0–M7:

| Milestone | Fase | Status |
|---|---|---|
| **M0** — Fundação + contratos | Fase 0 | ⚠️ Em curso |
| **M1** — Foundation (gRPC, SQLite, DI, plugin bridge) | Fase 1 | ❌ Não iniciada |
| **M2** — Spike pop/rock MTE completo (primeiro som) | Fase 2 | ❌ |
| **M3** — Musical Intelligence local (prompt NL → blueprint) | Fase 3 | ❌ |
| **M4** — MTE horizontal (todos instrumentos/modos) | Fase 4 | ❌ |
| **M5** — Audio Engine (Workflow 3) | Fase 5 | ❌ |
| **M6** — Plugin UI + Piano Roll + DX | Fase 6 | ❌ |
| **M7** — Hardening + bench + release v0.1.0 | Fase 7 | ❌ |

Resumo executivo: 3/63 checkboxes do MVP **feitas**; 14 **parciais**; 46 **não feitas**.

---

## Observação técnica sobre o MVP

Para manter o escopo controlado e entregar valor rapidamente, o MVP foca nos seguintes
módulos musicais:

- **Chord Engine**
- **Bass Engine**
- **Drum Engine**
- **Melody Engine**
- **Shared Musical Context**
- **New Composition**, **Instrument Composer** e **Audio Assisted Composer**

Os demais workflows (Continue Composition, Smart Regeneration, Reharmonize, Orchestrate
etc.) serão implementados sobre essa mesma base arquitetural, sem necessidade de
refatoração. Essa abordagem entrega um núcleo sólido, útil para a comunidade open source
e preparado para evoluir gradualmente sem comprometer a arquitetura inicial. Ver
[`docs/roadmap/ROADMAP.md`](docs/roadmap/ROADMAP.md) para o plano detalhado.

---

## Documentação

- [`CONTEXT.md`](CONTEXT.md) — missão, filosofia, escopo, objetivos.
- [`docs/roadmap/ROADMAP.md`](docs/roadmap/ROADMAP.md) — roadmap canônico M0–M7.
- [`docs/adr/`](docs/adr/) — Architecture Decision Records (ADR-0001 aceito).
- [`docs/architecture/overview.md`](docs/architecture/overview.md) — visão arquitetural (placeholder).
- [`docs/workflows/`](docs/workflows/) — um documento por workflow (a criar).
- [`standards/`](standards/) — padrões arquiteturais e de código.
- [`prompts/master_prompt.md`](prompts/master_prompt.md) — prompt-fonte canônico dos agentes.
- [`.opencode/`](.opencode/) — workspace de agentes, subagentes, skills e registries.

---

## Contribuir

Ainda não há `CONTRIBUTING.md` formal (previsto na Fase 12). Ver
[`standards/branching.md`](standards/branching.md),
[`standards/pull_request.md`](standards/pull_request.md) e
[`standards/commits.md`](standards/commits.md) para padrões em uso.
