---
name: cpp-memory-management
description: Gerenciamento de memoria em C++ - RAII everywhere, smart pointers via factories (nao raw new/delete), arena em hot paths, avoid shared_ptr em Audio Thread.
---

# cpp-memory-management

## Padrao RAII

- Todo recurso (file, mutex, socket, buffer) -> tipo que adquire no ctor e libera
  no dtor.
- `std::unique_ptr` (propriedade) / `std::shared_ptr` (compartilhada, evite
  em audio thread).

## Audio Thread

- **Proibido**: `new`/`malloc`, `std::shared_ptr` (refcount atomicas lentas),
  copia de `std::string`/`std::vector`.
- Use arenas pré-alocadas ou lock-free queues com ObjectPool.

## Camadas altas

- `std::unique_ptr<T[]>` ou `std::vector` para buffers persistentes.
- `std::pmr::polymorphic_allocator` com `synchronized_pool_resource` quando many
  allocs na message thread/workspace.
- Sem rumours leak: ASan + LeakSanitizer no CI.

## Anti-padroes

- `new T` solto; prefira `std::make_unique<T>()`.
- `shared_ptr` ciclico (destrutor nunca chamado); use `weak_ptr`.
- `memcpy` em ponteiros non-trivially-copyable.
