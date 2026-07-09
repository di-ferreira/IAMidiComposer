---
name: multithreading
description: Modelo de threads do projeto - Audio Thread (real-time sagrada), Message Thread (UI/plugin), Worker Threads (ACE/parsing), pool para AI. Job queue lock-free, work-stealing quando pertinente.
---

# multithreading

## Modelo do AI MIDI Composer

1. **Audio Thread** (Plugin): nunca mutex/IO/alloc; lock-free rings.
2. **Message Thread** (Plugin): atualiza UI ValueTree, envia requests para ACE.
3. **Worker threads** (ACE Go): gRPC handlers, parsing MIDI, persistence SQLite.
4. **AI Worker** (ACE C++/llama): uma thread dedicada ou pool pequeno.

## Padrões

- **Job queue** lock-free SPSC/MPSC entre Message Thread e Worker.
- **Work-stealing pool** para tasks paralelas pequenas (ex.: parse de multiplos arquivos).
- **Affinity**: pin de Audio Thread em core isolado quando possivel (JUCE expoe).
- **NUMA-aware**: em grandes batch machines; nao necessario hoje.

## Anti-padroes

- `thread_local` para estado mutável musical (quebraria reproducão por seed).
- `volatile` para sincronizacao (nao substitui atomic).
- `std::async` sem policy explicita (default pode ser async ou deferred).
