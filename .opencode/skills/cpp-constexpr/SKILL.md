---
name: cpp-constexpr
description: Constantes e funcoes constexpr/consteval/constinit - compile-time tables para escalas, modos, voicings, mapeamento GM. Diminui startup e provavelmente melhora cache.
---

# cpp-constexpr

## Quando use

- Tabelas de escalas, modos, intervalos (12-tone, 7-note modes).
- Mapeamento GM (program numbers -> nomes).
- Padroes de acordes fixos.
- Constante dimensional (tamanhos de buffers, cache line sizes).

## Padrao

```cpp
constexpr std::array<int, 12> major = {0,2,4,5,7,9,11};
constexpr std::array<const char*, 128> gm_names = { ... };  // compile-time table
consteval int minor_third() { return 3; }
constinit int static_cache_line_size = 64;
```

- `consteval`: funcao pura em compile-time; nenhum runtime.
- `constinit`: variavel static com inicializacao constante - sem static init order.
- Compile-time evaluation no CMake: `target_compile_options(... -fconstexpr-ops-limit=N)`.

## Anti-padroes

- `constexpr` em hot-path sem beneficio (compilador nao analisou) -> benchmark.
- Loops `constexpr` profundos estouram step-limit.
