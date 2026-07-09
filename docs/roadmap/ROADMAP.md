# Roadmap de Desenvolvimento — AI MIDI Composer

> Plano canônico de desenvolvimento. Dono: **CTO Agent** (`.opencode/agents/cto.md`).
> Atualizações só via PR com aprovação de CTO + Software Architect.
> Documento narrativo em PT-BR (ver `standards/documentation.md`); termos técnicos em EN.

---

## Estado atual (baseline M0 — 09/07/2026)

| Componente | Estado | Observação |
|---|---|---|
| Workspace agentes (38 arquivos `.md`) | ✅ | 14 primários + 24 subagentes |
| Skills registry (56 diretórios) | ✅ | Cada um com `SKILL.md` |
| YAML registros (5 arquivos) | ✅ | `agents/subagents/skills/workflows/standards.yaml` |
| `proto/aimidi/v1/aimidi.proto` | ⚠️ | 1 serviço, 3/10 RPCs, 8 mensagens |
| `engine/cpp/MTE` (`libaimidi_mte.a` 687 KB) | ⚠️ | Só `ScaleProvider` (7 escalas) real; `HarmonyEngine` = stub vazio |
| `engine/go/ACE` (`main.go` 25 linhas) | ⚠️ | Apenas loga e `select{}`; sem `go.mod` real, sem handlers |
| `plugin/` (JUCE VST3 esqueleto) | ⚠️ | `processBlock` só faz `buffer.clear()`; editor desenha string |
| `.github/workflows/ci.yml` | ⚠️ | 4 jobs (cpp/go blocking, plugin/lint non-blocking); Ubuntu only |
| `docs/roadmap/`, `docs/diagrams/`, `docs/benchmarks/` | ❌ | Vazios |
| SQLite, IA (llama.cpp/ONNX), Audio Analysis | ❌ | Não iniciados |

**Baseline técnica:** build C++ verde e testes compilando em `build/` (CMake 3.29.3 + GoogleTest 1.14).

---

## Princípios do roadmap

1. **Vertical spike primeiro**: fazer 1 gênero completo (pop/rock 4/4) tocável antes de generalizar.
2. **Invioláveis**: §5 do `prompts/master_prompt.md` — audio thread, IA não gera MIDI, DI, seed-reprodutível.
3. **Cada tarefa exige**: interface definida, DI, testes e benchmark (DoD do `qa-engineer`).
4. **Cada decisão estrutural** vira ADR em `docs/adr/` revisado por Software Architect + CTO.
5. **Arquitetura > velocidade**: atalhos que violem SOLID/Clean Arch são rejeitados, mesmo que prometam cortar 2 sprints.

---

## Fase 0 — Aterrissagem & Diagnóstico (1 sprint)

**Milestone M0:** Acordo de arquitetura + corrente verde contínua.

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Auditar baseline de código + diagrama AS-IS (C4 L1) | Software Architect | — |
| Aprovar/Ajustar ADR-0001 (boundary C++/Go via gRPC) | CTO | — |
| Documentar end-state arquitetura por camada (C4 L2) | Documentation Engineer | — |
| Definir métricas DoD de PR (cobertura mínima, bench obrigatório) | QA Engineer | — |
| CI: tapar buracos — `go mod init`, matrix Windows/macOS | DevOps Engineer | — |
| Plano de threat model offline | Security Engineer | — |

**Saída:** `docs/architecture/` L1/L2, ADR-0001, CI 4 jobs green em 3 OSes.

---

## Fase 1 — Foundation (Sprints 2–5)

**Milestone M1:** Shared Context persistido, transporte gRPC fim-a-fim, plugin fala com engine.

### 1.1 Infra de build & DI C++20 (Sprint 2)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Container de DI C++ (boost::di ou ServiceLocator próprio) + ADR-0002 | Software Architect | — |
| Refator MTE para usar DI (registrar `IScaleProvider`, `IHarmonyEngine`) | Music Theory Engineer | — |
| Configurar `spdlog` no MTE + traces estruturados | DevOps Engineer | — |
| Skills on-demand: `modern-cpp`, `cpp-memory-management` | — (skills) | — |

### 1.2 Motor gRPC Go (Sprint 3)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| `go mod init` + gRPC server stubbo em unix-socket `/tmp/aimidi.sock` | Backend Engineer | — |
| Gerar código Go do `aimidi.proto` no CI | DevOps Engineer | — |
| Implementar 3 RPCs stub (New/Continue/Variations) com contexto nulo | Backend Engineer | workflow-manager |
| Testes Go: server lifecycle, context cancel, timeout | QA Engineer | — |

### 1.3 Shared Musical Context (Sprints 3–4)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Schema SQLite (`shared_context`, `sections`, `instruments`, `history`, `locks`) + ADR-0003 | Database Engineer | — |
| Migration runner (`engine/go/internal/store/migrations/`) | Database Engineer | — |
| Repository Go (CRUD contexto, transacional) com `bun`/`sqlc` | Backend Engineer | — |
| ADR-0003: serialização do contexto entre C++ ↔ Go (proto vs JSON) | Software Architect | — |

### 1.4 Plugin ↔ ACE bridge (Sprint 5)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| FIFO lock-free SPSC no `PluginProcessor` (drain MIDI da gRPC) | Plugin Engineer | — |
| Cliente gRPC C++ no plugin (stub lazy connect, reconnect safe) | Plugin Engineer | — |
| Thread background do plugin para RPC (jamais na audio thread) | Plugin Engineer | — |
| Fuzz: inputs de DAW gigantes, sample rate change, panic | Security Engineer | — |
| Skills obrigatórias: `juce-realtime`, `lock-free` | — (skills) | — |

**Saída M1:** Plugin ↔ ACE ↔ SQLite fala, pipeline verde. Compila e passa em Windows/macOS/Linux.

---

## Fase 2 — Core Musical Pipeline / Vertical Spike "pop-rock 4/4" (Sprints 6–10)

**Milestone M2:** Digitar um prompt simples → ouvir uma faixa de pop/rock 4/4 tocável exportada em SMF.

**Princípio:** implementar apenas o suficiente para gerar pop/rock em 4/4 C major, 4 seções (intro-verse-chorus-outro). Tudo seed-reprodutível.

### 2.1 Harmony → Chord end-to-end (Sprint 6)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| `ScaleProvider` v2: transposição, modos maior/menor próprios | Music Theory Engineer | harmony-engine |
| `HarmonyEngine.generate()`: II-V-I e progressões diatônicas por setor, seed | Music Theory Engineer | harmony-engine |
| `IChordEngine`: voicings piano (root position + inversões), voice-leading básico | Music Theory Engineer | chord-engine |
| Tests golden: snapshot de MIDI por seed fixa | QA Engineer | — |
| Bench `HarmonyEngine`: <1ms para 4 seções de 4 compassos | Performance Engineer | — |

### 2.2 Rhythm → Bass → Drums (Sprint 7)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| `IRhythmEngine`: grid 4/4 (8th/16th), swing leve, shuffles por seed | Music Theory Engineer | rhythm-engine |
| `IBassEngine`: root+5th, walking 4-notas sobre II-V-I | Music Theory Engineer | bass-engine |
| `IDrumEngine`: groove pop-rock kit GM (kick/snare/hh), 1 fill por seção | Music Theory Engineer | drum-engine |
| Mapeamento GM fixo para drum | Music Theory Engineer | instrument-mapper |
| Tests golden + bench | QA Engineer + Performance Engineer | — |

### 2.3 Piano + Humanização + Render SMF (Sprints 8–9)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| `IPianoEngine`: comping block, 2 mãos, pedais sustain | Music Theory Engineer | piano-engine |
| `IHumanizationEngine`: micro-timing ±5ms, velocity swing (seed) | Music Theory Engineer | humanization-engine |
| `MidiRenderer`: eventos → SMF1 (type-1), trilhas por instrumento | Music Theory Engineer | midi-renderer |
| `MidiRenderer` alt: eventos live no gRPC stream para plugin | Music Theory Engineer | midi-renderer |
| C++ Arena allocator para pool de `MidiEvent` | Performance Engineer | — |
| Test golden: SMF byte-a-byte estável por seed | QA Engineer | — |

### 2.4 Workflow Manager + CompositionService real para W1 e W4 (Sprint 10)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| `WorkflowManager.go`: orquestra prompt→blueprint→timeline→arrangement→MTE→render | Backend Engineer | workflow-manager |
| Implementação real `NewComposition` (W1) em Go chamando MTE | Backend Engineer | — |
| ADR-0004: MTE em C++ como lib estática linkada no ACE Go, ou processo à parte | CTO + Software Architect | — |
| Test integrado: "warm pop piano grooves 4 bars" → SMF estável | QA Engineer | — |

**Saída M2:** Programa recebe `"upbeat pop-rock in C major, 16 bars"` e gera SMF reproduzível. W1 (parcial) + W4 funcionam. **Primeiro som tocável.**

---

## Fase 3 — Musical Intelligence local (Sprints 11–14)

**Milestone M3:** Prompt em linguagem natural (não template) gera blueprint automaticamente.

### 3.1 LLM Integration (Sprint 11)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Wrapper `llama.cpp` em Go (cgo) ou Python sidecar ou ONNX-only — ADR-0005 | AI Engineer + Software Architect | — |
| Carregar modelo GGUF pequeno (Llama-3.2-3B-Instruct Q4_K_M) | AI Engineer | prompt-interpreter |
| Install script baixa modelo + cache offline (respeita 100% offline após setup) | DevOps Engineer | — |
| Validar: modelo NÃO gera notas, só blueprint JSON | Security Engineer + CTO | — |

### 3.2 Prompt Interpreter + Style Detection (Sprint 12)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Schema final de `MusicBlueprint` (proto) + JSON Schema espelho | Software Architect | blueprint-generator |
| `PromptInterpreter`: extração {genre, mood, energy, instruments, structure} | AI Engineer | prompt-interpreter |
| `StyleDetection` ONNX classifier (14 gêneros básicos) | AI Engineer + DSP Engineer | style-detection |
| Valid input: rejeitar prompts que peçam literalmente notas | Security Engineer | — |

### 3.3 Blueprint → Timeline → Arrangement (Sprint 13)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| `BlueprintGenerator`: compila intents → proto reproduzível | AI Engineer | blueprint-generator |
| `TimelinePlanner`: bars/seções/transições em compassos | Music Theory Engineer + Backend Engineer | timeline-planner |
| `ArrangementPlanner`: quem toca em cada seção (energia) | Music Theory Engineer + Backend Engineer | arrangement-planner |
| Persistir blueprint no Shared Context | Database Engineer | — |

### 3.4 Plugar MI no WorkflowManager (Sprint 14)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| W1 completo: prompt NL → blueprint → MTE → render → SMF | Backend Engineer | workflow-manager |
| W7 Replace Instrument (orquestração adaptativa) | Music Theory Engineer | orchestration-engine, instrument-mapper |
| Bench end-to-end M3: <5s para 16 bars (sem GPU) | Performance Engineer | — |
| Dataset de prompts-teste (golden) | QA Engineer | — |

**Saída M3:** Workflows 1, 4, 6, 7 funcionando com prompt NL + LLM local. MTE expandido com mais gêneros.

---

## Fase 4 — MTE Horizontal Expansion (Sprints 15–19)

**Milestone M4:** Cobertura completa de instrumentos, modos, modulações e Workflows harmônicos.

### 4.1 Métrica + modos + modulação (Sprint 15)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Modos (jônico-dórico-…-lócrio) + transposição livre | Music Theory Engineer | harmony-engine |
| Modulação por seção (pivot chord), enharmonic shortcuts | Music Theory Engineer | harmony-engine |
| Counterpoint básico (espécies 1, 2) | Music Theory Engineer | strings-engine (parcial) |
| Bench | Performance Engineer | — |

### 4.2 Instrumentos restantes (Sprints 16–17)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| `GuitarEngine`: strumming patterns, power chords, open triads | Music Theory Engineer | guitar-engine |
| `StringsEngine`: divisi, bows, dynamic swells | Music Theory Engineer | strings-engine |
| `OrchestrationEngine`: distribuição de vozes, regs decision | Music Theory Engineer | orchestration-engine |
| GM patch maps expandidos (non-drums) | Music Theory Engineer | instrument-mapper |

### 4.3 Workflows harmônicos (Sprint 18)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| W8 Reharmonize (melodia locked, harmonia nova) | Music Theory Engineer | harmony-engine, chord-engine |
| W6 Generate Variations (mesma blueprint, seed delta) | Backend Engineer | workflow-manager |
| W5 Smart Regeneration (região) respeitando `locks` do Shared Context | Backend Engineer + Music Theory Engineer | workflow-manager |
| W2 Instrument Composer (só um instrumento) | Music Theory Engineer | — |

### 4.4 Humanização + Arrange + Orchestrate (Sprint 19)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Humanização por estilo (swing, laid-back push, snap-to-grid option) | Music Theory Engineer | humanization-engine |
| W9 Orchestrate (piano idea → arranjo orquestral) | Music Theory Engineer | orchestration-engine, strings-engine |
| W10 Arrange (chords → rhythm section + pads) | Music Theory Engineer | arrangement-planner, drum-engine, bass-engine |
| W3 paralelo ao W9 via import de `blueprint` externo | Backend Engineer | — |

**Saída M4:** Todos 12 subagentes MTE têm implementação real e testes golden. Workflows 1, 2, 4, 5, 6, 7, 8, 9, 10 funcionando.

---

## Fase 5 — Audio Engine (Sprints 20–23)

**Milestone M5:** Workflow 3 (Audio Assisted) funcional: importa áudio → analisa → gera complemento MIDI.

### 5.1 DSP core (Sprint 20)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| FFT (real) para spectral features, janelamento | DSP Engineer | — |
| Onset detection (spectral flux) → beat candidates | DSP Engineer | tempo-detection, beat-detection |
| Tempo tracker (autocorrelation) + confidence | DSP Engineer | tempo-detection |

### 5.2 Harmonia/Key a partir de áudio (Sprint 21)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Chromagram → template match (Krumhansl) | DSP Engineer | key-detection |
| Chord recognizer (CQT + HMM) | DSP Engineer | chord-detection |
| Performance: realtime on-device, jamais bloquear UI thread | Performance Engineer | — |

### 5.3 Fechar W3 (Sprints 22–23)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Pipeline audio → blueprint (reaproveitando MI) | AI Engineer + DSP Engineer | style-detection, blueprint-generator |
| Stream de áudio do plugin host (sidechain) | Plugin Engineer | — |
| Persistência de features no Shared Context | Database Engineer | — |
| Demo: importar loop → gerar baixo+bateria coerente | QA Engineer | — |

**Saída M5:** Workflow 3 + Workflow 8 (Reharmonize sobre audio importado) completos.

---

## Fase 6 — Plugin UI & DX (Sprints 24–27)

**Milestone M6:** Plugin VST3 usável em DAW real (Cubase/Reaper/FL), com todas as workflows expostas.

### 6.1 UI shell (Sprint 24)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Componente prompt box com history | Plugin Engineer | — |
| Workflow selector (10 workflows com enabled-state por capability check) | Plugin Engineer | — |
| State save/load via `ValueTree` → XML | Plugin Engineer | — |
| Style guide visual (dark theme, paleta) | — (skills ui) | — |
| ADR-0007: CLAP parity com VST3 | Software Architect + CTO | — |

### 6.2 Piano Roll (Sprint 25)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Piano roll read-only (preview do SMF gerado) | Plugin Engineer | — |
| Lock markers por região (drag) | Plugin Engineer | — |
| Tooltip micro-events (hover) | Plugin Engineer | — |

### 6.3 Regeneration in-place (Sprint 26)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Botão "regenerate this region" (W5 chamada) | Plugin Engineer | workflow-manager |
| Diff visualization (antes vs depois) só pitch/rhythm | Plugin Engineer | — |
| Drag-and-drop SMF → track da DAW | Plugin Engineer | — |

### 6.4 Parâmetros + Presets (Sprint 27)

| Tarefa | Agente primário | Subagentes |
|---|---|---|
| Parameter IDs estáveis para automação (seed, density, energy, etc.) | Plugin Engineer | — |
| Factory presets + user presets com tags | Backend Engineer | preset-manager |
| Plugin scanning de VSTi (W2 capability) | Plugin Engineer | plugin-scanner |

**Saída M6:** Plugin VST3/CLAP buildgreen em 3 OSes, instalável, com W1, W2, W5, W6, W7 verificáveis por usuário final.

---

## Fase 7 — Hardening, Benchmarks & Release v0.1 (Sprints 28–30)

**Milestone M7:** Release público Open Source v0.1.0.

### 7.1 Hardening (Sprint 28)

| Tarefa | Agente primário |
|---|---|
| Cobertura ≥80% componentes críticos (MTE, ACE, gRPC) | QA Engineer |
| Fuzz targets (proto, MIDI parser, prompt) | QA Engineer + Security Engineer |
| Audit dependências (SBOM) | DevOps Engineer + Security Engineer |
| Threat model review final | Security Engineer |

### 7.2 Benchmarks (Sprint 29)

| Tarefa | Agente primário |
|---|---|
| `docs/benchmarks/`: MTE <Nms/bar, MI <Nms/blueprint, render <Nms/1000 events | Performance Engineer |
| Tracy profiling hooks no MTE e DSP | Performance Engineer |
| CI benchmark regression gate | DevOps Engineer |

### 7.3 Release (Sprint 30)

| Tarefa | Agente primário |
|---|---|
| Changelog, ADR final consolidation, README release notes | Documentation Engineer |
| ADR-0006: AI model distribution (download first-run vs bundled sub-2GB) | CTO + Software Architect |
| GitHub Release matrix (Win/macOS/Linux VST3, Win/Linux CLAP) | DevOps Engineer |
| Zero-quickstart guide (5 min) para usuário final | Documentation Engineer |
| CTO sign-off da Definition of Success (ver `CONTEXT.md`) | CTO |

**Saída M7:** v0.1.0 publicada, Open Source, todos os 10 workflows concluindo, build verde em 3 OSes.

---

## Subagentes — primeira ativação por fase

| Subagente | Domain | Primeira ativação (fase.sprint) |
|---|---|---|
| workflow-manager | mi | Fase 1.2 (Sprint 3) como stub; real em Fase 2.4 |
| harmony-engine, chord-engine | mte | Fase 2.1 (Sprint 6) |
| rhythm-engine, bass-engine, drum-engine | mte | Fase 2.2 (Sprint 7) |
| piano-engine, humanization-engine, midi-renderer | mte | Fase 2.3 (Sprints 8–9) |
| instrument-mapper | instrument | Fase 2.2 e Fase 4.2 (drums, depois GM patches) |
| prompt-interpreter, style-detection, blueprint-generator | mi | Fase 3.1–3.2 (Sprints 11–12) |
| timeline-planner, arrangement-planner | mi | Fase 3.3 (Sprint 13) |
| orchestration-engine, strings-engine, guitar-engine | mte | Fase 4.2 (Sprints 16–17) |
| tempo-detection, beat-detection | audio | Fase 5.1 (Sprint 20) |
| key-detection, chord-detection | audio | Fase 5.2 (Sprint 21) |
| plugin-scanner, preset-manager | instrument | Fase 6.4 (Sprint 27) |

---

## Dependências críticas (cadeia de bloqueio)

```
Software Architect (contratos) ──► Backend Engineer (Go server)
                       │                  │
                       │                  ▼
                       │       Workflow Manager (subagent)
                       │                  │
                       ▼                  ▼
                AI Engineer ◄──► Music Theory Engineer ──► 12 MTE subagents
                       │                  │
                       ▼                  ▼
                Database Engineer   DSP Engineer ──► 4 Audio subagents
                                          │
                                          ▼
                                   Plugin Engineer ──► instrument-mapper
                                          │
                                          ▼
                  Performance Engineer   ── revisa cada hot path
                  Security Engineer     ── revisa cada IO/dep/parser
                  QA Engineer           ── revisa cada PR
                  Documentation Engineer── revisa cada ADR
                  DevOps Engineer       ── revisa cada CI/build
                  CTO                   ── decide trade-offs
```

---

## ADRs previstos (a abrir por fase)

| ADR | Título | Fase |
|---|---|---|
| ADR-0001 | Boundary C++ ↔ Go (gRPC socket vs FFI inline) | Fase 0 |
| ADR-0002 | DI framework C++ (boost::di vs manual ServiceLocator) | Fase 1.1 |
| ADR-0003 | Serialização do Shared Context (proto vs JSON) | Fase 1.3 |
| ADR-0004 | MTE como lib estática linkada no ACE Go, ou processo separado? | Fase 2.4 |
| ADR-0005 | Wrapper llama.cpp via cgo vs Python sidecar vs ONNX-only | Fase 3.1 |
| ADR-0006 | AI model distribution (download first-run vs bundled <2GB) | Fase 7.3 |
| ADR-0007 | CLAP parity com VST3 | Fase 6.1 |

Ver `docs/adr/README.md` para o índice atualizado.

---

## Timeline resumida (30 sprints ≈ 15 meses @ 2 semanas/sprint)

| Fase | Sprints | Entrega | Milestone |
|---|---|---|---|
| 0 | 1 | Baseline + contratos | M0 ✅ |
| 1 | 4 | Foundation (gRPC, SQLite, DI, plugin bridge) | M1 |
| 2 | 5 | Spike pop/rock MTE completo + W1/W4 | **M2 (primeiro som!)** |
| 3 | 4 | MI com LLM local, prompt NL → blueprint | M3 |
| 4 | 5 | MTE horizontal, W2/W5/W6/W7/W8/W9/W10 | M4 |
| 5 | 4 | Audio Engine, W3 | M5 |
| 6 | 4 | Plugin UI + Piano Roll + DX | M6 |
| 7 | 3 | Hardening + bench + release v0.1.0 | **M7 (Release!)** |

---

## Definition of Done do projeto (v0.1)

Ver `CONTEXT.md` → "Definição de Sucesso". Considerado concluído quando:

- [ ] Todos os 10 workflows implementados.
- [ ] Todos os módulos desacoplados.
- [ ] Sistema funcionando 100% offline.
- [ ] Plugin compatível com Cubase, Reaper, Studio One, FL Studio, Ableton Live, Bitwig.
- [ ] Toda geração musical vindo da Music Theory Engine determinística.
- [ ] Toda IA local (llama.cpp/ONNX).
- [ ] Documentação atualizada e ADRs consolidados.
- [ ] Testes automatizados + cobertura ≥80% nos críticos.
- [ ] Benchmarks públicos em `docs/benchmarks/`.
- [ ] Projeto apto a servir de referência Open Source para software musical baseado em IA.

---

## Histórico de revisões

| Data | Versão | Autor | Mudança |
|---|---|---|---|
| 2026-07-09 | 0.1 | CTO Agent (opencode) | Criação inicial a partir da baseline. |
