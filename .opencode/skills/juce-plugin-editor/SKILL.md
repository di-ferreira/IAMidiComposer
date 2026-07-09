---
name: juce-plugin-editor
description: PluginEditor JUCE - UI em message thread, composicao de componentes; nada de dependencia direta do AudioProcessor; atualiza state via apvts::Listener ou timer thread-safe.
---

# juce-plugin-editor

## Padrao

```cpp
class AiMidiComposerEditor : public juce::AudioProcessorEditor,
                             private juce::Timer {
    std::atomic<bool> dirty_{false};
public:
    void paint(Graphics&) override;
    void resized() override;
    void timerCallback() override;   // poll atomic dirty -> repaint()
};
```

## Regras

- Toda UI em **Message Thread**; nunca aciona Audio Thread.
- Sync UI -> State: attach via `apvts.getParameter(paramId)->getValue()`.
- Sync State -> UI: `std::atomic` flag + Timer PULL (nao push da audio thread).
- Componentes isolados e compostos (PianoRoll, PromptBox, WorkflowSelector...).

## Anti-padroes

- Mutex na UI-Changing na Audio Thread (via callback blocking).
- `juce::Component::repaint()` em loop apertado.
- Allocation pesada em `paint()` (fonte/bitmap cache na paint).
