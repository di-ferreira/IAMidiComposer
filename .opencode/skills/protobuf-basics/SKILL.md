---
name: protobuf-basics
description: Protocol Buffers - schema primeiro, versionamento (v1, v2), compatibilidade backward; proto3 wire format; strong types via enum/oneof.
---

# protobuf-basics

## Padroes do projeto

- Tudo em `proto/aimidi/<version>/<domain>.proto`; `v1` é padrao atual.
- `syntax = "proto3";` evita defaults persistentes.
- `enum` com `_UNSPECIFIED = 0` para proibir zero-default implicito.
- `oneof` para escolhas mutuamente exclusivas.
- `reserved` para campos removidos; nunca reutilizar numeros de campo.

## Regras

- Nunca `Scalars` sem indicacao explicita quando significativa.
- Mensagens com campos de comprimento variavel (string/repeated) -> definir limites.

## Codegen

- C++: `protoc --cpp_out=...` (via CMake function `aimidi_protobuf_cpp`).
- Go: `protoc --go_out=... --go-grpc_out=...` (`buf` toolchain preferida).
- Não comitar o generado (CI gera).
