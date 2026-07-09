---
name: juce-parameters
description: Parametros em JUCE via AudioProcessorValueTreeState. Atomic para audio thread ( sem lock); raw value <-> default; Attachment para UI controls.
---

# juce-parameters

## Padrao

```cpp
auto layout = juce::AudioProcessorValueTreeState::ParameterLayout{};
layout.add(std::make_unique<juce::AudioParameterFloat>(
    "energy", "Energy", juce::NormalisableRange(0.f, 1.f), 0.5f));
apvts.replaceState(juce::ValueTree("PARAMS"));
return apvts_;
```

## Regras

- Leitura em Audio Thread: `apvts.getRawParameterValue("energy")->load()` ->
  `std::atomic`, ACQUIRE silence.
- Attachment para UI: `SliderAttachment(att, "energy", slider);`.
- Skew para gamas nao-lineares (log) quando aplicavel.
- Valida UI vs Audio Thread: parametros podem ser limitados/clampados; UI always
  reload do state interno (nao confie na UI).

## Anti-padroes

- Variavel `float` paralela que o audio thread le direto sem sync.
- `getParameter`/`setParameter` legacy (use APVTS).
