---
name: simd
description: Vetorização C++ via intrinsics (SSE2 baseline, AVX2/AVX-512 quando detectado, NEON em ARM). Batches de MIDI/velocity/gain. Use auto-vectorization primeiro; intrinsics quando profiler justificar.
---

# simd

## Hierarquia alvo

- **SSE2** baseline obrigatorio (x86_64 universal).
- **AVX2** quando `__AVX2__` definido; use runtime dispatch (`cpuid`) para fallback.
- **NEON** para ARM64 (`__ARM_NEON`).
- Use `[ piscar ]` feature-test macros (`<version>`).

## Pratica

1. Tente **auto-vectorization** (`#pragma omp simd` / `[[gnu::optimize("fast-math")]]`)
   antes de intrinsics.
2. Use intrinsics explicitos somente em hot spots identificados pelo profiler.
3. Layout de dados SOA para permitir SIMD: `_mm256_load_ps` em blocos alinhados
   com `alignas(32)`.
4. Para gain/velocity shaping: `a * b * 1/127` -> converter para float, multiplicar,
   clampar e voltar.
5. Compare sempre com a versão escalar em benchmark.

## Cuidados

- Especificar `-mavx2` só via target_clang/gcc por target_compile_features; nao
  force globalmente (quebra hardware antigo).
- Aliasing: use `__restrict` em ponteiros de inner-loop.
- FMA (`_mm256_fmadd_ps`) quando disponivel para precisão/throughput.
