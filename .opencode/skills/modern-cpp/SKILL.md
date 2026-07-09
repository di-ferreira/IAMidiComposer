---
name: modern-cpp
description: C++20 features usadas no projeto - concepts, ranges, modules (cuiddo), constinit, consteval, std::span, std::bit_ceil, designated initializers. Padrões para C++ neste projeto.
---

# modern-cpp

## Usos aprovados

- **C++20 obrigatorio** (`targets_compile_features(... cxx_std_20)`).
- **concepts** para interfaces e template constraints (substitui SFINAE).
- **std::span** para passagem de buffers (substitui `T*, size_t`).
- **std::ranges** para iteracao declarativa; cuidado em hot path com provo.
- **constinit/consteval** para constantes validadas em compile-time.
- **Designated initializers** `{.x = 1, .y = 2}` (clareza).
- **std::format** (preferido a sprintf/iostream).
- **`std::bit_ceil`/** para proxima potencia de 2 (ring buffers).

## Cuidados

- **Modules**: ainda nao adotamos (suporte de build inconsistente). Use headers
  tradicionais + IWYU.
- **Coroutines**: ver `cpp-coroutines` skill quando necessario.
- **Exceptions/RTTI**: desligados em caminho critico (`-fno-exceptions -fno-rtti`)
  nessa parte; use variant/polymorphic sem rtti.

## Header hygiene

- `#pragma once` (preferido neste projeto) ou include guards; padrao unico.
- IWYU (`include-what-you-use`) no CI; sem includes transitivos implicitos.
