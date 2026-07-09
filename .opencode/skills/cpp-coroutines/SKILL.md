---
name: cpp-coroutines
description: Corrotinas C++20 (co_await/co_return) para workflows async ACBroadcast, gRPC client streaming. Nao usar na Audio Thread.
---

# cpp-coroutines

## Quando usar

- Streaming gRPC (AsyncReader/Writer) na ACE.
- Sequenciacao de workflows (coawait proxima engine pronto).
- GUI/Message thread (sem bloquear).

## Quando NUNCA

- Audio Thread (proibido suspension/alloc).
- Hot Paths com budget de ns.

## Anti-padroes

- Corrotina capturando `this` de objeto de vida curta -> dangling.
- Stackful await em funcao `noexcept`.

## Note

Adocao cautelosa; ver ADR antes de usar.
