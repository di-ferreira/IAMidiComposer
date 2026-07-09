---
name: object-pool
description: Object pool para eventos/mensagens reutilizadas entre workflows. Evita alocacao repetida em hot paths. Obrigatorio para MidiEvent, BlueprintMsg, AARequest.
---

# object-pool

## Quando usar

- `MidiEvent` eventos entre Threads.
- Mensagens gRPC Plugin <-> ACE.
- Frames de captura de audio para analise.

## Padrao

```cpp
template<class T>
class ObjectPool {
    std::vector<std::unique_ptr<T>> storage_;
    LockFreeStack<T*> free_list_;
public:
    std::shared_ptr<T> acquire();        // returns RAII handle
};
```

- Pool tamanho fixo por dominio; crescer retry/excecao explicita.
- `acquire` retorna RAII handle que devolve ao pool no fim do escopo.
- Nao chama destrutor de T; reset() manual tipo `T::reset()`.
- Initializo em message thread/main; nunca em Audio Thread.

## Anti-padroes

- `std::shared_ptr<T>` com `make_shared` em loops - alocacao.
- Pool global - quebra DI e dificulta testes.

## Benchmarks

`make_shared` vs `pool.acquire` -> 10k acquires; esperado ~10x mais rapido.
