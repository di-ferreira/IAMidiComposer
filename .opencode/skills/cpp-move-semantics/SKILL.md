---
name: cpp-move-semantics
description: Move semantics C++ - std::move, RVO/NRVO, perfect forwarding, no-allocation moves. Usar para evitar copias entre Music Theory Engines/pipeline de MIDI.
---

# cpp-move-semantics

## Padrao

- **RVO/NRVO**: o compilador elide copia; nao faca `std::move` em return local -
  isso DESATIVA elisao.
- **Mova grandes buffers** entre funcoes: `std::vector&&` ou return por valor.
- **Perfect forwarding** em factories/templates: `template<class T> auto make(T&& x)
  { return std::forward<T>(x); }`.
- **Move ctor/assign** = `default` quando trivial; nunca `throw`.

## Anti-padroes

- `std::move` em local stack object retornado (derefe o move; impede o RVO).
- Move de `const T` (nao move; vira copia implicita).
- Mover `std::shared_ptr` (legal, mas cuidado com cross-thread ownership).
- Move de tipos trivially-copyable (valo limitado).

## Validaçao

- ASan: move de buffer -> nenhum alloc/copy extra esperado.
