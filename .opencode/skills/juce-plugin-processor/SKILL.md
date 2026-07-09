---
name: juce-plugin-processor
description: PluginProcessor JUCE - AudioProcessor que nao processa audio (apenas gate MIDI), lock-free MIDI out via FIFO; parametros via APVTS; state ValueTree.
---

# juce-plugin-processor

## Padrao base

```cpp
class AiMidiComposerProcessor : public juce::AudioProcessor {
    juce::AudioProcessorValueTreeState apvts_;
    MidiFifo midi_out_fifo_;      // lock-free SPSC ring
    std::unique_ptr<GrcpClient> client_;
public:
    void processBlock(AudioBuffer<float>&, MidiBuffer&) override;
    void getStateInformation(MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
};
```

## Regras

- `processBlock`:
  - Nao chame IA; nao chame IO; nao aloque na heap.
  - Receba MIDI pre-renderizado da fifo e injete no ` MidiBuffer`.
  - Atualize parametros visiveis via `std::atomic` em `ParameterLayout`.
- State (save/load): `ValueTree` via `copyState()` serializado.

## Anti-padroes

- `std::lock_guard` em `processBlock`.
- `client_->call()` blocking em `processBlock`.
- Valores da UI diretamente sem validar.
