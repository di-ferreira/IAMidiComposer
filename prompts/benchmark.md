# Prompt: Benchmark

> Use para executar e documentar benchmarks de um modulo/código.

## Quando precisamos

- Toda mudanca em **hot path** (ver `standards/performance.md`).
- Nova feature de sporting musical (MTE, MTE, Audio, MIDI).FCja
- ALOCACOES ou memory footprint suspicion.

## Procedimento

1. Checkout no branch base (`main`).
2. Configure build Release com hardening (-O3 -DNDEBUG -fno-exceptions
   -fno-rtti).
3. Rode bench reference:
   ```sh
   cmake -B build -DCMAKE_BUILD_TYPE=Release ...
   cmake --build build
   ctest -R <module>Bench --output-on-failure
   ```
4. Anote output (model, hardware, build flags) em `docs/benchmarks/<module>.md`
   under "Antes".
5. Checkout no feature branch (rebuild).
6. Rode mesmo bench; anote em "Depois".
7. Compare metricas; se regressão > 5% throughput ou > 1ms latencia p99:
   - Obrar PR ate justificado com ADR (priority/safety > perf).
8. PR comentario com link para o doc bench.

## Metricas para reportar

- `ns/op` ou `ops/s` (modulo)
- `p50/p99 latencia` (audio/workflow)
- `cache missed 1k inst`
- `alloc/avgRun` & `memória peak`

## Anti-padroes

- Comparar CPU vs Local.Speed vs old machine; use a mesma HW da release runner
  (adequar em ADR se necessario).
- Run noisy environment;revezado. Use `taskset` para CPU affinity quando aplicavel.
- One-sample bench; sempre multipl + stdev.
