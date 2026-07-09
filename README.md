# AI MIDI Composer (Open Source MVP)

## Visão Geral

O projeto será uma **plataforma de composição assistida por IA**, onde a IA **não gera MIDI diretamente**. Ela interpreta a intenção do usuário e produz um plano musical estruturado (_Music Blueprint_). A composição efetiva é realizada por uma **Engine de Teoria Musical** determinística, responsável por gerar MIDI consistente e reproduzível.

O MVP será totalmente offline, modular, multiplataforma e open source.

---

# Objetivos do MVP

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

# Stack Tecnológica

## Linguagens

| Camada         | Tecnologia      |
| -------------- | --------------- |
| Plugin         | C++20           |
| Engine Musical | C++20           |
| MIDI Engine    | C++20           |
| Engine de IA   | Go              |
| Comunicação    | gRPC + Protobuf |

---

## Frameworks

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

# Arquitetura Geral

```text
┌───────────────────────────────────────────────────────────────────────┐
│                           DAW (Host)                                 │
│      Reaper | Cubase | Studio One | FL | Ableton | Bitwig            │
└───────────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌───────────────────────────────────────────────────────────────────────┐
│                    AI MIDI Plugin (JUCE / C++)                       │
├───────────────────────────────────────────────────────────────────────┤
│ • Prompt UI                                                          │
│ • Piano Roll Preview                                                 │
│ • Workflow Manager                                                   │
│ • Instrument Settings                                                │
│ • Preview                                                            │
│ • Configurações                                                      │
└───────────────────────────────────────────────────────────────────────┘
                               │
                     gRPC / Shared Memory
                               │
═══════════════════════════════════════════════════════════════════════════
                    AI COMPOSITION ENGINE (ACE)
═══════════════════════════════════════════════════════════════════════════

    ┌────────────────────────────────────────────────────────────┐
    │ 1. Shared Musical Context (SMC)                           │
    └────────────────────────────────────────────────────────────┘

             Compartilhado entre todas as instâncias

                     Tempo
                     Tom
                     Escala
                     Estrutura
                     Progressão
                     Instrumentos
                     Histórico
                     Locks

═══════════════════════════════════════════════════════════════════════════

    ┌────────────────────────────────────────────────────────────┐
    │ 2. Musical Intelligence (MI)                              │
    └────────────────────────────────────────────────────────────┘

        Prompt Interpreter
        Style Detection
        Genre Detection
        Mood Detection
        Musical Intent Parser
        Arrangement Planner
        Creative Decision Engine

                    ↓

             Music Blueprint

═══════════════════════════════════════════════════════════════════════════

    ┌────────────────────────────────────────────────────────────┐
    │ 3. Musical Planning Layer (MPL)                           │
    └────────────────────────────────────────────────────────────┘

        Timeline Planner
        Section Planner
        Track Planner
        Instrument Planner
        Chord Blueprint
        Rhythm Blueprint
        Melody Blueprint

═══════════════════════════════════════════════════════════════════════════

    ┌────────────────────────────────────────────────────────────┐
    │ 4. Music Theory Engine (MTE)                              │
    └────────────────────────────────────────────────────────────┘

        Harmony Engine
        Voice Leading
        Chord Engine
        Rhythm Engine
        Melody Engine
        Bass Engine
        Drum Engine
        Guitar Engine
        Piano Engine
        Strings Engine
        Humanization Engine

═══════════════════════════════════════════════════════════════════════════

    ┌────────────────────────────────────────────────────────────┐
    │ 5. MIDI Rendering Engine (MRE)                            │
    └────────────────────────────────────────────────────────────┘

        MIDI Notes
        Velocity
        Timing
        CC
        Expression
        Pitch Bend
        Keyswitches

═══════════════════════════════════════════════════════════════════════════

    ┌────────────────────────────────────────────────────────────┐
    │ 6. Audio Intelligence Layer (AIL)                         │
    └────────────────────────────────────────────────────────────┘

        BPM Detection
        Key Detection
        Chord Detection
        Beat Detection
        Audio Features

═══════════════════════════════════════════════════════════════════════════

    ┌────────────────────────────────────────────────────────────┐
    │ 7. Instrument Integration Layer (IIL)                     │
    └────────────────────────────────────────────────────────────┘

        Plugin Scanner
        Instrument Database
        Preset Manager
        Instrument Mapping
        Templates
```

---

# Fluxo Geral

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

# Workflows

## Workflow 1 — New Composition

Cria uma composição do zero.

Entrada

```
"Balada Rock anos 80"
```

Fluxo

```
Prompt

↓

Blueprint

↓

Planejamento

↓

Usuário escolhe módulos

↓

Gerar MIDI
```

Pode gerar

- Chords
- Bass
- Guitar
- Piano
- Drums
- Strings
- Pads
- Synth

---

## Workflow 2 — Instrument Composer

Gera apenas um instrumento.

Entrada

```
Metal Drum
```

↓

Seleciona

```
Drums
```

↓

Gera apenas

```
Drum MIDI
```

Outro exemplo

```
Jazz Walking Bass
```

↓

Somente

```
Bass MIDI
```

---

## Workflow 3 — Audio Assisted Composer

Entrada

Áudio

↓

Audio Intelligence

↓

Detecta

- BPM
- Key
- Escala

↓

Prompt opcional

```
Adicionar guitarra limpa
```

↓

Gera

```
Guitar MIDI
```

---

## Workflow 4 — Continue Composition

Entrada

MIDI existente

↓

Prompt

```
Continue
```

↓

Expande a música mantendo o contexto.

---

## Workflow 5 — Smart Regeneration

Selecionar

```
Compassos 17–24
```

↓

Prompt

```
Mais energia
```

↓

Regenera apenas essa região.

---

## Workflow 6 — Generate Variations

Entrada

Riff existente

↓

Quantidade

```
10 variações
```

↓

Mantém identidade musical.

---

## Workflow 7 — Replace Instrument

Entrada

```
Piano MIDI
```

↓

Trocar para

```
Strings
```

↓

Adapta automaticamente o MIDI.

---

## Workflow 8 — Reharmonize

Melodia travada

↓

Prompt

```
Jazz
```

↓

Regera

- acordes
- baixo
- acompanhamento

Mantém a melodia.

---

## Workflow 9 — Orchestrate

Entrada

```
Piano
```

↓

Cria

- Strings
- Brass
- Choir
- Percussion

---

## Workflow 10 — Arrange

Entrada

```
Chords
```

↓

Gera automaticamente

- Bass
- Drums
- Guitar
- Piano
- Strings
- Pads

---

# Shared Musical Context (SMC)

É o coração da plataforma.

Todas as instâncias do plugin compartilham o mesmo contexto.

Armazena

- BPM
- Tonalidade
- Escala
- Progressão
- Estrutura
- Compassos
- Instrumentação
- Histórico
- Preferências
- Elementos bloqueados (Locks)

Exemplo

```text
Projeto

├── Tempo = 120
├── Key = G
├── Scale = Major
├── Chords (LOCK)
├── Bass
├── Guitar
├── Piano
├── Drums
└── Strings
```

---

# Roadmap Completo

## Fase 0 — Fundação

- [ ] Definir arquitetura
- [ ] Criar monorepo
- [ ] Configurar CMake
- [ ] Configurar CI/CD
- [ ] Configurar testes
- [ ] Configurar documentação

---

## Fase 1 — Core da ACE

- [ ] Engine principal
- [ ] gRPC
- [ ] SQLite
- [ ] Configurações
- [ ] Logs
- [ ] Cache

---

## Fase 2 — Shared Musical Context

- [ ] Contexto compartilhado
- [ ] Persistência
- [ ] Locks
- [ ] Histórico
- [ ] Sincronização

---

## Fase 3 — Plugin

- [ ] Projeto JUCE
- [ ] Interface
- [ ] Prompt
- [ ] Piano Roll
- [ ] Preview
- [ ] Configuração

---

## Fase 4 — Musical Intelligence

- [ ] Prompt Parser
- [ ] Genre Detection
- [ ] Mood Detection
- [ ] Style Detection
- [ ] Musical Intent
- [ ] Blueprint Generator

Modelo inicial

- Qwen3 8B GGUF

---

## Fase 5 — Musical Planning Layer

- [ ] Song Planner
- [ ] Timeline Planner
- [ ] Track Planner
- [ ] Instrument Planner
- [ ] Section Planner

---

## Fase 6 — Music Theory Engine

### Harmonia

- [ ] Escalas
- [ ] Modos
- [ ] Campo harmônico
- [ ] Progressões

### Melodia

- [ ] Motivos
- [ ] Frases

### Ritmo

- [ ] Groove
- [ ] Swing

### Instrumentos

- [ ] Chords
- [ ] Bass
- [ ] Drums
- [ ] Guitar
- [ ] Piano

### Humanização

- [ ] Timing
- [ ] Velocity

---

## Fase 7 — MIDI Rendering Engine

- [ ] Eventos MIDI
- [ ] Velocity
- [ ] CC
- [ ] Expression
- [ ] Exportação MIDI

---

## Fase 8 — Instrument Integration

- [ ] Scanner de VSTis
- [ ] Banco local
- [ ] Associação Instrumento → VSTi
- [ ] Associação Instrumento → Preset
- [ ] Templates por estilo

---

## Fase 9 — Audio Intelligence

- [ ] BPM Detection
- [ ] Key Detection
- [ ] Audio Features
- [ ] Integração com Workflow 3

---

## Fase 10 — Workflows

- [ ] New Composition
- [ ] Instrument Composer
- [ ] Audio Assisted Composer
- [ ] Continue Composition
- [ ] Smart Regeneration
- [ ] Generate Variations
- [ ] Replace Instrument
- [ ] Reharmonize
- [ ] Orchestrate
- [ ] Arrange

---

## Fase 11 — Performance

- [ ] Thread Pool
- [ ] Lock-Free Queues
- [ ] Arena Allocators
- [ ] Cache de inferência
- [ ] Cache de padrões musicais
- [ ] Lazy Loading dos modelos
- [ ] Object Pool para eventos MIDI
- [ ] Profiling contínuo
- [ ] Benchmark automatizado

---

## Fase 12 — Open Source

- [ ] API para novos módulos
- [ ] Sistema de Providers (IA, teoria, instrumentos)
- [ ] Templates de estilos musicais
- [ ] Guia para contribuidores
- [ ] Testes automatizados
- [ ] Benchmarks públicos
- [ ] Exemplos de criação de novos geradores (Bass, Guitar, Drums, etc.)

## Observação técnica para o MVP

Para manter o escopo controlado e entregar valor rapidamente, eu reduziria o **MVP** aos seguintes módulos musicais:

- **Chord Engine**
- **Bass Engine**
- **Drum Engine**
- **Melody Engine**
- **Shared Musical Context**
- **New Composition**, **Instrument Composer** e **Audio Assisted Composer**

Os demais workflows (Continue Composition, Smart Regeneration, Reharmonize, Orchestrate etc.) seriam implementados sobre essa mesma base arquitetural, sem necessidade de refatoração. Essa abordagem entrega um núcleo sólido, útil para a comunidade open source e preparado para evoluir gradualmente sem comprometer a arquitetura inicial.

