---
name: vst3-format
description: VST3 - formato SDK, IComponent/IProcessor, bus routing, parameter IDs, factory. Algo em JUCE abstrai, mas conhecer base e importante para debugging em DAWs.
---

# vst3-format

## Conceitos

- **Processor e Controller**: 2 classes (Processor tem IAudioProcessor; Controller
  tem IEditController). No JUCE: AudioPluginInstance + AudioProcessorEditor compartilham
  APVTS.
- **Buses**: input/output audio buses + event buses (MIDI in/out).
- **Parameters**: ID-based (192-bit fingerprint); ranges/units.
- **Factory**: declares components por GUID.
- **SubCategories**: "Fx|Instrument" para classificacao do host.

## JUCE abstracos

- `AudioProcessor` -> define numero de buses, channels; Project Savable.
- Audio Thread: processBlock obtem buffers; MIDI transportado via MidiBuffer.
- Editor: JUCE oferece; respeitar Message Thread.

## Problemas comuns

- ID mismatch Processor <-> Controller -> param nao aparece/valida.
- Bus configuracoes erradas em DAWs headless.
- State serialize binary vs XML (JUCE copia como XML por default).
