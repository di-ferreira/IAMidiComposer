# C++20 Guidelines — AI MIDI Composer

## Padrao

- **C++20** obrigatorio (`cxx_std_20`).
- Compiladores suportados: GCC 11+, Clang 14+, MSVC 19.34+.
- Build flags padrao: `-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion
  -Wnon-virtual-dtor -Wold-style-cast -Wnoexcept`.
- Hardening Release: `-O3 -DNDEBUG -fno-exceptions -fno-rtti` no path critico.
  - Em testes e bindings Go, podem ligar RTTI/exceções temporariamente.

## Modern C++

- Concepts no lugar de SFINAE.
- `std::span` para buffers (nao `T* + size`).
- `std::format` (C++20) para strings (nao snprintf).
- `std::ranges` em pipelines (cuidado em hot paths).
- `constexpr` / `consteval` para tabelas statics.
- `std::optional` / `std::variant` em vez de pointers nullable.

## Memory

- RAII everywhere; prefer `unique_ptr` a `shared_ptr`.
- **Audio Thread**: zero aloc no heap; arena/pool + lock-free.
- Ver `cpp-memory-management`, `arena-allocator`, `object-pool` skills.

## Interfaces / DI

- Cada engine expoe interface pura (IHarmonyEngine, IMidiRenderer, ...) em pasta
  `include/`.
- Implementacao concreta oculta no .cpp; injetada por factory na DI do teste/CMake.
- Sem derived-types (ruidos) em hot path; CRTP quando polimorphism é diff costly.

## Performance

- Layouts SOA/cache-friendly em hot paths (ver `cache-optimization`).
- SIMD nos loops de MIDI e DSP (ver `simd`).
- Tracy zone em funções críticas (ver `profiling`).

## Logs

- `spdlog` (header-only ou compiled); macros caso disable logging em hot path
  nao custe (macros no_null Null log).

## Sanity

- `-fsanitize=address,undefined` em CI builds debug.
- clang-tidy ativo (`.clang-tidy`).
