---
name: lock-free
description: Estruturas lock-free para uso entre audio thread e message thread (SPSC/MPSC rings, atomic seq_cst/acq_rel, ABA). Obrigatorio para filas MIDI e paramtros atômicos.
---

# lock-free

## Quando usar

Se a comunicacao entre threads envolver a Audio Thread, NUNCA use `std::mutex`.
Use estruturas lock-free.

## Padrao recomendado

- **SPSC ring buffer**: idealmente Audio Thread (1 produtor) <-> message thread
  (1 consumidor). 
- Implemente com `std::atomic<size_t>` para head/tail com `memory_order_acq_rel` /
  `release` / `acquire`.
- **Atomics**: parametros visveis para audio thread sao `std::atomic` -> leitura
  no audio thread com `acquire`, escrita na message thread com `release`.

## Anti-padroes

- Doubly-checked locking.
- Spinlocks em audio thread (cada busy-wait rouba tempo de processamento).
- Wait-free geral (muito dificil); SPSC/MPSC scoped e o que usamos.

## ABA

Em SPSC/MPSC ring indexados por tamanho potencia de 2, ABA nao e problema
(reaproveita slots). Em stacks/queues linkedas use tagged pointer ou epoch-based
RCU (so se estritamente necessario).
