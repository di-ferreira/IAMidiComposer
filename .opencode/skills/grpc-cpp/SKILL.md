---
name: grpc-cpp
description: Cliente gRPC C++ no Plugin - stub async, no blocking em Audio Thread; pool de canais, timeouts configuraveis, retry exponencial em message thread.
---

# grpc-cpp

## Padrao Plugin -> ACE

- Canal gRPC local (unix socket ou `localhost:50051`); 1 canal compartilhado por Plugin.
- Stub async; **nunca bloqueia**:
  - Inicia `CompletionQueue` em uma worker thread dedicada no Plugin.
  - Request/response em std::future resolvida via poll em message Thread **nao
    em Audio Thread**.

## Timeouts

- Default 30s para tasks de composicao completa.
- 5s para previews / UI status.

## Anti-padroes

- `grpc::Status` SYNC blocking (`reader.Read()` chosen) em Audio Thread - proibido.
- Reutilizar canal ap�s destruir - funciona mas vira bug fut_ro; isolou.

## Retry

- Retry exponencial em message thread (1->4->16 s); max 3.
