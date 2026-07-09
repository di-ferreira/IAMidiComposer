---
name: profiling
description: Profiling C++ com Tracy (zonas, plot de cache-miss, lock waits) e Go com pprof. Obrigatorio em PR de hot path; comparar antes/depois em docs/benchmarks/.
---

# profiling

## Tracy (C++)

```cpp
#include <tracy/Tracy.hpp>
void process(...) {
    ZoneScopedN("MidiProcess");
    // ...
    TracyPlot("cacheMiss", misses);
}
```

- Compile com `-DTRACY_ENABLE=ON`.
- Capture em tempo real ou em arquivo; zonas aninhadas.
- Verifique lock contention, allocs, cache misses, gpu (se aplicavel).

## pprof (Go)

```sh
go test -cpuprofile cpu.out -bench .
go tool pprof -web cpu.out
```

- Para goroutine profile, lock profile e block profile tambem.

## Regras

- PR que altera hot path SEM ANTES/DEPOIS de perfil -> recusado.
- Resultados registrados em `docs/benchmarks/<feature>.md`.
- Use IPC, cache-miss/1k instr, throughput como metricas.
