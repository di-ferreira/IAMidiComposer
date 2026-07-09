---
name: juce-realtime
description: Realtime constraints no plugin JUCE - Audio Thread sagrada, sem alloc/lock/IO; parametros atomicos; lock-free FIFOs; probe preemptive.
---

# juce-realtime

## Inviolaveis

- Em `processBlock`: nenhum `malloc`/`new`; nenhum lock; nenhum IO de arquivo/rede;
  nenhum `std::cout`; nenhum `std::thread`.
- Use `juce::SpinLock` APENAS se conseguir provar que nunca contented + nunca em
  audio thread; default: proibido.

## Padroes obrigatorios

- Parametros: atomic float (`std::atomic<float>` wrapped por apvts).
- Comunicacao audio <-> message: lock-free FIFO `juce::AbstractFifo` ou SPSC
  custom.
- Buffer pool para MIDI e para mensagens de UI.
- Never block on `future`/`condition_variable` em audio thread.

## Verificacao

- JUCE: `-DJUCE_CHECK_MEMORY_LEAKS=ON` em debug.
- AudioProcessorProbe / falksea para medir glitch detect.
- Tracy + RT latency measurement no CI (audio performance test).
