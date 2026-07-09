# Padrão Arquitetural — AI MIDI Composer

## 1. Princípios

- **Clean Architecture**: domínio não conhece infraestrutura; fluxo de dependência
  aponta para o domínio.
- **SOLID**: Single Responsibility, Open/Closed, Liskov, Interface Segregation,
  Dependency Inversion.
- **Dependency Injection**: nenhum `new` cruzando camadas; factories injetam
  interfaces.
- **Baixo acoplamento / Alta coesão**: cada módulo tem um propósito; comunicação
  por interfaces.

## 2. Visão de Componentes

```
┌────────────────────┐    gRPC     ┌──────────────────────────────────┐
│      Plugin VST3   │◀───────────▶│       AI Composition Engine       │
│  (C++20 / JUCE)    │             │                                   │
│                    │             │  ┌─────────────────────────────┐  │
│  - UI (Message thr)│             │  │  Musical Intelligence (IA)   │  │
│  - Piano Roll      │             │  │  - Prompt Interpreter         │  │
│  - MIDI out LF     │             │  │  - Style/Mood Detector        │  │
│  - gRPC client     │             │  │  - Blueprint Generator         │  │
└────────────────────┘             │  └─────┬───────────────────────┘  │
                                   │        ▼                            │
                                   │  ┌──────────────────────────────┐  │
                                   │  │  Planning Layer                │  │
                                   │  │  - Timeline / Arrangement     │  │
                                   │  │  - Workflow Manager           │  │
                                   │  └─────┬──────────────────────────┘  │
                                   │        ▼                            │
                                   │  ┌──────────────────────────────┐  │
                                   │  │  Music Theory Engine (C++)    │  │
                                   │  │  - Harmony/Chords/Melody ...  │  │
                                   │  │  - Humanization               │  │
                                   │  └─────┬──────────────────────────┘  │
                                   │        ▼                            │
                                   │  ┌──────────────────────────────┐  │
                                   │  │  MIDI Renderer / Mapper        │  │
                                   │  └─────┬──────────────────────────┘  │
                                   │        ▼                            │
                                   │  ┌──────────────────────────────┐  │
                                   │  │  Audio Analysis (tempo/key/...) │  │
                                   │  └──────────────────────────────┘  │
                                   └────────────────────────────────────┘
```

## 3. Camadas e fluxo de dependências

- **UI** → **AudioProcessor** (Plugin).
- **Plugin gRPC client** → **ACE gRPC server** (Go).
- **ACE Go** chama **Music Theory Engine (C++)** via cgo ou gRPC local (a definir
  em ADR 0001).
- **Cada engine dentro da MTE** recebe Blueprint + Context por interface; gera
  MIDI interno (struct MidiEvent); pipeline unificado em `MidiRenderer`.

## 4. Comunicação entre módulos

- Interno C++: passagem por `std::span` / struct POD; locks prohibited hot path.
- Plugin ↔ ACE: protobuf via gRPC (só `_=1` canal por Plugin).
- Workflow Manager orquestra sequências; cada step recebe seed.

## 5. Restrições

- Audio Thread sagrada (ver `realtime_audio.md`).
- IA nunca gera MIDI (ver `prompts/master_prompt.md` §2).
- Toda aleatoriedade deriva de seed.

## 6. ADRs

Toda decisão arquitetural relevante vira ADR em `docs/adr/`. Ver `adr-format` skill.
