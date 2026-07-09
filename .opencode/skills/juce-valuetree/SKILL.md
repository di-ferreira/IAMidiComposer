---
name: juce-valuetree
description: State via ValueTree JUCE - arvore serializavel para projeto, variaveis observaveis, undo/redo, save/load. Fonte unica para UI; nao duplica estado no AudioProcessor.
---

# juce-valuetree

## Padrao

```cpp
auto state = juce::ValueTree("AIMidiComposer");
auto proj  = juce::ValueTree("Project");
proj.setProperty("key", "C", nullptr);
state.addChild(proj, -1, nullptr);
state.sendPropertyChangeMessage("key");
```

## Regras

- Source-of-truth para UI; AudioProcessor usa copias imutaveis via get / atomic.
- Listener dispatch via `ListenerWithLambda`.
- UndoManager atachado para undo/redo.
- XML / binary serialize via `state.toXml()` / `ValueTree::fromXml`.

## Anti-padroes

- Estado paralelo no AudioProcessor (viola fonte unica).
- Modify Tree de thread desconhecida.
- Listener em Audio Thread (laga dispatch).
