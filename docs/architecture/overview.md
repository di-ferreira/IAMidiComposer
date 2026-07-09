# Architecture Overview

> Placeholder. Será expandido conforme Sprint de Código A cria interfaces.

## Componentes (alto nível)

- **Plugin** (C++20 / JUCE): UI, MIDI out, gRPC client.
- **AI Composition Engine (ACE)**:
  - **Musical Intelligence (MI)**: IA local (llama.cpp/ONNX); gera Blueprint.
  - **Planning Layer**: Timeline + Arrangement + Workflow Manager.
  - **Music Theory Engine (MTE)**: harmonia/melodia/bateria/etc deterministico.
  - **Audio Analysis**: BPM/beat/key/chord detection.
  - **MIDI Renderer**: struct events -> bytes SMF / eventos live.
  - **Instrument Mapper**: roteamento instrumentos <-> VSTis/channels.
  - **Shared Musical Context**: estado de projeto compartilhado.

## Diagrama topo

Ver `diagrams/topology.puml` (placeholder/template).

## Interfaces principais

Serao definidas em `engine/cpp/include/aimidi/<domain>/I*.hpp`:
- `IHarmonyEngine`
- `IChordEngine`
- `IMelodyEngine`
- `IRhythmEngine`
- `IBassEngine`
- `IMidiRenderer`
- `IInstrumentMapper`
- `ISharedContext`

A interface entre workflow_manager e engines e definida pela equipe de arquitetura.
