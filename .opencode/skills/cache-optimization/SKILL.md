---
name: cache-optimization
description: Padrões para cache-friendly code em C++ — layout contiguo (SOA/AOS), cache line poisoning, prefetching, hot/cold split e analise via perf/Tracy. Use em loops de processamento de MIDI e MIDI buffers.
---

# cache-optimization

## Padrões-chave

- **Layout contiguo**: prefira `std::array`/`std::span`/buffers SOA para batches de
  eventos/processamentos, evitando ponteiros soltos entre elementos do inner-loop.
- **Cache line poisoning**: garanta que structs frequentes caibam em 64 bytes (uma
  cache line) e estejam aligned (`alignas(64)`); evite false sharing entre threads.
- **Hot/cold split**: `[[noreturn]]` + `__attribute__((cold))` para raras; separe
  branch rare em funcao propria para nao poluir iTLB.
- **Prefetch**: `__builtin_prefetch` apenas quando manual benchmark mostra ganho;
  nao confunda o prefetcher automatico.
- **Branchless inner loops**: converta `if` em aritmetica selecionada para reduzir
  branch mispredict; valide com perf-stat.

## Anti-padroes

- `std::vector<bool>` (cache-hostil); use `std::vector<uint8_t>` ou bitmask.
- Loop fused-over-by-class com ponteiros randomicos entre elementos.
- `std::map`/`std::unordered_map` em hot path; substitua por flat map/linear scan.

## Como validar

1. Perf record -> `perf report` no caminho critico.
2. Tracy zone macro + plot de cache-misses.
3. Antes/depois: medir instructions/cycle (IPC) alvo > 1.0.
