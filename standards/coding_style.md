# Coding Style — AI MIDI Composer

## Formatação

- **C++**: clang-format com `.clang-format` no repo (LLVM base, 4-space indent,
  column 120, attach braces).
- **Go**: `gofmt` (padrão); `goimports` para imports.

## Naming

- **C++ namespaces**: `aimidi::theory::harmony`, snake_case em arquivos.
- **C++ types**: `PascalCase` (HarmonyEngine, MidiEvent).
- **C++ functions/members**: `snake_case`.
- **C++ constexpr/const**: `kCamelCase` (kMajorScale, kMaxVoices).
- **Go**: Go padrão (`exported` PascalCase, `internal` camelCase).

## Headers

- `#pragma once`.
- Includes ordenados: proprio (com aspas) ↓ sistema (com <>); grupos separados
  por linha em branco.
- IWYU no CI.

## Anti-padroes

- Comments em código "// seta x para y" (use selfdoc): proibido.
- Comments narrativos em header que repetem a doc.
- Comentarios em portugues no codigo: o repo mantem **comments em ingles** quando
  necessários (conforme review global). Doc largos em PT-BR OK.

## Erros

- **C++**: `std::expected`/`tl::expected` em substituicao a excessões no path
  core; exceptions desligadas no build hardening.
- **Go**: explicito `error` no retorno.

## Tamanho

- Funções: idealmente < 50 linhas; ate 100 toleradas com bom nome.
- Arquivos: < 400 linhas target; refatorar acima de 600.

## Tests

- Arquivos de teste: `<module>_test.cpp` ou `<module>_test.go` no mesmo pacote.
- Nome do teste descreve o comportamento, nao internals.
