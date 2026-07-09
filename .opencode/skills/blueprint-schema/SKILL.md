---
name: blueprint-schema
description: Schema do Music Blueprint (proto) - campos obrigatorios e opcionalis, validacao em compile-time via `buf`/`protoc-gen-validate`. Consumido por Planning Layer e MTE.
---

# blueprint-schema

## MusicBlueprint (proto simplificado)

```proto
message MusicBlueprint {
  int64           seed              = 1;   // reproducão
  string          root_key          = 2;   // ex.: "C"|"C#|Db"|..."
  string          scale             = 3;   // ex.: "major"|"minor"|"dorian"|...
  repeated string genres            = 4;
  Energy          energy            = 5;
  Mood            mood              = 6;
  Timeline        timeline          = 7;
  repeated Instrument instruments   = 8;
  int32           bpm               = 9;
  int32           ppq              = 10;  // default 480
  // ...
}
```

## Validadores (protoc-gen-validate)

- `seed` > 0 (nao default/zero - para detectar configuration missing).
- `bpm` 20-250.
- `ppq` potencia de 2 integers [60..960].
- `root_key` em conjunto permitido (12 notas cromaticas, any enharmonic normalizado).

## Fluxo

1. `prompt-interpreter` -> ple "intent proto"
2. `style-detection` -> merge
3. `blueprint-generator` -> compila em `MusicBlueprint` (proto)
4. Validação falha -> erro (nauto PR passivel).
5. MTE recebe `MusicBlueprint` imutavel; apenas le e consome.
