# Workspace de Engenharia

```text
Workspace

├── Master Prompt
│
├── Agents
│
├── SubAgents
│
├── Skills
│
├── Standards
│
└── Docs
```

---

# Estrutura completa

```text
ai-midi-composer/

.docs/
│
├── architecture/
│
├── adr/
│
├── roadmap/
│
├── benchmarks/
│
├── workflows/
│
└── diagrams/

.agents/

│
├── cto.md
├── software_architect.md
├── plugin_engineer.md
├── backend_engineer.md
├── music_theory_engineer.md
├── ai_engineer.md
├── dsp_engineer.md
├── ui_engineer.md
├── database_engineer.md
├── security_engineer.md
├── qa_engineer.md
├── devops_engineer.md
├── documentation_engineer.md

.subagents/

│
├── harmony_engine.md
├── chord_engine.md
├── melody_engine.md
├── rhythm_engine.md
├── bass_engine.md
├── drum_engine.md
├── guitar_engine.md
├── piano_engine.md
├── strings_engine.md
├── orchestration_engine.md
├── humanization_engine.md
│
├── prompt_interpreter.md
├── style_detection.md
├── blueprint_generator.md
├── arrangement_planner.md
├── timeline_planner.md
├── workflow_manager.md
│
├── tempo_detection.md
├── key_detection.md
├── beat_detection.md
├── chord_detection.md
│
├── plugin_scanner.md
├── preset_manager.md
├── instrument_mapper.md
│
└── midi_renderer.md

.skills/

│
├── cpp/
│
├── go/
│
├── juce/
│
├── vst3/
│
├── clap/
│
├── grpc/
│
├── protobuf/
│
├── sqlite/
│
├── dsp/
│
├── midi/
│
├── theory/
│
├── llama_cpp/
│
├── onnx/
│
├── performance/
│
├── testing/
│
├── security/
│
└── documentation/

.standards/

│
├── architecture.md
├── coding_style.md
├── cpp_guidelines.md
├── go_guidelines.md
├── realtime_audio.md
├── performance.md
├── security.md
├── testing.md
├── branching.md
├── commits.md
├── pull_request.md
├── review.md
└── documentation.md

.prompts/

│
├── master_prompt.md
├── create_module.md
├── refactor_module.md
├── generate_tests.md
├── generate_docs.md
├── code_review.md
├── benchmark.md
└── bugfix.md
```

---

# Os agentes

```text
CTO

↓

Software Architect

↓

Project Manager

↓

Backend Engineer

↓

Plugin Engineer

↓

DSP Engineer

↓

Music Theory Engineer

↓

AI Engineer

↓

Database Engineer

↓

Security Engineer

↓

Performance Engineer

↓

QA Engineer

↓

DevOps Engineer

↓

Documentation Engineer
```

---

# Novo agente

## Performance Engineer

Esse agente será um dos mais importantes.

### Responsabilidade

Tudo relacionado a desempenho.

### Skills

- Cache Optimization
- SIMD
- SSE
- AVX2
- Lock Free
- Arena Allocator
- Object Pool
- CPU Profiling
- Tracy Profiler
- Benchmark
- Memory Layout

Prompt

```text
Você é responsável por desempenho.

Toda decisão deve considerar:

Cache

CPU

RAM

Threads

SIMD

Nunca aceitar código lento.

Sempre sugerir melhorias.

Todo código deve ser benchmarkado.

Todo algoritmo deve ser analisado.

Priorize estruturas contíguas na memória.

Evite virtualização excessiva.

Evite cópias.

Evite alocação dinâmica.
```

---

# Os SubAgentes

Agora a arquitetura muda.

Cada módulo da Engine possui seu próprio agente.

```text
Musical Intelligence

├── Prompt Interpreter

├── Genre Detector

├── Mood Detector

├── Style Detector

├── Blueprint Generator

├── Context Manager

└── Arrangement Planner
```

---

```text
Music Theory Engine

├── Harmony

├── Chords

├── Voice Leading

├── Counterpoint

├── Melody

├── Rhythm

├── Bass

├── Guitar

├── Piano

├── Strings

├── Drums

├── Humanization

└── Orchestration
```

---

```text
Audio Intelligence

├── BPM

├── Beat

├── Key

├── Chords

├── Genre

└── Structure
```

---

```text
Instrument Layer

├── Scanner

├── Mapping

├── Presets

├── Templates

└── Favorites
```

---

# As Skills

Agora vem a parte que considero mais importante.

Eu NÃO colocaria conhecimento dentro dos agentes.

Os agentes devem ser pequenos.

Quem possui o conhecimento são as Skills.

Exemplo.

```text
.skills/

cpp/

    modern_cpp.md

    memory_management.md

    templates.md

    move_semantics.md

    constexpr.md

    coroutines.md
```

---

```text
.skills/

performance/

    cache.md

    lock_free.md

    simd.md

    multithreading.md

    arena_allocator.md

    object_pool.md

    profiling.md
```

---

```text
.skills/

juce/

    plugin_processor.md

    plugin_editor.md

    valuetree.md

    parameters.md

    realtime.md

    midi.md
```

---

```text
.skills/

theory/

    harmony.md

    scales.md

    modes.md

    counterpoint.md

    voice_leading.md

    rhythm.md

    orchestration.md

    chord_progression.md
```

---

# Os Prompts reutilizáveis

Também criaria prompts prontos.

```text
.prompts/

create_module.md

create_agent.md

generate_tests.md

generate_docs.md

benchmark.md

refactor.md

bugfix.md

security_review.md

performance_review.md

code_review.md
```

---

# Padrões

Toda IA deve seguir obrigatoriamente.

```
Nunca bloquear Audio Thread.

Nunca utilizar mutex na Audio Thread.

Nunca fazer IO na Audio Thread.

Nunca confiar no frontend.

Nunca criar dependências circulares.

Sempre escrever testes.

Sempre atualizar documentação.

Sempre executar benchmark.

Sempre utilizar interfaces.

Sempre utilizar DI.

Sempre escrever código desacoplado.

Sempre seguir SOLID.

Sempre seguir Clean Architecture.
```

---

# Roadmap dos Agentes

## Sprint 1

- Criar `Master Prompt`.
- Criar `CTO Agent`.
- Criar `Software Architect`.
- Criar `Project Manager`.
- Criar `Performance Engineer`.
- Criar `Documentation Engineer`.

---

## Sprint 2

- Criar agentes de desenvolvimento.
- Criar agentes de testes.
- Criar agentes de segurança.
- Criar agentes de DevOps.

---

## Sprint 3

- Criar todos os subagentes da ACE.
- Criar todos os subagentes da MTE.
- Criar todos os subagentes da MI.

---

## Sprint 4

- Criar biblioteca de Skills.
- Criar padrões arquiteturais.
- Criar templates de prompts.
- Criar exemplos de uso.

---

## Sprint 5

- Integrar o Workspace aos agentes de codificação (OpenCode, Claude Code, Codex CLI, Gemini CLI ou Aider).
- Automatizar seleção de agentes por tipo de tarefa.
- Criar comandos para geração de módulos, testes, documentação e revisões.

## Minha única alteração em relação ao plano atual

Eu adicionaria um **Agent Registry** e um **Skill Registry**, em vez de depender apenas de pastas.

```text
.workspace/

├── registry/
│   ├── agents.yaml
│   ├── subagents.yaml
│   ├── skills.yaml
│   ├── workflows.yaml
│   └── standards.yaml
```

Esses arquivos descrevem metadados como nome, responsabilidades, dependências, skills obrigatórias e critérios de ativação de cada agente. Assim, qualquer ferramenta de codificação baseada em agentes pode descobrir automaticamente quem deve atuar em cada tarefa, tornando o workspace mais organizado, extensível e fácil de manter conforme o projeto cresce.

