---
name: arena-allocator
description: Arena allocator por dominio musical (compasso, frase, project). Pre-aloca grandes blocos contínuos, sub-aloca O(1) sem free individual. Ideal para eventos/voicings.
---

# arena-allocator

## Quando usar

- Bard taxation de eventos MIDI por compasso (centenas/milhares de notas).
- Voicings em uma secao harmonica.
- Buffers temporários em workflows.

## Padrao

```cpp
class Arena {
    std::span<std::byte> block_;
    size_t offset_ = 0;
public:
    template<class T> T* alloc(size_t n = 1);
    void reset();            // nao dita free por item; chama reset no fim do escopo
};
```

- Sub-alinhacao com `alignas` por tipo.
- Reset eo fim de cada bar/frase/workflow (nao free-granular).
- Use `Arena` como dependency injected por workflow/context, nunca global.

## Anti-padroes

- Arena global (viola DI e deteminismo跨 workflows).
- `new`/`delete` por nota dentro de loops - swap por arena.

## Benchmarks

Compare `malloc` vs `Arena::alloc` para 10k notas; esperado speedup de ordens.
