---
name: go-modules
description: Go modules - go.mod versionado (Go 1.22+), dependencias minimas, replace local para dev de bindings C++/Go, vendor opcional no CI offline.
---

# go-modules

## Regras

- `module aimidi.composer/ace` (organizacao do projeto).
- `go 1.22` (minimo baseline atual).
- Minimal deps: gRPC, protobuf, modernc.org/sqlite, slog.
- `replace` para bindings C++ durante dev local; nunca em release.
- `vendor/` opcional; gitignored por default.

## Antipadroes

- `go.mod` com muitos indirects nao curados -> `go mod tidy`.
- `proto-generated` versionado (nao); CI regenera.
- Versao `v0.0.0-(pseudo)` se commitado sem tag - rode `git tag v0.1.0` para release.
