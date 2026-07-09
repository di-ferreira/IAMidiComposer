---
name: cpp-templates
description: Padroes de templates C++20 - concepts, variadic templates, CRTP, SFINAE residual; templates apenas quando trazem valor real (nao por gosto).
---

# cpp-templates

## Quando usar

- Generic por tipo de dado (ex.: buffer `T` -> `float`, `int8_t`, `MidiEvent`).
- Em interfaces e dim DOMIN onde polimorfismo dinamico trades tarde.
- Algoritmos que beneficiam specialization.

## C++20 mudanca

- Use **concepts** para restricoes semanticas (`std::integral`, custom):
  ```cpp
  template<class T> concept Note = requires(T n) { n.pitch(); n.velocity(); };
  ```
- Evite SFINAE residual.

## Padroes aprovados

- **CRTP** para static polymorphism (sem vtable): Engine<T> base.
- **Variadic templates** com fold-expressions (clara).
- **Type traits** via `<type_traits>` + `if constexpr`.

## Anti-padroes

- Templates deeply nested (nao instanciavel em isolation); destrói compilação.
- `typename enable_if<...>` residual (use concepts).
- Templates em header publico sem testar instantiation.
